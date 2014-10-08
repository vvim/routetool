#include <QDebug>
#include <QSqlQuery>
#include <QSqlError>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QApplication>
#include <QHeaderView>
#include <QMessageBox>
#include <QDate>
#include "kiesophaalpunten.h"

#define vvimDebug()\
    qDebug() << "[" << Q_FUNC_INFO << "]"


/*
#define AANMELDING_DATE 0
#define OPHAALPUNT_NAAM 1
#define WEIGHT_KURK 2
#define ZAK_KURK 3
#define WEIGHT_KAARS 4
#define ZAK_KAARS 5
#define AANMELDING_ID 6
#define OPHAALPUNT_ID 7
#define STRAAT 8
#define HUISNR 9
#define BUSNR 10
#define POSTCODE 11
#define PLAATS 12
#define LAND 13
#define OPMERKINGEN 14
*/

KiesOphaalpunten::KiesOphaalpunten(QWidget *parent) :
    QWidget(parent)
{
    model = NULL;
    legeAanmeldingenModel = NULL;

    normal = new QPalette();
    normal->setColor(QPalette::Text,Qt::AutoColor);

    warning = new QPalette();
    warning->setColor(QPalette::Text,Qt::red);

    sortingascending = true;

    legeAanmeldingenLabel = new QLabel(tr("Aanmeldingen - klik op een kolom om te sorteren:"));

    legeAanmeldingenTreeView = new QTreeView();

    legeAanmeldingenTreeView->setRootIsDecorated(false);
    legeAanmeldingenTreeView->setAlternatingRowColors(true);
    legeAanmeldingenTreeView->setSortingEnabled(true);
    legeAanmeldingenTreeView->sortByColumn(1, Qt::AscendingOrder);


    totalWeightLabel = new QLabel(tr("Totaal gewicht:"));
    totalVolumeLabel = new QLabel(tr("Totaal volume:"));

    //mag ook spinbox zijn
    totalWeightEdit = new QLineEdit();
    totalVolumeEdit = new QLineEdit();

    totalWeightEdit->setEnabled(false);
    totalVolumeEdit->setEnabled(false);

    initialise();

    // setTotalWeightTotalVolume(); -> niet nodig, zit al in de "uncheckAll()" in "initialise()"

    // buttonroles: http://doc.qt.digia.com/qt-maemo/qdialogbuttonbox.html#ButtonRole-enum
    buttonBox = new QDialogButtonBox();
    buttonBox->addButton(tr("Wijzigingen opslaan"),
                 QDialogButtonBox::AcceptRole);
    buttonBox->addButton(tr("Niet opslaan"),
                         QDialogButtonBox::RejectRole);
    allButton = new QPushButton(tr("Allen"));
    buttonBox->addButton(allButton,QDialogButtonBox::ActionRole);
    resetButton = new QPushButton(tr("Geen"));
    buttonBox->addButton(resetButton,QDialogButtonBox::ActionRole); // NoRole ???
    deleteButton = new QPushButton(tr("Verwijder"));
    buttonBox->addButton(deleteButton,QDialogButtonBox::DestructiveRole); // NoRole ???

    connect(buttonBox, SIGNAL(accepted()),this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()),this, SLOT(reject()));
    connect(resetButton, SIGNAL(pressed()), this, SLOT(uncheckAll()));
    connect(allButton, SIGNAL(pressed()), this, SLOT(checkAll()));
    connect(deleteButton, SIGNAL(pressed()), this, SLOT(deleteSelected()));

    //why does this one not work???
    connect(legeAanmeldingenModel, SIGNAL(checkChanges()), this, SLOT(setTotalWeightTotalVolume()));
    /* --> now we need to use this one instead: */ connect(legeAanmeldingenTreeView, SIGNAL(clicked(QModelIndex)), this, SLOT(setTotalWeightTotalVolume()));

    QHBoxLayout *weightAndVolumeLayout = new QHBoxLayout();
    weightAndVolumeLayout->addWidget(totalWeightLabel);
    weightAndVolumeLayout->addWidget(totalWeightEdit);
    weightAndVolumeLayout->addWidget(totalVolumeLabel);
    weightAndVolumeLayout->addWidget(totalVolumeEdit);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(legeAanmeldingenLabel);
    layout->addWidget(legeAanmeldingenTreeView);
    layout->addLayout(weightAndVolumeLayout);
    layout->addWidget(buttonBox);
    setLayout(layout);
    setMinimumWidth(600);
    setWindowTitle("Kies aangemelde ophaalpunten");

    // do we have to destroy each QListWidgetItem in 'legeAanmeldingen' in de destructor??
}

