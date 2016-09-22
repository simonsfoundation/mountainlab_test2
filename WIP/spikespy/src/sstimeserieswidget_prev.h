#ifndef SSTIMESERIESWIDGET_H
#define SSTIMESERIESWIDGET_H

#include "sscommon.h"
#include "ssabstractview_prev.h"

/// Todo, remove this file, obsolete

class SSTimeSeriesWidgetPrivate;
class SSTimeSeriesWidget : public QWidget {
    Q_OBJECT
public:
    friend class SSTimeSeriesWidgetPrivate;
    explicit SSTimeSeriesWidget(QWidget* parent = 0);
    ~SSTimeSeriesWidget();

    Q_INVOKABLE void addView(SSAbstractView* V);
    void setClipData(const Mda& X);
    void hideMenu();
    SSAbstractView* view(int index = 0);

private:
    SSTimeSeriesWidgetPrivate* d;

private slots:
    void slot_current_x_changed();
    void slot_current_channel_changed();
    void slot_x_range_changed();
    void slot_replot_needed();
    void slot_selection_range_changed();
    void slot_view_clicked();

    void slot_extract_clips();
    void slot_extract_comparison_clips();
    void slot_center_on_cursor();
    void slot_view_clusters();

    void slot_navigation_instructions();

signals:

public slots:
};

#endif // SSTIMESERIESWIDGET_H
