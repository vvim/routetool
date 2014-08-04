#include "transportationlistwriter.h"
#include <QFormLayout>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QPushButton>


TransportationListWriter::TransportationListWriter(QWidget *parent) :
    QWidget(parent)
{
    nameTransportationListEdit = new QLineEdit();
    startTimeEdit = new QTimeEdit();
    expectedArrivalTimeEdit = new QTimeEdit();
    expectedArrivalTimeEdit->setDisabled(true);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                                    | QDialogButtonBox::Cancel);
    QPushButton* resetButton = new QPushButton(tr("Reset"));
    buttonBox->addButton(resetButton,QDialogButtonBox::ResetRole);

    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(resetButton, SIGNAL(pressed()), this, SLOT(setOriginalValues()));

    QFormLayout *formlayout = new QFormLayout();
    formlayout->addRow(tr("Bestandsnaam:"),nameTransportationListEdit);
    formlayout->addRow(tr("Start om:"),startTimeEdit);
    formlayout->addRow(tr("Terug om:"),expectedArrivalTimeEdit);
    formlayout->addRow(buttonBox);

//    QVBoxLayout *layout = new QVBoxLayout();
//    layout->addLayout(formlayout);

    setLayout(formlayout);
}

TransportationListWriter::~TransportationListWriter()
{
    qDebug() << "start to deconstruct TransportationListWriter()";
    delete nameTransportationListEdit;
    delete startTimeEdit;
    delete expectedArrivalTimeEdit;
 //   delete buttonBox; -> is this taken care of by ~QWidget?
 //   delete resetButton; -> is this taken care of by ~QWidget?
    qDebug() << "TransportationListWriter() deconstructed";
}
