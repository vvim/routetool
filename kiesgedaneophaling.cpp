#include "kiesgedaneophaling.h"
#include <QVBoxLayout>
#include <QDebug>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>


#include "opgehaaldehoeveelheid.h"


#define vvimDebug()\
    qDebug() << "[" << Q_FUNC_INFO << "]"

KiesGedaneOphaling::KiesGedaneOphaling(bool confirm, QWidget *parent) :
    QWidget(parent)
{
    vvimDebug() << "<vvim> [TODO] if window KiesGedaneOphaling gets closed: NOTHING HAPPENS! how to delete from memory???";

    m_confirm_or_cancel = confirm;

    if(m_confirm_or_cancel)
    {
        // select route to confirm it
        ophalingenLabel = new QLabel(tr("Voor welke ophaalronde wil je de hoeveelheden bevestigen?"));
        setWindowTitle(tr("Bevestigen opgehaalde hoeveelheden"));
    }
    else
    {
        // select route to cancel it
        ophalingenLabel = new QLabel(tr("Welke ophaalronde wil je annuleren?"));
        setWindowTitle(tr("Annuleer geplande ophaalronde"));
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

    if(m_confirm_or_cancel)
    {
        vvimDebug() << "moving on to confirming a finished route";
        confirmRoute(ophaalronde_datum);
    }
    else
    {
        vvimDebug() << "moving on to CANCELING a finished route, first ask permission to cancel selected route";
        cancelRoute(ophaalronde_datum);
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

    QSqlQuery query;
    query.prepare("SELECT DISTINCT ophaalronde_datum FROM aanmelding WHERE volgorde is not null ORDER BY ophaalronde_datum");
    if(!query.exec())
    {
        qCritical(QString(tr("SELECT DISTINCT ophaalronde_datum FROM aanmelding WHERE volgorde is not null ORDER BY ophaalronde_datum FAILED!").append(query.lastError().text())).toStdString().c_str());
        return -1;
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
    //assuming m_confirm_or_cancel == TRUE -> tested by accept()
    OpgehaaldeHoeveelheid *dialogboxToConfirmCollectedQuantities = new OpgehaaldeHoeveelheid(ophaalronde_datum);
    dialogboxToConfirmCollectedQuantities->show();
    reject();
}

void KiesGedaneOphaling::cancelRoute(QDate ophaalronde_datum)
{
    //assuming m_confirm_or_cancel == FALSE -> tested by accept()
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, tr("Annuleer geselecteerde ophaalronde"),
                    tr("Ben je zeker dat je de geplande ophaalronde van %1 wilt annuleren?").arg(ophaalronde_datum.toString()), QMessageBox::Yes|QMessageBox::No);

    if(reply == QMessageBox::No)
    {
        vvimDebug() << "user declined";
        return;
    }

    vvimDebug() << "user agreed";
    QSqlQuery query;
    query.prepare("UPDATE aanmelding SET ophaalronde_datum=NULL, volgorde=NULL where ophaalronde_datum = :ophaalrondedatum");
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

    reject();
}
