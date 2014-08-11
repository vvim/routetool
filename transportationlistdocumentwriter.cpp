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
    QString client = "<b>De Vlaspit vzw</b><br />"
            "Basilieklaan 53<br />"
            "B-3270 Scherpenheuvel<br />"
            "BTW BE-0461 019 224";
    QString legal = "Opgenomen in het register van erkende overbrengers en vervoerders van afvalstoffen onder het nummer 60337/E/11574<br />"
            "Enregistr&eacute;e en qualit&eacute; de collecteur et de transporteur de d&eacute;chets autres que dangereux sous le num&eacute;ro 2011-04-01-17";

    QString title = "Vervoerslijst";


    QString document_heading = "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 3.2//EN\">"
                        "<HTML>"
                        "<HEAD>"
                            "<META HTTP-EQUIV=\"CONTENT-TYPE\" CONTENT=\"text/html; charset=utf-8\">"
                            "<TITLE></TITLE>"
                            "<STYLE>\n"
                                "<!-- "
                                "BODY,DIV,TABLE,THEAD,TBODY,TFOOT,TR,TH,TD,P { font-family:\"Calibri\"; font-size:x-small }"
                                " -->"
                            "</STYLE>"
                        "</HEAD>"
                        "<BODY>"
                        "<TABLE FRAME=VOID CELLSPACING=0 COLS=6 RULES=NONE BORDER=0>"
                        "<COLGROUP><COL WIDTH=86><COL WIDTH=229><COL WIDTH=383><COL WIDTH=119><COL WIDTH=119><COL WIDTH=119></COLGROUP>"
                        "<TBODY>"
                        "<TR>"
                            "<TD WIDTH=86 HEIGHT=97 ALIGN=CENTER VALIGN=BOTTOM><B><FONT SIZE=5><BR></FONT></B></TD>"
                            "<TD WIDTH=229 ALIGN=LEFT VALIGN=TOP><B><FONT SIZE=4><vvimroutetool>CLIENT</vvimroutetool></FONT></B></TD>"
                            "<TD WIDTH=383 ALIGN=LEFT VALIGN=TOP><FONT><BR></FONT></TD>"
                            "<TD COLSPAN=3 WIDTH=358 ALIGN=CENTER VALIGN=BOTTOM><FONT SIZE=1><vvimroutetool>LEGAL</vvimroutetool></FONT></TD>"
                        "</TR>"
                        "<TR>"
                            "<TD HEIGHT=53 ALIGN=CENTER VALIGN=MIDDLE><B><FONT SIZE=5><BR></FONT></B></TD>"
                            "<TD ALIGN=LEFT VALIGN=MIDDLE><B><FONT SIZE=5><vvimroutetool>TITLE</vvimroutetool></FONT></B></TD>"
                            "<TD ALIGN=LEFT VALIGN=TOP><FONT><BR></FONT></TD>"
                            "<TD ALIGN=LEFT VALIGN=MIDDLE><B><FONT SIZE=4>Datum:</FONT></B></TD>"
                            "<TD ALIGN=LEFT VALIGN=MIDDLE><FONT><vvimroutetool>DATUM</vvimroutetool></FONT></TD>"
                            "<TD ALIGN=LEFT VALIGN=MIDDLE><FONT><BR></FONT></TD>"
                        "</TR>"
                        "<TR>"
                            "<TD COLSPAN=3 ALIGN=RIGHT><B>Meenemen:</B></TD>"
                            "<TD COLSPAN=3><vvimroutetool>LEGEZAKKENKURK</vvimroutetool> lege zakken voor kurk</TD>"
                        "</TR>"
                        "<TR>"
                        "<TD COLSPAN=3><B> </B></TD>"
                        "<TD COLSPAN=3><vvimroutetool>LEGEZAKKENKAARS</vvimroutetool> lege zakken voor kurk</TD>"
                        "</TR>";

    template_ophaling = " <TR>  <TD STYLE=\"border-top: 3px solid #000000; border-right: 1px solid #000000\" HEIGHT=31 ALIGN=CENTER VALIGN=BOTTOM><B><FONT SIZE=5><BR></FONT></B></TD>  <TD STYLE=\"border-top: 3px solid #000000; border-bottom: 1px solid #000000; border-left: 1px solid #000000; border-right: 1px solid #000000\" ALIGN=LEFT VALIGN=TOP><FONT>Naam</FONT></TD>  <TD STYLE=\"border-top: 3px solid #000000; border-bottom: 1px solid #000000; border-right: 1px solid #000000\" ALIGN=LEFT VALIGN=TOP><FONT><vvimroutetool>NAAM</vvimroutetool></FONT></TD>  <TD STYLE=\"border-top: 3px solid #000000; border-left: 1px solid #000000\" COLSPAN=3 ALIGN=CENTER VALIGN=TOP BGCOLOR=\"#F2F2F2\"><B><FONT>Soort vervoer</FONT></B></TD>  </TR>  <TR>  <TD HEIGHT=42 ALIGN=CENTER VALIGN=BOTTOM><B><FONT SIZE=5><vvimroutetool>COUNTER</vvimroutetool></FONT></B></TD>  <TD STYLE=\"border-top: 1px solid #000000; border-bottom: 1px solid #000000; border-left: 1px solid #000000; border-right: 1px solid #000000\" ALIGN=LEFT VALIGN=TOP><FONT>Adres</FONT></TD>  <TD STYLE=\"border-top: 1px solid #000000; border-bottom: 1px solid #000000; border-right: 1px solid #000000\" ALIGN=LEFT VALIGN=TOP><vvimroutetool>ADRES</vvimroutetool></TD>  <TD STYLE=\"border-top: 1px solid #000000; border-bottom: 1px solid #000000; border-left: 1px solid #000000\" COLSPAN=3 ALIGN=CENTER VALIGN=MIDDLE><FONT COLOR=\"#FF0000\"><vvimroutetool>SOORTVERVOER</vvimroutetool></FONT></TD>  </TR>  <TR>  <TD HEIGHT=24 ALIGN=CENTER VALIGN=BOTTOM><FONT><vvimroutetool>ARRIVALTIME</vvimroutetool></FONT></TD>  <TD STYLE=\"border-top: 1px solid #000000; border-bottom: 1px solid #000000; border-left: 1px solid #000000; border-right: 1px solid #000000\" ALIGN=LEFT VALIGN=TOP><FONT>Postcode</FONT></TD>  <TD STYLE=\"border-top: 1px solid #000000; border-bottom: 1px solid #000000; border-right: 1px solid #000000\" ALIGN=LEFT VALIGN=TOP><FONT><vvimroutetool>POSTCODE</vvimroutetool></FONT></TD>  <TD STYLE=\"border-bottom: 1px solid #000000; border-left: 1px solid #000000\" ALIGN=CENTER VALIGN=TOP BGCOLOR=\"#F2F2F2\"><B><FONT>Kaartnr.</FONT></B></TD>  <TD STYLE=\"border-top: 1px solid #000000; border-bottom: 1px solid #000000; border-right: 1px solid #000000\" ALIGN=LEFT VALIGN=BOTTOM><FONT><vvimroutetool>KAARTNR</vvimroutetool></FONT></TD>  <TD ALIGN=LEFT VALIGN=BOTTOM><FONT><BR></FONT></TD>  </TR>  <TR>  <TD HEIGHT=34 ALIGN=CENTER VALIGN=BOTTOM><B><FONT SIZE=5><BR></FONT></B></TD>  <TD STYLE=\"border-top: 1px solid #000000; border-bottom: 1px solid #000000; border-left: 1px solid #000000; border-right: 1px solid #000000\" ALIGN=LEFT VALIGN=TOP><FONT>Gemeente</FONT></TD>  <TD STYLE=\"border-top: 1px solid #000000; border-bottom: 1px solid #000000; border-right: 1px solid #000000\" ALIGN=LEFT VALIGN=TOP><B><FONT><vvimroutetool>GEMEENTE</vvimroutetool></FONT></B></TD>  <TD ALIGN=LEFT VALIGN=BOTTOM><FONT><BR></FONT></TD>  <TD ALIGN=CENTER VALIGN=TOP BGCOLOR=\"#D9D9D9\"><B><FONT>aangemelde zakken</FONT></B></TD>  <TD ALIGN=CENTER VALIGN=TOP BGCOLOR=\"#D9D9D9\"><B><FONT>zakken opgehaald</FONT></B></TD>  </TR>  <TR>  <TD HEIGHT=40 ALIGN=CENTER VALIGN=BOTTOM><B><FONT SIZE=5><BR></FONT></B></TD>  <TD STYLE=\"border-top: 1px solid #000000; border-bottom: 1px solid #000000; border-left: 1px solid #000000; border-right: 1px solid #000000\" ALIGN=LEFT VALIGN=TOP><FONT>Telefoon</FONT></TD>  <TD STYLE=\"border-top: 1px solid #000000; border-bottom: 1px solid #000000; border-right: 1px solid #000000\" ALIGN=LEFT VALIGN=TOP><FONT><vvimroutetool>TELEFOONNR</vvimroutetool></FONT></TD>  <TD ALIGN=LEFT VALIGN=BOTTOM><B><FONT>Kurk</FONT></B></TD>  <TD STYLE=\"border-top: 1px solid #000000; border-bottom: 1px solid #000000; border-left: 1px solid #000000; border-right: 1px solid #000000\" ALIGN=LEFT VALIGN=BOTTOM><FONT><vvimroutetool>KURK</vvimroutetool></FONT></TD>  <TD STYLE=\"border-top: 1px solid #000000; border-bottom: 1px solid #000000; border-left: 1px solid #000000; border-right: 1px solid #000000\" ALIGN=LEFT VALIGN=BOTTOM><FONT>_____________</FONT></TD>  </TR>  <TR>  <TD HEIGHT=33 ALIGN=CENTER VALIGN=BOTTOM><B><FONT SIZE=5><BR></FONT></B></TD>  <TD STYLE=\"border-top: 1px solid #000000; border-bottom: 1px solid #000000; border-left: 1px solid #000000; border-right: 1px solid #000000\" ALIGN=LEFT VALIGN=TOP><FONT>Contactpersoon</FONT></TD>  <TD STYLE=\"border-top: 1px solid #000000; border-bottom: 1px solid #000000; border-right: 1px solid #000000\" ALIGN=LEFT VALIGN=TOP><FONT><vvimroutetool>CONTACTPERSOON</vvimroutetool></FONT></TD>  <TD ALIGN=LEFT VALIGN=BOTTOM><B><FONT>Kaars</FONT></B></TD>  <TD STYLE=\"border-top: 1px solid #000000; border-bottom: 1px solid #000000; border-left: 1px solid #000000; border-right: 1px solid #000000\" ALIGN=LEFT VALIGN=BOTTOM><FONT><vvimroutetool>KAARSRESTEN</vvimroutetool></FONT></TD>  <TD STYLE=\"border-top: 1px solid #000000; border-bottom: 1px solid #000000; border-left: 1px solid #000000; border-right: 1px solid #000000\" ALIGN=LEFT VALIGN=BOTTOM><FONT>_____________</FONT></TD>  </TR>  <TR>  <TD HEIGHT=33 ALIGN=CENTER VALIGN=BOTTOM><B><FONT SIZE=5><BR></FONT></B></TD>  <TD STYLE=\"border-top: 1px solid #000000; border-bottom: 1px solid #000000; border-left: 1px solid #000000; border-right: 1px solid #000000\" ALIGN=LEFT VALIGN=TOP><FONT>Openingsuren</FONT></TD>  <TD STYLE=\"border-top: 1px solid #000000; border-bottom: 1px solid #000000; border-right: 1px solid #000000\" ALIGN=LEFT VALIGN=TOP><vvimroutetool>OPENINGSUREN</vvimroutetool></TD>  <TD ALIGN=LEFT VALIGN=BOTTOM><FONT><BR></FONT></TD>  <TD COLSPAN=2 ALIGN=CENTER BGCOLOR=\"#D9D9D9\"><B><FONT>Lege zakken afgegeven</FONT></B></TD>  </TR>  <TR>  <TD HEIGHT=33 ALIGN=CENTER VALIGN=BOTTOM><B><FONT SIZE=5><BR></FONT></B></TD>  <TD STYLE=\"border-top: 1px solid #000000; border-left: 1px solid #000000; border-right: 1px solid #000000\" ALIGN=LEFT VALIGN=TOP><FONT>Speciale opmerkingen</FONT></TD>  <TD STYLE=\"border-top: 1px solid #000000; border-right: 1px solid #000000\" ALIGN=LEFT VALIGN=TOP><FONT COLOR=\"#FF0000\"><vvimroutetool>OPMERKINGEN</vvimroutetool></FONT></TD><TD></TD><TD ALIGN=LEFT VALIGN=BOTTOM><FONT>kurk:</FONT></TD>  <TD ALIGN=LEFT VALIGN=BOTTOM><FONT>kaars:</FONT></TD>  </TR> ";
    template_levering = "<TR><TD STYLE=\"border-top: 3px solid #000000; border-right: 1px solid #000000\" HEIGHT=31 ALIGN=CENTER VALIGN=BOTTOM><B><FONT SIZE=5><BR></FONT></B></TD><TD STYLE=\"border-top: 3px solid #000000; border-bottom: 1px solid #000000; border-left: 1px solid #000000; border-right: 1px solid #000000\" ALIGN=LEFT VALIGN=TOP><FONT>Naam</FONT></TD><TD STYLE=\"border-top: 3px solid #000000; border-bottom: 1px solid #000000; border-right: 1px solid #000000\" ALIGN=LEFT VALIGN=TOP><FONT><vvimroutetool>NAAM</vvimroutetool></FONT></TD><TD STYLE=\"border-top: 3px solid #000000; border-left: 1px solid #000000\" COLSPAN=3 ALIGN=CENTER VALIGN=TOP BGCOLOR=\"#F2F2F2\"><B><FONT>Soort vervoer</FONT></B></TD></TR><TR><TD HEIGHT=42 ALIGN=CENTER VALIGN=BOTTOM><B><FONT SIZE=5><vvimroutetool>COUNTER</vvimroutetool></FONT></B></TD><TD STYLE=\"border-top: 1px solid #000000; border-bottom: 1px solid #000000; border-left: 1px solid #000000; border-right: 1px solid #000000\" ALIGN=LEFT VALIGN=TOP><FONT>Adres</FONT></TD><TD STYLE=\"border-top: 1px solid #000000; border-bottom: 1px solid #000000; border-right: 1px solid #000000\" ALIGN=LEFT VALIGN=TOP><vvimroutetool>ADRES</vvimroutetool></TD><TD STYLE=\"border-top: 1px solid #000000; border-bottom: 1px solid #000000; border-left: 1px solid #000000\" COLSPAN=3 ALIGN=CENTER VALIGN=MIDDLE><FONT COLOR=\"#FF0000\"><vvimroutetool>SOORTVERVOER</vvimroutetool></FONT></TD></TR><TR><TD HEIGHT=24 ALIGN=CENTER VALIGN=BOTTOM><FONT><vvimroutetool>ARRIVALTIME</vvimroutetool></FONT></TD><TD STYLE=\"border-top: 1px solid #000000; border-bottom: 1px solid #000000; border-left: 1px solid #000000; border-right: 1px solid #000000\" ALIGN=LEFT VALIGN=TOP><FONT>Postcode</FONT></TD><TD STYLE=\"border-top: 1px solid #000000; border-bottom: 1px solid #000000; border-right: 1px solid #000000\" ALIGN=LEFT VALIGN=TOP><FONT><vvimroutetool>POSTCODE</vvimroutetool></FONT></TD><TD STYLE=\"border-bottom: 1px solid #000000; border-left: 1px solid #000000\" ALIGN=CENTER VALIGN=TOP BGCOLOR=\"#F2F2F2\"><B><FONT>Kaartnr.</FONT></B></TD><TD STYLE=\"border-top: 1px solid #000000; border-bottom: 1px solid #000000; border-right: 1px solid #000000\" ALIGN=LEFT VALIGN=BOTTOM><FONT><vvimroutetool>KAARTNR</vvimroutetool></FONT></TD><TD ALIGN=LEFT VALIGN=BOTTOM><FONT><BR></FONT></TD></TR><TR><TD HEIGHT=34 ALIGN=CENTER VALIGN=BOTTOM><B><FONT SIZE=5><BR></FONT></B></TD><TD STYLE=\"border-top: 1px solid #000000; border-bottom: 1px solid #000000; border-left: 1px solid #000000; border-right: 1px solid #000000\" ALIGN=LEFT VALIGN=TOP><FONT>Gemeente</FONT></TD><TD STYLE=\"border-top: 1px solid #000000; border-bottom: 1px solid #000000; border-right: 1px solid #000000\" ALIGN=LEFT VALIGN=TOP><B><FONT><vvimroutetool>GEMEENTE</vvimroutetool></FONT></B></TD><TD ALIGN=LEFT VALIGN=BOTTOM><FONT><BR></FONT></TD><TD COLSPAN=2 ALIGN=CENTER BGCOLOR=\"#D9D9D9\"><B><FONT>Vracht</FONT></B></TD></TR><TR><TD HEIGHT=40 ALIGN=CENTER VALIGN=BOTTOM><B><FONT SIZE=5><BR></FONT></B></TD><TD STYLE=\"border-top: 1px solid #000000; border-bottom: 1px solid #000000; border-left: 1px solid #000000; border-right: 1px solid #000000\" ALIGN=LEFT VALIGN=TOP><FONT>Telefoon</FONT></TD><TD STYLE=\"border-top: 1px solid #000000; border-bottom: 1px solid #000000; border-right: 1px solid #000000\" ALIGN=LEFT VALIGN=TOP><FONT><vvimroutetool>TELEFOONNR</vvimroutetool></FONT></TD><TD></TD><TD STYLE=\"border-top: 1px solid #000000; border-bottom: 1px solid #000000; border-left: 1px solid #000000; border-right: 1px solid #000000\" ALIGN=LEFT VALIGN=BOTTOM><FONT><vvimroutetool>WEIGHT</vvimroutetool> kg</FONT></TD><TD STYLE=\"border-top: 1px solid #000000; border-bottom: 1px solid #000000; border-left: 1px solid #000000; border-right: 1px solid #000000\" ALIGN=LEFT VALIGN=BOTTOM><FONT><vvimroutetool>VOLUME</vvimroutetool> liter</FONT></TD></TR><TR><TD HEIGHT=33 ALIGN=CENTER VALIGN=BOTTOM><B><FONT SIZE=5><BR></FONT></B></TD><TD STYLE=\"border-top: 1px solid #000000; border-bottom: 1px solid #000000; border-left: 1px solid #000000; border-right: 1px solid #000000\" ALIGN=LEFT VALIGN=TOP><FONT>Contactpersoon</FONT></TD><TD STYLE=\"border-top: 1px solid #000000; border-bottom: 1px solid #000000; border-right: 1px solid #000000\" ALIGN=LEFT VALIGN=TOP><FONT><vvimroutetool>CONTACTPERSOON</vvimroutetool></FONT></TD><TD COLSPAN=3></TD></TR>";
    template_adres = "<TR><TD STYLE=\"border-top: 3px solid #000000; border-right: 1px solid #000000\" HEIGHT=31 ALIGN=CENTER VALIGN=BOTTOM><B><FONT SIZE=5><BR></FONT></B></TD><TD STYLE=\"border-top: 3px solid #000000\" COLSPAN=2><vvimroutetool>CAPTION</vvimroutetool></TD><TD STYLE=\"border-top: 3px solid #000000; border-left: 1px solid #000000\" COLSPAN=3 ALIGN=CENTER VALIGN=TOP BGCOLOR=\"#F2F2F2\"><B><FONT>Soort vervoer</FONT></B></TD></TR><TR><TD HEIGHT=42 ALIGN=CENTER VALIGN=BOTTOM><B><FONT SIZE=5><vvimroutetool>COUNTER</vvimroutetool></FONT></B></TD><TD COLSPAN=2>Aankomst: <vvimroutetool>ARRIVALTIME</vvimroutetool></TD><TD STYLE=\"border-top: 1px solid #000000; border-bottom: 1px solid #000000; border-left: 1px solid #000000\" COLSPAN=3 ALIGN=CENTER VALIGN=MIDDLE><FONT COLOR=\"#FF0000\"><vvimroutetool>SOORTVERVOER</vvimroutetool></FONT></TD></TR>";

    m_cursor.insertText(document_heading.replace("<vvimroutetool>CLIENT</vvimroutetool>",client)
                        .replace("<vvimroutetool>LEGAL</vvimroutetool>",legal)
                        .replace("<vvimroutetool>TITLE</vvimroutetool>",title)
                        .replace("<vvimroutetool>DATUM</vvimroutetool>",QLocale().toString(date_of_transportation,"d/MM/yyyy"))
                        .replace("<vvimroutetool>LEGEZAKKENKURK</vvimroutetool>",QString("%1").arg(empty_bags_of_kurk_to_bring))
                        .replace("<vvimroutetool>LEGEZAKKENKAARS</vvimroutetool>",QString("%1").arg(empty_bags_of_kaarsresten_to_bring))
                        );
}

