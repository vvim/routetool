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
#define WEIGHT OPHAALPUNT+1
#define VOLUME WEIGHT+1
#define ADRES VOLUME+1


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
        total_weight += aanmelding->data(WEIGHT).toDouble();
        total_volume += aanmelding->data(VOLUME).toDouble();
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
        total_weight += aanmelding->data(WEIGHT).toDouble();
        total_volume += aanmelding->data(VOLUME).toDouble();
    }
    else
    {
        aanmelding->setCheckState(Qt::Unchecked);
        total_weight -= aanmelding->data(WEIGHT).toDouble();
        total_volume -= aanmelding->data(VOLUME).toDouble();
    }

    setTotalWeightTotalVolume();
}


void KiesOphaalpunten::itemSelected()
{
    QListWidgetItem* aanmelding = legeAanmeldingenList->currentItem();
    if(aanmelding->checkState() == Qt::Unchecked)
    {
        aanmelding->setCheckState(Qt::Checked);
        total_weight += aanmelding->data(WEIGHT).toDouble();
        total_volume += aanmelding->data(VOLUME).toDouble();
    }
    else
    {
        aanmelding->setCheckState(Qt::Unchecked);
        total_weight -= aanmelding->data(WEIGHT).toDouble();
        total_volume -= aanmelding->data(VOLUME).toDouble();
    }

    setTotalWeightTotalVolume();
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
                    total_weight += aanmelding->data(WEIGHT).toDouble();
                    total_volume += aanmelding->data(VOLUME).toDouble();
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

    QSqlQuery query("select ophaalpunten.naam, aanmelding.kg_kurk, aanmelding.kg_kaarsresten, aanmelding.zakken_kurk, aanmelding.zakken_kaarsresten, CONCAT_WS(' ', ophaalpunten.straat, ophaalpunten.nr,  ophaalpunten.bus, ophaalpunten.postcode, ophaalpunten.plaats, ophaalpunten.land) AS ADRES from aanmelding, ophaalpunten where ophaalpunten.id = aanmelding.ophaalpunt AND aanmelding.ophaalronde_nr is NULL");

    while (query.next())
    {
        QString ophaalpunt = query.value(0).toString();
        double weight = query.value(1).toDouble()+query.value(2).toDouble();
        double volume = (query.value(3).toDouble() * settings.value("zak_kurk_volume").toDouble()) +(query.value(4).toDouble() * settings.value("zak_kaarsresten_volume").toDouble());
        QString ophaalpunt_adres = query.value(5).toString();
        QListWidgetItem * item = new QListWidgetItem();
        item->setData(Qt::DisplayRole,QString("%1 (%2 kg , %3 liter)").arg(ophaalpunt).arg(weight).arg(volume));
        item->setData(OPHAALPUNT,ophaalpunt);
        item->setData(WEIGHT,weight);
        item->setData(VOLUME,volume);
        item->setData(ADRES,ophaalpunt_adres);
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
          QMessageBox::critical(this, tr("Overgewicht of overvolume!"),
                      tr("De voorgestelde ophaalpunten geven overgewicht of overvolume: %1kg (maximum %2kg) en %3 liter (maximum %4 liter).").arg(total_weight).arg(maximum_weight).arg(total_volume).arg(maximum_volume), QMessageBox::Cancel);
    }
    else
    {
        qDebug() << "TODO: Insert into DB";
        QList<QString> *listOfAanmeldingen = new QList<QString>();
        qDebug() << "<vvim> TODO: maak een klasse waarin we de naam, het adres en het gewicht/volume kurk/kaars kunnen opslaan. Ook beter voor QListWidget!";
        for(int i = 0 ; i < legeAanmeldingenList->count(); i++)
        {
            QListWidgetItem *aanmelding = legeAanmeldingenList->item(i);
            if(aanmelding->checkState() == Qt::Checked)
            {
                QString aanmelding_string = aanmelding->data(OPHAALPUNT).toString().append(", %1").arg(aanmelding->data(ADRES).toString());
                listOfAanmeldingen->push_back(aanmelding_string);
            }
        }
        emit aanmelding_for_route(listOfAanmeldingen);

        qDebug() << "<vvim> add data to the database so that we can reconstruct 'ophaalrondes'.";
        this->close();
    }

}

void KiesOphaalpunten::reject()
{
    this->close();
}

KiesOphaalpunten::~KiesOphaalpunten()
{
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
}

void KiesOphaalpunten::initialise()
{
    maximum_weight = settings.value("max_gewicht_vrachtwagen").toDouble();
    maximum_volume = settings.value("max_volume_vrachtwagen").toDouble();

    populateLegeAanmeldingen();

    uncheckAll();
}