void KiesOphaalpunten::checkAll()
{
    total_weight = 0;
    total_volume = 0;

    for(int i = 0; i < model->rowCount(); i++)
    {
        QStandardItem* item = model->itemFromIndex(model->index(i,OPHAALPUNT_NAAM));
        item->setCheckState(Qt::Checked);
        total_weight += getWeightOfRow(i);
        total_volume += getVolumeOfRow(i);
        vvimDebug() << "total weight:" << total_weight;
        vvimDebug() << "total volume:" << total_volume;
        vvimDebug() << "----";
    }

    setTotalWeightTotalVolume();
}

void KiesOphaalpunten::uncheckAll()
{
    for(int i = 0; i < model->rowCount(); i++)
    {
        QStandardItem* item = model->itemFromIndex(model->index(i,OPHAALPUNT_NAAM));
        item->setCheckState(Qt::Unchecked);
    }
    total_weight = 0;
    total_volume = 0;
    setTotalWeightTotalVolume();
}

void KiesOphaalpunten::setTotalWeightTotalVolume()
{
    //recalculate total weight and total volume: see BUG
          // <vvim> BUG: als een item geselecteerd wordt met SPATIE, dan wordt er niets bijgeteld?????";
            total_weight = 0;
            total_volume = 0;

            for(int i = 0; i < model->rowCount(); i++)
            {
                QStandardItem* item = model->itemFromIndex(model->index(i,OPHAALPUNT_NAAM));
                if(item->checkState() == Qt::Checked)
                {
                    total_weight += getWeightOfRow(i);
                    total_volume += getVolumeOfRow(i);
                }
            }
            ///////////////// dit zou niet nodig moeten zijn

    totalWeightEdit->setText(QString("%1 kg").arg(total_weight));
    totalVolumeEdit->setText(QString("%1 liter").arg(total_volume));

    if(total_weight > maximum_weight)
        totalWeightEdit->setPalette(*warning);
    else
        totalWeightEdit->setPalette(*normal);

    if(total_volume > maximum_volume)
        totalVolumeEdit->setPalette(*warning);
    else
        totalVolumeEdit->setPalette(*normal);
}

/**
   -> listwidgetitem met alle aanmeldingen die nog leeg zijn, samen met de kilo's/zakken er achter
   -> vakje met totaal gewicht / volume
   -> OK / Cancel
**/

void KiesOphaalpunten::populateLegeAanmeldingen()
{
    vvimDebug() << "ook datum van laatste contact en laatste ophaling toevoegen";

#ifndef QT_NO_CURSOR
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
#endif

    initModel();

    QSqlQuery query("SELECT ophaalpunten.naam, aanmelding.kg_kurk, aanmelding.kg_kaarsresten, aanmelding.zakken_kurk, aanmelding.zakken_kaarsresten,"
                          " aanmelding.id, ophaalpunten.id, aanmelding.opmerkingen,"
                          " ophaalpunten.straat, ophaalpunten.nr, ophaalpunten.bus, ophaalpunten.postcode, ophaalpunten.plaats, ophaalpunten.land, aanmelding.datum "
                   " FROM aanmelding, ophaalpunten"
                   " WHERE ophaalpunten.id = aanmelding.ophaalpunt AND aanmelding.ophaalronde_datum is NULL");

    if(query.exec())
    {
        while (query.next())
        {
            QString ophaalpunt_naam = query.value(0).toString();

            addToTreeModel(     ophaalpunt_naam,
                                query.value(1).toDouble(),  // kg_kurk
                                query.value(2).toDouble(),  // kg_kaars
                                query.value(3).toDouble(),  // zakken_kurk
                                query.value(4).toDouble(),  // zakken_kaars
                                query.value(5).toInt(),     // aanmelding_id
                                query.value(6).toInt(),     // ophaalpunt_id
                                query.value(7).toString(),  // opmerkingen (uit table aanmelding)
                                query.value(8).toString(),  // straat
                                query.value(9).toString(),  // huisnr
                                query.value(10).toString(), // busnr
                                query.value(11).toString(), // postcode
                                query.value(12).toString(), // plaats
                                query.value(13).toString(), // land
                                query.value(14).toDate()    // datum
                            );
        }
    }
    else
    {
        vvimDebug() << "FATAL:" << "Something went wrong, could not execute query: SELECT ophaalpunten.naam, aanmelding.kg_kurk, aanmelding.kg_kaarsresten, aanmelding.zakken_kurk, aanmelding.zakken_kaarsresten, CONCAT_WS(' ', ophaalpunten.straat, ophaalpunten.nr,  ophaalpunten.bus, ophaalpunten.postcode, ophaalpunten.plaats, ophaalpunten.land) AS ADRES, aanmelding.id, ophaalpunten.id from aanmelding, ophaalpunten where ophaalpunten.id = aanmelding.ophaalpunt AND aanmelding.ophaalronde_datum is NULL; error:" << query.lastError();
        qFatal(QString("Something went wrong, could not execute query: SELECT ophaalpunten.naam, aanmelding.kg_kurk, aanmelding.kg_kaarsresten, aanmelding.zakken_kurk, aanmelding.zakken_kaarsresten, CONCAT_WS(' ', ophaalpunten.straat, ophaalpunten.nr,  ophaalpunten.bus, ophaalpunten.postcode, ophaalpunten.plaats, ophaalpunten.land) AS ADRES, aanmelding.id, ophaalpunten.id from aanmelding, ophaalpunten where ophaalpunten.id = aanmelding.ophaalpunt AND aanmelding.ophaalronde_datum is NULL, error is: ").append(query.lastError().text()).toStdString().c_str());
    }

#ifndef QT_NO_CURSOR
    QApplication::restoreOverrideCursor();
#endif
}


