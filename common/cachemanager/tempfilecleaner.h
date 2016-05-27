#ifndef TEMPFILECLEANER_H
#define TEMPFILECLEANER_H

#include <QObject>

class TempFileCleanerPrivate;
class TempFileCleaner : public QObject
{
    Q_OBJECT
public:
    friend class TempFileCleanerPrivate;
    TempFileCleaner();
    virtual ~TempFileCleaner();
    void addPath(QString path, double max_gb);
private slots:
    void slot_timer();
private:
    TempFileCleanerPrivate *d;
};

#endif // TEMPFILECLEANER_H

