#include "process_msh.h"
#include "textfile.h"
#include <QCoreApplication>
#include <QFileInfo>
#include <QProcess>
#include <QDebug>

int process_msh(const QString &path,int argc,char *argv[])
{
    QString txt=read_text_file(path);

    QString check_exit_status="rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi\n";

    QString txt2="";
    txt2+=QString("\n\nmountainview=%1/../mountainview/bin/mountainview\n\n").arg(qApp->applicationDirPath());
    txt2+=QString("\n\nscriptdir=%1\n\n").arg(QFileInfo(path).absolutePath());
    QList<QString> lines=txt.split("\n");
    QString buffer="";
    bool in_mscmd=false;
    QString command_for_run;
    for (int i=0; i<lines.count(); i++) {
        QString line=lines[i];
        if (line.indexOf("mscmd ")==0) {
            if (in_mscmd) {
                txt2+="echo "+command_for_run+"\n";
                txt2+=QString("%1/mountainsort ").arg(qApp->applicationDirPath())+" "+command_for_run+"\n";
                command_for_run="";
                txt2+=check_exit_status;
                txt2+="echo\n";
            }
            command_for_run=line.mid(QString("mscmd ").count());

            in_mscmd=true;
        }
        else if (!line.trimmed().startsWith("--")) {
            if (in_mscmd) {
                txt2+="echo "+command_for_run+"\n";
                txt2+=QString("%1/mountainsort ").arg(qApp->applicationDirPath())+" "+command_for_run+"\n";
                command_for_run="";
                txt2+=check_exit_status;
                txt2+="echo\n";
            }
            in_mscmd=false;
            txt2+=line+"\n";
        }
        else {
            if (in_mscmd) command_for_run+=" "+line;
            else txt2+=line+"\n";
        }
    }
    if (in_mscmd) {
        txt2+="echo "+command_for_run+"\n";
        txt2+=QString("%1/mountainsort ").arg(qApp->applicationDirPath())+" "+command_for_run+"\n";
        command_for_run="";
        txt2+=check_exit_status;
        txt2+="echo\n";
    }

    QStringList args; args << path+".tmp";
    QString set_args_txt;
    for (int i=2; i<argc; i++) {
        QString arg0=argv[i];
        if ((arg0.startsWith("--"))&&(arg0.contains("="))) {
            QStringList tmplist=arg0.mid(2).split("=");
            if (tmplist.value(0)=="include") {
                set_args_txt+=QString("#including %1\n").arg(tmplist.value(1));
                QString tmp0=read_text_file(tmplist.value(1));
                set_args_txt+=tmp0;
                if (tmp0.isEmpty()) qWarning() << "Unable to read file (or file is empty): " << tmplist.value(1);
                set_args_txt+="\n";
            }
            else {
                set_args_txt+=QString("%1=%2\n").arg(tmplist.value(0)).arg(tmplist.value(1));
            }
        }
        else {
            args << argv[i];
        }
    }
    if (txt.indexOf("#USER CONFIG#")>=0) {
        txt2=txt2.replace("#USER CONFIG#",set_args_txt);
    }
    else {
        txt2=set_args_txt+txt2;
    }
    write_text_file(path+".tmp",txt2);
    return QProcess::execute("/bin/bash",args);
}