TransportationListDocumentWriter::~TransportationListDocumentWriter()
{
  qDebug() << "start to deconstruct TransportationListDocumentWriter()";
  delete m_document;
  qDebug() << "TransportationListDocumentWriter() deconstructed";
}

void TransportationListDocumentWriter::addOphaalpunt(const TransportationListDocumentWriter::Ophaalpunt &ophaalpunt)
{
    QString document_ophaling = template_ophaling;
    m_cursor.insertText(document_ophaling.replace("<vvimroutetool>NAAM</vvimroutetool>",ophaalpunt.naam)
                        .replace("<vvimroutetool>COUNTER</vvimroutetool>",QString("%1").arg(ophaalpunt.counter))
                        .replace("<vvimroutetool>ADRES</vvimroutetool>",getAdres(ophaalpunt.straat,ophaalpunt.nr,ophaalpunt.bus))
                        .replace("<vvimroutetool>SOORTVERVOER</vvimroutetool>","Ophaling")
                        .replace("<vvimroutetool>ARRIVALTIME</vvimroutetool>",QLocale().toString(ophaalpunt.arrivaltime,"hh:mm"))
                        .replace("<vvimroutetool>POSTCODE</vvimroutetool>",ophaalpunt.postcode)
                        .replace("<vvimroutetool>KAARTNR</vvimroutetool>",QString("%1").arg(ophaalpunt.kaart_nr))
                        .replace("<vvimroutetool>GEMEENTE</vvimroutetool>",getGemeente(ophaalpunt.gemeente,ophaalpunt.land))
                        .replace("<vvimroutetool>TELEFOONNR</vvimroutetool>",ophaalpunt.telefoonnummer)
                        .replace("<vvimroutetool>KURK</vvimroutetool>",QString("%1").arg(ophaalpunt.zakken_kurk))
                        .replace("<vvimroutetool>CONTACTPERSOON</vvimroutetool>",ophaalpunt.contactpersoon)
                        .replace("<vvimroutetool>KAARSRESTEN</vvimroutetool>",QString("%1").arg(ophaalpunt.zakken_kaarsresten))
                        .replace("<vvimroutetool>OPENINGSUREN</vvimroutetool>",ophaalpunt.openingsuren)
                        .replace("<vvimroutetool>OPMERKINGEN</vvimroutetool>",ophaalpunt.opmerkingen)
                        );
}

