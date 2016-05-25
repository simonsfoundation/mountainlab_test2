/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/27/2016
*******************************************************/

#ifndef SCRIPTCONTROLLER_H
#define SCRIPTCONTROLLER_H

#include <QObject>

class ScriptControllerPrivate;
class ScriptController : public QObject {
    Q_OBJECT
public:
    friend class ScriptControllerPrivate;
    ScriptController();
    virtual ~ScriptController();
    void setNoDaemon(bool val);

    Q_INVOKABLE QString fileChecksum(const QString& fname);
    Q_INVOKABLE QString stringChecksum(const QString& str);
    Q_INVOKABLE QString createTemporaryFileName(const QString& code);
    //Q_INVOKABLE bool runProcess(const QString& processor_name, const QString& parameters_json);
    Q_INVOKABLE bool runPipeline(const QString& json);
    Q_INVOKABLE void log(const QString& message);

private:
    ScriptControllerPrivate* d;
};

#endif // SCRIPTCONTROLLER_H
