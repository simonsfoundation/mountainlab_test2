#ifndef MOMAINWINDOW_H
#define MOMAINWINDOW_H

#include <QWidget>

class MOMainWindowPrivate;
class MOMainWindow : public QWidget {
    Q_OBJECT
public:
    friend class MOMainWindowPrivate;
    MOMainWindow();
    virtual ~MOMainWindow();
    void read(const QString& mof_path);
    void load(const QString& mof_url);
private slots:
    void slot_open_result(QString name);

private:
    MOMainWindowPrivate* d;
};

#endif // MOMAINWINDOW_H
