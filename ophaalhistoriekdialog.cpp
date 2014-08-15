#include "ophaalhistoriekdialog.h"
#include "ui_ophaalhistoriekdialog.h"
#include <QListWidgetItem>
#include <QDebug>
#include <QDate>
#include <QSqlQuery>
#include <QSqlError>

OphaalHistoriekDialog::OphaalHistoriekDialog(int ophaalpunt_id, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OphaalHistoriekDialog)
{
    ui->setupUi(this);
    qDebug() << "showing OphaalHistoriekDialog for ophaalpunt_id" << ophaalpunt_id;

    QSqlQuery query;
    // id 	timestamp 	ophalingsdatum 	chauffeur 	ophaalpunt 	zakken_kurk 	kg_kurk 	zakken_kaarsresten 	kg_kaarsresten 	opmerkingen
    query.prepare("SELECT * "
                  " FROM ophalinghistoriek"
                  " WHERE ophaalpunt = :ophaal "
                  " ORDER BY ophalingsdatum DESC;");
    query.bindValue(":ophaal",ophaalpunt_id);

    // does the table OPHALINGHISTORIEK has an 'id' as well? -> use this in the ListWidget, or is the date "ophalingsdatum" enough to now which 'historiek' has been selected?

    if(!query.exec())
        qDebug() << "[OphaalHistoriekDialog]" << "Something went wrong with querying ophaalpunt" << ophaalpunt_id << ":" << query.lastError();

    while(query.next())
    {
        QListWidgetItem* item = new QListWidgetItem();
        item->setText(QLocale().toString(query.value(2).toDate(), "d MMM yyyy")); // DB "ophalingsdatum"
        //item->setData(OPHAAL_HIST_DETAILS,);
        ui->historiekListWidget->addItem(item);
    }

    if(ui->historiekListWidget->count() == 0)
    {
        //nothing in historiek? Tell user "empty!"
        QListWidgetItem* item = new QListWidgetItem();
        item->setText(tr("Geen ophaalhistoriek voor dit ophaalpunt"));
        ui->historiekListWidget->addItem(item);
    }

    //connect( DOUBLECLICK, showDetails(ophalingsdatum) );
}

OphaalHistoriekDialog::~OphaalHistoriekDialog()
{
    qDebug() << "start to deconstruct OphaalHistoriekDialog()";
    delete ui;
    qDebug() << "OphaalHistoriekDialog() deconstructed";
}

void OphaalHistoriekDialog::on_buttonBox_accepted()
{
    this->close();
}

void OphaalHistoriekDialog::on_buttonBox_rejected()
{
    this->close();
}
