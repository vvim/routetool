#ifndef OphaalpuntenWidgetSortFilterProxyModel_H
#define OphaalpuntenWidgetSortFilterProxyModel_H

#define OPHAALPUNTQTREEVIEW_OPHAALPUNT_ID       0
#define OPHAALPUNTQTREEVIEW_OPHAALPUNT_NAAM     1
#define OPHAALPUNTQTREEVIEW_OPHAALPUNT_STRAAT   2
#define OPHAALPUNTQTREEVIEW_OPHAALPUNT_HUISNR   3
#define OPHAALPUNTQTREEVIEW_OPHAALPUNT_BUSNR    4
#define OPHAALPUNTQTREEVIEW_OPHAALPUNT_POSTCODE 5
#define OPHAALPUNTQTREEVIEW_OPHAALPUNT_PLAATS   6
#define OPHAALPUNTQTREEVIEW_OPHAALPUNT_LAND     7
#define OPHAALPUNTQTREEVIEW_AANMELDING_PRESENT  8

//#define LIST_ITEMCOLUMNS LIST_FORECAST_NEW_OPHALING_DATE + 1


#include <QSortFilterProxyModel>
#include <QDate>

class OphaalpuntenWidgetSortFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT;

public:
    OphaalpuntenWidgetSortFilterProxyModel(QObject *parent = 0);
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

protected:
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const;
    bool setData(const QModelIndex & index, const QVariant & value, int role);
};

#endif // OphaalpuntenWidgetSortFilterProxyModel_H
