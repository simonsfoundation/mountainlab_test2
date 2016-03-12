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
#include "msprocessormanager.h"
#include <QDebug>

void print_usage(const MSProcessorManager &PM);

int main(int argc, char *argv[]) {
	QCoreApplication app(argc,argv);

	CLParams CLP=get_command_line_params(argc,argv);

	MSProcessorManager PM;
	PM.loadDefaultProcessors();

	if (CLP.unnamed_parameters.count()==0) {
		print_usage(PM);
		return -1;
	}

	if (CLP.unnamed_parameters.count()==1) {
		QString processor_name=CLP.unnamed_parameters[0];
		if (!PM.containsProcessor(processor_name)) {
			printf("Unable to find processor: %s\n",processor_name.toLatin1().data());
			return -1;
		}
		if (!PM.checkProcessor(processor_name,CLP.named_parameters)) {
			printf("Problem checking processor: %s\n",processor_name.toLatin1().data());
			return -1;
		}
		if (!PM.runProcessor(processor_name,CLP.named_parameters)) {
			printf("Problem running processor: %s\n",processor_name.toLatin1().data());
			return -1;
		}
	}

	return 0;
}

void print_usage(const MSProcessorManager &PM) {
	QString str=PM.usageString();
	printf("%s\n",str.toLatin1().data());
}
