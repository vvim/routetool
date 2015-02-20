#include "exportcollectionhistory.h"
#include <QtGui>
#include <QSqlQuery>

#define vvimDebug()\
    qDebug() << "[" << Q_FUNC_INFO << "]"

#define EndOfLine "\r\n"

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

    connect(buttonBox, SIGNAL(accepted()),this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()),this, SLOT(reject()));
    vvimDebug() << "TODO <vvim>: when this window gets closed, there is no trigger to DELETE the content. How to?";

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
    if(timeperiod_startEdit->date() > timeperiod_endEdit->date())
    {
        vvimDebug() << "INVALID timeperiod, " << timeperiod_startEdit->date().toString() << "is >" << timeperiod_endEdit->date().toString();
        QMessageBox::information(this, tr("Startdatum valt na de einddatum"), tr("Exporteren onmogelijk, de startdatum valt na de einddatum. Gelieve dit aan te passen."));
        return;
    }

    if(!saveToCSV())
        return; // empty string, so no filename was indicated to save it ; or could not open file ; or database error

    vvimDebug() << "History exported, huray!";

    // all done, let's go!
    reject();
}

void ExportCollectionHistory::reject()
{
    vvimDebug() << "closing";
    close();
    vvimDebug() << "deleting";
    delete this;
}

