#include "ophaalpuntenwidgetsortfilterproxymodel.h"
#include <QDebug>

OphaalpuntenWidgetSortFilterProxyModel::OphaalpuntenWidgetSortFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}

bool OphaalpuntenWidgetSortFilterProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
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

QVariant OphaalpuntenWidgetSortFilterProxyModel::data(const QModelIndex &index, int role) const
{
    QModelIndex aanmelding_present = index.sibling(index.row(), OPHAALPUNTQTREEVIEW_AANMELDING_PRESENT);

    switch(role)
    {
        case Qt::ForegroundRole:

            if(data(aanmelding_present).toBool())
            {
                return Qt::blue;
            }
            else
            {
                return Qt::black;
            }
            break; // end    case Qt::ForegroundRole:
    }

    return QSortFilterProxyModel::data(index,role);
}
