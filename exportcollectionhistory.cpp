#include "exportcollectionhistory.h"
#include <QtGui>

#define vvimDebug()\
    qDebug() << "[" << Q_FUNC_INFO << "]"

ExportCollectionHistory::ExportCollectionHistory(QWidget *parent) :
    QWidget(parent)
{
    timeperiodLabel = new QLabel(tr("Geef de start- en einddatum in van de periode die je wil exporteren:"));
    ophaalpuntLabel = new QLabel(tr("Ophaalpunt:"));
    ophaalpuntEdit = new QLineEdit();
    timeperiod_startLabel = new QLabel(tr("Van:"));
    timeperiod_endLabel = new QLabel(tr("Tot:"));
    timeperiod_startEdit = new QDateEdit();
    timeperiod_startEdit->setDisplayFormat("dd MMM yyyy");
    timeperiod_startEdit->setLocale(QLocale::Dutch);
    timeperiod_startEdit->setCalendarPopup(true);  //zie http://stackoverflow.com/questions/7031962/qdateedit-calendar-popup
    timeperiod_endEdit = new QDateEdit();
    timeperiod_endEdit->setDisplayFormat("dd MMM yyyy");
    timeperiod_endEdit->setLocale(QLocale::Dutch);
    timeperiod_endEdit->setCalendarPopup(true);  //zie http://stackoverflow.com/questions/7031962/qdateedit-calendar-popup
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    //QFileDialog* exportToFile = new QFileDialog()

    connect(buttonBox, SIGNAL(accepted()),this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()),this, SLOT(reject()));

    QGridLayout *gridLayout = new QGridLayout();
    int row = 0;
    gridLayout->addWidget(timeperiod_startLabel,row,0,1,1);
    gridLayout->addWidget(timeperiod_startEdit,row,1,1,1);
    gridLayout->addWidget(timeperiod_endLabel,row,2,1,1);
    gridLayout->addWidget(timeperiod_endEdit,row,3,1,1);
    row++;
    gridLayout->addWidget(ophaalpuntLabel,row,0,1,1);
    gridLayout->addWidget(ophaalpuntEdit,row,1,1,3); // 3 kolommen breed

    QVBoxLayout *verticalLayout = new QVBoxLayout();
    verticalLayout->addWidget(timeperiodLabel);
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
    //delete exportToFile
    vvimDebug() << "ExportCollectionHistory() deconstructed";
}

void ExportCollectionHistory::accept()
{
    vvimDebug() << "test if start >= end";
    vvimDebug() << "export accepted";
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
