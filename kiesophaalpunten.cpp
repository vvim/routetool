#include <QStringList>

#include <QDebug>
#include <QSqlQuery>
#include <QSqlError>
#include <QVBoxLayout>
#include <QHBoxLayout>
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
    // TODO neem dit uit de databank

    legeAanmeldingenList->clear();

    legeAanmeldingenList->setSortingEnabled(true);
    QStringList joris;
    joris << "Oak" << "Banana" << "Apple" << "Orange" << "Grapes" << "Jayesh" << "Pineapple" << "Groundnut" << "Sugarcane" << "Coconut" << "Remote" << "Mango";
    foreach (QString j, joris)
    {
        QListWidgetItem * item = new QListWidgetItem(j);
        item->setData(Qt::DisplayRole,QString(j).append(" (%1 kg , %2 liter)").arg(1000.4).arg(2000.5));
        item->setData(OPHAALPUNT,j);
        item->setData(WEIGHT,1000.4);
        item->setData(VOLUME,2000.5);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setFlags(item->flags() &~ Qt::ItemIsSelectable);
        item->setCheckState(Qt::Unchecked); // http://www.qtcentre.org/threads/7032-QListWidget-with-check-box-s , thank you J-P Nurmi
        legeAanmeldingenList->addItem(item);
    }
}


void KiesOphaalpunten::accept()
{
    //recalculate total weight and total volume: see BUG
          // <vvim> BUG: als een item geselecteerd wordt met SPATIE, dan wordt er niets bijgeteld?????";
          setTotalWeightTotalVolume();

    for(int i = 0 ; i < legeAanmeldingenList->count(); i++)
    {
        QListWidgetItem *aanmelding = legeAanmeldingenList->item(i);
        aanmelding->setData(1, 15 * i);
        if(aanmelding->checkState() == Qt::Checked)
            qDebug() << "yes" << aanmelding->data(OPHAALPUNT).toString() << ":" << aanmelding->data(WEIGHT).toDouble() << "kg" << aanmelding->data(VOLUME).toDouble() << "liter";
        else
            qDebug() << "NO" << aanmelding->data(OPHAALPUNT).toString() << "!";
    }
    this->close();
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
