#include "documentwriter.h"
#include <math.h>
#include <QTextTable>
#include <QPainter>
#include <QDebug>

// read http://doc.qt.digia.com/qq/qq27-odfwriter.html

DocumentWriter::DocumentWriter(const QString &address, const QString &legal, const QString &date)
    : m_document(new QTextDocument()),
    m_cursor(m_document)
{
    m_cursor.insertHtml(QString("<table><tr><td>%1</td><td>%2</td></tr><tr></table>").arg(address).arg(legal));

  QTextTableFormat tableFormat;
  tableFormat.setCellPadding(5);
  tableFormat.setHeaderRowCount(1);
  tableFormat.setBorderStyle(
              QTextFrameFormat::BorderStyle_Solid);
  tableFormat.setWidth(QTextLength(
              QTextLength::PercentageLength, 100));
  m_cursor.insertTable(1, 4, tableFormat);
  m_cursor.movePosition(QTextCursor::NextCell);
  m_cursor.insertHtml(QObject::tr("<b>Vervoerslijst</b>"));
  m_cursor.movePosition(QTextCursor::NextCell);
  m_cursor.movePosition(QTextCursor::NextCell);
  m_cursor.insertText(QString("%1").arg(date));
}

DocumentWriter::~DocumentWriter()
{
  qDebug() << "start to deconstruct DocumentWriter()";
  delete m_document;
  qDebug() << "DocumentWriter() deconstructed";
}

void DocumentWriter::addVisit(
              const DocumentWriter::VisitLocation &location)
{
  QTextTable *table = m_cursor.currentTable();
  table->appendRows(1);
  m_cursor.movePosition(QTextCursor::PreviousRow);
  m_cursor.movePosition(QTextCursor::NextCell);
  m_cursor.insertText(QObject::tr("----"));
  m_cursor.movePosition(QTextCursor::NextCell);
  m_cursor.insertText(QObject::tr("----"));
  m_cursor.movePosition(QTextCursor::NextCell);
  m_cursor.insertText(QObject::tr("----"));
  m_cursor.movePosition(QTextCursor::NextCell);
  m_cursor.insertText(QObject::tr("----"));

  table->appendRows(1);
  m_cursor.movePosition(QTextCursor::PreviousRow);
  m_cursor.movePosition(QTextCursor::NextCell);
  m_cursor.insertText(QObject::tr("Afstand:"));
  m_cursor.movePosition(QTextCursor::NextCell);
  m_cursor.insertText(QString("%1 km, %2").arg(location.distance_meters/1000).arg(seconds_human_readable(location.distance_seconds)));
  m_cursor.movePosition(QTextCursor::NextCell);
  m_cursor.movePosition(QTextCursor::NextCell);

  table->appendRows(1);
  m_cursor.movePosition(QTextCursor::PreviousRow);
  m_cursor.movePosition(QTextCursor::NextCell);
  m_cursor.movePosition(QTextCursor::NextCell);
  m_cursor.insertText(QString("Naam"));
  m_cursor.movePosition(QTextCursor::NextCell);
  m_cursor.insertText(QString("%1").arg(location.Naam));
  m_cursor.movePosition(QTextCursor::NextCell);
  m_cursor.insertText(QObject::tr("Soort vervoer"));

  table->appendRows(1);
  m_cursor.movePosition(QTextCursor::PreviousRow);
  m_cursor.movePosition(QTextCursor::NextCell);
  m_cursor.insertText(QString("%1").arg(location.order));
  m_cursor.movePosition(QTextCursor::NextCell);
  m_cursor.insertText(QString("Adres"));
  m_cursor.movePosition(QTextCursor::NextCell);
  m_cursor.insertText(QString("%1").arg(location.Adres));
  m_cursor.movePosition(QTextCursor::NextCell);
  m_cursor.insertText(QString("%1").arg(location.Soort_vervoer));

  table->appendRows(1);
  m_cursor.movePosition(QTextCursor::PreviousRow);
  m_cursor.movePosition(QTextCursor::NextCell);
  m_cursor.movePosition(QTextCursor::NextCell);
  m_cursor.insertText(QString("Postcode"));
  m_cursor.movePosition(QTextCursor::NextCell);
  m_cursor.insertText(QString("%1").arg(location.Postcode));
  m_cursor.movePosition(QTextCursor::NextCell);
  m_cursor.insertText(QString("Kaartnr"));

    table->appendRows(1);
  m_cursor.movePosition(QTextCursor::PreviousRow);
  m_cursor.movePosition(QTextCursor::NextCell);
  m_cursor.movePosition(QTextCursor::NextCell);
  m_cursor.insertText(QString("Gemeente"));
  m_cursor.movePosition(QTextCursor::NextCell);
  m_cursor.insertHtml(QString("<b>%1</b>").arg(location.Gemeente));
  m_cursor.movePosition(QTextCursor::NextCell);
  m_cursor.insertText(QString("%1").arg(location.Kaartnr));

  table->appendRows(1);
  m_cursor.movePosition(QTextCursor::PreviousRow);
  m_cursor.movePosition(QTextCursor::NextCell);
  m_cursor.movePosition(QTextCursor::NextCell);
  m_cursor.insertText(QString("Telefoon"));
  m_cursor.movePosition(QTextCursor::NextCell);
  m_cursor.insertText(QString("%1").arg(location.Telefoon));
  m_cursor.movePosition(QTextCursor::NextCell);
  m_cursor.insertText(QString("Kurk of Kaars?"));

  table->appendRows(1);
  m_cursor.movePosition(QTextCursor::PreviousRow);
  m_cursor.movePosition(QTextCursor::NextCell);
  m_cursor.movePosition(QTextCursor::NextCell);
  m_cursor.insertText(QString("Telefoon"));
  m_cursor.movePosition(QTextCursor::NextCell);
  m_cursor.insertText(QString("%1").arg(location.Telefoon));
  m_cursor.movePosition(QTextCursor::NextCell);

  table->appendRows(1);
  m_cursor.movePosition(QTextCursor::PreviousRow);
  m_cursor.movePosition(QTextCursor::NextCell);
  m_cursor.movePosition(QTextCursor::NextCell);
  m_cursor.insertText(QString("Contactpersoon"));
  m_cursor.movePosition(QTextCursor::NextCell);
  m_cursor.insertText(QString("%1").arg(location.Contactpersoon));
  m_cursor.movePosition(QTextCursor::NextCell);
  m_cursor.insertHtml(QString("<i>kurk?</i>"));

  table->appendRows(1);
  m_cursor.movePosition(QTextCursor::PreviousRow);
  m_cursor.movePosition(QTextCursor::NextCell);
  m_cursor.movePosition(QTextCursor::NextCell);
  m_cursor.insertText(QString("Openingsuren"));
  m_cursor.movePosition(QTextCursor::NextCell);
  m_cursor.insertText(QString("%1").arg(location.Openingsuren));
  m_cursor.movePosition(QTextCursor::NextCell);
  m_cursor.insertHtml(QString("Op te halen zakken: %1").arg(location.Kurk_op_te_halen_zakken));

  table->appendRows(1);
  m_cursor.movePosition(QTextCursor::PreviousRow);
  m_cursor.movePosition(QTextCursor::NextCell);
  m_cursor.movePosition(QTextCursor::NextCell);
  m_cursor.insertText(QString("Verwachtte aankomsttijd"));
  m_cursor.movePosition(QTextCursor::NextCell);
  m_cursor.insertText(QString("nog te berekenen"));
  m_cursor.movePosition(QTextCursor::NextCell);
  m_cursor.insertHtml(QString("%1").arg(location.Speciale_opmerkingen));

}

