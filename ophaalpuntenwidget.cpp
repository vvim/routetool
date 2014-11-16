#include <QDebug>
#include <QSqlQuery>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include "ophaalpuntenwidget.h"

#define vvimDebug()\
    qDebug() << "[" << Q_FUNC_INFO << "]"

OphaalpuntenWidget::OphaalpuntenWidget(QWidget *parent) :
    QWidget(parent)
{
    completer = NULL;
    info = new InfoOphaalpunt();
    info->showAanmeldingAndHistoriekButton(true);
    info->setWindowTitle(tr("info over ophaalpunt"));

    nieuweaanmeldingWidget = new NieuweAanmelding();

    ophaalpuntLabel = new QLabel(tr("Ophaalpunt:")); //wordt een keuzelijst uit de databank!
    ophaalpuntEdit = new MyLineEdit(); //wordt een keuzelijst uit de databank!
    toonOphaalpunt  = new QPushButton(tr("Toon info"));
    toonOphaalpunt->setEnabled(false);

    connect(toonOphaalpunt, SIGNAL(clicked()), this, SLOT(toonOphaalpuntInformatie()));
    connect(ophaalpuntEdit, SIGNAL(textChanged(QString)), this, SLOT(ophaalpuntTextChanged()));
    connect(info,SIGNAL(nieuweAanmelding(int)),nieuweaanmeldingWidget,SLOT(aanmeldingVoorOphaalpunt(int)));
    connect(info, SIGNAL(infoChanged()), this, SLOT(databaseBeenUpdated()));

    QHBoxLayout *layout = new QHBoxLayout();
    layout->addWidget(ophaalpuntLabel);
    layout->addWidget(ophaalpuntEdit);
    layout->addWidget(toonOphaalpunt);

    /* in case we want to add a buttonBox or so:
    QVBoxLayout *mainlayout = new QVBoxLayout();
    mainlayout->addLayout(layout);
    */

    setLayout(layout);

    ophaalpuntEdit->setFocus(); // always setFocus() as last action: http://stackoverflow.com/questions/526761/set-qlineedit-focus-in-qt

    setWindowTitle(tr("Bekijk informatie bestaande ophaalpunten"));
    setMinimumSize(600, 300);
}

OphaalpuntenWidget::~OphaalpuntenWidget()
{
    vvimDebug() << "start to deconstruct OphaalpuntenWidget()";
    delete ophaalpuntLabel;
    delete ophaalpuntEdit;
    delete info;
    delete nieuweaanmeldingWidget;
    delete toonOphaalpunt;
    if(completer)
    {
        vvimDebug() << "completer _not_ NULL";
        delete completer;
    }
    else
        vvimDebug() << "completer == NULL";
    vvimDebug() << "OphaalpuntenWidget() deconstructed";
}

void OphaalpuntenWidget::loadOphaalpunten()
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

    QSqlQuery query("SELECT naam, straat, nr, bus, postcode, plaats, land, id FROM ophaalpunten");
    while (query.next()) {
        QString naam	= query.value(0).toString();
        QString straat	= query.value(1).toString();
        QString nr	    = query.value(2).toString();
        QString bus	    = query.value(3).toString();
        QString postcode	= query.value(4).toString();
        QString plaats	= query.value(5).toString();
        QString land	= query.value(6).toString();

        int id = query.value(7).toInt();

        QString ophaalpunt = naam;
        ophaalpunt.append(QString(", %1 %2, %3 %4, %5").arg(straat).arg(nr).arg(postcode).arg(plaats).arg(land));

        ophaalpunten[ophaalpunt] = id;

        words << ophaalpunt;
    }

#ifndef QT_NO_CURSOR
    QApplication::restoreOverrideCursor();
#endif

    if(completer)
        delete completer;

    completer = new MyCompleter(words, this);
    completer->setCaseSensitivity(Qt::CaseInsensitive);

    ophaalpuntEdit->setCompleter(completer);
    vvimDebug() << "done, completer (re)loaded.";
}

void OphaalpuntenWidget::toonOphaalpuntInformatie()
{
    info->setWindowTitle("info over ophaalpunt");
    info->showOphaalpunt(ophaalpunten[ophaalpuntEdit->text()]);
}

void OphaalpuntenWidget::ophaalpuntTextChanged()
{
    if (ophaalpunten[ophaalpuntEdit->text()] > 0)
    {
        toonOphaalpunt->setEnabled(true);
    }
    else
        toonOphaalpunt->setEnabled(false);

}

void OphaalpuntenWidget::initialise()
{
    loadOphaalpunten();
    ophaalpuntEdit->setText("");
}

void OphaalpuntenWidget::databaseBeenUpdated()
{
    emit contentsOfDatabaseChanged();
}
