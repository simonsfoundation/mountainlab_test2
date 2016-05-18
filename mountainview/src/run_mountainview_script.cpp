/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 3/24/2016
*******************************************************/

#include "run_mountainview_script.h"
#include <QImageWriter>
#include <QScriptEngine>
#include "mvclusterdetailwidget.h"

/// TODO get rid of mountainview scripting and non-gui image export

int run_mountainview_script(const QString& script, QMap<QString, QVariant>& params)
{
    QScriptEngine* engine = new QScriptEngine;

    MVController Controller;
    QScriptValue MV = engine->newQObject(&Controller);
    engine->globalObject().setProperty("MV", MV);
    QScriptValue result = engine->evaluate(script);
    if (result.isError()) {
        qWarning() << "Error running script" << result.toString();
        return -1;
    }
    return 0;
}

void MVController::writeImage(const QImage& img, const QString& fname)
{
    QImageWriter IW(fname);
    if (!IW.write(img)) {
        qWarning() << "Unable to write image to" << fname;
    }
}

void MVController::appExec()
{
    qApp->exec();
}
