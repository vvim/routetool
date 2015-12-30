#ifndef ListOfOphaalpuntenToContactSortFilterProxyModel_H
#define ListOfOphaalpuntenToContactSortFilterProxyModel_H

#define LIST_OPHAALPUNT_NAAM 0
#define LIST_OPHAALPUNT_ID 1
#define LIST_POSTCODE 2
#define LIST_LAST_CONTACT_DATE 3
#define LIST_LAST_OPHALING_DATE 4
#define LIST_FORECAST_NEW_OPHALING_DATE 5
#define LIST_AANMELDING_PRESENT 6

//#define LIST_ITEMCOLUMNS LIST_FORECAST_NEW_OPHALING_DATE + 1


#include <QSortFilterProxyModel>
#include <QDate>

class ListOfOphaalpuntenToContactSortFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT;

public:
    ListOfOphaalpuntenToContactSortFilterProxyModel(QObject *parent = 0);
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

protected:
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const;
    bool setData(const QModelIndex & index, const QVariant & value, int role);
};

#endif // ListOfOphaalpuntenToContactSortFilterProxyModel_H
