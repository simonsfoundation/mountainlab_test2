/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 5/24/2016
*******************************************************/

#include <QCoreApplication>
#include <QDir>
#include "textfile.h"

QStringList find_paths(QString basepath, QStringList patterns, bool recursive);
struct TODORecord {
    QString path;
    int line_number;
    QString text;
};

int main(int argc, char* argv[])
{
    QCoreApplication a(argc, argv);

    QList<TODORecord> todo_records;

    QStringList patterns;
    patterns << "*.h"
             << "*.cpp"
             << "*.js"
             << "*.m";
    bool recursive = true;
    QString basepath = a.applicationDirPath() + "/../../..";
    QStringList paths = find_paths(basepath, patterns, recursive);
    foreach (QString path, paths) {
        QStringList lines = read_text_file(path).split("\n");
        for (int i = 0; i < lines.count(); i++) {
            QString line = lines[i].trimmed();
            if ((line.startsWith("/// TODO")) || (line.startsWith("%%% TODO"))) {
                TODORecord R;
                R.path = path;
                R.line_number = i + 1;
                R.text = line;
                todo_records << R;
            }
        }
    }

    QString todo_text;
    foreach (TODORecord R, todo_records) {
        QString relpath = R.path.mid(basepath.count());
        todo_text += QString("%1:%2 %3\n").arg(relpath).arg(R.line_number).arg(R.text);
    }
    write_text_file(basepath + "/todo.txt", todo_text);

    return 0;
}

QStringList find_paths(QString basepath, QStringList patterns, bool recursive)
{
    QStringList ret;
    QStringList fnames = QDir(basepath).entryList(patterns, QDir::Files, QDir::Name);
    foreach (QString fname, fnames) {
        ret << basepath + "/" + fname;
    }

    if (recursive) {
        QStringList dirnames = QDir(basepath).entryList(QStringList("*"), QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
        foreach (QString dirname, dirnames) {
            ret.append(find_paths(basepath + "/" + dirname, patterns, recursive));
        }
    }
    return ret;
}
