/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/19/2016
*******************************************************/

#ifndef MSSCRIPTCONTROLLER
#define MSSCRIPTCONTROLLER

#include <QObject>


class MSScriptControllerPrivate;
class MSScriptController : public QObject
{
    Q_OBJECT
public:
    friend class MSScriptControllerPrivate;
    MSScriptController();
    virtual ~MSScriptController();
    Q_INVOKABLE QString fileChecksum(const QString &fname);
    Q_INVOKABLE QString stringChecksum(const QString &str);
    Q_INVOKABLE QString createTemporaryFileName(const QString &code);
    Q_INVOKABLE bool runProcess(const QString &processor_name,const QString &parameters_json);
    Q_INVOKABLE void log(const QString &message);
private:
    MSScriptControllerPrivate *d;
};

#endif // MSSCRIPTCONTROLLER