void KiesOphaalpunten::accept()
{
    /**

      ALS ER MEER DAN één OPHAALPUNT MET DEZELFDE NAAM (en informatie???) STAAT EN ER WORDT ER EENTJE AANGEKLIKT
      WORDT AUTOMATISCH DE EERSTE GEKOZEN.

      KOMT DIT DOOR DEZELFDE NAAM???

    **/

    //recalculate total weight and total volume: see BUG
          // <vvim> BUG: als een item geselecteerd wordt met SPATIE, dan wordt er niets bijgeteld?????";
          setTotalWeightTotalVolume();

    if((total_weight > maximum_weight) || (total_volume > maximum_volume))
    {
        // if overweight or overvolume: request feedback from user

        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, tr("Overgewicht of overvolume!"),
                      tr("De voorgestelde ophaalpunten geven overgewicht of overvolume: %1kg (maximum %2kg) en %3 liter (maximum %4 liter).\n\nWeet u zeker dat u deze ophaalpunten aan de route wilt toevoegen?").arg(total_weight).arg(maximum_weight).arg(total_volume).arg(maximum_volume),
                                QMessageBox::Yes|QMessageBox::No);

        if(reply == QMessageBox::No)
            return;
    }

    // could be done better by using a new member of KiesOphaalpunten
    //      QMap <int, SOphaalpunt *> m_map_aanmeldingen
    // we fill m_map_aanmeldingen in 'populateLegeAanmeldingen' with the id number of the aanmeldingen and the information in SOphaalpunt
    // therefore we do not need all the SetData() in the QListWidgetItems, only the id for the QMap.
    // We can let this id be the same as the id in the table 'aanmelding'.
    //
    // Then this code will be simply Qt::Checked() => listOfAanmeldingen->append(m_map_aanmeldingen[aanmelding->data(AANMELDING_ID).toInt()];
    QList<SOphaalpunt> *listOfAanmeldingen = new QList<SOphaalpunt>();

    for(int i = 0; i < model->rowCount(); i++)
    {
        QStandardItem* item = model->itemFromIndex(model->index(i,OPHAALPUNT_NAAM));
        if(item->checkState() == Qt::Checked)
        {

            SOphaalpunt _ophaalpunt(
                            model->itemFromIndex(model->index(i,OPHAALPUNT_NAAM))->data(Qt::DisplayRole).toString(),                      //_naam
                            model->itemFromIndex(model->index(i,STRAAT))->data(Qt::DisplayRole).toString(),                               //_street
                            model->itemFromIndex(model->index(i,HUISNR))->data(Qt::DisplayRole).toString(),                               //_housenr
                            model->itemFromIndex(model->index(i,BUSNR))->data(Qt::DisplayRole).toString(),                                //_busnr
                            model->itemFromIndex(model->index(i,POSTCODE))->data(Qt::DisplayRole).toString(),                             //_postalcode
                            model->itemFromIndex(model->index(i,PLAATS))->data(Qt::DisplayRole).toString(),                               //_plaats
                            model->itemFromIndex(model->index(i,LAND))->data(Qt::DisplayRole).toString(),                                 //_country
                            model->itemFromIndex(model->index(i,WEIGHT_KURK))->data(Qt::DisplayRole).toDouble(),    //_kg_kurk
                            model->itemFromIndex(model->index(i,WEIGHT_KAARS))->data(Qt::DisplayRole).toDouble(),    //_kg_kaarsresten
                            model->itemFromIndex(model->index(i,ZAK_KURK))->data(Qt::DisplayRole).toDouble(),                  //_zakken_kurk
                            model->itemFromIndex(model->index(i,ZAK_KAARS))->data(Qt::DisplayRole).toDouble(),                 //_zakken_kaarsresten
                            model->itemFromIndex(model->index(i,AANMELDING_ID))->data(Qt::DisplayRole).toInt(),                //_aanmelding_id
                            model->itemFromIndex(model->index(i,OPHAALPUNT_ID))->data(Qt::DisplayRole).toInt(),                //_ophaalpunt_id
                            model->itemFromIndex(model->index(i,OPMERKINGEN))->data(Qt::DisplayRole).toString()                           //_opmerkingen
                        );
            listOfAanmeldingen->append(_ophaalpunt);

            _ophaalpunt.PrintInformation();
        }
    }
    emit aanmelding_for_route(listOfAanmeldingen);

    // vvimDebug() << "<vvim> TODO add data to the database so that we can reconstruct 'ophaalrondes'.";
    // -> is done when the TransportationList is compiled! , see function TransportationListWriter::print()
    this->close();
}

