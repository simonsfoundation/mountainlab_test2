/******************************************************
**
** Copyright (C) 2016 by Jeremy Magland
**
** This file is part of the MountainSort C++ project
**
** Some rights reserved.
** See accompanying LICENSE and README files.
**
*******************************************************/

#include <QCoreApplication>
#include <stdio.h>
#include "msprocessmanager.h"
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <cachemanager.h>
#include "diskreadmda.h"
//#include "unit_tests.h"
#include "mlcommon.h"
#include "pca.h"

void print_usage();
void list_processors(const MSProcessManager* PM);
bool run_process(MSProcessManager* PM, QJsonObject process);
QJsonArray test_processor(MSProcessManager *PM,QString processor_name);

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    setbuf(stdout, NULL);

    CLParams CLP(argc, argv);

    MSProcessManager* PM = MSProcessManager::globalInstance();
    PM->loadDefaultProcessors();

    QString arg1 = CLP.unnamed_parameters.value(0);
    QString arg2 = CLP.unnamed_parameters.value(1);

    if (arg1 == "process") {
        if (arg2.isEmpty()) {
            print_usage();
            return -1;
        }
        QString json = TextFile::read(arg2);
        if (json.isEmpty()) {
            printf("Unable to open file or file is empty: %s\n", arg2.toLatin1().data());
        }
        QJsonObject obj = QJsonDocument::fromJson(json.toLatin1()).object();
        if (!run_process(PM, obj))
            return -1;
        else
            return 0;
    }
    else if (arg1 == "list-processors") {
        list_processors(PM);
        return 0;
    }
    else if (arg1 == "detail-processors") {
        PM->printDetails();
        return 0;
    }
    else if (arg1 == "spec") {
        PM->printJsonSpec();
        return 0;
    }
    else if (arg1 == "test") {
        QString processor_name=arg2;
        QJsonArray all_results;
        if (!processor_name.isEmpty()) {
            QJsonArray results=test_processor(PM,processor_name);
            foreach (QJsonValue result,results) {
                all_results.append(result.toObject());
            }
        }
        else {
            QStringList names=PM->allProcessorNames();
            foreach (QString name,names) {
                QJsonArray results=test_processor(PM,name);
                foreach (QJsonValue result,results) {
                    all_results.append(result.toObject());
                }
            }
        }
        printf("\n");
        for (int i=0; i<all_results.count(); i++) {
            QString procname=all_results[i].toObject()["processor_name"].toString();
            bool success=all_results[i].toObject()["success"].toBool();
            QString input_code=all_results[i].toObject()["input_code"].toString();
            QString output_code=all_results[i].toObject()["output_code"].toString();
            QString passed_str;
            if (success) passed_str="PASSED";
            else passed_str="FAILED";
            printf("%s %s %d %s %s\n",passed_str.toLatin1().data(),procname.toLatin1().data(),i,input_code.toLatin1().data(),output_code.toLatin1().data());
        }
        return 0;
    }

    if (CLP.unnamed_parameters.isEmpty()) {
        print_usage();
        return -1;
    }

    if (CLP.unnamed_parameters.count() == 1) {
        QString processor_name = CLP.unnamed_parameters[0];
        QJsonObject process;
        process["processor_name"] = processor_name;
        QJsonObject parameters;
        QStringList keys = CLP.named_parameters.keys();
        foreach (QString key, keys) {
            parameters[key] = CLP.named_parameters[key].toString();
        }
        process["parameters"] = parameters;
        if (run_process(PM, process))
            return 0;
        else
            return -1;
    }
    else {
        printf("Unexpected number of unnamed parameters: %d\n", CLP.unnamed_parameters.count());
    }

    return 0;
}

void print_usage()
{
    printf("mountainsort process [process.json]\n");
    printf("mountainsort pipeline [pipeline.json]\n");
    printf("mountainsort list-processors\n");
    printf("mountainsort detail-processors\n");
    printf("mountainsort [processor_name] --[param1]=[value1] --[param2]=[value2] ...\n");
}

void list_processors(const MSProcessManager* PM)
{
    QString str = PM->usageString();
    printf("%s\n", str.toLatin1().data());
}

bool run_process(MSProcessManager* PM, QJsonObject process)
{
    QString processor_name = process["processor_name"].toString();
    QJsonObject parameters = process["parameters"].toObject();
    QStringList keys = parameters.keys();
    QMap<QString, QVariant> params;
    foreach (QString key, keys) {
        params[key] = parameters[key].toString();
    }

    if (!PM->checkAndRunProcess(processor_name, params)) {
        return false;
    }

    return true;
}

QJsonArray test_processor(MSProcessManager *PM,QString processor_name) {
    MSProcessor *P=PM->processor(processor_name);
    if (!P) {
        QJsonArray ret;
        QJsonObject tmp;
        tmp["processor_name"]=processor_name;
        tmp["test_number"]=0;
        tmp["success"]=false;
        tmp["error"]="Unable to find processor: "+processor_name;
        return ret;
    }
    QStringList inparams=P->inputFileParameters();
    QStringList outparams=P->outputFileParameters();
    QStringList reqparams=P->requiredParameters();
    QStringList optparams=P->optionalParameters();
    QMap<QString,QVariant> file_params;
    foreach (QString p,inparams) {
        file_params[p]=CacheManager::globalInstance()->makeLocalFile("",CacheManager::ShortTerm);
    }
    foreach (QString p,outparams) {
        file_params[p]=CacheManager::globalInstance()->makeLocalFile("",CacheManager::ShortTerm);
    }
    int test_num=0;
    bool done=false;
    QJsonArray all_results;
    while (!done) {
        MSProcessorTestResults results=P->runTest(test_num,file_params);
        if (results.test_exists) {
            QJsonObject obj;
            obj["processor_name"]=processor_name;
            obj["test_number"]=test_num;
            obj["success"]=results.success;
            QJsonObject params;
            {
                foreach (QString p,reqparams) {
                    params[p]=QJsonValue::fromVariant(results.params[p]);
                }
                foreach (QString p,optparams) {
                    params[p]=QJsonValue::fromVariant(results.params[p]);
                }
                obj["params"]=params;
            }
            QJsonArray input_file_info;
            {
                foreach (QString p,inparams) {
                    QJsonObject info;
                    info["parameter_name"]=p;
                    info["checksum"]=MLUtil::computeSha1SumOfFile(file_params[p].toString());
                    input_file_info.append(info);
                }
                obj["input_file_info"]=input_file_info;
            }
            QJsonArray output_file_info;
            {
                foreach (QString p,outparams) {
                    QJsonObject info;
                    info["parameter_name"]=p;
                    info["checksum"]=MLUtil::computeSha1SumOfFile(file_params[p].toString());
                    output_file_info.append(info);
                }
                obj["output_file_info"]=output_file_info;
            }
            {
                QJsonObject obj_input;
                obj_input["input_file_info"]=input_file_info;
                obj_input["params"]=params;
                QString json=QJsonDocument(obj_input).toJson(QJsonDocument::Compact);
                obj["input_code"]=MLUtil::computeSha1SumOfString(json);
            }
            {
                QJsonObject obj_output;
                obj_output["output_file_info"]=output_file_info;
                QString json=QJsonDocument(obj_output).toJson(QJsonDocument::Compact);
                obj["output_code"]=MLUtil::computeSha1SumOfString(json);
            }
            {
                printf("Results:\n");
                QString json=QJsonDocument(obj).toJson(QJsonDocument::Indented);
                printf("%s\n",json.toLatin1().data());
                all_results.append(obj);
                test_num++;
            }
        }
        else {
            done=true;
        }
    }
    return all_results;
}

