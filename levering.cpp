#include "levering.h"
#include <QDebug>
#include <QSqlQuery>
#include <QSqlError>
#include <math.h>
#include <QMessageBox>

Levering::Levering(QWidget *parent) :
    QWidget(parent)
{
    info = new InfoOphaalpunt();
    locationLabel = new QLabel(tr("Ophaalpunt:")); //wordt een keuzelijst uit de databank!
    locationEdit = new MyLineEdit(); //wordt een keuzelijst uit de databank!
    toonOphaalpunt  = new QPushButton(tr("Toon info"));
    toonOphaalpunt->setEnabled(false);

    connect(toonOphaalpunt, SIGNAL(clicked()), this, SLOT(toonOphaalpuntInformatie()));
    connect(info, SIGNAL(infoChanged()), this, SLOT(loadOphaalpunten()));

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                          | QDialogButtonBox::Cancel);

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    locationLabel->setBuddy(locationEdit);

    loadOphaalpunten();

    QGridLayout *layout = new QGridLayout();

        // ROW,  COLUMN,  rowspan columnspan
    layout->addWidget(locationLabel,0,0,1,1);
    layout->addWidget(locationEdit,0,1,1,5);
    layout->addWidget(toonOphaalpunt,0,6,1,1);

    setLayout(layout);
}

void Levering::resetValues()
{

}

Levering::~Levering()
{
    qDebug() << "start to deconstruct Levering()";
    qDebug() << "NIET VERGETEN!!! delete pointers!!";
    /*
    delete info;
    delete locationLabel;
    delete locationEdit;
    delete toonOphaalpunt;
    delete completer;
    delete buttonBox;
    delete this->layout();
    */
    qDebug() << "Levering() deconstructed";
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

void Levering::toonOphaalpuntInformatie()
{
    info->setWindowTitle("info over ophaalpunt");
    info->showOphaalpunt(ophaalpunten[locationEdit->text()]);
}
