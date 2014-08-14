#ifndef MYSORTFILTERPROXYMODEL_H
#define MYSORTFILTERPROXYMODEL_H

#define OPHAALPUNT_NAAM 0
#define AANMELDING_DATE 1
#define WEIGHT_KURK 2
#define ZAK_KURK 3
#define WEIGHT_KAARS 4
#define ZAK_KAARS 5
#define AANMELDING_ID 6
#define OPHAALPUNT_ID 7
#define STRAAT 8
#define HUISNR 9
#define BUSNR 10
#define POSTCODE 11
#define PLAATS 12
#define LAND 13
#define OPMERKINGEN 14

#define ITEMCOLUMNS OPMERKINGEN + 1


#include <QSortFilterProxyModel>
#include <QDate>

class MySortFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT;

public:
    MySortFilterProxyModel(QObject *parent = 0);

signals:
    void checkChanges();

protected:
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex & index, const QVariant & value, int role);
};

#endif // MYSORTFILTERPROXYMODEL_H
