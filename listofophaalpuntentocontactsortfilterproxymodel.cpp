#include "listofophaalpuntentocontactsortfilterproxymodel.h"
#include <QDebug>

ListOfOphaalpuntenToContactSortFilterProxyModel::ListOfOphaalpuntenToContactSortFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}

bool ListOfOphaalpuntenToContactSortFilterProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
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

QVariant ListOfOphaalpuntenToContactSortFilterProxyModel::data(const QModelIndex &index, int role) const
{
    // not used: int row = index.row();
    int col = index.column();

    switch(role)
    {

    case Qt::DisplayRole:
        if((col == LIST_LAST_CONTACT_DATE) || (col == LIST_LAST_OPHALING_DATE) || (col == LIST_FORECAST_NEW_OPHALING_DATE))
            return QLocale().toString(QSortFilterProxyModel::data(index,role).toDate(),"d MMM yyyy");
        break;
    }

    return QSortFilterProxyModel::data(index,role);
}

bool ListOfOphaalpuntenToContactSortFilterProxyModel::setData(const QModelIndex & index, const QVariant & value, int role)
{
    bool result = QSortFilterProxyModel::setData(index, value, role);
    if (role == Qt::CheckStateRole)
    {
        emit checkChanges();
    }
    return result;
}
