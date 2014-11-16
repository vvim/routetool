#include "exportcollectionhistory.h"
#include <QtGui>
#include <QSqlQuery>

#define vvimDebug()\
    qDebug() << "[" << Q_FUNC_INFO << "]"

ExportCollectionHistory::ExportCollectionHistory(QWidget *parent) :
    QWidget(parent)
{
    timeperiodLabel = new QLabel(tr("Geef de start- en einddatum in van de periode die je wil exporteren:"));
    timeperiod_startLabel = new QLabel(tr("Van:"));
    timeperiod_endLabel = new QLabel(tr("Tot:"));
    timeperiod_startEdit = new QDateEdit(QDate::currentDate().addYears(-1));
    timeperiod_startEdit->setDisplayFormat("dd MMM yyyy");
    timeperiod_startEdit->setLocale(QLocale::Dutch);
    timeperiod_startEdit->setCalendarPopup(true);  //zie http://stackoverflow.com/questions/7031962/qdateedit-calendar-popup
    timeperiod_endEdit = new QDateEdit(QDate::currentDate());
    timeperiod_endEdit->setDisplayFormat("dd MMM yyyy");
    timeperiod_endEdit->setLocale(QLocale::Dutch);
    timeperiod_endEdit->setCalendarPopup(true);  //zie http://stackoverflow.com/questions/7031962/qdateedit-calendar-popup

    completer = NULL;
    ophaalpuntLabel = new QLabel(tr("Geef het ophaalpunt waar je de historiek van wilt zien,\nof laat leeg als je de historiek van ALLE ophaalpunten wilt zien voor de opgegeven periode:"));
    ophaalpuntEdit = new MyLineEdit(); //wordt een keuzelijst uit de databank!
    loadOphaalpunten();

    spaceritem_beforeOphaalpunt = new QSpacerItem(this->width(),20);
    spaceritem_beforeButtonBox = new QSpacerItem(this->width(),40);

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    //QFileDialog* exportToFile = new QFileDialog()

    connect(buttonBox, SIGNAL(accepted()),this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()),this, SLOT(reject()));

    QGridLayout *gridLayout = new QGridLayout();
    int row_ui = 0;
    gridLayout->addWidget(timeperiodLabel,row_ui,0,1,6);
    row_ui++;
    gridLayout->addWidget(timeperiod_startLabel,row_ui,1,1,1);
    gridLayout->addWidget(timeperiod_startEdit,row_ui,2,1,1);
    gridLayout->addWidget(timeperiod_endLabel,row_ui,3,1,1);
    gridLayout->addWidget(timeperiod_endEdit,row_ui,4,1,1);
    row_ui++;
    gridLayout->addItem(spaceritem_beforeOphaalpunt,row_ui,0,1,6);
    row_ui++;
    gridLayout->addWidget(ophaalpuntLabel,row_ui,0,1,6);
    row_ui++;
    gridLayout->addWidget(ophaalpuntEdit,row_ui,1,1,5);
    row_ui++;
    gridLayout->addItem(spaceritem_beforeButtonBox,row_ui,0,1,6);

    QVBoxLayout *verticalLayout = new QVBoxLayout();
    verticalLayout->addLayout(gridLayout);
    verticalLayout->addWidget(buttonBox);

    setWindowTitle(tr("Exporteer ophalinghistoriek"));
    setLayout(verticalLayout);
    setMinimumWidth(500);
    setMinimumHeight(200);
}

ExportCollectionHistory::~ExportCollectionHistory()
{
    vvimDebug() << "start to deconstruct ExportCollectionHistory()";
    delete timeperiodLabel;
    delete ophaalpuntLabel;
    delete ophaalpuntEdit;
    delete timeperiod_startLabel;
    delete timeperiod_startEdit;
    delete timeperiod_endLabel;
    delete timeperiod_endEdit;
    delete buttonBox;
    if(completer)
    {
        vvimDebug() << "completer _not_ NULL, therefore it has been initialised and we must delete it";
        delete completer;
    }
    else
        vvimDebug() << "completer == NULL, therefore it has NOT been initialised and we should not delete it";

    //delete exportToFile
    vvimDebug() << "ExportCollectionHistory() deconstructed";
}

void ExportCollectionHistory::loadOphaalpunten()
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


void ExportCollectionHistory::accept()
{
    int ophaalpunt_id = ophaalpunten[ophaalpuntEdit->text()];

    vvimDebug() << "all info:";
    vvimDebug() << "... starting date:" << timeperiod_startEdit->date().toString();
    vvimDebug() << "... ending date:" << timeperiod_endEdit->date().toString();
    vvimDebug() << "... location:" << ophaalpuntEdit->text();
    vvimDebug() << "... location ID:" << ophaalpunt_id;

    vvimDebug() << "test if start >= end";
    if(timeperiod_startEdit->date() <= timeperiod_endEdit->date())
        vvimDebug() << "correct period, start is before ending";
    else
        vvimDebug() << "INVALID timeperiod, start is NOT before ending";

    vvimDebug() << "export accepted";
    if(ophaalpunt_id < 1)
    {
        vvimDebug() << "Show history of ALL locations within timeperiod";
    }
    else
        vvimDebug() << "Show history of location" << ophaalpuntEdit->text();

    vvimDebug() << "show QFileDialog";
    // misschien moet dat helemaal geen pointer zijn??? testen?
    reject();
}

void ExportCollectionHistory::reject()
{
    vvimDebug() << "closing";
    close();
    vvimDebug() << "deleting";
    delete this;
}
