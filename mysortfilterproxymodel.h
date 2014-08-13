#ifndef MYSORTFILTERPROXYMODEL_H
#define MYSORTFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>
#include <QDate>

class MySortFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT;

public:
    MySortFilterProxyModel(QObject *parent = 0);


protected:
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const;

};

#endif // MYSORTFILTERPROXYMODEL_H
