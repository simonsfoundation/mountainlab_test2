/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/15/2016
*******************************************************/

#include "run_pipeline.h"

#include <QJsonArray>
#include <QSet>
#include <QFile>
#include <QMap>

struct ProcessTreeNode {
    QString file_path;
    QSet<QString> depends_on_files;
    int process_index; //-1 means it's not an output of any process in the pipeline
};

QMap<QString, QVariant> to_variant_map(QJsonObject obj)
{
    QMap<QString, QVariant> ret;
    QStringList pkeys = obj.keys();
    foreach (QString pkey, pkeys) {
        ret[pkey] = obj[pkey].toString();
    }
    return ret;
}

bool run_pipeline(MSProcessManager& PM, QJsonObject pipeline)
{
    //Get the list of processes to run
    QJsonArray processes = pipeline["processes"].toArray();

    //Set up the process tree, so we know what depends on what
    //(although this info isn't strictly used at this time to set up the process order, still we will probably want to analyze the tree at some point)
    //as we go through, we also check that the proper parameters are in place
    QMap<QString, ProcessTreeNode> process_tree;
    {
        for (int i = 0; i < processes.count(); i++) {
            QJsonObject process = processes[i].toObject();
            QString processor_name = process["processor_name"].toString();
            QJsonObject parameters = process["parameters"].toObject();
            QMap<QString, QVariant> params = to_variant_map(parameters);
            if (!PM.containsProcessor(processor_name)) {
                printf("Unable to find processor: %s\n", processor_name.toLatin1().data());
                return false;
            }
            MSProcessor* processor = PM.processor(processor_name);
            QStringList inputs = processor->inputFileParameters();
            QStringList outputs = processor->outputFileParameters();
            QStringList required = processor->requiredParameters();
            QStringList optional = processor->optionalParameters();

            //make sure each parameter is either required or optional
            QStringList pkeys = params.keys();
            foreach (QString pname, pkeys) {
                if ((!inputs.contains(pname)) && (!outputs.contains(pname)) && (!required.contains(pname)) && (!optional.contains(pname))) {
                    printf("Unknown parameter for %s: %s\n", processor_name.toLatin1().data(), pname.toLatin1().data());
                    return false;
                }
            }

            //for each input file, make a new node (if doesn't exist)
            foreach (QString pname, inputs) {
                if (!params.contains(pname)) {
                    printf("Missing input parameter for %s: %s\n", processor_name.toLatin1().data(), pname.toLatin1().data());
                    return false;
                }
                QString fname = params[pname].toString();
                if (!process_tree.contains(fname)) {
                    ProcessTreeNode node;
                    node.file_path = fname;
                    node.process_index = -1; //this means it's not an output of any process in the pipeline
                    process_tree[fname] = node;
                }
            }
            //for each output file, make a new node (if doesn't exist) and set the input files as dependencies
            foreach (QString pname, outputs) {
                if (!params.contains(pname)) {
                    printf("Missing output parameter for %s: %s\n", processor_name.toLatin1().data(), pname.toLatin1().data());
                    return false;
                }
                QString fname = params[pname].toString();
                if (process_tree.contains(fname)) {
                    if (process_tree[fname].process_index >= 0) {
                        printf("The same file cannot be the output of more than one process for %s. (%s)\n", processor_name.toLatin1().data(), fname.toLatin1().data());
                        return false;
                    }
                    process_tree[fname].process_index = i;
                }
                else {
                    ProcessTreeNode node;
                    node.file_path = fname;
                    node.process_index = i;
                    process_tree[fname] = node;
                }
                foreach (QString pname2, inputs) {
                    process_tree[fname].depends_on_files.insert(params.value(pname2).toString());
                }
            }
            //check that the required parameters are provided
            foreach (QString pname, required) {
                if (!params.contains(pname)) {
                    printf("Missing required parameter for %s: %s\n", processor_name.toLatin1().data(), pname.toLatin1().data());
                    return false;
                }
            }
        }
    }

    //check that the input files exist
    QSet<QString> input_files_generated;
    {
        QStringList fnames = process_tree.keys();
        foreach (QString fname, fnames) {
            ProcessTreeNode NN = process_tree[fname];
            if (NN.process_index < 0) {
                if (!QFile::exists(NN.file_path)) {
                    printf("Input file does not exist: %s\n", NN.file_path.toLatin1().data());
                    return false;
                }
                input_files_generated.insert(fname);
            }
        }
    }

    //sort in order of execution
    QList<int> sorted_process_indices;
    bool something_changed = true;
    while (something_changed) {
        something_changed = false;
        for (int i = 0; i < processes.count(); i++) {
            if (!sorted_process_indices.contains(i)) {
                QJsonObject process = processes[i].toObject();
                QString processor_name = process["processor_name"].toString();
                QJsonObject parameters = process["parameters"].toObject();
                QMap<QString, QVariant> params = to_variant_map(parameters);
                if (!PM.containsProcessor(processor_name)) {
                    printf("Unable to find processor: %s\n", processor_name.toLatin1().data());
                    return false;
                }
                MSProcessor* processor = PM.processor(processor_name);
                QStringList inputs = processor->inputFileParameters();
                QStringList outputs = processor->outputFileParameters();

                //check that the inputs have all been generated
                bool okay = true;
                foreach (QString pname, inputs) {
                    QString input_fname = params[pname].toString();
                    if (!input_files_generated.contains(input_fname)) {
                        okay = false;
                    }
                }
                if (okay) {
                    //now we can add this process index to the list
                    something_changed = true;
                    sorted_process_indices << i;
                    foreach (QString pname, outputs) {
                        //and add each output to the list of input files generated
                        input_files_generated.insert(params[pname].toString());
                    }
                }
            }
        }
    }

    //check that we have scheduled all of the processes
    for (int i = 0; i < processes.count(); i++) {
        if (!sorted_process_indices.contains(i)) {
            QString processor_name = processes[i].toObject()["processor_name"].toString();
            printf("Process could not be scheduled because it depends on an input file that will not be generated (perhaps a cyclic dependency): %s\n", processor_name.toLatin1().data());
            return false;
        }
    }

    //finally, run the processes in the proper order
    for (int j = 0; j < sorted_process_indices.count(); j++) {
        int i = sorted_process_indices[j];
        QJsonObject process = processes[i].toObject();
        QString processor_name = process["processor_name"].toString();
        QJsonObject parameters = process["parameters"].toObject();
        QMap<QString, QVariant> params = to_variant_map(parameters);

        printf("[--------] RUNNING PROCESS %s\n",processor_name.toLatin1().data());
        if (!PM.checkAndRunProcessIfNecessary(processor_name,params)) {
            printf("Error in process: %s\n", processor_name.toLatin1().data());
            return false;
        }
    }

    //we made it through the entire pipeline!
    return true;
}
