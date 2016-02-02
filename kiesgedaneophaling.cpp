#include "kiesgedaneophaling.h"
#include <QVBoxLayout>
#include <QDebug>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>


#include "opgehaaldehoeveelheid.h"
#include "globalfunctions.h"

KiesGedaneOphaling::KiesGedaneOphaling(KGOPurpose purpose, QWidget *parent) :
    QWidget(parent)
{
    vvimDebug() << "<vvim> [TODO] if window KiesGedaneOphaling gets closed: NOTHING HAPPENS! how to delete from memory???";

    m_confirm_or_cancel = purpose;

    switch(m_confirm_or_cancel)
    {
        case Confirming:
            vvimDebug() << "purpose is to confirm finished routes";
            // select route to confirm it
            ophalingenLabel = new QLabel(tr("Voor welke ophaalronde wil je de hoeveelheden bevestigen?"));
            setWindowTitle(tr("Bevestigen opgehaalde hoeveelheden"));
            break;

        case Deleting:
            vvimDebug() << "purpose is to delete a planned route";
            // select route to cancel it
            ophalingenLabel = new QLabel(tr("Welke ophaalronde wil je annuleren?"));
            setWindowTitle(tr("Annuleer geplande ophaalronde"));
            break;

        case Editing:
            vvimDebug() << "purpose is to edit a planned route";
            ophalingenLabel = new QLabel(tr("Welke ophaalronde wil je aanpassen?"));
            setWindowTitle(tr("Wijzig geplande ophaalronde"));
            break;

        default:
            vvimDebug() << "this should not be happening, Default option triggered in switch(m_confirm_or_cancel) , ERROR";
    }

    ophalingenComboBox = new QComboBox();
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    QVBoxLayout *layout = new QVBoxLayout();

    connect(buttonBox, SIGNAL(accepted()),this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()),this, SLOT(reject()));

    layout->addWidget(ophalingenLabel);
    layout->addWidget(ophalingenComboBox);
    layout->addWidget(buttonBox);

    setLayout(layout);
    setMinimumWidth(500);
    setMinimumHeight(200);
}

KiesGedaneOphaling::~KiesGedaneOphaling()
{
    vvimDebug() << "start to deconstruct KiesGedaneOphaling()";
    delete ophalingenLabel;
    delete ophalingenComboBox;
    delete buttonBox;
    vvimDebug() << "KiesGedaneOphaling() deconstructed";
}

void KiesGedaneOphaling::reject()
{
    close();
    vvimDebug() << "before deleting self";
/* <vvim> TODO: does this one result in a crash? */    delete this;
    vvimDebug() << "after deleting self (shouldn't show though :-) )";
}

void KiesGedaneOphaling::accept()
{
    if( ophalingenComboBox->currentIndex() < 1)
    {
        QMessageBox::warning(this, tr("Geen ophalingsdatum gekozen"), tr("Kies een ophaalronde om de effectief opgehaalde hoeveelheden te bevestigen."));
        return;
    }
    QDate ophaalronde_datum = ophalingenMap[ophalingenComboBox->currentIndex()];
    vvimDebug() << "choice has been made" << ophalingenComboBox->currentIndex() << ophalingenComboBox->itemText(ophalingenComboBox->currentIndex()) << ophalingenMap[ophalingenComboBox->currentIndex()].toString();

    switch(m_confirm_or_cancel)
    {
        case Confirming:
            vvimDebug() << "moving on to CONFIRMING a finished route";
            confirmRoute(ophaalronde_datum);
            break;

        case Deleting:
            vvimDebug() << "moving on to CANCELING a planned route, first ask permission to cancel selected route";
            cancelRoute(ophaalronde_datum);
            break;

        case Editing:
            vvimDebug() << "moving on to EDITING a planned route, should notify Routetool that we are _editing_";
            editRoute(ophaalronde_datum);
            break;

        default:
            vvimDebug() << "this should not be happening, Default option triggered in switch(m_confirm_or_cancel) , ERROR";
    }
}

