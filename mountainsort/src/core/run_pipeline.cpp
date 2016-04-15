/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/15/2016
*******************************************************/

#include "run_pipeline.h"

#include <QJsonArray>
#include <QSet>

struct ProcessTreeNode {
    QString file_path;
    QSet<QString> depends_on_files;
};

bool run_pipeline(MSProcessManager& PM, QJsonObject pipeline)
{
    QJsonArray processes = pipeline["processes"].toArray();

    QMap<QString, ProcessTreeNode> process_tree;

    for (int i = 0; i < processes.count(); i++) {
        QJsonObject process = processes[i].toObject();
        QString processor_name = process["processor_name"].toString();
        QJsonObject parameters = process["parameters"].toObject();
        QMap<QString, QVariant> params;
        QStringList pkeys = parameters.keys();
        foreach (QString pkey, pkeys) {
            params[pkey] = parameters[pkey].toString();
        }
        if (!PM.containsProcessor(processor_name)) {
            printf("Unable to find processor: %s\n", processor_name.toLatin1().data());
            return false;
        }
        MSProcessor* processor = PM.processor(processor_name);
        QStringList inputs = processor->inputFileParameters();
        QStringList outputs = processor->outputFileParameters();
        QStringList required = processor->requiredParameters();
        QStringList optional = processor->optionalParameters();

        foreach (QString pname, pkeys) {
            if ((!inputs.contains(pname)) && (!outputs.contains(pname)) && (!required.contains(pname)) && (!optional.contains(pname))) {
                printf("Unknown parameter for %s: %s\n", processor_name.toLatin1().data(), pname.toLatin1().data());
                return false;
            }
        }
        foreach (QString pname, inputs) {
            if (!params.contains(pname)) {
                printf("Missing input parameter for %s: %s\n", processor_name.toLatin1().data(), pname.toLatin1().data());
                return false;
            }
            QString fname = params[pname].toString();
            if (!process_tree.contains(fname)) {
                ProcessTreeNode node;
                node.file_path = fname;
                process_tree[fname] = node;
            }
        }
        foreach (QString pname, outputs) {
            if (!params.contains(pname)) {
                printf("Missing output parameter for %s: %s\n", processor_name.toLatin1().data(), pname.toLatin1().data());
                return false;
            }
            QString fname = params[pname].toString();
            //todo: check for case where the same file is output twice, which is not allowed.
            if (!process_tree.contains(fname)) {
                ProcessTreeNode node;
                node.file_path = fname;
                process_tree[fname] = node;
            }
            foreach (QString pname2, inputs) {
                process_tree[fname].depends_on_files.insert(params.value(pname2).toString());
            }
        }
        foreach (QString pname, required) {
            if (!params.contains(pname)) {
                printf("Missing required parameter for %s: %s\n", processor_name.toLatin1().data(), pname.toLatin1().data());
                return false;
            }
        }
    }

    //finish!!!!!!!!!
    //parse tree, checking for errors
    //go through and run each process, as the inputs come available
}
