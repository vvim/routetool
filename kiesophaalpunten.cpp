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

#define OPHAALPUNT 0
#define WEIGHT_KURK 1
#define ZAK_KURK 2
#define WEIGHT_KAARS 3
#define ZAK_KAARS 4
#define POSTCODE 5
#define AANMELDING_ID 6
#define OPHAALPUNT_ID 7
#define OPMERKINGEN 8
#define VOLLEDIGADRES 9

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
    legeAanmeldingenTree->setColumnCount(10);

    QStringList labels;
    labels << "Ophaalpunt" << "Kurk (kg)" << "Kurk (zakken)" << "Kaars (kg)" << "Kaars (zakken)"
           << "Postcode"   << "Aanmelding_id" << "Ophaalpunt_id" << "Opmerkingen" << "Volledig adres";
    legeAanmeldingenTree->setHeaderLabels(labels);

    /*
        #define OPHAALPUNT Qt::UserRole         0
        #define WEIGHT_KURK OPHAALPUNT+1        1
        #define ZAK_KURK WEIGHT_KURK+1          2
        #define WEIGHT_KAARS ZAK_KURK+1         3
        #define ZAK_KAARS WEIGHT_KAARS+1        4
        #define POSTCODE                        5
        #define AANMELDING_ID ADRES+1           6
        #define OPHAALPUNT_ID AANMELDING_ID+1   7
        #define OPMERKINGEN OPHAALPUNT_ID+1     8
        #define VOLLEDIGADRES                   9
    */
    legeAanmeldingenTree->setColumnHidden(6,true);  // aanmelding_id
    legeAanmeldingenTree->setColumnHidden(7,true);  // ophaalpunt_id
    legeAanmeldingenTree->setColumnHidden(9,true);  // volledig adres

    connect(legeAanmeldingenTree->header(), SIGNAL(sectionDoubleClicked(int)), this, SLOT(sortTreeWidget(int)));

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
        total_weight += aanmelding->text(WEIGHT_KAARS).toDouble() + aanmelding->text(WEIGHT_KURK).toDouble();
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
                    total_weight += aanmelding->text(WEIGHT_KAARS).toDouble() + aanmelding->text(WEIGHT_KURK).toDouble();
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
    // TODO neem dit uit de databank : mysql> select * from aanmelding where ophaalronde_datum is NULL;

#ifndef QT_NO_CURSOR
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
#endif

    legeAanmeldingenTree->clear();

    QSqlQuery query("SELECT ophaalpunten.naam, aanmelding.kg_kurk, aanmelding.kg_kaarsresten, aanmelding.zakken_kurk, aanmelding.zakken_kaarsresten,"
                          " CONCAT_WS(' ', ophaalpunten.straat, ophaalpunten.nr,  ophaalpunten.bus, ophaalpunten.postcode, ophaalpunten.plaats, ophaalpunten.land) AS ADRES,"
                          " aanmelding.id, ophaalpunten.id, aanmelding.opmerkingen, ophaalpunten.postcode "
                   " FROM aanmelding, ophaalpunten"
                   " WHERE ophaalpunten.id = aanmelding.ophaalpunt AND aanmelding.ophaalronde_datum is NULL");

    if(query.exec())
    {
        while (query.next())
        {
            QString ophaalpunt = query.value(0).toString();
            QString ophaalpunt_adres = query.value(5).toString();

            addToTreeWidget(ophaalpunt, query.value(1).toDouble(), query.value(2).toDouble(),
                            query.value(3).toDouble(), query.value(4).toDouble(), query.value(9).toString(),
                            query.value(6).toInt(), query.value(7).toInt(), query.value(8).toString() ,ophaalpunt_adres);
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
                            aanmelding->text(OPHAALPUNT),   // naam
                            aanmelding->text(WEIGHT_KURK).toDouble(),  // kg_kurk
                            aanmelding->text(WEIGHT_KAARS).toDouble(), // kg_kaarsresten
                            aanmelding->text(ZAK_KURK).toDouble(),     // zakken_kurk
                            aanmelding->text(ZAK_KAARS).toDouble(),    // zakken_kaarsresten
                            aanmelding->text(VOLLEDIGADRES),        // adres
                            aanmelding->text(AANMELDING_ID).toInt(),   // aanmelding_id
                            aanmelding->text(OPHAALPUNT_ID).toInt(),   // ophaalpunt_id
                            aanmelding->text(OPMERKINGEN)   // speciale_opmerkingen
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
                                   double ZakKurk, double ZakKaars, QString postcode,
                                   int AanmeldingId, int OphaalpuntId, QString Opmerkingen,
                                   QString VolledigAdres)
{
    QTreeWidgetItem *item = new QTreeWidgetItem(legeAanmeldingenTree);
    item->setText(0, NaamOphaalpunt);
    item->setText(1, QString("%1 kg").arg(WeightKurk));
    item->setText(2, QString::number(ZakKurk));
    item->setText(3, QString("%1 kg").arg(WeightKaars));
    item->setText(4, QString::number(ZakKaars));
    item->setText(5, postcode);
    item->setText(6, QString::number(AanmeldingId));
    item->setText(7, QString::number(OphaalpuntId));
    item->setText(8, Opmerkingen);
    item->setText(9, VolledigAdres);

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
