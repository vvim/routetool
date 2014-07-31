#include <QDebug>
#include <QSqlQuery>
#include <QSqlError>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QApplication>
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

#define OPHAALPUNT Qt::UserRole
#define WEIGHT_KURK OPHAALPUNT+1
#define ZAK_KURK WEIGHT_KURK+1
#define WEIGHT_KAARS ZAK_KURK+1
#define ZAK_KAARS WEIGHT_KAARS+1
#define ADRES ZAK_KAARS+1
#define AANMELDING_ID ADRES+1


KiesOphaalpunten::KiesOphaalpunten(QWidget *parent) :
    QWidget(parent)
{
    normal = new QPalette();
    normal->setColor(QPalette::Text,Qt::AutoColor);

    warning = new QPalette();
    warning->setColor(QPalette::Text,Qt::red);

    legeAanmeldingenLabel = new QLabel(tr("Aanmeldingen:"));
    legeAanmeldingenList = new QListWidget();

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
    connect(legeAanmeldingenList, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(itemSelected(QListWidgetItem*)));
    qDebug() << "<vvim> BUG: als een item geselecteerd wordt met SPATIE, dan wordt er niets bijgeteld?????";

    QHBoxLayout *weightAndVolumeLayout = new QHBoxLayout();
    weightAndVolumeLayout->addWidget(totalWeightLabel);
    weightAndVolumeLayout->addWidget(totalWeightEdit);
    weightAndVolumeLayout->addWidget(totalVolumeLabel);
    weightAndVolumeLayout->addWidget(totalVolumeEdit);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(legeAanmeldingenLabel);
    layout->addWidget(legeAanmeldingenList);
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
    for (int i = 0; i < legeAanmeldingenList->count(); i++)
    {
        QListWidgetItem * aanmelding = legeAanmeldingenList->item(i);
        aanmelding->setCheckState(Qt::Checked);
        total_weight += aanmelding->data(WEIGHT_KAARS).toDouble() + aanmelding->data(WEIGHT_KURK).toDouble();
        total_volume += (aanmelding->data(ZAK_KURK).toDouble() * settings.value("zak_kurk_volume").toDouble()) + (aanmelding->data(ZAK_KAARS).toDouble() * settings.value("zak_kaarsresten_volume").toDouble());
    }

    //foreach (QListWidgetItem *aanmelding, legeAanmeldingenList)
    //    will not work as QListWidget is not a _list_ in a container way, it's a simple _list_view_ .
    //    see http://www.qtcentre.org/threads/40430-how-can-iterate-foreach-item-in-QListWidget
    setTotalWeightTotalVolume();
}

void KiesOphaalpunten::uncheckAll()
{
    for (int i = 0; i < legeAanmeldingenList->count(); i++)
    {
        legeAanmeldingenList->item(i)->setCheckState(Qt::Unchecked);
    }
    total_weight = 0;
    total_volume = 0;
    setTotalWeightTotalVolume();
}

void KiesOphaalpunten::itemSelected(QListWidgetItem* aanmelding)
{
    if(aanmelding->checkState() == Qt::Unchecked)
    {
        aanmelding->setCheckState(Qt::Checked);
        total_weight += aanmelding->data(WEIGHT_KAARS).toDouble() + aanmelding->data(WEIGHT_KURK).toDouble();
        total_volume += (aanmelding->data(ZAK_KURK).toDouble() * settings.value("zak_kurk_volume").toDouble()) + (aanmelding->data(ZAK_KAARS).toDouble() * settings.value("zak_kaarsresten_volume").toDouble());
    }
    else
    {
        aanmelding->setCheckState(Qt::Unchecked);
        total_weight -= aanmelding->data(WEIGHT_KAARS).toDouble() + aanmelding->data(WEIGHT_KURK).toDouble();
        total_volume -= (aanmelding->data(ZAK_KURK).toDouble() * settings.value("zak_kurk_volume").toDouble()) + (aanmelding->data(ZAK_KAARS).toDouble() * settings.value("zak_kaarsresten_volume").toDouble());
    }

    setTotalWeightTotalVolume();
}


void KiesOphaalpunten::itemSelected()
{
    QListWidgetItem* aanmelding = legeAanmeldingenList->currentItem();
    itemSelected(aanmelding);
}