int KiesGedaneOphaling::initialise()
{
    vvimDebug() << "starting initialisation";

    ophalingenComboBox->clear();
    ophalingenComboBox->insertItem(0,"");

    /**
      3. dubbelen verwijderen? => zie select in 1.
                ???=[3.]=> select distinct aanmelding.ophaalronde_datum from aanmelding where volgorde is not null
                                            AND not exists
                            (select null from ophalinghistoriek
                             wehere ophalinghistoriek.ophaalpunt = aanmelding.ophaalpunt and ophalinghistoriek.datum = ophaalronde_datum)
                    zie regel 361 van void ListOfOphaalpuntenToContact::show_never_contacted_ophaalpunten()
    **/

    QString SQLquery = "SELECT DISTINCT ophaalronde_datum FROM aanmelding WHERE volgorde is not null ORDER BY ophaalronde_datum";
    QSqlQuery query(SQLquery);

    if(!query.exec())
    {
        if(!reConnectToDatabase(query.lastError(), SQLquery, QString("[%1]").arg(Q_FUNC_INFO)))
        {
            vvimDebug() << "unable to reconnect to DB, returning -1";
            qCritical(QString(tr("Needed to reconnect to database, but this failed. SELECT DISTINCT ophaalronde_datum FROM aanmelding WHERE volgorde is not null ORDER BY ophaalronde_datum FAILED!").append(query.lastError().text())).toStdString().c_str());
            return -1;
        }
        query = QSqlQuery(SQLquery);
        if(!query.exec())
        {
            vvimDebug() << "Reconnection to database was successfull, but query failed, returning -1";
            qCritical(QString(tr("Reconnection to database was successfull, but query failed. SELECT DISTINCT ophaalronde_datum FROM aanmelding WHERE volgorde is not null ORDER BY ophaalronde_datum FAILED!").append(query.lastError().text())).toStdString().c_str());
            return -1;
        }
    }


    int i = 0;
    while(query.next())
    {
        i++;
        QDate ophaalronde = query.value(0).toDate();
        ophalingenComboBox->insertItem(i,QLocale().toString(ophaalronde)); // ,"ddd d MMM yyyy"
        ophalingenMap.insert(i,ophaalronde);
        vvimDebug() << "ophaalronde" << i << ":" << ophaalronde.toString();
    }

    vvimDebug() << "totaal:" << i << "ophaalrondes";
    if(i == 0)
    {
        vvimDebug() << "De teller staat nog steeds op 0:" << i << "dus geen onbevestigde ophaalrondes. Het heeft geen zin om deze dialogbox te tonen: abort!";
        return 0;
    }

    vvimDebug() << "All okay, we found" << i << " onbevestigde ophaalrondes. Show dialogbox?";
    return 1;
    /// -> if i == 1 => geen ophaalrondes gevonden! => messagebox, no execute of this dialogbox!!
    /// => geef de functie initialise() een return waarde int: -1 is error, 0 is niets gevonden, +1 is alles ok
}


void KiesGedaneOphaling::confirmRoute(QDate ophaalronde_datum)
{
    //assuming m_confirm_or_cancel == Confirming -> tested by accept()
    OpgehaaldeHoeveelheid *dialogboxToConfirmCollectedQuantities = new OpgehaaldeHoeveelheid(ophaalronde_datum);
    dialogboxToConfirmCollectedQuantities->show();
    reject();
}

