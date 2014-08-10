#include <QDebug>
#include <QSqlQuery>
#include <QSqlError>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QApplication>
#include <QHeaderView>
#include <QMessageBox>
#include "kiesophaalpunten.h"

/** userroles to store data from QListWidgetItem

    see http://qt-project.org/doc/qt-4.8/qt.html#ItemDataRole-enum and
    +---------------+-------+--------------------------------------------------------------------+
    | Constant      | Value | Description                                                        |
    | Qt::UserRole  | 32    | The first role that can be used for application-specific purposes. |
    +---------------+-------+--------------------------------------------------------------------+

    <vvim> : might not be used as this, probably have to use an Item-class that inherits QListWidgetItem
**/

#define OPHAALPUNT_NAAM 0
#define WEIGHT_KURK 1
#define ZAK_KURK 2
#define WEIGHT_KAARS 3
#define ZAK_KAARS 4
#define AANMELDING_ID 5
#define OPHAALPUNT_ID 6
#define OPMERKINGEN 7
#define STRAAT 8
#define HUISNR 9
#define BUSNR 10
#define POSTCODE 11
#define PLAATS 12
#define LAND 13

KiesOphaalpunten::KiesOphaalpunten(QWidget *parent) :
    QWidget(parent)
{
    normal = new QPalette();
    normal->setColor(QPalette::Text,Qt::AutoColor);

    warning = new QPalette();
    warning->setColor(QPalette::Text,Qt::red);

    sortingascending = true;

    legeAanmeldingenLabel = new QLabel(tr("Aanmeldingen:"));

    legeAanmeldingenTree = new QTreeWidget();
    legeAanmeldingenTree->setColumnCount(14);

    QStringList labels;
    labels << "Ophaalpunt" << "Kurk (kg)" << "Kurk (zakken)" << "Kaars (kg)" << "Kaars (zakken)"
           << "Aanmelding_id" << "Ophaalpunt_id" << "Opmerkingen"
           << "Straat" << "Nr" << "Bus" << "Postcode" << "Plaats" << "Land";
    legeAanmeldingenTree->setHeaderLabels(labels);

    legeAanmeldingenTree->setColumnHidden(AANMELDING_ID,true);
    legeAanmeldingenTree->setColumnHidden(OPHAALPUNT_ID,true);

    legeAanmeldingenTree->setColumnHidden(STRAAT,true);
    legeAanmeldingenTree->setColumnHidden(HUISNR,true);
    legeAanmeldingenTree->setColumnHidden(BUSNR,true);

    connect(legeAanmeldingenTree->header(), SIGNAL(sectionClicked(int)), this, SLOT(sortTreeWidget(int)));
    connect(legeAanmeldingenTree, SIGNAL(clicked(QModelIndex)), this, SLOT(setTotalWeightTotalVolume()));

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
    buttonBox = new QDialogButtonBox;
    buttonBox->addButton(tr("Wijzigingen opslaan"),
                 QDialogButtonBox::AcceptRole);
    buttonBox->addButton(tr("Niet opslaan"),
                         QDialogButtonBox::RejectRole);
    allButton = new QPushButton(tr("Allen"));
    buttonBox->addButton(allButton,QDialogButtonBox::ActionRole);
    resetButton = new QPushButton(tr("Geen"));
    buttonBox->addButton(resetButton,QDialogButtonBox::ActionRole); // NoRole ???

    connect(buttonBox, SIGNAL(accepted()),this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()),this, SLOT(reject()));
    connect(resetButton, SIGNAL(pressed()), this, SLOT(uncheckAll()));
    connect(allButton, SIGNAL(pressed()), this, SLOT(checkAll()));

    QHBoxLayout *weightAndVolumeLayout = new QHBoxLayout();
    weightAndVolumeLayout->addWidget(totalWeightLabel);
    weightAndVolumeLayout->addWidget(totalWeightEdit);
    weightAndVolumeLayout->addWidget(totalVolumeLabel);
    weightAndVolumeLayout->addWidget(totalVolumeEdit);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(legeAanmeldingenLabel);
    layout->addWidget(legeAanmeldingenTree);
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
    QTreeWidgetItemIterator it(legeAanmeldingenTree);
    while(*it)
    {
        QTreeWidgetItem * aanmelding = *it;
        aanmelding->setCheckState(0,Qt::Checked);
        total_weight += getWeightOfItem(aanmelding);
        total_volume += (aanmelding->text(ZAK_KURK).toDouble() * settings.value("zak_kurk_volume").toDouble()) + (aanmelding->text(ZAK_KAARS).toDouble() * settings.value("zak_kaarsresten_volume").toDouble());

        it++;
    }

    setTotalWeightTotalVolume();
}

