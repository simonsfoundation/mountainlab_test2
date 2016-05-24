/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 5/24/2016
*******************************************************/

#include <QCoreApplication>

QStringList recursive_find_paths(QString basepath,QStringList patterns);

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QStringList patterns; patterns << "*.h" << "*.cpp" << "*.js" << "*.m";
    QStringList paths=recursive_find_paths(a.applicationDirPath()+"/../../..",patterns);
    foreach (QString path,paths) {
        QStringList lines=read_text_file
    }

    return 0;
}

