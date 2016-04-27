/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/27/2016
*******************************************************/

#ifndef SCRIPTCONTROLLER_H
#define SCRIPTCONTROLLER_H

class ScriptControllerPrivate;
class ScriptController
{
public:
    friend class ScriptControllerPrivate;
    ScriptController();
    virtual ~ScriptController();
private:
    ScriptControllerPrivate *d;
};

#endif // SCRIPTCONTROLLER_H