void KiesOphaalpunten::reject()
{
    this->close();
}

KiesOphaalpunten::~KiesOphaalpunten()
{
    vvimDebug() << "start to deconstruct KiesOphaalpunten()";
    delete warning;
    delete normal;
    delete legeAanmeldingenLabel;
    delete totalWeightLabel;
    delete totalVolumeLabel;
    delete totalWeightEdit;
    delete totalVolumeEdit;
    delete resetButton;
    delete allButton;
    delete deleteButton;
    delete buttonBox;
    delete model;
    delete legeAanmeldingenModel;
    delete legeAanmeldingenTreeView;
    vvimDebug() << "KiesOphaalpunten() deconstructed";
}

void KiesOphaalpunten::initialise()
{
    maximum_weight = settings.value("max_gewicht_vrachtwagen").toDouble();
    maximum_volume = settings.value("max_volume_vrachtwagen").toDouble();

    populateLegeAanmeldingen();

    uncheckAll();
}

void KiesOphaalpunten::addToTreeModel(QString NaamOphaalpunt, double WeightKurk, double WeightKaars,
                                   double ZakKurk, double ZakKaars, int AanmeldingId, int OphaalpuntId, QString Opmerkingen,
                                   QString Straat, QString HuisNr, QString BusNr, QString Postcode, QString Plaats, QString Land,
                                   QDate Aanmeldingsdatum)
{
    model->insertRow(0);
    //model->setData(model->index(0, OPHAALPUNT_NAAM), NaamOphaalpunt);
    model->setData(model->index(0, WEIGHT_KURK), WeightKurk);
    model->setData(model->index(0, ZAK_KURK), ZakKurk);
    model->setData(model->index(0, WEIGHT_KAARS), WeightKaars);
    model->setData(model->index(0, ZAK_KAARS), ZakKaars);
    model->setData(model->index(0, AANMELDING_ID), AanmeldingId);
    model->setData(model->index(0, OPHAALPUNT_ID), OphaalpuntId);
    model->setData(model->index(0, OPMERKINGEN), Opmerkingen);
    model->setData(model->index(0, STRAAT), Straat);
    model->setData(model->index(0, HUISNR), HuisNr);
    model->setData(model->index(0, BUSNR), BusNr);
    model->setData(model->index(0, POSTCODE), Postcode);
    model->setData(model->index(0, PLAATS), Plaats);
    model->setData(model->index(0, LAND), Land);
    model->setData(model->index(0, AANMELDING_DATE), Aanmeldingsdatum);

    // to make the row 'checkable'
    QStandardItem* item0 = new QStandardItem(true);
    item0->setCheckable(true);
    item0->setCheckState(Qt::Unchecked);
    item0->setText(NaamOphaalpunt);
    model->setItem(0, OPHAALPUNT_NAAM, item0);
}

double KiesOphaalpunten::getWeightOfRow(const int row)
{
    double weight_kaars = model->itemFromIndex(model->index(row,WEIGHT_KAARS))->data(Qt::DisplayRole).toDouble();
    double weight_kurk = model->itemFromIndex(model->index(row,WEIGHT_KURK))->data(Qt::DisplayRole).toDouble();

    vvimDebug() << "[KiesOphaalpunten::getWeightOfItem]" << "kaars:" << weight_kaars  << "kurk:" << weight_kurk;
    return weight_kaars + weight_kurk;
}


