#ifndef DOCUMENTWRITER_H
#define DOCUMENTWRITER_H

// read http://doc.qt.digia.com/qq/qq27-odfwriter.html

#include <QString>
#include <QDateTime>
#include <QTextDocument>
#include <QTextDocumentWriter>
#include <QTextCursor>
#include <QSettings>

extern QSettings settings;

class DocumentWriter
{
public:
  DocumentWriter(const QString &address, const QString &legal, const QString &date);
  ~DocumentWriter();
  struct VisitLocation {
    int order;
    int distance_seconds;
    int distance_meters;
    QString Naam;
    QString Adres;
    QString Postcode;
    QString Gemeente;
    QString Telefoon;
    QString Contactpersoon;
    QString Openingsuren;
    QString Speciale_opmerkingen;
    QString Soort_vervoer;
    QString Kaartnr;
    QString Kurk_op_te_halen_zakken;
    //zakken opgehaald
    //Lege zakken afgegeven
  };
  void addVisit(const VisitLocation &location);
  void addGraph(QList<int> values,
                const QString &subtext);
  void write(const QString &fileName);

private:
  QTextDocument * const m_document;
  QTextCursor m_cursor;

  QString seconds_human_readable(int totalseconds);
};


#endif // DOCUMENTWRITER_H
