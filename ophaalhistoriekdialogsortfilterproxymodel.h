#ifndef OphaalHistoriekDialogSortFilterProxyModel_H
#define OphaalHistoriekDialogSortFilterProxyModel_H

#define HIST_HISTORIEK_ID 0
#define HIST_OPHALINGSDATUM 1
#define HIST_CHAUFFEUR 2
#define HIST_OPHAALPUNT_ID 3
#define HIST_WEIGHT_KURK 4
#define HIST_WEIGHT_KAARS 5
#define HIST_ZAK_KURK 6
#define HIST_ZAK_KAARS 7
#define HIST_OPMERKINGEN 8

//#define HIST_ITEMCOLUMNS HIST_OPMERKINGEN + 1


#include <QSortFilterProxyModel>
#include <QDate>

class OphaalHistoriekDialogSortFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT;

public:
    OphaalHistoriekDialogSortFilterProxyModel(QObject *parent = 0);

signals:
    void checkChanges();

protected:
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex & index, const QVariant & value, int role);
};

#endif // OphaalHistoriekDialogSortFilterProxyModel_H
