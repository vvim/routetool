#include "levering.h"
#include <QDebug>
#include <QSqlQuery>
#include <QSqlError>
#include <math.h>
#include <QMessageBox>
#include <QFormLayout>
#include <QVBoxLayout>

Levering::Levering(QWidget *parent) :
    QWidget(parent)
{
    info = new InfoOphaalpunt();
    locationLabel = new QLabel(tr("Bekende locatie:")); //wordt een keuzelijst uit de databank!
    locationEdit = new MyLineEdit(); //wordt een keuzelijst uit de databank!
    toonOphaalpunt  = new QPushButton(tr("Toon info"));
    vulOphaalpuntIn = new QPushButton(tr("Vul in"));
    toonOphaalpunt->setEnabled(false);
    vulOphaalpuntIn->setEnabled(false);

    nameEdit = new QLineEdit();
    streetEdit = new QLineEdit();
    houseNrEdit = new QLineEdit();
    busNrEdit = new QLineEdit();
    postalCodeEdit = new QLineEdit();
    plaatsEdit = new QLineEdit();
    countryEdit = new QLineEdit();
    contactPersonEdit = new QLineEdit();
    telephoneEdit = new QLineEdit();
    weightSpinBox = new QSpinBox();
    volumeSpinBox = new QSpinBox();
    timeNeededSpinBox = new QSpinBox();
    weightSpinBox->setMaximum(9999);
    volumeSpinBox->setMaximum(9999);
    timeNeededSpinBox->setMaximum(9999);

    connect(toonOphaalpunt, SIGNAL(clicked()), this, SLOT(toonOphaalpuntInformatie()));
    connect(vulOphaalpuntIn, SIGNAL(clicked()), this, SLOT(vulInformatieOphaalpuntIn()));
    connect(info, SIGNAL(infoChanged()), this, SLOT(loadOphaalpunten()));
    connect(locationEdit, SIGNAL(editingFinished()), this, SLOT(ophaalpuntChanged()));


    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                          | QDialogButtonBox::Cancel);

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    locationLabel->setBuddy(locationEdit);

    loadOphaalpunten();

    QGridLayout *toplayout = new QGridLayout();
        // ROW,  COLUMN,  rowspan columnspan
    toplayout->addWidget(locationLabel,0,0,1,1);
    toplayout->addWidget(locationEdit,0,1,1,5);
    toplayout->addWidget(vulOphaalpuntIn,0,6,1,1);
    toplayout->addWidget(toonOphaalpunt,0,7,1,1);

    QFormLayout *formlayout = new QFormLayout();
    formlayout->addRow(tr("Naam locatie (eventueel):"), nameEdit);
    // line: leveringsadres
    formlayout->addRow(tr("Straat:"), streetEdit);
    formlayout->addRow(tr("Huisnummer:"), houseNrEdit);
    formlayout->addRow(tr("Bus:"), busNrEdit);
    formlayout->addRow(tr("Postcode:"), postalCodeEdit);
    formlayout->addRow(tr("Plaats:"), plaatsEdit);
    formlayout->addRow(tr("Land:"), countryEdit);
    // line: contactpersoon
    formlayout->addRow(tr("Contactpersoon:"), contactPersonEdit);
    formlayout->addRow(tr("Telefoonnummer:"), telephoneEdit);
    // line: lading
    formlayout->addRow(tr("Gewicht lading (kg):"), weightSpinBox);
    formlayout->addRow(tr("Volume lading (liter):"), volumeSpinBox);
    formlayout->addRow(tr("Aantal minuten die nodig voor de levering:"), timeNeededSpinBox);

    QVBoxLayout *mainlayout = new QVBoxLayout();
    mainlayout->addLayout(toplayout);
    mainlayout->addLayout(formlayout);
    mainlayout->addWidget(buttonBox);
    setLayout(mainlayout);
    setMinimumWidth(600);
}

void Levering::resetValues()
{
    locationEdit->setText("");
    toonOphaalpunt->setText("");
    vulOphaalpuntIn->setText("");
    nameEdit->setText("");
    streetEdit->setText("");
    houseNrEdit->setText("");
    busNrEdit->setText("");
    postalCodeEdit->setText("");
    plaatsEdit->setText("");
    countryEdit->setText("");
    weightSpinBox->setValue(0);
    volumeSpinBox->setValue(0);
    timeNeededSpinBox->setValue(0);
    contactPersonEdit->setText("");
    telephoneEdit->setText("");
}

Levering::~Levering()
{
    qDebug() << "start to deconstruct Levering()";
    qDebug() << "NIET VERGETEN!!! delete pointers!!";

    delete info;
    delete locationLabel;
    delete locationEdit;
    delete toonOphaalpunt;
    delete completer;
    delete buttonBox;
    delete vulOphaalpuntIn;
    delete nameEdit;
    delete streetEdit;
    delete houseNrEdit;
    delete busNrEdit;
    delete postalCodeEdit;
    delete plaatsEdit;
    delete countryEdit;
    delete weightSpinBox;
    delete volumeSpinBox;
    delete timeNeededSpinBox;
    delete contactPersonEdit;
    delete telephoneEdit;

    // I guess QWidget takes care of this?
    delete this->layout();

    qDebug() << "Levering() deconstructed";
}

void Levering::accept()
{
    qDebug() << "gebruik een nieuwe struct SLevering met daar alle informatie in om via connect door te geven, zie void KiesOphaalpunten::accept()";

    // some sweet action
    // this->close();
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
        vulOphaalpuntIn->setEnabled(true);
    }
    else
    {
        toonOphaalpunt->setEnabled(false);
        vulOphaalpuntIn->setEnabled(false);
    }

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

void Levering::toonOphaalpuntInformatie()
{
    info->setWindowTitle("info over ophaalpunt");
    info->showOphaalpunt(ophaalpunten[locationEdit->text()]);
}

void Levering::vulInformatieOphaalpuntIn()
{
    qDebug() << "gebruik SQL met id erin, id:" << ophaalpunten[locationEdit->text()];
}
