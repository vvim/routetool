#include "opgehaaldehoeveelheid.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QVariant>
#include <QMessageBox>

#define vvimDebug()\
    qDebug() << "[" << Q_FUNC_INFO << "]"

OpgehaaldeHoeveelheid::OpgehaaldeHoeveelheid(QDate ophaalronde_datum, QWidget *parent) :
    QWidget(parent)
{

    // zie SimpleWidgetMapper example

            QDate	ophalingsdatum;
            QString	chauffeur;
            int	ophaalpunt;
            int	zakken_kurk;
            int	kg_kurk;
            int	zakken_kaarsresten;
            int	kg_kaarsresten;
            QString	opmerkingen;



   QSqlQuery query;
   //query.prepare("SELECT * FROM aanmelding, ophaalpunten WHERE aanmelding.ophaalronde_datum = :ophaalrondedatum AND aanmelding.ophaalpunt = ophaalpunten.id ORDER BY aanmelding.volgorde");

   query.prepare("SELECT aanmelding.ophaalronde_datum, aanmelding.volgorde, ophaalpunten.naam "
                 "FROM aanmelding, ophaalpunten "
                 "WHERE aanmelding.ophaalronde_datum = :ophaalrondedatum AND aanmelding.ophaalpunt = ophaalpunten.id "
                 "ORDER BY aanmelding.volgorde");
   query.bindValue(":ophaalrondedatum",ophaalronde_datum);

   if(!query.exec())
   {
       qCritical(QString(tr("SELECT * FROM aanmelding, ophaalpunten WHERE aanmelding.ophaalronde_datum = %1 AND aanmelding.ophaalpunt = ophaalpunten.id ORDER BY aanmelding.volgorde FAILED!").arg(ophaalronde_datum.toString()).append(query.lastError().text())).toStdString().c_str());
       return; // errorboodschap tonen???
   }
   else
   {
       while(query.next())
       {
           vvimDebug() << query.value(0).toDate().toString() << "volgorde" << query.value(1).toInt() << "ophaalpunt:" << query.value(2).toString();
       }
   }

}