void KiesGedaneOphaling::editRoute(QDate ophaalronde_datum)
{
    vvimDebug() << "[0] FIRST, CLEAN CURRENT ROUTE. Save it ???";

    // will be done in RouteTool::showOphaalrondeAanpassen() , by asking the user in a MessageBox

    //assuming m_confirm_or_cancel == Editing -> tested by accept()

    vvimDebug() << "get ready to edit" << "[A] SELECT ALL locations from route" << ophaalronde_datum.toString();

    QString SQLquery = QString("SELECT * from aanmelding, ophaalpunten where aanmelding.ophaalpunt = ophaalpunten.id AND ophaalronde_datum = %1 ORDER BY volgorde ASC").arg(ophaalronde_datum.toString());

    QSqlQuery query;
    query.prepare(" SELECT ophaalpunten.naam, aanmelding.kg_kurk, aanmelding.kg_kaarsresten, "
                  "        aanmelding.zakken_kurk, aanmelding.zakken_kaarsresten, aanmelding.id, "
                  "        ophaalpunten.id, aanmelding.opmerkingen, ophaalpunten.straat, "
                  "        ophaalpunten.nr, ophaalpunten.bus, ophaalpunten.postcode, "
                  "        ophaalpunten.plaats, ophaalpunten.land "
                  " FROM aanmelding, ophaalpunten "
                  " WHERE aanmelding.ophaalpunt = ophaalpunten.id AND ophaalronde_datum = :ophaalrondedatum "
                  " ORDER BY volgorde ASC");
    query.bindValue(":ophaalrondedatum",ophaalronde_datum);

    if(!query.exec())
    {
        vvimDebug() << "first try went wrong, trying to reconnect to DB" << query.lastError().text();

        if(!reConnectToDatabase(query.lastError(), SQLquery, QString("[%1]").arg(Q_FUNC_INFO)))
        {
            vvimDebug() << "unable to reconnect to DB, showing critical QMessagebox";
            QMessageBox::critical(this,tr("Geen verbinding met databank, dus wijzigen van ophaalronde niet gelukt"),
                                query.lastError().text().append(tr("\n\nHerstel de fout en probeer opnieuw.")), QMessageBox::Cancel);
            qCritical(QString(tr("Wijzigen van ophaalronde %1 is niet gelukt! Kon geen verbinding met databank maken... SELECT * from aanmelding, ophaalpunten where aanmelding.ophaalpunt = ophaalpunten.id AND ophaalronde_datum = %1 ORDER BY volgorde ASC , error: ").arg(ophaalronde_datum.toString()).append(query.lastError().text())).toStdString().c_str());
            return;
        }
        QSqlQuery query2;
        query2.prepare(" SELECT ophaalpunten.naam, aanmelding.kg_kurk, aanmelding.kg_kaarsresten, "
                       "        aanmelding.zakken_kurk, aanmelding.zakken_kaarsresten, aanmelding.id, "
                       "        ophaalpunten.id, aanmelding.opmerkingen, ophaalpunten.straat, "
                       "        ophaalpunten.nr, ophaalpunten.bus, ophaalpunten.postcode, "
                       "        ophaalpunten.plaats, ophaalpunten.land "
                       " FROM aanmelding, ophaalpunten "
                       " WHERE aanmelding.ophaalpunt = ophaalpunten.id AND ophaalronde_datum = :ophaalrondedatum "
                       " ORDER BY volgorde ASC");
        query = query2;
        query.bindValue(":ophaalrondedatum",ophaalronde_datum);

        if(!query.exec())
        {
            QMessageBox::critical(this,tr("Wijzigen van ophaalronde niet gelukt"),
                                query.lastError().text().append(tr("\n\nHerstel de fout en probeer opnieuw.")), QMessageBox::Cancel);
            qCritical(QString(tr("Wijzigen van ophaalronde %1 is niet gelukt! SELECT * from aanmelding, ophaalpunten where aanmelding.ophaalpunt = ophaalpunten.id AND ophaalronde_datum = %1 ORDER BY volgorde ASC , error: ").arg(ophaalronde_datum.toString()).append(query.lastError().text())).toStdString().c_str());
            return; // errorboodschap tonen???
        }
    }

    vvimDebug() << "get ready to edit" << "[B] set FLAG / Boolean in Routetool , voor als de gebruiker nu klikt op 'route opslaan', dat we de vorige overschrijven (geef messagebox: overschrijven of nieuwe?)";

    QDate routeCurrentlyBeingEdited = ophaalronde_datum;

    vvimDebug() << "get ready to edit" << "[C] put locations in routetool";
    QList<SOphaalpunt> *listOfAanmeldingen = new QList<SOphaalpunt>();

    while(query.next())
    {
        QString ophaalpunt_naam = query.value(0).toString();
        double kg_kurk = query.value(1).toDouble();
        double kg_kaars = query.value(2).toDouble();
        double zakken_kurk = query.value(3).toDouble();
        double zakken_kaars = query.value(4).toDouble();
        int aanmelding_id = query.value(5).toInt();
        int ophaalpunt_id = query.value(6).toInt();
        QString opmerkingen = query.value(7).toString();  // opmerkingen (uit table aanmelding)
        QString ophaalpunt_straat = query.value(8).toString();
        QString huisnr = query.value(9).toString();
        QString busnr = query.value(10).toString();
        QString postcode = query.value(11).toString();
        QString plaats = query.value(12).toString();
        QString land = query.value(13).toString();

        SOphaalpunt _ophaalpunt(
                        ophaalpunt_naam,
                        ophaalpunt_straat,
                        huisnr,
                        busnr,
                        postcode,
                        plaats,
                        land,
                        kg_kurk,
                        kg_kaars,
                        zakken_kurk,
                        zakken_kaars,
                        aanmelding_id,
                        ophaalpunt_id,
                        opmerkingen
                    );
        listOfAanmeldingen->append(_ophaalpunt);
    }

    vvimDebug() << "get ready to edit" << "[D] emit list to routetool";
    vvimDebug() << "List size:" << listOfAanmeldingen->count();
    emit showPlannedRoute(listOfAanmeldingen, routeCurrentlyBeingEdited);
    //emit LIST


    reject();

}