void TransportationListDocumentWriter::write(const QString &fileName, const QString &map)
{
    QString document_tail= "</TBODY></TABLE><IMG src=\"<vvimroutetool>GOOGLEMAPS</vvimroutetool>\"</BODY></HTML>";
    m_cursor.insertText(document_tail.replace("<vvimroutetool>GOOGLEMAPS</vvimroutetool>",map));

    QString fn = fileName;
    if(!(fn.right(4) == ".doc"))
        fn.append(".doc");

    QTextDocumentWriter writer(fn);
    writer.setFormat("plaintext");
    qDebug() << "filename van document is:" << fn;
    writer.write(m_document);
}

void TransportationListDocumentWriter::addLevering(const Levering &levering)
{
    QString document_levering = template_levering;
    m_cursor.insertText(document_levering.replace("<vvimroutetool>NAAM</vvimroutetool>",levering.naam)
                        .replace("<vvimroutetool>COUNTER</vvimroutetool>",QString("%1").arg(levering.counter))
                        .replace("<vvimroutetool>ADRES</vvimroutetool>",getAdres(levering.straat,levering.nr,levering.bus))
                        .replace("<vvimroutetool>SOORTVERVOER</vvimroutetool>","Levering")
                        .replace("<vvimroutetool>ARRIVALTIME</vvimroutetool>",QLocale().toString(levering.arrivaltime,"hh:mm"))
                        .replace("<vvimroutetool>POSTCODE</vvimroutetool>",levering.postcode)
                        .replace("<vvimroutetool>KAARTNR</vvimroutetool>",QString("%1").arg(levering.kaart_nr))
                        .replace("<vvimroutetool>GEMEENTE</vvimroutetool>",getGemeente(levering.gemeente,levering.land))
                        .replace("<vvimroutetool>TELEFOONNR</vvimroutetool>",levering.telefoonnummer)
                        .replace("<vvimroutetool>WEIGHT</vvimroutetool>",QString("%1").arg(levering.weight))
                        .replace("<vvimroutetool>CONTACTPERSOON</vvimroutetool>",levering.contactpersoon)
                        .replace("<vvimroutetool>VOLUME</vvimroutetool>",QString("%1").arg(levering.volume))
                        );
}

void TransportationListDocumentWriter::addAdres(const Adres &adres)
{
    QString document_adres = template_adres;
    m_cursor.insertText(document_adres.replace("<vvimroutetool>CAPTION</vvimroutetool>",adres.caption)
                        .replace("<vvimroutetool>COUNTER</vvimroutetool>",QString("%1").arg(adres.counter))
                        .replace("<vvimroutetool>SOORTVERVOER</vvimroutetool>","Adres")
                        .replace("<vvimroutetool>ARRIVALTIME</vvimroutetool>",QLocale().toString(adres.arrivaltime,"hh:mm"))
                        );
}

QString TransportationListDocumentWriter::getGemeente(QString gemeente, QString land)
{
    if(land.left(4).toLower() != "belg")
    {
        QString _gemeente = "";
        _gemeente.append(gemeente).append(" (").append(land).append(")");
        return _gemeente;
    }
    else
        return gemeente;

}

QString TransportationListDocumentWriter::getAdres(QString straat, QString huisnr, QString busnr)
{
    QString address = "";

    address.append(straat).append(" ").append(huisnr);

    if(!busnr.trimmed().isEmpty())
        address.append(" bus ").append(busnr);

    return address;
}
