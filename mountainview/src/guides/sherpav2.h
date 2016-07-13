/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 7/5/2016
*******************************************************/

#ifndef SHERPAV2
#define SHERPAV2

#include <QWizard>
#include <mvcontext.h>
#include <mvmainwindow.h>

class SherpaV2Private;
class SherpaV2 : public QWizard {
    Q_OBJECT
public:
    friend class SherpaV2Private;
    SherpaV2(MVContext* mvcontext, MVMainWindow* mw);
    virtual ~SherpaV2();

private slots:
    void slot_button_clicked();
    void slot_select_merge_candidates();
    void slot_merge_all_merge_candidates();

private:
    SherpaV2Private* d;
};

#endif // SHERPAV2
