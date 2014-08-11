#ifndef TRPLDOCUMENTWRITER_H
#define TRPLDOCUMENTWRITER_H

// read http://doc.qt.digia.com/qq/qq27-odfwriter.html

#include <QString>
#include <QDateTime>
#include <QTextDocument>
#include <QTextDocumentWriter>
#include <QTextCursor>
#include <QSettings>

extern QSettings settings;

class TransportationListDocumentWriter
{
public:
  TransportationListDocumentWriter(QDate date_of_transportation, int empty_bags_of_kurk_to_bring, int empty_bags_of_kaarsresten_to_bring);
  ~TransportationListDocumentWriter();

  struct Ophaalpunt {
      int counter;
      char kaart_nr;
      QTime arrivaltime;
      QString naam;
      QString straat;
      QString nr;
      QString bus;
      QString postcode;
      QString gemeente;
      QString land;
      QString openingsuren;
      QString contactpersoon;
      QString telefoonnummer;
      QString opmerkingen;
      double kg_kurk;
      double kg_kaarsresten;
      double zakken_kurk;
      double zakken_kaarsresten;
  };

  struct Levering {
      int counter;
      char kaart_nr;
      QTime arrivaltime;
      QString naam;
      QString straat;
      QString nr;
      QString bus;
      QString postcode;
      QString gemeente;
      QString land;
      QString contactpersoon;
      QString telefoonnummer;
      double weight;
      double volume;
  };

  struct Adres {
      int counter;
      char kaart_nr;
      QTime arrivaltime;
      QString caption;
  };

  void addOphaalpunt(const Ophaalpunt &ophaalpunt);

  void addLevering(const Levering &levering);

  void addAdres(const Adres &adres);

  void write(const QString &fileName, const QString &map);

private:
  QTextDocument * const m_document;
  QTextCursor m_cursor;
  QString template_ophaling;
  QString template_levering;
  QString template_adres;

  QString getGemeente(QString gemeente, QString land);
  QString getAdres(QString straat, QString huisnr, QString busnr);
};


#endif // TRPLDOCUMENTWRITER_H