double KiesOphaalpunten::getVolumeOfRow(const int row)
{
    double volume_kaars = model->itemFromIndex(model->index(row,ZAK_KAARS))->data(Qt::DisplayRole).toDouble() * settings.value("zak_kaarsresten_volume").toDouble();
    double volume_kurk = model->itemFromIndex(model->index(row,ZAK_KURK))->data(Qt::DisplayRole).toDouble()* settings.value("zak_kurk_volume").toDouble();

    vvimDebug() << "[KiesOphaalpunten::getVolumeOfItem]" << "kaars:" << volume_kaars  << "kurk:" << volume_kurk;
    return volume_kaars + volume_kurk;
}

void KiesOphaalpunten::initModel()
{
    vvimDebug() << "[KiesOphaalpunten::initModel]" << "start";
    if(model)
        delete model;
    if(legeAanmeldingenModel)
        delete legeAanmeldingenModel;

    QStringList labels;
    labels << "Ophaalpunt" << "Aanmeldingsdatum" << "Kurk (kg)" << "Kurk (zakken)" << "Kaars (kg)" << "Kaars (zakken)"
           << "Aanmelding_id" << "Ophaalpunt_id" << "Straat" << "Nr" << "Bus" << "Postcode" << "Plaats" << "Land" << "Opmerkingen";

    model = new QStandardItemModel(0, labels.count());

    legeAanmeldingenModel = new MySortFilterProxyModel(this);
    legeAanmeldingenModel->setDynamicSortFilter(true);
    legeAanmeldingenModel->setSourceModel(model);

    for(int i = 0; i < labels.count(); i++)
    {
       model->setHeaderData(i,Qt::Horizontal, /* QObject::tr( */ labels[i] /*)*/ );  // why does 'tr()' not work? -> QString& instead of QString...
    }

    legeAanmeldingenTreeView->setModel(legeAanmeldingenModel);

    legeAanmeldingenTreeView->hideColumn(AANMELDING_ID);
    legeAanmeldingenTreeView->hideColumn(OPHAALPUNT_ID);

    legeAanmeldingenTreeView->hideColumn(STRAAT);
    legeAanmeldingenTreeView->hideColumn(HUISNR);
    legeAanmeldingenTreeView->hideColumn(BUSNR);
    legeAanmeldingenTreeView->setEditTriggers(QAbstractItemView::NoEditTriggers); // thanks to http://www.qtcentre.org/threads/22511-QTreeWidget-read-only

}


void KiesOphaalpunten::deleteSelected()
{
    int nr_of_checked_items = 0;
    QStringList checked_items_ids;
    for(int i = 0; i < model->rowCount(); i++)
    {
        QStandardItem* item = model->itemFromIndex(model->index(i,OPHAALPUNT_NAAM));
        if(item->checkState() == Qt::Checked)
        {
            nr_of_checked_items++;
            checked_items_ids << model->itemFromIndex(model->index(i,AANMELDING_ID))->data(Qt::DisplayRole).toString();
            qDebug() << "..." << checked_items_ids;

        }
    }

    if(nr_of_checked_items < 1)
    {
        vvimDebug() << "No checked aanmeldingen to remove: nr_of_checked_items =" << nr_of_checked_items << ", checked_items_ids" << checked_items_ids;
        return;
    }

    QString question = tr("Ben je zeker dat je deze aanmelding wilt verwijderen?");
    if (nr_of_checked_items > 1)
        question = QString(tr("Ben je zeker dat je deze %1 aanmeldingen wilt verwijderen?").arg(nr_of_checked_items));

    vvimDebug() << "Asking permission before removing aanmeldingen:" << question;

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, tr("Verwijder geselecteerde aanmeldingen"),
                  question, QMessageBox::Yes|QMessageBox::No);

    if(reply == QMessageBox::No)
    {
        vvimDebug() << "user declined";
        return;
    }

    QString prepare_query = QString("DELETE FROM aanmelding WHERE id = ").append(checked_items_ids[0]);

    for(int i = 1; i < checked_items_ids.count(); i++)
        prepare_query.append(QString(" OR id = %1").arg(checked_items_ids[i]));

    QSqlQuery query(prepare_query);

    if(!query.exec())
    {
        QMessageBox::critical(this,tr("Verwijderen van aanmeldingen niet gelukt"),
                            query.lastError().text().append(tr("\n\nHerstel de fout en probeer opnieuw.")), QMessageBox::Cancel);
        qFatal(QString("Something went wrong, could not execute %1, error is: %2").arg(prepare_query).arg(query.lastError().text()).toStdString().c_str());
        return;
    }

    vvimDebug() << prepare_query << "DONE";

    initialise();
}
