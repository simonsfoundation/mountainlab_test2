/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
*******************************************************/

#ifndef MVCROSSCORRELOGRAMSWIDGET_H
#define MVCROSSCORRELOGRAMSWIDGET_H

#include <QWidget>
#include "diskreadmda.h"
#include <QMap>
#include <QColor>

/** \class MVCrossCorrelogramsWidget
 *  \brief Presents a grid of cross-correlograms as histogram views
 */

class MVCrossCorrelogramsWidgetPrivate;
class MVCrossCorrelogramsWidget : public QWidget {
    Q_OBJECT
public:
    friend class MVCrossCorrelogramsWidgetPrivate;
    MVCrossCorrelogramsWidget();
    virtual ~MVCrossCorrelogramsWidget();

    ///Set the cross-correlogram data computed, for example, in MVOverview2Widget
    void setCrossCorrelogramsPath(const QString& path);
    ///Set the cross-correlogram data computed, for example, in MVOverview2Widget
    void setCrossCorrelogramsData(const DiskReadMda& X);
    ///Set the string labels for display
    void setTextLabels(const QStringList& labels);
    ///To make a uniform look. TODO: handle this properly
    void setColors(const QMap<QString, QColor>& colors);
    ///Recreate all the subwidgets
    void updateWidget();

    ///The current label (or cluster number)
    int currentLabel();
    ///The selected labels (or cluster numbers)
    QList<int> selectedLabels();
    ///Set current label (or cluster number)
    void setCurrentLabel(int num);
    ///Set selected labels (or cluster numbers)
    void setSelectedLabels(const QList<int>& nums);
    ///Not to be explained at this time for lack of a clear explanation
    int baseLabel();
    ///Not to be explained at this time for lack of a clear explanation
    void setBaseLabel(int num);

    ///Hmmmm....
    void setLabelNumbers(const QList<int>& numbers);

signals:
    ///The current label (or cluster number) has changed
    void currentLabelChanged();
    ///The current label (or cluster number) has been double clicked (or enter pressed?)
    void labelActivated(int num);
    ///The set of selected labels (or cluster numbers) have changed
    void selectedLabelsChanged();

private
slots:
    void slot_histogram_view_clicked();
    void slot_histogram_view_control_clicked();
    void slot_histogram_view_activated();

protected:
    void keyPressEvent(QKeyEvent*);

private:
    MVCrossCorrelogramsWidgetPrivate* d;
};

#endif // MVCROSSCORRELOGRAMSWIDGET_H
