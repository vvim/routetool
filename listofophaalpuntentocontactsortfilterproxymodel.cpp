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
    //int row = index.row();
    int col = index.column();
    QDate date_to_display;

    QModelIndex aanmelding_present = index.sibling(index.row(), LIST_AANMELDING_PRESENT);

    switch(role)
    {

        case Qt::DisplayRole:
            switch(col)
            {
            // todo vvim: IF data(index.role).toDate().isEmpty() then return "nog geen ophaling gepland" ??
                case LIST_LAST_CONTACT_DATE:
                    date_to_display = QSortFilterProxyModel::data(index,role).toDate();
                    if(date_to_display.isNull())
                        return "geen contact bekend";
                    return QLocale().toString(date_to_display,"d MMM yyyy");
                break;
                case LIST_LAST_OPHALING_DATE:
                    date_to_display = QSortFilterProxyModel::data(index,role).toDate();
                    if(date_to_display.isNull())
                        return "geen ophaling bekend";
                    return QLocale().toString(date_to_display,"d MMM yyyy");
                break;
                case LIST_FORECAST_NEW_OPHALING_DATE:
                    date_to_display = QSortFilterProxyModel::data(index,role).toDate();
                    if(date_to_display.isNull())
                        return "geen voorspelling mogelijk";
                    return QLocale().toString(date_to_display,"d MMM yyyy");
                break;
            }
            break; // end     case Qt::DisplayRole:


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
