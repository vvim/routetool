#include "levering.h"
#include <QDebug>
#include <QSqlQuery>
#include <QSqlError>
#include <math.h>
#include <QMessageBox>
#include <QFormLayout>
#include <QVBoxLayout>

#define vvimDebug()\
    qDebug() << "[" << Q_FUNC_INFO << "]"

Levering::Levering(QWidget *parent) :
    QWidget(parent)
{
    completer = NULL;
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
    weightSpinBox->setMaximum(99999);
    volumeSpinBox->setMaximum(99999);
    timeNeededSpinBox->setMaximum(99999);

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
    formlayout->addRow(tr("Aantal minuten nodig om te leveren:"), timeNeededSpinBox);

    QVBoxLayout *mainlayout = new QVBoxLayout();
    //mainlayout->addLayout(toplayout); -> let's leave this out for a future version, shall we?
    mainlayout->addLayout(formlayout);
    mainlayout->addWidget(buttonBox);
    setLayout(mainlayout);
    setMinimumWidth(600);

    resetValues();
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
    countryEdit->setText("België");
    weightSpinBox->setValue(0);
    volumeSpinBox->setValue(0);
    timeNeededSpinBox->setValue(30); // by default 30 minutes for a delivery
    contactPersonEdit->setText("");
    telephoneEdit->setText("");
}

Levering::~Levering()
{
    vvimDebug() << "start to deconstruct Levering()";

    delete info;
    delete locationLabel;
    delete locationEdit;
    delete toonOphaalpunt;
    if(completer)
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

    vvimDebug() << "Levering() deconstructed";
}

void Levering::accept()
{
    SLevering levering(nameEdit->text(), streetEdit->text(), houseNrEdit->text(), busNrEdit->text(), postalCodeEdit->text(), plaatsEdit->text(), countryEdit->text(), contactPersonEdit->text(), telephoneEdit->text(), weightSpinBox->value(), volumeSpinBox->value(), timeNeededSpinBox->value());
    emit levering_for_route(levering);
    this->close();
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

void Levering::loadOphaalpunten() // is deze functie wel ergens voor nodig?
{
    vvimDebug() << "database has been changed, so we should reload the Completer";
    // autocompletion for locationEdit:
    // telkens aanroepen na aanmaken / wijzigen van een ophaalpunt?
    QStringList words; // "don't come easy, to me, la la la laaa la la"

    vvimDebug() << "empty QMap 'ophaalpunten'";
    ophaalpunten.clear();

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


    if(completer)
        delete completer;

    completer = new MyCompleter(words, this);
    completer->setCaseSensitivity(Qt::CaseInsensitive);

    locationEdit->setCompleter(completer);
    vvimDebug() << "done, completer (re)loaded.";
}

void Levering::toonOphaalpuntInformatie()
{
    info->setWindowTitle("info over ophaalpunt");
    info->showOphaalpunt(ophaalpunten[locationEdit->text()]);
}

void Levering::vulInformatieOphaalpuntIn()
{
    vvimDebug() << "gebruik SQL met id erin, id:" << ophaalpunten[locationEdit->text()];
}
