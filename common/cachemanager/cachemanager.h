/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/5/2016
*******************************************************/

#ifndef MSCACHEMANAGER_H
#define MSCACHEMANAGER_H

#include <QObject>

class MSCacheManagerPrivate;
class MSCacheManager : public QObject {
public:
    enum Duration {
        ShortTerm,
        LongTerm
    };

    friend class MSCacheManagerPrivate;
    MSCacheManager();
    virtual ~MSCacheManager();

    void setLocalBasePath(const QString& path);
    QString makeRemoteFile(const QString& remote_name, const QString& file_name = "", Duration duration = ShortTerm);
    QString makeLocalFile(const QString& file_name = "", Duration duration = ShortTerm);
    void cleanUp();

    static MSCacheManager *globalInstance();

private slots:
    void slot_remove_on_delete();

private:
    MSCacheManagerPrivate* d;
};

#endif // MSCACHEMANAGER_H