void KiesOphaalpunten::uncheckAll()
{
    QTreeWidgetItemIterator it(legeAanmeldingenTree);
    while(*it)
    {
        QTreeWidgetItem * aanmelding = *it;
        aanmelding->setCheckState(0,Qt::Unchecked);
        it++;
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

            QTreeWidgetItemIterator it(legeAanmeldingenTree);
            while(*it)
            {
                QTreeWidgetItem * aanmelding = *it;
                if(aanmelding->checkState(0) == Qt::Checked)
                {
                    total_weight += getWeightOfItem(aanmelding);
                    total_volume += (aanmelding->text(ZAK_KURK).toDouble() * settings.value("zak_kurk_volume").toDouble()) + (aanmelding->text(ZAK_KAARS).toDouble() * settings.value("zak_kaarsresten_volume").toDouble());
                }
                it++;
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
    qDebug() << "[KiesOphaalpunten::populateLegeAanmeldingen()]" << "ook datum van laatste contact en laatste ophaling toevoegen";

#ifndef QT_NO_CURSOR
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
#endif

    legeAanmeldingenTree->clear();

    QSqlQuery query("SELECT ophaalpunten.naam, aanmelding.kg_kurk, aanmelding.kg_kaarsresten, aanmelding.zakken_kurk, aanmelding.zakken_kaarsresten,"
                          " aanmelding.id, ophaalpunten.id, aanmelding.opmerkingen,"
                          " ophaalpunten.straat, ophaalpunten.nr, ophaalpunten.bus, ophaalpunten.postcode, ophaalpunten.plaats, ophaalpunten.land "
                   " FROM aanmelding, ophaalpunten"
                   " WHERE ophaalpunten.id = aanmelding.ophaalpunt AND aanmelding.ophaalronde_datum is NULL");

    if(query.exec())
    {
        while (query.next())
        {
            QString ophaalpunt_naam = query.value(0).toString();

            addToTreeWidget(    ophaalpunt_naam,
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
                                query.value(13).toString()  // land
                            );
        }
    }
    else
    {
        qDebug() << "FATAL:" << "Something went wrong, could not execute query: SELECT ophaalpunten.naam, aanmelding.kg_kurk, aanmelding.kg_kaarsresten, aanmelding.zakken_kurk, aanmelding.zakken_kaarsresten, CONCAT_WS(' ', ophaalpunten.straat, ophaalpunten.nr,  ophaalpunten.bus, ophaalpunten.postcode, ophaalpunten.plaats, ophaalpunten.land) AS ADRES, aanmelding.id, ophaalpunten.id from aanmelding, ophaalpunten where ophaalpunten.id = aanmelding.ophaalpunt AND aanmelding.ophaalronde_datum is NULL; error:" << query.lastError();
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
    QTreeWidgetItemIterator it(legeAanmeldingenTree);
    while(*it)
    {
        QTreeWidgetItem * aanmelding = *it;
        if(aanmelding->checkState(0) == Qt::Checked)
        {

            SOphaalpunt _ophaalpunt(
                            aanmelding->text(OPHAALPUNT_NAAM),                      //_naam
                            aanmelding->text(STRAAT),                               //_street
                            aanmelding->text(HUISNR),                               //_housenr
                            aanmelding->text(BUSNR),                                //_busnr
                            aanmelding->text(POSTCODE),                             //_postalcode
                            aanmelding->text(PLAATS),                               //_plaats
                            aanmelding->text(LAND),                                 //_country
                            weightColumnToDouble(aanmelding->text(WEIGHT_KURK)),    //_kg_kurk
                            weightColumnToDouble(aanmelding->text(WEIGHT_KAARS)),   //_kg_kaarsresten
                            aanmelding->text(ZAK_KURK).toDouble(),                  //_zakken_kurk
                            aanmelding->text(ZAK_KAARS).toDouble(),                 //_zakken_kaarsresten
                            aanmelding->text(AANMELDING_ID).toInt(),                //_aanmelding_id
                            aanmelding->text(OPHAALPUNT_ID).toInt(),                //_ophaalpunt_id
                            aanmelding->text(OPMERKINGEN)                           //_opmerkingen
                        );
            listOfAanmeldingen->append(_ophaalpunt);
        }
        it++;

    }
    emit aanmelding_for_route(listOfAanmeldingen);

    qDebug() << "<vvim> TODO add data to the database so that we can reconstruct 'ophaalrondes'.";
    this->close();
}

void KiesOphaalpunten::reject()
{
    this->close();
}

KiesOphaalpunten::~KiesOphaalpunten()
{
    qDebug() << "start to deconstruct KiesOphaalpunten()";
    delete warning;
    delete normal;
    delete legeAanmeldingenTree;
    delete legeAanmeldingenLabel;
    delete totalWeightLabel;
    delete totalVolumeLabel;
    delete totalWeightEdit;
    delete totalVolumeEdit;
    delete resetButton;
    delete allButton;
    delete buttonBox;
    qDebug() << "KiesOphaalpunten() deconstructed";
}

void KiesOphaalpunten::initialise()
{
    maximum_weight = settings.value("max_gewicht_vrachtwagen").toDouble();
    maximum_volume = settings.value("max_volume_vrachtwagen").toDouble();

    populateLegeAanmeldingen();

    uncheckAll();
}

void KiesOphaalpunten::addToTreeWidget(QString NaamOphaalpunt, double WeightKurk, double WeightKaars,
                                   double ZakKurk, double ZakKaars, int AanmeldingId, int OphaalpuntId, QString Opmerkingen,
                                   QString Straat, QString HuisNr, QString BusNr, QString Postcode, QString Plaats, QString Land)
{
    QTreeWidgetItem *item = new QTreeWidgetItem(legeAanmeldingenTree);
    item->setText(OPHAALPUNT_NAAM, NaamOphaalpunt);
    item->setText(WEIGHT_KURK, QString("%1 kg").arg(WeightKurk));
    item->setText(ZAK_KURK, QString::number(ZakKurk));
    item->setText(WEIGHT_KAARS, QString("%1 kg").arg(WeightKaars));
    item->setText(ZAK_KAARS, QString::number(ZakKaars));
    item->setText(AANMELDING_ID, QString::number(AanmeldingId));
    item->setText(OPHAALPUNT_ID, QString::number(OphaalpuntId));
    item->setText(OPMERKINGEN, Opmerkingen);
    item->setText(STRAAT, Straat);
    item->setText(HUISNR, HuisNr);
    item->setText(BUSNR, BusNr);
    item->setText(POSTCODE, Postcode);
    item->setText(PLAATS, Plaats);
    item->setText(LAND, Land);

    item->setFlags(item->flags()|Qt::ItemIsUserCheckable);
    item->setCheckState(0,Qt::Unchecked);
    //legeAanmeldingenTree->addTopLevelItem(item);
}

void KiesOphaalpunten::sortTreeWidget(int column)
{
    qDebug() << "<vvim>" << "for use of filters in KiesOphaalpunten::QTreeWidgetItem, think legeAanmeldingenTree->itemBelow(headeritem)->isHidden();";
    qDebug() << "<vvim>" << "[KiesOphaalpunten::sortTreeWidget]" << "goes wrong for sorting numbers, see" << "http://stackoverflow.com/questions/363200/is-it-possible-to-sort-numbers-in-a-qtreewidget-column";
    // goes wrong for sorting numbers, see
    // http://stackoverflow.com/questions/363200/is-it-possible-to-sort-numbers-in-a-qtreewidget-column
    if(sortingascending)
        legeAanmeldingenTree->sortByColumn(column,Qt::AscendingOrder);
    else
        legeAanmeldingenTree->sortByColumn(column,Qt::DescendingOrder);
    sortingascending = !sortingascending;
}

double KiesOphaalpunten::weightColumnToDouble(QString kg)
{
    // turn "220 kg" in "220"
    QString temp = kg;
    temp.chop(3);
    return temp.toDouble();
}

double KiesOphaalpunten::getWeightOfItem(QTreeWidgetItem* item)
{
    double weight_kaars = weightColumnToDouble(item->text(WEIGHT_KAARS));
    double weight_kurk = weightColumnToDouble(item->text(WEIGHT_KURK));

    return weight_kaars + weight_kurk;
}
