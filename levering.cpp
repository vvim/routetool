#include "levering.h"
#include <QDebug>
#include <QSqlQuery>
#include <QSqlError>
#include <math.h>
#include <QMessageBox>

Levering::Levering(QWidget *parent) :
    QWidget(parent)
{
    //setupModel();

    /**
        * ophaalpunt dat zich heeft aangemled (kan je uit een keuzelijst kiezen)
        * datum melding
        * naam contactpersoon
        * # zakken kurk    | echte # zakken  (aparte knop "melding ingeven" , "ophaalronde bevestigen"
        * # kg kurk        | echte # kg
        * # zakken kaars   | echte # zakken
        * # kg kaars       | echte # kg
        * eventueel opmerkingen
    **/

    locationLabel = new QLabel(tr("Ophaalpunt:")); //wordt een keuzelijst uit de databank!
    locationEdit = new MyLineEdit(); //wordt een keuzelijst uit de databank!
    toonOphaalpunt  = new QPushButton(tr("Toon info"));
    toonOphaalpunt->setEnabled(false);
    nieuwOphaalpunt = new QPushButton(tr("&Nieuw ophaalpunt aanmaken"));
    nieuwOphaalpunt->setEnabled(true);

    nameLabel = new QLabel(tr("Contactpersoon:"));
    nameEdit = new QLineEdit();
    dateLabel = new QLabel(tr("Datum aanmelding:"));
    dateEdit = new QDateEdit( QDate::currentDate(), this ); // zie http://doc.qt.digia.com/3.3/qdateedit.html
    dateEdit->setDisplayFormat("dd MMM yyyy");
    dateEdit->setLocale(QLocale::Dutch);
    dateEdit->setCalendarPopup(true);  //zie http://stackoverflow.com/questions/7031962/qdateedit-calendar-popup

    zakkenkurkLabel = new QLabel(tr("Zakken kurk:"));
    zakkenkurkSpinBox = new QSpinBox();
    kgkurkLabel = new QLabel(tr("Kg kurk:"));
    kgkurkSpinBox = new QSpinBox();
    kgkurkSpinBox->setMaximum(9999);

    zakkenkaarsenLabel = new QLabel(tr("Zakken kaarsen:"));
    zakkenkaarsenSpinBox = new QSpinBox();
    kgkaarsenLabel = new QLabel(tr("Kg kaarsen:"));
    kgkaarsenSpinBox = new QSpinBox();
    kgkaarsenSpinBox->setMaximum(9999);

    opmerkingenLabel = new QLabel(tr("Opmerkingen:"));
    opmerkingenEdit = new QTextEdit();

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                          | QDialogButtonBox::Cancel);

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    // zie mapper zoals custom widget mapper om ID te linken ??
    connect(locationEdit, SIGNAL(editingFinished()), this, SLOT(ophaalpuntChanged()));

    locationLabel->setBuddy(locationEdit);

    loadOphaalpunten();


    nameLabel->setBuddy(nameEdit);
    dateLabel->setBuddy(dateEdit);

    zakkenkurkLabel->setBuddy(zakkenkurkSpinBox);
    kgkurkLabel->setBuddy(kgkurkSpinBox);

    zakkenkaarsenLabel->setBuddy(zakkenkaarsenSpinBox);
    kgkaarsenLabel->setBuddy(kgkaarsenSpinBox);

    opmerkingenLabel->setBuddy(opmerkingenEdit);

    QGridLayout *layout = new QGridLayout();

        // ROW,  COLUMN,  rowspan columnspan
    layout->addWidget(locationLabel,0,0,1,1);
    layout->addWidget(locationEdit,0,1,1,5);
    layout->addWidget(toonOphaalpunt,0,6,1,1);
    layout->addWidget(nameLabel,1,0,1,1);
    layout->addWidget(nameEdit,1,1,1,5);
    layout->addWidget(dateLabel,2,0,1,1);
    layout->addWidget(dateEdit,2,1,1,2);

    layout->addWidget(zakkenkurkLabel,3,0,1,1);
    layout->addWidget(zakkenkurkSpinBox,3,1,1,1);
    layout->addWidget(kgkurkLabel,3,2,1,1);
    layout->addWidget(kgkurkSpinBox,3,3,1,1);

    layout->addWidget(zakkenkaarsenLabel,4,0,1,1);
    layout->addWidget(zakkenkaarsenSpinBox,4,1,1,1);
    layout->addWidget(kgkaarsenLabel,4,2,1,1);
    layout->addWidget(kgkaarsenSpinBox,4,3,1,1);

    layout->addWidget(opmerkingenLabel,5,0,1,1);
    layout->addWidget(opmerkingenEdit,5,1,3,6);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(nieuwOphaalpunt);
    mainLayout->addLayout(layout);
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);


    locationEdit->setFocus(); // always setFocus() as last action: http://stackoverflow.com/questions/526761/set-qlineedit-focus-in-qt

    setWindowTitle(tr("Nieuwe aanmelding op te halen hoeveelheid"));
    setMinimumWidth(600);

    /**
            mapper niet nodig voor "Aanmeldingen" , wel voor "bevestigen van ophaling",
            de ophaalpunten zijn dan een "dropdownbox" en de selectie daar bepaald
            welke "row" er getoond wordt




            naam van de ophaling
             -> adres laten zien
             -> telefoonnumer
             -> email
             -> ...

             Beter een knop aanmaken "geef fiche met ophaalpunt weer" (mogelijkheid om gegevens te wijzigen, nieuw ophalapunt aanmaken)
    **/
}

void Levering::resetValues()
{
    locationEdit->setText("");
    nameEdit->setText("");
    dateEdit->setDate(QDate::currentDate());

    zakkenkurkSpinBox->setValue(0);
    kgkurkSpinBox->setValue(0);

    zakkenkaarsenSpinBox->setValue(0);
    kgkaarsenSpinBox->setValue(0);

    opmerkingenEdit->setText("");

    locationEdit->setFocus(); // always setFocus() as last action: http://stackoverflow.com/questions/526761/set-qlineedit-focus-in-qt

    toonOphaalpunt->setEnabled(false);

    setMinimumWidth(600);
}

Levering::~Levering()
{
    qDebug() << "NIET VERGETEN!!! delete pointers!!";
}

void Levering::accept()
{
}

void Levering::reject()
{
    this->close();
}

void Levering::ophaalpuntChanged()
{
    if (ophaalpunten[locationEdit->text()] > 0)
    {
        toonOphaalpunt->setEnabled(true);
    }
    else
        toonOphaalpunt->setEnabled(false);

}

void Levering::loadOphaalpunten()
{
    // autocompletion for locationEdit:
    // telkens aanroepen na aanmaken / wijzigen van een ophaalpunt?
    QStringList words; // "don't come easy, to me, la la la laaa la la"

    #ifndef QT_NO_CURSOR
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    #endif

    QSqlQuery query("SELECT id, naam FROM ophaalpunten"); // adres ??
        while (query.next()) {
            int id = query.value(0).toInt();
            QString naam	= query.value(1).toString();
            words << naam;

            ophaalpunten[naam] = id;
            // adres[id_of_naam] = ADRES ??
        }

    #ifndef QT_NO_CURSOR
        QApplication::restoreOverrideCursor();
    #endif



    completer = new MyCompleter(words, this);
    completer->setCaseSensitivity(Qt::CaseInsensitive);

    locationEdit->setCompleter(completer);
}