void KiesGedaneOphaling::cancelRoute(QDate ophaalronde_datum)
{
    //assuming m_confirm_or_cancel == Deleting -> tested by accept()
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, tr("Annuleer geselecteerde ophaalronde"),
                    tr("Ben je zeker dat je de geplande ophaalronde van %1 wilt annuleren?").arg(ophaalronde_datum.toString()), QMessageBox::Yes|QMessageBox::No);

    if(reply == QMessageBox::No)
    {
        vvimDebug() << "user declined";
        return;
    }

    vvimDebug() << "user agreed";

    QString SQLquery = QString("UPDATE aanmelding SET ophaalronde_datum=NULL, volgorde=NULL where ophaalronde_datum = %1").arg(ophaalronde_datum.toString());

    QSqlQuery query;
    query.prepare("UPDATE aanmelding SET ophaalronde_datum=NULL, volgorde=NULL where ophaalronde_datum = :ophaalrondedatum");
    query.bindValue(":ophaalrondedatum",ophaalronde_datum);

    if(!query.exec())
    {
        vvimDebug() << "first try went wrong, trying to reconnect to DB" << query.lastError().text();

        if(!reConnectToDatabase(query.lastError(), SQLquery, QString("[%1]").arg(Q_FUNC_INFO)))
        {
            vvimDebug() << "unable to reconnect to DB, showing critical QMessagebox";
            QMessageBox::critical(this,tr("Geen verbinding met databank, dus verwijderen ophaalronde niet gelukt"),
                                query.lastError().text().append(tr("\n\nHerstel de fout en probeer opnieuw.")), QMessageBox::Cancel);
            qCritical(QString(tr("Verwijderen van ophaalronde %1 is niet gelukt! Kon geen verbinding met databank maken... UPDATE aanmelding SET ophaalronde_datum=NULL and volgorde=NULL where ophaalronde_datum = ... , error: ").arg(ophaalronde_datum.toString()).append(query.lastError().text())).toStdString().c_str());
            return;
        }
        QSqlQuery query2;
        query2.prepare("UPDATE aanmelding SET ophaalronde_datum=NULL, volgorde=NULL where ophaalronde_datum = :ophaalrondedatum");
        query = query2;
        query.bindValue(":ophaalrondedatum",ophaalronde_datum);

        if(!query.exec())
        {
            QMessageBox::critical(this,tr("Verwijderen van ophaalronde niet gelukt"),
                                query.lastError().text().append(tr("\n\nHerstel de fout en probeer opnieuw.")), QMessageBox::Cancel);
            qCritical(QString(tr("Verwijderen van ophaalronde %1 is niet gelukt! UPDATE aanmelding SET ophaalronde_datum=NULL and volgorde=NULL where ophaalronde_datum = ... , error: ").arg(ophaalronde_datum.toString()).append(query.lastError().text())).toStdString().c_str());
            return; // errorboodschap tonen???
        }
        else
        {
            vvimDebug() << "canceling route" << ophaalronde_datum.toString() << "DONE";
        }
    }
    else
    {
        vvimDebug() << "canceling route" << ophaalronde_datum.toString() << "DONE";
    }


    reject();
}