void KiesOphaalpunten::setTotalWeightTotalVolume()
{
    //recalculate total weight and total volume: see BUG
          // <vvim> BUG: als een item geselecteerd wordt met SPATIE, dan wordt er niets bijgeteld?????";
            total_weight = 0;
            total_volume = 0;
            for (int i = 0; i < legeAanmeldingenList->count(); i++)
            {
                QListWidgetItem * aanmelding = legeAanmeldingenList->item(i);
                if(aanmelding->checkState() == Qt::Checked)
                {
                    total_weight += aanmelding->data(WEIGHT_KAARS).toDouble() + aanmelding->data(WEIGHT_KURK).toDouble();
                    total_volume += (aanmelding->data(ZAK_KURK).toDouble() * settings.value("zak_kurk_volume").toDouble()) + (aanmelding->data(ZAK_KAARS).toDouble() * settings.value("zak_kaarsresten_volume").toDouble());
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
    // TODO neem dit uit de databank : mysql> select * from aanmelding where ophaalronde_nr is NULL;

#ifndef QT_NO_CURSOR
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
#endif

    legeAanmeldingenList->clear();
    legeAanmeldingenList->setSortingEnabled(true);

    QSqlQuery query("select ophaalpunten.naam, aanmelding.kg_kurk, aanmelding.kg_kaarsresten, aanmelding.zakken_kurk, aanmelding.zakken_kaarsresten, CONCAT_WS(' ', ophaalpunten.straat, ophaalpunten.nr,  ophaalpunten.bus, ophaalpunten.postcode, ophaalpunten.plaats, ophaalpunten.land) AS ADRES, aanmelding.id from aanmelding, ophaalpunten where ophaalpunten.id = aanmelding.ophaalpunt AND aanmelding.ophaalronde_nr is NULL");

    while (query.next())
    {
        QString ophaalpunt = query.value(0).toString();
        double weight = query.value(1).toDouble()+query.value(2).toDouble();
        double volume = (query.value(3).toDouble() * settings.value("zak_kurk_volume").toDouble()) +(query.value(4).toDouble() * settings.value("zak_kaarsresten_volume").toDouble());
        QString ophaalpunt_adres = query.value(5).toString();
        QListWidgetItem * item = new QListWidgetItem();
        item->setData(Qt::DisplayRole,QString("%1 (%2 kg , %3 liter)").arg(ophaalpunt).arg(weight).arg(volume));
        item->setData(OPHAALPUNT,ophaalpunt);
        item->setData(WEIGHT_KURK,query.value(1).toDouble());
        item->setData(WEIGHT_KAARS,query.value(2).toDouble());
        item->setData(ZAK_KURK,query.value(3).toDouble());
        item->setData(ZAK_KAARS,query.value(4).toDouble());
        item->setData(ADRES,ophaalpunt_adres);
        item->setData(AANMELDING_ID,query.value(6).toInt());
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setFlags(item->flags() &~ Qt::ItemIsSelectable);
        item->setCheckState(Qt::Unchecked); // http://www.qtcentre.org/threads/7032-QListWidget-with-check-box-s , thank you J-P Nurmi
        legeAanmeldingenList->addItem(item);
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

    qDebug() << "TODO: Insert into DB";
    QList<SOphaalpunt> *listOfAanmeldingen = new QList<SOphaalpunt>();
    for(int i = 0 ; i < legeAanmeldingenList->count(); i++)
    {
        // could be done better by using a new member of KiesOphaalpunten
        //      QMap <int, SOphaalpunt *> m_map_aanmeldingen
        // we fill m_map_aanmeldingen in 'populateLegeAanmeldingen' with the id number of the aanmeldingen and the information in SOphaalpunt
        // therefore we do not need all the SetData() in the QListWidgetItems, only the id for the QMap.
        // We can let this id be the same as the id in the table 'aanmelding'.
        //
        // Then this code will be simply Qt::Checked() => listOfAanmeldingen->append(m_map_aanmeldingen[aanmelding->data(AANMELDING_ID).toInt()];
        QListWidgetItem *aanmelding = legeAanmeldingenList->item(i);
        if(aanmelding->checkState() == Qt::Checked)
        {
            SOphaalpunt _ophaalpunt(
                            aanmelding->data(OPHAALPUNT).toString(),   // naam
                            aanmelding->data(WEIGHT_KURK).toDouble(),  // kg_kurk
                            aanmelding->data(WEIGHT_KAARS).toDouble(), // kg_kaarsresten
                            aanmelding->data(ZAK_KURK).toDouble(),     // zakken_kurk
                            aanmelding->data(ZAK_KAARS).toDouble(),    // zakken_kaarsresten
                            aanmelding->data(ADRES).toString(),        // adres
                            aanmelding->data(AANMELDING_ID).toInt()    // id
                        );

            listOfAanmeldingen->append(_ophaalpunt);
        }
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
    delete legeAanmeldingenList;
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
