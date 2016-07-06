/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 7/5/2016
*******************************************************/

#ifndef SHERPAV1
#define SHERPAV1

#include <QWizard>
#include <mvcontext.h>
#include <mvmainwindow.h>

class SherpaV1Private;
class SherpaV1 : public QWizard {
    Q_OBJECT
public:
    friend class SherpaV1Private;
    SherpaV1(MVContext* mvcontext, MVMainWindow* mw);
    virtual ~SherpaV1();

private
slots:
    void slot_button_clicked();
    void slot_next_channel(int offset=1);
    void slot_previous_channel();
    void slot_cluster_channel_matrix_computed();

private:
    SherpaV1Private* d;
};

#endif // SHERPAV1
