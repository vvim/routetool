#include "ophaalhistoriekdialog.h"
#include "ui_ophaalhistoriekdialog.h"
#include <QListWidgetItem>
#include <QDebug>
#include <QDate>
#include <QSqlQuery>
#include <QSqlError>
#include "globalfunctions.h"

OphaalHistoriekDialog::OphaalHistoriekDialog(int ophaalpunt_id, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OphaalHistoriekDialog)
{
    vvimDebug() << "showing OphaalHistoriekDialog for ophaalpunt_id" << ophaalpunt_id;

    ui->setupUi(this);
    QStringList labels;
    labels << "Historiek Id" << "Ophalingsdatum" << "Chauffeur" << "Ophaalpunt"
           << "Kurk (kg)" << "Kaars (kg)" << "Kurk (zakken)" << "Kaars (zakken)" << "Opmerkingen";



    model = new QStandardItemModel(0, labels.count());
    model->setHorizontalHeaderLabels(labels);

    QSqlQuery query;
    // id 	timestamp 	ophalingsdatum 	chauffeur 	ophaalpunt 	zakken_kurk 	kg_kurk 	zakken_kaarsresten 	kg_kaarsresten 	opmerkingen
    query.prepare("SELECT * "
                  " FROM ophalinghistoriek"
                  " WHERE ophaalpunt = :ophaal "
                  " ORDER BY ophalingsdatum DESC;");
    query.bindValue(":ophaal",ophaalpunt_id);

    // does the table OPHALINGHISTORIEK have an 'id' as well? -> use this in the ListWidget, or is the date "ophalingsdatum" enough to now which 'historiek' has been selected?
    if(!query.exec())
    {
        vvimDebug() << "Could not query ophaalpunt" << ophaalpunt_id << "trying to reconnect to DB" << query.lastError();
        QString SQLquery = QString("SELECT * FROM ophalinghistoriek WHERE ophaalpunt = %1 ORDER BY ophalingsdatum DESC;").arg(ophaalpunt_id);
        if(!reConnectToDatabase(query.lastError(), SQLquery, QString("[%1]").arg(Q_FUNC_INFO)))
        {
            vvimDebug() << "Reconnection failed";
        }
        else
        {
            vvimDebug() << "Reconnection succesful, will try query again.";
            QSqlQuery query2;
            query2.prepare("SELECT * "
                          " FROM ophalinghistoriek"
                          " WHERE ophaalpunt = :ophaal "
                          " ORDER BY ophalingsdatum DESC;");
            query = query2;
            query.bindValue(":ophaal",ophaalpunt_id);

            if(!query.exec())
            {
                vvimDebug() << "query failed after reconnecting to DB, halting" << SQLquery << query.lastError();
                exit(-1);
            }
        }
    }


    while(query.next())
    {
        int historiek_id = query.value(0).toInt();
        QDate ophalingsdatum = query.value(2).toDate();
        QString chauffeur = query.value(3).toString();
        int ophaalpunt_id = query.value(4).toInt();
        double zakken_kurk = query.value(5).toDouble();
        double kg_kurk = query.value(6).toDouble();
        double zakken_kaars = query.value(7).toDouble();
        double kg_kaars = query.value(8).toDouble();
        QString opmerkingen = query.value(9).toString();

        addToTreeModel(historiek_id, ophalingsdatum, chauffeur, ophaalpunt_id, kg_kurk, kg_kaars, zakken_kurk, zakken_kaars, opmerkingen);
    }

    bool emptymodel = false;
    if(model->rowCount() == 0)
    {
        //nothing in historiek? Tell user "empty!"
        emptymodel = true;
        model->insertRow(0);
        model->setData(model->index(0, HIST_OPMERKINGEN), tr("Geen ophaalhistoriek voor dit ophaalpunt"));
    }

    ophalingHistoriekModel = new OphaalHistoriekDialogSortFilterProxyModel(this);
    ophalingHistoriekModel->setDynamicSortFilter(true);
    ophalingHistoriekModel->setSourceModel(model);

    ui->historiekTreeView->setModel(ophalingHistoriekModel);
    ui->historiekTreeView->setRootIsDecorated(false);
    ui->historiekTreeView->setAlternatingRowColors(true);
    ui->historiekTreeView->setColumnHidden(HIST_HISTORIEK_ID,true);
    ui->historiekTreeView->setColumnHidden(HIST_OPHAALPUNT_ID,true);

    if(emptymodel)
    {
        //nothing in historiek? Hide all irrelevant columns
        ui->historiekTreeView->setColumnHidden(HIST_OPHALINGSDATUM,true);
        ui->historiekTreeView->setColumnHidden(HIST_CHAUFFEUR,true);
        ui->historiekTreeView->setColumnHidden(HIST_WEIGHT_KURK,true);
        ui->historiekTreeView->setColumnHidden(HIST_WEIGHT_KAARS,true);
        ui->historiekTreeView->setColumnHidden(HIST_ZAK_KURK,true);
        ui->historiekTreeView->setColumnHidden(HIST_ZAK_KAARS,true);
    }

    ui->historiekTreeView->resizeColumnToContents(HIST_ZAK_KAARS);
    ui->historiekTreeView->resizeColumnToContents(HIST_ZAK_KURK);
    ui->historiekTreeView->resizeColumnToContents(HIST_WEIGHT_KAARS);
    ui->historiekTreeView->resizeColumnToContents(HIST_WEIGHT_KURK);
    ui->historiekTreeView->resizeColumnToContents(HIST_OPMERKINGEN);

    ui->historiekTreeView->setSortingEnabled(true);
    ui->historiekTreeView->sortByColumn(1, Qt::AscendingOrder);

    ui->historiekTreeView->setEditTriggers(QAbstractItemView::NoEditTriggers); // thanks to http://www.qtcentre.org/threads/22511-QTreeWidget-read-only
}

OphaalHistoriekDialog::~OphaalHistoriekDialog()
{
    vvimDebug() << "start to deconstruct OphaalHistoriekDialog()";
    delete model;
    delete ophalingHistoriekModel;
    delete ui;
    vvimDebug() << "OphaalHistoriekDialog() deconstructed";
}

void OphaalHistoriekDialog::on_buttonBox_accepted()
{
    this->close();
}

void OphaalHistoriekDialog::on_buttonBox_rejected()
{
    this->close();
}

void OphaalHistoriekDialog::addToTreeModel(int _historiek_id, QDate _ophalingsdatum, QString _chauffeur, int _ophaalpunt_id, double _kg_kurk, double _kg_kaarsresten, double _zakken_kurk, double _zakken_kaarsresten, QString _opmerkingen)
{
    model->insertRow(0);
    //model->setData(model->index(0, OPHAALPUNT_NAAM), NaamOphaalpunt);
    model->setData(model->index(0, HIST_HISTORIEK_ID), _historiek_id);
    model->setData(model->index(0, HIST_OPHALINGSDATUM), _ophalingsdatum);
    model->setData(model->index(0, HIST_CHAUFFEUR), _chauffeur);
    model->setData(model->index(0, HIST_OPHAALPUNT_ID), _ophaalpunt_id);
    model->setData(model->index(0, HIST_WEIGHT_KURK), _kg_kurk);
    model->setData(model->index(0, HIST_WEIGHT_KAARS), _kg_kaarsresten);
    model->setData(model->index(0, HIST_ZAK_KURK), _zakken_kurk);
    model->setData(model->index(0, HIST_ZAK_KAARS), _zakken_kaarsresten);
    model->setData(model->index(0, HIST_OPMERKINGEN), _opmerkingen);
}