bool ExportCollectionHistory::saveToCSV()
{
    /**
      see http://stackoverflow.com/questions/12546031/qfiledialoggetsavefilename-and-default-selectedfilter
      and http://stackoverflow.com/questions/1953631/qfiledialog-adding-extension-automatically-when-saving-file
    **/

    vvimDebug() << "export accepted";

    vvimDebug() << "show QFileDialog";

    QString filters("CSVbestanden (*.csv);;Tekstbestanden (*.txt);;Microsoft Excel (*.xls *.xlsx);;All files (*.*)");
    QString defaultFilter("Microsoft Excel (*.xls *.xlsx)");
    QString filename = QFileDialog::getSaveFileName(0, tr("Exporteer historiek naar..."), QDir::currentPath(), filters, &defaultFilter);

    if(filename.count() < 1)
    {
        vvimDebug() << "no filename given, the user must have pressed 'cancel' or 'close' in the FileDialogBox. Let's return to the Export Collection History Dialog Box like nothing happened...";
        // no need to show QMessageBox
        return false;
    }

    // would be better with "QFileDialog.SetDefaultSuffix()", but I simply don't get it: http://stackoverflow.com/questions/1953631/qfiledialog-adding-extension-automatically-when-saving-file
    // so, a little hack:
    if((filename.right(4) == ".xls") || (filename.right(5) == ".xlsx") || (filename.right(4) == ".txt") || (filename.right(4) == ".csv") )
        vvimDebug() << "file-extension is alright:" << filename.right(4);
    else
        filename.append(".csv");

    vvimDebug() << "filename:" << filename;


    /// 2. now that we finally have this filename-stuff settled, let's go down to business!

    int ophaalpunt_id = ophaalpunten[ophaalpuntEdit->text()];
    QString startdate = QLocale().toString(timeperiod_startEdit->date(),"dd MMM yyyy");
    QString enddate = QLocale().toString(timeperiod_endEdit->date(),"dd MMM yyyy");

    QFile f( filename );

    if(!f.open(QFile::WriteOnly | QFile::Truncate)) // 'truncate' == overwrite
    {
        vvimDebug() << "FAILED: We could not opened file" << filename << "show messagebox to user and return to Export Collection History Dialog Box";
        QMessageBox::information(this, tr("Kan bestand niet openen"), tr("Bestand %1 kan niet geopend worden, probeer opnieuw. Als deze fout zich blijft voordoen, stuur het logbestand naar Wim of neem contact op met de systeembeheerder.").arg(filename));
    }

    QTextStream data( &f );

    QSqlQuery query;

    if(ophaalpunt_id < 1)
    {
        vvimDebug() << "Show history of ALL locations within timeperiod";
        query.prepare("SELECT ophalinghistoriek.*, ophaalpunten.naam FROM ophalinghistoriek, ophaalpunten WHERE ophalingsdatum >= :startdate AND ophalingsdatum <= :enddate AND ophalinghistoriek.ophaalpunt = ophaalpunten.id ORDER BY ophalingsdatum");
        data << tr("\"Export van de ophalinghistoriek uit databank voor alle ophaalpunten van %1 tot %2\"").arg(startdate).arg(enddate)+EndOfLine;
        data << EndOfLine;
    }
    else
    {
        vvimDebug() << "Show history of location" << ophaalpuntEdit->text();
        query.prepare("SELECT ophalinghistoriek.*, ophaalpunten.naam FROM ophalinghistoriek, ophaalpunten WHERE ophaalpunt = :ophaalpuntid AND ophalingsdatum >= :startdate AND ophalingsdatum <= :enddate AND ophalinghistoriek.ophaalpunt = ophaalpunten.id ORDER BY ophalingsdatum");
        query.bindValue(":ophaalpuntid", ophaalpunt_id);
        data << tr("\"Export van de ophalinghistoriek uit databank\"")+EndOfLine;
        data << EndOfLine;
        data << tr("\"Ophaalpunt\";\"Adres\";\"Postcode\";\"Plaats\";\"Soort ophaalpunt\";\"Export startdatum\";\"Export einddatum\"")+EndOfLine;

        // informatie van ophaalpunt :ophaalpuntid uit DB halen
        QSqlQuery ophaalpunt;
        ophaalpunt.prepare("SELECT ophaalpunten.naam, ophaalpunten.straat, ophaalpunten.nr, ophaalpunten.bus, ophaalpunten.postcode, ophaalpunten.plaats, soort_ophaalpunt.soort FROM ophaalpunten, soort_ophaalpunt WHERE ophaalpunten.id = :ophaalpuntid AND ophaalpunten.code = soort_ophaalpunt.code");
        ophaalpunt.bindValue(":ophaalpuntid", ophaalpunt_id);

        if(!ophaalpunt.exec())
        {
            data << tr("\"ERROR: kon geen verbinding maken met de databank voor informatie van ophaalpunt %1\"").arg(ophaalpunt_id);
            data << ";\"\"";
            data << ";\"\"";
            data << ";\"\"";
            data << ";\"\"";
        }
        else
        {
            if(ophaalpunt.next())
            {
                QString adres = ophaalpunt.value(1).toString() + " " + ophaalpunt.value(2).toString();
                if(ophaalpunt.value(3).toString().length() > 0)
                    adres += ", bus " + ophaalpunt.value(3).toString();

                data << "\"" << ophaalpunt.value(0).toString() << "\"" << ";"; // naam ophaalpunt
                data << "\"" << adres << "\"" << ";"; // straat ophaalpunt
                data << "\"" << ophaalpunt.value(4).toString() << "\"" << ";"; // postcode ophaalpunt
                data << "\"" << ophaalpunt.value(5).toString() << "\"" << ";"; // plaats ophaalpunt
                data << "\"" << ophaalpunt.value(6).toString() << "\"" << ";"; // soort ophaalpunt
            }
            else
            {
                data << tr("\"ERROR: geen informatie gevonden in databank over ophaalpunt %1\"").arg(ophaalpunt_id);
                data << ";\"\"";
                data << ";\"\"";
                data << ";\"\"";
                data << ";\"\"";
            }
        }
        data << tr("\"%1\";\"%2\"").arg(startdate).arg(enddate)+EndOfLine;
        data << EndOfLine;
    }

    query.bindValue(":startdate",timeperiod_startEdit->date());
    query.bindValue(":enddate",timeperiod_endEdit->date());

    if(!query.exec())
    {
        vvimDebug() << "FAILED: We could not execute the query SELECT ophalinghistoriek.*, ophaalpunten.naam FROM ophalinghistoriek, ophaalpunten WHERE ophaalpunt = :ophaalpuntid AND ophalingsdatum >= :startdate AND ophalingsdatum <= :enddate AND ophalinghistoriek.ophaalpunt = ophaalpunten.id .";
        vvimDebug() << "show messagebox to user and return to Export Collection History Dialog Box";
        QMessageBox::information(this, tr("Fout bij verbinding met de databank"), tr("De databank kon niet geraadpleegd worden, probeer opnieuw. Als deze fout zich blijft voordoen, stuur het logbestand naar Wim of neem contact op met de systeembeheerder."));
    }

    QStringList strList;

    int records = 0;
    int columns_in_queryresult = 11 + 1; // 12 columns in total, we will skip the first two of ophalinhistoriek (column 'ID' and column 'TIMESTAMP') and print out the rest. The 12th column is "ophaalpunten.naam" with the name of the collector's location

    strList << "\"ophalingsdatum\"";
    strList << "\"chauffeur\"";
    strList <<"\"ophaalpunt\"";
    strList <<"\"zakken_kurk\"";
    strList <<"\"kg_kurk\"";
    strList <<"\"zakken_kaarsresten\"";
    strList <<"\"kg_kaarsresten\"";
    strList <<"\"opmerkingen\"";
    strList <<"\"aanmeldingsdatum\"";
    strList <<"\"ophaalpunt\"";
    data << strList.join( ";" )+EndOfLine;

    while (query.next())
    {
        strList.clear();
        for( int c = 2; c < columns_in_queryresult; ++c ) // "ophalinghistoriek.id" and "ophalinghistoriek.timestamp" do not need to be exported
        {
            strList << "\""+query.value(c).toString()+"\"";
        }

        /** MS Excel from Geert treats fields that contain a \n as a new row.
            This can be very confusing, therefore we add the call to "replace()"
            as to change every \n by a white space.
        **/
        data << strList.join( ";" ).replace("\n"," ")+EndOfLine;

        records++;
    }

    if(records < 1)
    {
        vvimDebug() << "no results from this query";
        data << tr("Geen historiek gevonden voor deze periode.")+";\n";
    }
    else
    {
        vvimDebug() << records << "results found and recorded";
    }
    QMessageBox::information(this, tr("Historiek geëxporteerd"), tr("Er werden %1 records gevonden en geëxporteerd naar bestand %2.").arg(records).arg(filename));

    f.close();

    return true;
}