void DocumentWriter::addGraph(QList<int> values, const QString &subtext)
{
// <vvim> example to add Google Maps Image
  const int columnSize = 10;
  int width = values.count() * columnSize;
  int max = 0;
  foreach (int x, values)
    max = qMax(max, x);
  QImage image(width, 100, QImage::Format_Mono);
  QPainter painter(&image);
  painter.fillRect(0, 0, image.width(), image.height(),
                   Qt::white); // background
  for (int index = 0; index < values.count(); ++index) {
    // Adjust scale to our 100 pixel tall image:
    int height = values[index] * 100 / max;
    painter.fillRect(index * columnSize,
        image.height() - height, columnSize, height,
        Qt::black);
  }
  painter.end();

  QTextCursor cursor(m_document);
  cursor.movePosition(QTextCursor::End);
  cursor.insertText(subtext);
  cursor.insertBlock();
  cursor.insertImage(image);
}


void DocumentWriter::write(const QString &fileName)
{
  QTextDocumentWriter writer(fileName);
  writer.write(m_document);
}


QString DocumentWriter::seconds_human_readable(int totalseconds)
{
    QString human_readable = "";
    int hours = floor(totalseconds/3600.0);
    int minutes = floor(fmod(totalseconds,3600.0)/60.0);
    int seconds = fmod(totalseconds,60.0);

    /* // check for calculation error:
    int secondscheck = (((((0*24)+hours)*60) + minutes)*60) + seconds;
    if (secondscheck == totalseconds)
        qDebug() << "OK";
    else
        qDebug() << "[error]";
    */

    human_readable.sprintf("%01du %02dm %02ds", hours, minutes, seconds);
    return human_readable;
}
