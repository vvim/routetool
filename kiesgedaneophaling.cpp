#include "kiesgedaneophaling.h"
#include <QVBoxLayout>
#include <QDebug>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>

#define vvimDebug()\
    qDebug() << "[" << Q_FUNC_INFO << "]"

KiesGedaneOphaling::KiesGedaneOphaling(QWidget *parent) :
    QWidget(parent)
{
    ophalingenLabel = new QLabel(tr("Voor welke ophaalronde wil je de hoeveelheden bevestigen?"));
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
    setWindowTitle(tr("Bevestigen opgehaalde hoeveelheden"));
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
    delete this;
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


    QSqlQuery query;
    query.prepare("SELECT * FROM aanmelding WHERE ophaalronde_datum = :ophaalrondedatum ORDER BY volgorde");
    query.bindValue(":ophaalrondedatum",ophaalronde_datum);
    if(!query.exec())
    {
        qCritical(QString(tr("SELECT * FROM aanmelding WHERE ophaalronde_datum = %1 ORDER BY volgorde FAILED!").arg(ophaalronde_datum.toString()).append(query.lastError().text())).toStdString().c_str());
        return; // errorboodschap tonen???
    }
    else
    {
        while(query.next())
        {
            vvimDebug() << query.value(10).toDate().toString() << "volgorde" << query.value(11).toInt();
        }
    }

    reject();
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
