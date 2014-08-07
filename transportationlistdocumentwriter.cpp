#include "transportationlistdocumentwriter.h"
#include <QTextTable>
#include <QPainter>
#include <QDebug>

// read http://doc.qt.digia.com/qq/qq27-odfwriter.html

// interesting read on PDFs: https://qt-project.org/forums/viewthread/38065

TransportationListDocumentWriter::TransportationListDocumentWriter(QDate date_of_transportation, int empty_bags_of_kurk_to_bring, int empty_bags_of_kaarsresten_to_bring)
    : m_document(new QTextDocument()),
    m_cursor(m_document)
{
    QString address = "<b>De Vlaspit vzw</b><br />"
            "Basilieklaan 53<br />"
            "B-3270 Scherpenheuvel<br />"
            "BTW BE-0461 019 224";
    QString legal = "Opgenomen in het register van erkende overbrengers en vervoerders van afvalstoffen onder het nummer 60337/E/11574<br />"
            "Enregistr&eacute;e en qualit&eacute; de collecteur et de transporteur de d&eacute;chets autres que dangereux sous le num&eacute;ro 2011-04-01-17";

    QString title = "Vervoerslijst";

    m_cursor.insertHtml(QString("<table><tr><td>%1</td><td>%2</td></tr></table><br />").arg(address).arg(legal));

    m_cursor.insertHtml(QString("<h1>%1</h1><br />").arg(title));
    m_cursor.insertHtml(QString("Datum: %1<br />").arg(QLocale().toString(date_of_transportation,"d/MM/yyyy"))); // 1/09/2014 = 1 september 2014


    m_cursor.insertHtml(QString("<table><tr><th>Meenemen:</td><td>%1 lege zakken voor kurk</td></tr><tr><th></td><td>%2 lege zakken voor kaarsresten</td></tr></table><br /><br />").arg(empty_bags_of_kurk_to_bring).arg(empty_bags_of_kaarsresten_to_bring));
}

TransportationListDocumentWriter::~TransportationListDocumentWriter()
{
  qDebug() << "start to deconstruct TransportationListDocumentWriter()";
  delete m_document;
  qDebug() << "TransportationListDocumentWriter() deconstructed";
}

void TransportationListDocumentWriter::addOphaalpunt(const TransportationListDocumentWriter::Ophaalpunt &ophaalpunt)
{
    m_cursor.insertText( QString("Locatie # %1 OPHALING\n").arg(ophaalpunt.counter));
    m_cursor.insertText( QString("..Kaart # %1\n").arg(ophaalpunt.kaart_nr));
    m_cursor.insertText( QString("..Verwachtte aankomsttijd %1\n").arg(QLocale().toString(ophaalpunt.arrivaltime,"hh:mm")));
    m_cursor.insertText( QString("..Naam: ").append(ophaalpunt.naam).append("\n"));
    m_cursor.insertText( QString("..Adres: ").append(ophaalpunt.straat).append(" ").append(ophaalpunt.nr).append(" ").append(ophaalpunt.bus).append("\n"));
    m_cursor.insertText( QString("..Postcode: ").append(ophaalpunt.postcode).append("\n"));
    if(ophaalpunt.land.left(4).toLower() != "belg")
    {
        m_cursor.insertText( QString("..Gemeente: ").append(ophaalpunt.gemeente).append(" (").append(ophaalpunt.land).append(")").append("\n"));
    }
    else
        m_cursor.insertText( QString("..Gemeente: ").append(ophaalpunt.gemeente).append("\n"));
    m_cursor.insertText( QString("..Telefoon: ").append(ophaalpunt.telefoonnummer).append("\n"));
    m_cursor.insertText( QString("..Contactpersoon: ").append(ophaalpunt.contactpersoon).append("\n"));
    m_cursor.insertText( QString("..Openingsuren: ").append(ophaalpunt.openingsuren).append("\n"));
    m_cursor.insertText( QString("..Speciale opmerkingen: ").append(ophaalpunt.opmerkingen).append("\n"));
    m_cursor.insertText( QString("....................................\n"));
    m_cursor.insertText( QString("..op te halen kurk: %1 kg, %2 zakken").arg(ophaalpunt.kg_kurk).arg(ophaalpunt.zakken_kurk).append("\n"));
    m_cursor.insertText( QString("....werkelijk opgehaald: ___ kg , ___ zakken\n"));
    m_cursor.insertText( QString("..op te halen kaarsresten: %1 kg, %2 zakken").arg(ophaalpunt.kg_kaarsresten).arg(ophaalpunt.zakken_kaarsresten).append("\n"));
    m_cursor.insertText( QString("....werkelijk opgehaald: ___ kg , ___ zakken\n"));
    m_cursor.insertText( QString("..lege zakken afgegeven: ____\n"));
    m_cursor.insertText( QString("\n\n"));
}

void TransportationListDocumentWriter::addGraph(QList<int> values, const QString &subtext)
{
// <vvim> example to add Google Maps Image
}


void TransportationListDocumentWriter::write(const QString &fileName)
{
    QString fn = fileName;
    if(!(fn.right(4) == ".odt"))
        fn.append(".odt");
    QTextDocumentWriter writer(fn);
  qDebug() << "filename van document is:" << fn;
  writer.write(m_document);
}

void TransportationListDocumentWriter::addLevering(const Levering &levering)
{
    m_cursor.insertText( QString("Locatie # %1 LEVERING\n").arg(levering.counter));
    m_cursor.insertText( QString("..Kaart # %1\n").arg(levering.kaart_nr));
    m_cursor.insertText( QString("..Verwachtte aankomsttijd %1\n").arg(QLocale().toString(levering.arrivaltime,"hh:mm")));
    m_cursor.insertText( QString("..Naam: ").append(levering.naam).append("\n"));
    m_cursor.insertText( QString("..Adres: ").append(levering.straat).append(levering.nr).append(levering.bus).append("\n"));
    m_cursor.insertText( QString("..Postcode: ").append(levering.postcode).append("\n"));
    if(levering.land.left(4).toLower() != "belg")
    {
        m_cursor.insertText( QString("..Gemeente: ").append(levering.gemeente).append(" (").append(levering.land).append(")").append("\n"));
    }
    else
        m_cursor.insertText( QString("..Gemeente: ").append(levering.gemeente).append("\n"));
    m_cursor.insertText( QString("..Telefoon: ").append(levering.telefoonnummer).append("\n"));
    m_cursor.insertText( QString("..Contactpersoon: ").append(levering.contactpersoon).append("\n"));
    m_cursor.insertText( QString("....................................\n"));
    m_cursor.insertText( QString("..af te leveren: %1 kg, %2 liter").arg(levering.weight).arg(levering.volume).append("\n"));
    m_cursor.insertText( QString("\n\n"));
}

void TransportationListDocumentWriter::addAdres(const Adres &adres)
{
    m_cursor.insertText( QString("Locatie # %1 ADRES\n").arg(adres.counter));
    m_cursor.insertText( QString("..Kaart # %1\n").arg(adres.kaart_nr));
    m_cursor.insertText( QString("..Verwachtte aankomsttijd %1\n").arg(QLocale().toString(adres.arrivaltime,"hh:mm")));
    m_cursor.insertText( QString("..Adres: ").append(adres.caption).append("\n"));
    qDebug() << "caption:" << adres.caption;
    m_cursor.insertText( QString("\n\n"));
}
