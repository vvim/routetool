#include "mysortfilterproxymodel.h"
#include <QDebug>

MySortFilterProxyModel::MySortFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}

bool MySortFilterProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    // Qt::DisplayRole nog aanpassen voor QDate?
    // zie eerste vb van modelview

    QVariant leftData = sourceModel()->data(left);
    QVariant rightData = sourceModel()->data(right);

    if (leftData.type() == QVariant::DateTime)
    {
            return leftData.toDateTime() < rightData.toDateTime();
    }
    else if (leftData.type() == QVariant::Int)
    {
        return leftData.toInt() < rightData.toInt();
    }
    else if (leftData.type() == QVariant::Double)
    {
        return leftData.toDouble() < rightData.toDouble();
    }
    else
    {
        QString leftString = leftData.toString();
        QString rightString = rightData.toString();
        return QString::localeAwareCompare(leftString, rightString) < 0;
    }
}

QVariant MySortFilterProxyModel::data(const QModelIndex &index, int role) const
{
    // not used: int row = index.row();
    int col = index.column();

    switch(role)
    {

    case Qt::DisplayRole:
        if((col == WEIGHT_KURK) || (col == WEIGHT_KAARS))
            return QString("%1 kg").arg(QSortFilterProxyModel::data(index,role).toString());
        if(col == AANMELDING_DATE)
            return QLocale().toString(QSortFilterProxyModel::data(index,role).toDate(),"d MMM yyyy");
        break;
    }

    return QSortFilterProxyModel::data(index,role);
}

bool MySortFilterProxyModel::setData(const QModelIndex & index, const QVariant & value, int role)
{
    bool result = QSortFilterProxyModel::setData(index, value, role);
    if (role == Qt::CheckStateRole)
    {
        emit checkChanges();
    }
    return result;
}
