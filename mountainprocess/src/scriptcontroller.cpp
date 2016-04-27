/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/27/2016
*******************************************************/

#include "scriptcontroller.h"

class ScriptControllerPrivate {
public:
    ScriptController *q;
};

ScriptController::ScriptController()
{
    d=new ScriptControllerPrivate;
    d->q=this;
}

ScriptController::~ScriptController()
{
    delete d;
}
