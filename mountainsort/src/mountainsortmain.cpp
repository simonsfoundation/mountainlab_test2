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
#include "diskreadmda.h"
#include "unit_tests.h"
#include "process_msh.h"

void print_usage(const MSProcessManager &PM);

int main(int argc, char *argv[]) {
	QCoreApplication app(argc,argv);

	CLParams CLP=get_command_line_params(argc,argv);

	MSProcessManager PM;
	PM.loadDefaultProcessors();

	if (CLP.unnamed_parameters.count()==0) {
		print_usage(PM);
		return -1;
	}

    QString arg1=CLP.unnamed_parameters.value(0);

    if (arg1=="-diskreadmda_unit_test") {
		diskreadmda_unit_test();
		return 0;
	}
    if (arg1=="-unit_test") {
		QString test_name=CLP.unnamed_parameters.value(1);
		run_unit_test(test_name);
		return 0;
	}
    if (arg1.endsWith(".msh")) {
		return process_msh(CLP.unnamed_parameters.value(0),argc,argv);
	}
    if (arg1=="-details") {
        PM.printDetails();
        return 0;
    }

	if (CLP.unnamed_parameters.count()==1) {
		QString processor_name=CLP.unnamed_parameters[0];
		if (!PM.containsProcessor(processor_name)) {
			printf("Unable to find processor: %s\n",processor_name.toLatin1().data());
			return -1;
		}
		if (!PM.checkProcess(processor_name,CLP.named_parameters)) {
			printf("Problem checking processor: %s\n",processor_name.toLatin1().data());
			return -1;
		}
		if (PM.findCompletedProcess(processor_name,CLP.named_parameters)) {
			printf("Process already completed: %s\n",processor_name.toLatin1().data());
		}
		else {
			if (!PM.runProcess(processor_name,CLP.named_parameters)) {
				printf("Problem running processor: %s\n",processor_name.toLatin1().data());
				return -1;
			}
		}
	}
	else {
		printf("Unexpected number of unnamed parameters: %d\n",CLP.unnamed_parameters.count());
	}

	return 0;
}

void print_usage(const MSProcessManager &PM) {
	QString str=PM.usageString();
	printf("%s\n",str.toLatin1().data());
}
