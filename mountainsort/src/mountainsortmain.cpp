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
#include "get_command_line_params.h"
#include "msprocessmanager.h"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <msscriptcontroller.h>
#include "diskreadmda.h"
#include "unit_tests.h"
#include "process_msh.h"
#include "textfile.h"
#include <QJSEngine>
#include "mscachemanager.h"

void print_usage();
void list_processors(const MSProcessManager* PM);
int run_process(MSProcessManager* PM, QJsonObject process);
int run_script(const QStringList &script_fnames);

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    MSCacheManager::globalInstance()->setLocalBasePath(app.applicationDirPath()+"/../../tmp");

    CLParams CLP = get_command_line_params(argc, argv);

    MSProcessManager *PM=MSProcessManager::globalInstance();
    PM->loadDefaultProcessors();

    QString arg1 = CLP.unnamed_parameters.value(0);
    QString arg2 = CLP.unnamed_parameters.value(1);

    QStringList script_fnames;
    for (int i=0; i<CLP.unnamed_parameters.count(); i++) {
        if ((CLP.unnamed_parameters[i].endsWith(".js"))||(CLP.unnamed_parameters[i].endsWith(".ms"))) {
            script_fnames << CLP.unnamed_parameters[i];
        }
    }
    if (script_fnames.count()>=1) {
        return run_script(script_fnames);
    }

    if (arg1 == "process") {
        if (arg2.isEmpty()) {
            print_usage();
            return -1;
        }
        QString json = read_text_file(arg2);
        if (json.isEmpty()) {
            printf("Unable to open file or file is empty: %s\n", arg2.toLatin1().data());
        }
        QJsonObject obj = QJsonDocument::fromJson(json.toLatin1()).object();
        return run_process(PM, obj);
    }
    else if (arg1 == "list-processors") {
        list_processors(PM);
        return 0;
    }
    else if (arg1 == "detail-processors") {
        PM->printDetails();
        return 0;
    }

    if (CLP.unnamed_parameters.count() == 0) {
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
        return run_process(PM, process);
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

int run_process(MSProcessManager* PM, QJsonObject process)
{
    QString processor_name = process["processor_name"].toString();
    QJsonObject parameters = process["parameters"].toObject();
    QStringList keys = parameters.keys();
    QMap<QString, QVariant> params;
    foreach (QString key, keys) {
        params[key] = parameters[key].toString();
    }

    if (!PM->checkAndRunProcessIfNecessary(processor_name, params)) {
        return -1;
    }

    return 0;
}

void display_error(QJSValue result) {
    qDebug()  << result.property("name").toString();
    qDebug()  << result.property("message").toString();
    qDebug()  << QString("%1 line %2").arg(result.property("fileName").toString()).arg(result.property("lineNumber").toInt());
}

int run_script(const QStringList &script_fnames) {
    QJSEngine* engine = new QJSEngine;
    MSScriptController Controller;
    QJSValue MS = engine->newQObject(&Controller);
    engine->globalObject().setProperty("MS",MS);
    foreach (QString fname,script_fnames) {
        QJSValue result=engine->evaluate(read_text_file(fname),fname);
        if (result.isError()) {
            display_error(result);
            qCritical() << "Error running script.";
            return -1;
        }
    }

    {
        QJSValue result=engine->evaluate("main();");
        if (result.isError()) {
            display_error(result);
            qCritical() << "Error running script.";
            return -1;
        }
    }

    return 0;
}
