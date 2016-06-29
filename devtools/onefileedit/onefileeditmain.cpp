#include <QCoreApplication>
#include "get_command_line_params.h"
#include "textfile.h"
#include <stdio.h>
#include <QDir>
#include <iostream>
#include "onefile.h"
#include <QStringList>
#include <QDebug>

void print_usage();
QStringList find_file_paths(QString dirpath,QString filter,bool recursive);

int main(int argc, char *argv[])
{
    //QCoreApplication a(argc, argv);

    CLParams CLP=get_command_line_params(argc,argv);

    if (CLP.unnamed_parameters.count()==0) {
        print_usage();
        return 0;
    }

    QString dirpath=CLP.unnamed_parameters.value(0);
    QString type0=CLP.named_parameters.value("type","m").toString();
    QString editor=CLP.named_parameters.value("editor","/usr/bin/gedit").toString();
    bool recursive=CLP.named_parameters.value("recursive",0).toBool();
    QString filter="*."+type0;

    QStringList file_paths=find_file_paths(dirpath,filter,recursive);

    QString txt=create_one_file_text(file_paths,type0);
    QString tmp_fname=create_temporary_file(type0);
    write_text_file(tmp_fname,txt);
    open_editor(editor,tmp_fname);
    printf("Press [ENTER] to save changes to files: ");
    std::cin.ignore();
    onefile_save_changes(tmp_fname);

    //return a.exec();
    return 0;
}

QStringList find_file_paths(QString dirpath,QString filter,bool recursive) {
    QStringList file_paths;
    QStringList list=QDir(dirpath).entryList(QStringList(filter),QDir::Files,QDir::Name);
    foreach (QString fname,list) {
        file_paths << dirpath+"/"+fname;
    }

    if (recursive) {
        QStringList list2=QDir(dirpath).entryList(QStringList("*"),QDir::Dirs|QDir::NoDotAndDotDot,QDir::Name);
        foreach (QString str2,list2) {
            QStringList tmp=find_file_paths(dirpath+"/"+str2,filter,recursive);
            file_paths.append(tmp);
        }
    }
    return file_paths;
}

void print_usage() {
    printf("Usage: onefileedit [dirname] --type=m --editor=/usr/bin/gedit --recursive=1\n");
}
