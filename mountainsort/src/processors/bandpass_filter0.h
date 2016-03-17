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

#ifndef BANDPASS_FILTER0_H
#define BANDPASS_FILTER0_H

#include <QString>

bool bandpass_filter0(const QString &input,const QString &output,double samplerate,double freq_min,double freq_max);

#endif // BANDPASS_FILTER0_H

