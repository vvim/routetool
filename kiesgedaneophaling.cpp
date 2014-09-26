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

    ophalingenComboBox->insertItem(0,"");

///// initialise -- START
    QSqlQuery query;
    query.prepare("SELECT DISTINCT ophaalronde_datum FROM aanmelding WHERE volgorde is not null ORDER BY ophaalronde_datum");
    if(!query.exec())
        qCritical(QString(tr("SELECT DISTINCT ophaalronde_datum FROM aanmelding WHERE volgorde is not null ORDER BY ophaalronde_datum FAILED!").append(query.lastError().text())).toStdString().c_str());

    int i = 1;
    while(query.next())
    {
        QDate ophaalronde = query.value(0).toDate();
        ophalingenComboBox->insertItem(i,QLocale().toString(ophaalronde)); // ,"ddd d MMM yyyy"
        ophalingenMap.insert(i,ophaalronde);
        i++;
        qDebug() << i << ":" << ophaalronde.toString();
    }

    qDebug() << "totaal:" << i;
    /// -> if i == 1 => geen ophaalrondes gevonden! => messagebox, no execute of this dialogbox!!
    /// => geef de functie initialise() een return waarde int: -1 is error, 0 is niets gevonden, +1 is alles ok
///// initialise -- END

    layout->addWidget(ophalingenLabel);
    layout->addWidget(ophalingenComboBox);
    layout->addWidget(buttonBox);

    setLayout(layout);
    setMinimumWidth(500);
    setMinimumHeight(200);
    setWindowTitle(tr("Bevestigen opgehaalde hoeveelheden"));
    show();

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
    vvimDebug() << "choice has been made" << ophalingenComboBox->currentIndex() << ophalingenComboBox->itemText(ophalingenComboBox->currentIndex()) << ophalingenMap[ophalingenComboBox->currentIndex()].toString();
    reject();
}
