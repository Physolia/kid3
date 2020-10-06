/**
 * \file discogsimporter.cpp
 * Discogs importer.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 13 Oct 2006
 *
 * Copyright (C) 2006-2020  Urs Fleisch
 *
 * This file is part of Kid3.
 *
 * Kid3 is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Kid3 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "discogsimporter.h"
#include <QUrl>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include "serverimporterconfig.h"
#include "trackdatamodel.h"
#include "discogsconfig.h"
#include "config.h"
#include "genres.h"

namespace {

/**
 * Remove trailing stars and numbers like (2) from a string.
 *
 * @param str string
 *
 * @return fixed up string.
 */
QString fixUpArtist(QString str)
{
  str.replace(QRegExp(QLatin1String(",(\\S)")), QLatin1String(", \\1"));
  str.replace(QLatin1String("* / "), QLatin1String(" / "));
  str.replace(QLatin1String("*,"), QLatin1String(","));
  str.remove(QRegExp(QLatin1String("\\*$")));
  str.remove(QRegExp(QLatin1String(R"([*\s]*\(\d+\)\(tracks:[^)]+\))")));
  str.replace(QRegExp(
    QLatin1String("[*\\s]*\\((?:\\d+|tracks:[^)]+)\\)(\\s*/\\s*,|\\s*&amp;|"
                  "\\s*And|\\s*and)")),
    QLatin1String("\\1"));
  str.remove(QRegExp(QLatin1String(R"([*\s]*\((?:\d+|tracks:[^)]+)\)$)")));
  return ServerImporter::removeHtml(str);
}

/**
 * Create a string with artists contained in an artist list.
 * @param artists list containing artist maps
 * @return string with artists joined appropriately.
 */
QString getArtistString(const QJsonArray& artists)
{
  QString artist;
  if (!artists.isEmpty()) {
    QString join;
    for (const auto& val : artists) {
      auto map = val.toObject();
      if (!artist.isEmpty()) {
        artist += join;
      }
      artist += fixUpArtist(map.value(QLatin1String("name")).toString());
      join = map.value(QLatin1String("join")).toString();
      if (join.isEmpty() || join == QLatin1String(",")) {
        join = QLatin1String(", ");
      } else {
        join = QLatin1Char(' ') + join + QLatin1Char(' ');
      }
    }
  }
  return artist;
}

/**
 * Add involved people to a frame.
 * The format used is (should be converted according to tag specifications):
 * involvee 1 (involvement 1)\n
 * involvee 2 (involvement 2)\n
 * ...
 * involvee n (involvement n)
 *
 * @param frames      frame collection
 * @param type        type of frame
 * @param involvement involvement (e.g. instrument)
 * @param involvee    name of involvee (e.g. musician)
 */
void addInvolvedPeople(
  FrameCollection& frames, Frame::Type type,
  const QString& involvement, const QString& involvee)
{
  QString value = frames.getValue(type);
  if (!value.isEmpty()) value += Frame::stringListSeparator();
  value += involvement;
  value += Frame::stringListSeparator();
  value += involvee;
  frames.setValue(type, value);
}

/**
 * Get frame for a role.
 * @param role role or credit of involved people, can be adapted to canonical
 * value
 * @return suitable frame type, Frame::FT_UnknownFrame if not found.
 */
Frame::Type frameTypeForRole(QString& role)
{
  static const struct {
    const char* credit;
    Frame::Type type;
  } creditToType[] = {
    { "Composed By", Frame::FT_Composer },
    { "Conductor", Frame::FT_Conductor },
    { "Orchestra", Frame::FT_AlbumArtist },
    { "Lyrics By", Frame::FT_Lyricist },
    { "Written-By", Frame::FT_Author },
    { "Written By", Frame::FT_Author },
    { "Remix", Frame::FT_Remixer },
    { "Music By", Frame::FT_Composer },
    { "Songwriter", Frame::FT_Composer }
  };
  for (const auto& c2t : creditToType) {
    if (role.contains(QString::fromLatin1(c2t.credit))) {
      return c2t.type;
    }
  }

  static const struct {
    const char* credit;
    const char* arrangement;
  } creditToArrangement[] = {
    { "Arranged By", "Arranger" },
    { "Mixed By", "Mixer" },
    { "DJ Mix", "DJMixer" },
    { "Dj Mix", "DJMixer" },
    { "Engineer", "Engineer" },
    { "Mastered By", "Engineer" },
    { "Producer", "Producer" },
    { "Co-producer", "Producer" },
    { "Executive Producer", "Producer" }
  };
  for (const auto& c2a : creditToArrangement) {
    if (role.contains(
          QString::fromLatin1(c2a.credit))) {
      role = QString::fromLatin1(c2a.arrangement);
      return Frame::FT_Arranger;
    }
  }

  static const char* const instruments[] = {
    "Performer", "Vocals", "Voice", "Featuring", "Choir", "Chorus",
    "Baritone", "Tenor", "Rap", "Scratches", "Drums", "Percussion",
    "Keyboards", "Cello", "Piano", "Organ", "Synthesizer", "Keys",
    "Wurlitzer", "Rhodes", "Harmonica", "Xylophone", "Guitar", "Bass",
    "Strings", "Violin", "Viola", "Banjo", "Harp", "Mandolin",
    "Clarinet", "Horn", "Cornet", "Flute", "Oboe", "Saxophone",
    "Trumpet", "Tuba", "Trombone"
  };
  for (auto instrument : instruments) {
    if (role.contains(QString::fromLatin1(instrument))) {
      return Frame::FT_Performer;
    }
  }

  return Frame::FT_UnknownFrame;
}

/**
 * Set tags from a string with credits lines.
 * The string must have lines like "Composed By - Iommi", separated by \\n.
 *
 * @param str    credits string
 * @param frames tags will be added to these frames
 *
 * @return true if credits found.
 */
bool parseCredits(const QString& str, FrameCollection& frames)
{
  bool result = false;
  QStringList lines = str.split(QLatin1Char('\n'));
  for (auto it = lines.constBegin(); it != lines.constEnd(); ++it) {
    int nameStart = (*it).indexOf(QLatin1String(" - "));
    if (nameStart != -1) {
      const QStringList names = (*it).mid(nameStart + 3).split(QLatin1String(", "));
      QString name;
      for (const QString& namesPart : names) {
        if (!name.isEmpty()) {
          name += QLatin1String(", ");
        }
        name += fixUpArtist(namesPart);
      }
      QStringList credits = (*it).left(nameStart).split(QLatin1String(", "));
      for (auto cit = credits.constBegin(); cit != credits.constEnd(); ++cit) {
        QString role = *cit;
        Frame::Type frameType = frameTypeForRole(role);
        if (frameType == Frame::FT_Arranger ||
            frameType == Frame::FT_Performer) {
          addInvolvedPeople(frames, frameType, role, name);
          result = true;
        } else if (frameType != Frame::FT_UnknownFrame) {
          frames.setValue(frameType, name);
          result = true;
        }
      }
    }
  }
  return result;
}

/**
 * Add name to frame with credits.
 * @param frames frame collection
 * @param type   type of frame
 * @param name   name of person to credit
 */
void addCredit(FrameCollection& frames, Frame::Type type, const QString& name)
{
  QString value = frames.getValue(type);
  if (!value.isEmpty()) value += QLatin1String(", ");
  value += name;
  frames.setValue(type, value);
}

/**
 * Stores information about extra artists.
 * The information can be used to add frames to the appropriate tracks.
 */
class ExtraArtist {
public:
  /**
   * Constructor.
   * @param obj JSON object containing extra artist information
   */
  explicit ExtraArtist(const QJsonObject& varMap);

  /**
   * Add extra artist information to frames.
   * @param frames   frame collection
   * @param trackPos optional position, the extra artist information will
   *                 only be added if this track position is listed in the
   *                 track restrictions or is empty
   */
  void addToFrames(FrameCollection& frames,
                   const QString& trackPos = QString()) const;

  /**
   * Check if extra artist information is only valid for a subset of the tracks.
   * @return true if extra artist has track restriction.
   */
  bool hasTrackRestriction() const { return !m_tracks.isEmpty(); }

private:
  QString m_name;
  QString m_role;
  QStringList m_tracks;
};

/**
 * Constructor.
 * @param obj JSON object containing extra artist information
 */
ExtraArtist::ExtraArtist(const QJsonObject& obj) :
  m_name(fixUpArtist(obj.value(QLatin1String("name")).toString())),
  m_role(obj.value(QLatin1String("role")).toString().trimmed())
{
  static const QRegExp tracksSepRe(QLatin1String(",\\s*"));
  QString tracks = obj.value(QLatin1String("tracks")).toString();
  if (!tracks.isEmpty()) {
    m_tracks = tracks.split(tracksSepRe);
  }
}

/**
 * Add extra artist information to frames.
 * @param frames   frame collection
 * @param trackPos optional position, the extra artist information will
 *                 only be added if this track position is listed in the
 *                 track restrictions or is empty
 */
void ExtraArtist::addToFrames(FrameCollection& frames,
                              const QString& trackPos) const
{
  if (!trackPos.isEmpty() && !m_tracks.contains(trackPos))
    return;

  QString role = m_role;
  Frame::Type frameType = frameTypeForRole(role);
  if (frameType == Frame::FT_Arranger ||
      frameType == Frame::FT_Performer) {
    addInvolvedPeople(frames, frameType, role, m_name);
  } else if (frameType != Frame::FT_UnknownFrame) {
    addCredit(frames, frameType, m_name);
  }
}

}


/**
 * Abstract base class for Discogs importer implementations.
 */
class DiscogsImporter::BaseImpl {
public:
  BaseImpl(DiscogsImporter* importer, const char* url);
  virtual ~BaseImpl();

  virtual void parseFindResults(const QByteArray& searchStr) = 0;
  virtual void parseAlbumResults(const QByteArray& albumStr) = 0;
  virtual void sendFindQuery(
      const ServerImporterConfig* cfg,
      const QString& artist, const QString& album) = 0;
  virtual void sendTrackListQuery(
      const ServerImporterConfig* cfg,
      const QString& cat, const QString& id) = 0;

  AlbumListModel* albumListModel() { return m_importer->m_albumListModel; }
  TrackDataModel* trackDataModel() { return m_importer->m_trackDataModel; }
  QMap<QByteArray, QByteArray>& headers() { return m_discogsHeaders; }

protected:
  QMap<QByteArray, QByteArray> m_discogsHeaders;
  DiscogsImporter* m_importer;
  const char* const m_discogsServer;
};

DiscogsImporter::BaseImpl::BaseImpl(DiscogsImporter* importer, const char* url)
  : m_importer(importer), m_discogsServer(url)
{
}

DiscogsImporter::BaseImpl::~BaseImpl()
{
}


/**
 * Importer implementation to import HTML data from the Discogs web site.
 */
class DiscogsImporter::HtmlImpl : public DiscogsImporter::BaseImpl {
public:
  explicit HtmlImpl(DiscogsImporter* importer);
  virtual ~HtmlImpl() override;

  virtual void parseFindResults(const QByteArray& searchStr) override;
  virtual void parseAlbumResults(const QByteArray& albumStr) override;
  virtual void sendFindQuery(
      const ServerImporterConfig* cfg,
      const QString& artist, const QString& album) override;
  virtual void sendTrackListQuery(
      const ServerImporterConfig* cfg,
      const QString& cat, const QString& id) override;
};

DiscogsImporter::HtmlImpl::HtmlImpl(DiscogsImporter* importer)
  : BaseImpl(importer, "www.discogs.com")
{
  m_discogsHeaders["User-Agent"] =
      "Mozilla/5.0 (iPhone; U; CPU iPhone OS 4_3_2 like Mac OS X; en-us) "
      "AppleWebKit/533.17.9 (KHTML, like Gecko) Version/5.0.2 Mobile/8H7 "
      "Safari/6533.18.5";
  m_discogsHeaders["Cookie"] = "language2=en";
}

DiscogsImporter::HtmlImpl::~HtmlImpl()
{
}

void DiscogsImporter::HtmlImpl::parseFindResults(const QByteArray& searchStr)
{
  // releases have the format:
  // <a href="/artist/256076-Amon-Amarth">Amon Amarth</a>         </span> -
  // <a class="search_result_title " href="/Amon-Amarth-The-Avenger/release/398878" data-followable="true">The Avenger</a>
  QString str = QString::fromUtf8(searchStr);
  QRegExp idTitleRe(QLatin1String(
      "<a href=\"/artist/[^>]+>([^<]+)</a>[^-]*-"
      "\\s*<a class=\"search_result_title[ \"]+href=\"/([^/]*/?release)/"
      "([0-9]+)\"[^>]*>([^<]+)</a>(.*card_actions)"));
  idTitleRe.setMinimal(true);

  QRegExp yearRe(QLatin1String("<span class=\"card_release_year\">([^<]+)</span>"));
  QRegExp formatRe(QLatin1String("<span class=\"card_release_format\">([^<]+)</span>"));

  albumListModel()->clear();
  int pos = 0;
  while ((pos = idTitleRe.indexIn(str, pos)) != -1) {
    int len = idTitleRe.matchedLength();
    QString artist = fixUpArtist(idTitleRe.cap(1).trimmed());
    QString title = removeHtml(idTitleRe.cap(4).trimmed());
    if (!title.isEmpty()) {
      QString result(artist + QLatin1String(" - ") + title);

      QString metadata = idTitleRe.cap(5);
      if (yearRe.indexIn(metadata)) {
          result.append(QLatin1String(" (") + yearRe.cap(1).trimmed() + QLatin1Char(')'));
      }

      if (formatRe.indexIn(metadata)) {
          result.append(QLatin1String(" [") + formatRe.cap(1).trimmed() + QLatin1Char(']'));
      }

      albumListModel()->appendItem(
        result,
        idTitleRe.cap(2),
        idTitleRe.cap(3));
    }
    pos += len;
  }
}

void DiscogsImporter::HtmlImpl::parseAlbumResults(const QByteArray& albumStr)
{
  QRegExp nlSpaceRe(QLatin1String("[\r\n]+\\s*"));
  QRegExp atDiscogsRe(QLatin1String("\\s*\\([^)]+\\) (?:at|\\|) Discogs\n?$"));
  QString str = QString::fromUtf8(albumStr);

  FrameCollection framesHdr;
  int start, end;
  const bool standardTags = m_importer->getStandardTags();
  if (standardTags) {
    /*
     * artist and album can be found in the title:
<title>Amon Amarth - The Avenger (CD, Album, Dig) at Discogs</title>
     */
    start = str.indexOf(QLatin1String("<title>"));
    if (start >= 0) {
      start += 7; // skip <title>
      end = str.indexOf(QLatin1String("</title>"), start);
      if (end > start) {
        QString titleStr = str.mid(start, end - start);
        titleStr.replace(atDiscogsRe, QLatin1String(""));
        // reduce new lines and space after them
        titleStr.replace(nlSpaceRe, QLatin1String(" "));
        start = 0;
        end = titleStr.indexOf(QLatin1String(" - "), start);
        if (end > start) {
          framesHdr.setArtist(fixUpArtist(titleStr.mid(start, end - start)));
          start = end + 3; // skip " - "
        }
        framesHdr.setAlbum(removeHtml(titleStr.mid(start)));
      }
    }
    /*
     * the year can be found in "Released:"
<div class="head">Released:</div><div class="content">02 Nov 1999</div>
     */
    start = str.indexOf(QLatin1String("Released:<"));
    if (start >= 0) {
      start += 9; // skip "Released:"
      end = str.indexOf(QLatin1String("</div>"), start + 1);
      if (end > start) {
        QString yearStr = str.mid(start, end - start);
        // strip new lines and space after them
        yearStr.replace(nlSpaceRe, QLatin1String(""));
        yearStr = removeHtml(yearStr); // strip HTML tags and entities
        // this should skip day and month numbers
        QRegExp yearRe(QLatin1String("(\\d{4})"));
        if (yearRe.indexIn(yearStr) >= 0) {
          framesHdr.setYear(yearRe.cap(1).toInt());
        }
      }
    }
    /*
     * the genre can be found in "Genre:" or "Style:" (lines with only whitespace
     *  in between):
<div class="head">Genre:</div><div class="content">
      Rock
</div>
<div class="head">Style:</div><div class="content">
    Viking Metal,
    Death Metal
</div>
     */
    // All genres found are checked for an ID3v1 number, starting with those
    // in the Style field.
    QStringList genreList;
    static const char* const fields[] = { "Style:", "Genre:" };
    for (auto field : fields) {
      start = str.indexOf(QString::fromLatin1(field) + QLatin1Char('<'));
      if (start >= 0) {
        start += qstrlen(field); // skip field
        end = str.indexOf(QLatin1String("</div>"), start + 1);
        if (end > start) {
          QString genreStr = str.mid(start, end - start);
          // strip new lines and space after them
          genreStr.replace(nlSpaceRe, QLatin1String(""));
          genreStr = removeHtml(genreStr); // strip HTML tags and entities
          if (genreStr.indexOf(QLatin1Char(',')) >= 0) {
            genreList += genreStr.split(QRegExp(QLatin1String(",\\s*")));
          } else {
            if (!genreStr.isEmpty()) {
              genreList += genreStr;
            }
          }
        }
      }
    }
    QStringList genres;
    for (auto it = genreList.begin(); it != genreList.end();) {
      int genreNum = Genres::getNumber(*it);
      if (genreNum != 255) {
        genres.append(QString::fromLatin1(Genres::getName(genreNum)));
        it = genreList.erase(it);
      } else {
        ++it;
      }
    }
    genres.append(genreList);
    if (!genres.isEmpty()) {
      framesHdr.setGenre(genres.join(Frame::stringListSeparator()));
    }
  }

  const bool additionalTags = m_importer->getAdditionalTags();
  if (additionalTags) {
    /*
     * publisher can be found in "Label:"
     */
    start = str.indexOf(QLatin1String("Label:<"));
    if (start >= 0) {
      start += 6; // skip "Label:"
      end = str.indexOf(QLatin1String("</div>"), start + 1);
      if (end > start) {
        QString labelStr = str.mid(start, end - start);
        // strip new lines and space after them
        labelStr.replace(nlSpaceRe, QLatin1String(""));
        labelStr = fixUpArtist(labelStr);
        QRegExp catNoRe(QLatin1String(" \\s*(?:&lrm;)?- +(\\S[^,]*[^, ])"));
        int catNoPos = catNoRe.indexIn(labelStr);
        if (catNoPos != -1) {
          QString catNo = catNoRe.cap(1);
          labelStr.truncate(catNoPos);
          if (!catNo.isEmpty()) {
            framesHdr.setValue(Frame::FT_CatalogNumber, catNo);
          }
        }
        if (labelStr != QLatin1String("Not On Label")) {
          framesHdr.setValue(Frame::FT_Publisher, fixUpArtist(labelStr));
        }
      }
    }

    /*
     * media can be found in "Format:"
     */
    start = str.indexOf(QLatin1String("Format:<"));
    if (start >= 0) {
      start += 7; // skip "Format:"
      end = str.indexOf(QLatin1String("</div>"), start + 1);
      if (end > start) {
        QString mediaStr = str.mid(start, end - start);
        // strip new lines and space after them
        mediaStr.replace(nlSpaceRe, QLatin1String(""));
        mediaStr = removeHtml(mediaStr); // strip HTML tags and entities
        framesHdr.setValue(Frame::FT_Media, mediaStr);
      }
    }

    /*
     * Release country can be found in "Country:"
     */
    start = str.indexOf(QLatin1String("Country:<"));
    if (start >= 0) {
      start += 8; // skip "Country:"
      end = str.indexOf(QLatin1String("</div>"), start + 1);
      if (end > start) {
        QString countryStr = str.mid(start, end - start);
        // strip new lines and space after them
        countryStr.replace(nlSpaceRe, QLatin1String(""));
        countryStr = removeHtml(countryStr); // strip HTML tags and entities
        framesHdr.setValue(Frame::FT_ReleaseCountry, countryStr);
      }
    }

    /*
     * credits can be found in "Credits"
     */
    start = str.indexOf(QLatin1String(">Credits</h"));
    if (start >= 0) {
      start += 13; // skip "Credits" plus end of element (e.g. "3>")
      end = str.indexOf(QLatin1String("</div>"), start + 1);
      if (end > start) {
        QString creditsStr = str.mid(start, end - start);
        // strip new lines and space after them
        creditsStr.replace(nlSpaceRe, QLatin1String(""));
        creditsStr.replace(QLatin1String("<br />"), QLatin1String("\n"));
        creditsStr.replace(QLatin1String("</li>"), QLatin1String("\n"));
        creditsStr.replace(QLatin1String("&ndash;"), QLatin1String(" - "));
        creditsStr = removeHtml(creditsStr); // strip HTML tags and entities
        parseCredits(creditsStr, framesHdr);
      }
    }
  }

  ImportTrackDataVector trackDataVector(trackDataModel()->getTrackData());
  trackDataVector.setCoverArtUrl(QUrl());
  if (m_importer->getCoverArt()) {
    /*
     * cover art can be found in image source
     */
    // Using a raw string literal in the next line would disturb doxygen.
    start = str.indexOf(QLatin1String("<meta property=\"og:image\" content=\""));
    if (start >= 0) {
      start += 35;
      end = str.indexOf(QLatin1String("\""), start);
      if (end > start) {
        trackDataVector.setCoverArtUrl(QUrl(str.mid(start, end - start)));
      }
    }
  }

  /*
   * album tracks have the format (lines with only whitespace in between):
<div id="tracklist" class="section tracklist" data-toggle="tracklist">
                    <td class="tracklist_track_pos">1</td>
<span class="tracklist_track_title" itemprop="name">Bleed For Ancient Gods</span>
        <td width="25" class="tracklist_track_duration">
            <meta itemprop="duration" content="PT0H04M31S">
            <span>4:31</span>
        </td>

<h1>Tracklist</h1>
<div class="section_content">
<table>
  <tr class="first">
    <td class="track_pos">1</td>
      <td>&nbsp;</td>
    <td class="track_title">Bleed For Ancient Gods</td>
    <td class="track_duration">4:31</td>
    <td class="track_itunes"></td>
  </tr>
  <tr>
    <td class="track_pos">2</td>
(..)
</table>
   *
   * Variations: strange track numbers, no durations, links instead of tracks,
   * only "track" instead of "track_title", align attribute in "track_duration"
   */
  start = str.indexOf(QLatin1String("class=\"section tracklist\""));
  if (start >= 0) {
    end = str.indexOf(QLatin1String("</table>"), start);
    if (end > start) {
      str = str.mid(start, end - start);
      // strip whitespace
      str.replace(nlSpaceRe, QLatin1String(""));

      FrameCollection frames(framesHdr);
      QRegExp posRe(QLatin1String(
        R"(<td [^>]*class="tracklist_track_pos">(\d+)</td>)"));
      QRegExp artistsRe(QLatin1String(
        "class=\"tracklist_content_multi_artist_dash\">&ndash;</span>"
        "<a href=\"/artist/[^>]+>([^<]+)</a>"));
      QRegExp moreArtistsRe(QLatin1String(
        "^([^<>]+)<a href=\"/artist/[^>]+>([^<]+)</a>"));
      QRegExp titleRe(QLatin1String(
        "class=\"tracklist_track_title\"[^>]*>([^<]+)<"));
      QRegExp durationRe(QLatin1String(
        "<td [^>]*class=\"tracklist_track_duration\"[^>]*>(?:<meta[^>]*>)?"
        "(?:<span>)?(\\d+):(\\d+)</"));
      QRegExp indexRe(QLatin1String("<td class=\"track_index\">([^<]+)$"));
      QRegExp rowEndRe(QLatin1String(R"(</td>[\s\r\n]*</tr>)"));
      auto it = trackDataVector.begin();
      bool atTrackDataListEnd = (it == trackDataVector.end());
      int trackNr = 1;
      start = 0;
      while ((end = rowEndRe.indexIn(str, start)) > start) {
        QString trackDataStr = str.mid(start, end - start);
        QString title;
        int duration = 0;
        int pos = trackNr;
        if (titleRe.indexIn(trackDataStr) >= 0) {
          title = removeHtml(titleRe.cap(1));
        }
        if (durationRe.indexIn(trackDataStr) >= 0) {
          duration = durationRe.cap(1).toInt() * 60 +
            durationRe.cap(2).toInt();
        }
        if (posRe.indexIn(trackDataStr) >= 0) {
          pos = posRe.cap(1).toInt();
        }
        if (additionalTags) {
          if (artistsRe.indexIn(trackDataStr) >= 0) {
            // use the artist in the header as the album artist
            // and the artist in the track as the artist
            QString artist(fixUpArtist(artistsRe.cap(1)));
            // Look if there are more artists
            int artistEndPos = artistsRe.pos() + artistsRe.matchedLength();
            while (moreArtistsRe.indexIn(
                     trackDataStr, artistEndPos, QRegExp::CaretAtOffset) >=
                   artistEndPos) {
              artist += moreArtistsRe.cap(1);
              artist += fixUpArtist(moreArtistsRe.cap(2));
              int endPos = moreArtistsRe.pos() + moreArtistsRe.matchedLength();
              if (endPos <= artistEndPos) // must be true for regexp
                break;
              artistEndPos = endPos;
            }
            if (standardTags) {
              frames.setArtist(artist);
            }
            frames.setValue(Frame::FT_AlbumArtist, framesHdr.getArtist());
          }
        }
        start = end + 10; // skip </td></tr>
        if (indexRe.indexIn(trackDataStr) >= 0) {
          if (additionalTags) {
            QString subtitle(removeHtml(indexRe.cap(1)));
            framesHdr.setValue(Frame::FT_Description, subtitle);
            frames.setValue(Frame::FT_Description, subtitle);
          }
          continue;
        }
        if (additionalTags) {
          int blockquoteStart =
              trackDataStr.indexOf(QLatin1String("<blockquote>"));
          if (blockquoteStart >= 0) {
            blockquoteStart += 12;
            int blockquoteEnd =
                trackDataStr.indexOf(QLatin1String("</blockquote>"),
                                     blockquoteStart);
            if (blockquoteEnd == -1) {
              // If the element is not correctly closed, search for </span>
              blockquoteEnd = trackDataStr.indexOf(QLatin1String("</span>"),
                                                   blockquoteStart);
            }
            if (blockquoteEnd > blockquoteStart) {
              QString blockquoteStr(trackDataStr.mid(blockquoteStart,
                blockquoteEnd - blockquoteStart));
              // additional track info like "Music By, Lyrics By - "
              blockquoteStr.replace(QLatin1String("<br />"),
                                    QLatin1String("\n"));
              blockquoteStr.replace(QLatin1String("</li>"),
                                    QLatin1String("\n"));
              blockquoteStr.replace(QLatin1String("</span>"),
                                    QLatin1String("\n"));
              blockquoteStr.replace(QLatin1String(" &ndash; "),
                                    QLatin1String(" - "));
              blockquoteStr.replace(QLatin1String("&ndash;"),
                                    QLatin1String(" - "));
              blockquoteStr = removeHtml(blockquoteStr);
              parseCredits(blockquoteStr, frames);
            }
          }
        }

        if (!title.isEmpty() || duration != 0) {
          if (standardTags) {
            frames.setTrack(pos);
            frames.setTitle(title);
          }
          if (atTrackDataListEnd) {
            ImportTrackData trackData;
            trackData.setFrameCollection(frames);
            trackData.setImportDuration(duration);
            trackDataVector.push_back(trackData);
          } else {
            while (!atTrackDataListEnd && !it->isEnabled()) {
              ++it;
              atTrackDataListEnd = (it == trackDataVector.end());
            }
            if (!atTrackDataListEnd) {
              (*it).setFrameCollection(frames);
              (*it).setImportDuration(duration);
              ++it;
              atTrackDataListEnd = (it == trackDataVector.end());
            }
          }
          ++trackNr;
        }
        frames = framesHdr;
      }

      // handle redundant tracks
      frames.clear();
      while (!atTrackDataListEnd) {
        if (it->isEnabled()) {
          if ((*it).getFileDuration() == 0) {
            it = trackDataVector.erase(it);
          } else {
            (*it).setFrameCollection(frames);
            (*it).setImportDuration(0);
            ++it;
          }
        } else {
          ++it;
        }
        atTrackDataListEnd = (it == trackDataVector.end());
      }
    }
  }
  trackDataModel()->setTrackData(trackDataVector);
}

void DiscogsImporter::HtmlImpl::sendFindQuery(
  const ServerImporterConfig*,
  const QString& artist, const QString& album)
{
  /*
   * Query looks like this:
   * http://www.discogs.com/search/?q=amon+amarth+avenger&type=release&layout=sm
   */
  m_importer->sendRequest(QString::fromLatin1(m_discogsServer),
              QString(QLatin1String("/search/?q=")) +
              encodeUrlQuery(artist + QLatin1Char(' ') + album) +
              QLatin1String("&type=release&layout=sm"), QLatin1String("https"),
              m_discogsHeaders);
}

void DiscogsImporter::HtmlImpl::sendTrackListQuery(
  const ServerImporterConfig*, const QString& cat, const QString& id)
{
  /*
   * Query looks like this:
   * http://www.discogs.com/release/761529
   */
  m_importer->sendRequest(QString::fromLatin1(m_discogsServer), QLatin1Char('/') +
              QString::fromLatin1(QUrl::toPercentEncoding(cat)) +
              QLatin1Char('/') + id, QLatin1String("https"), m_discogsHeaders);
}


/**
 * Importer implementation to import JSON data via the Discogs API.
 * A token is required to get data from the Discogs API.
 */
class DiscogsImporter::JsonImpl : public DiscogsImporter::BaseImpl {
public:
  explicit JsonImpl(DiscogsImporter* importer);
  virtual ~JsonImpl() override;

  virtual void parseFindResults(const QByteArray& searchStr) override;
  virtual void parseAlbumResults(const QByteArray& albumStr) override;
  virtual void sendFindQuery(
      const ServerImporterConfig* cfg,
      const QString& artist, const QString& album) override;
  virtual void sendTrackListQuery(
      const ServerImporterConfig* cfg,
      const QString& cat, const QString& id) override;
};

DiscogsImporter::JsonImpl::JsonImpl(DiscogsImporter* importer)
  : BaseImpl(importer, "api.discogs.com")
{
  m_discogsHeaders["User-Agent"] = "Kid3/" VERSION
      " +https://kid3.kde.org";
}

DiscogsImporter::JsonImpl::~JsonImpl()
{
}

void DiscogsImporter::JsonImpl::parseFindResults(const QByteArray& searchStr)
{
  // search results have the format (JSON, simplified):
  // {"results": [{"style": ["Heavy Metal"], "title": "Wizard (23) - Odin",
  //               "type": "release", "id": 2487778}]}
  albumListModel()->clear();
  auto doc = QJsonDocument::fromJson(searchStr);
  if (!doc.isNull()) {
    auto obj = doc.object();
    const auto results = obj.value(QLatin1String("results")).toArray();
    for (const auto& val : results) {
      auto result = val.toObject();
      QString title =
          fixUpArtist(result.value(QLatin1String("title")).toString());
      if (!title.isEmpty()) {
        QString year = result.value(QLatin1String("year")).toString().trimmed();
        if (!year.isEmpty()) {
          title += QLatin1String(" (") + year + QLatin1Char(')');
        }
        const auto fmts = result.value(QLatin1String("format")).toArray();
        if (!fmts.isEmpty()) {
          QStringList formats;
          for (const auto& fmt : fmts) {
            QString format = fmt.toString().trimmed();
            if (!format.isEmpty()) {
              formats.append(format);
            }
          }
          if (!formats.isEmpty()) {
            title += QLatin1String(" [") +
                formats.join(QLatin1String(", ")) +
                QLatin1Char(']');
          }
        }
        albumListModel()->appendItem(
          title,
          QLatin1String("releases"),
          QString::number(result.value(QLatin1String("id")).toInt()));
      }
    }
  }
}

void DiscogsImporter::JsonImpl::parseAlbumResults(const QByteArray& albumStr)
{
  // releases have the format (JSON, simplified):
  // { "styles": ["Heavy Metal"],
  //   "labels": [{"name": "LMP"}],
  //   "year": 2003,
  //   "artists": [{"name": "Wizard (23)"}],
  //   "images": [
  //   { "uri": "http://api.discogs.com/image/R-2487778-1293847958.jpeg",
  //     "type": "primary" },
  //   { "uri": "http://api.discogs.com/image/R-2487778-1293847967.jpeg",
  //     "type": "secondary" }],
  //   "id": 2487778,
  //   "genres": ["Rock"],
  //   "thumb": "http://api.discogs.com/image/R-150-2487778-1293847958.jpeg",
  //   "extraartists": [],
  //   "title": "Odin",
  //   "tracklist": [
  //     {"duration": "5:19", "position": "1", "title": "The Prophecy"},
  //     {"duration": "", "position": "Video", "title": "Betrayer"}
  //   ],
  //   "released": "2003",
  //   "formats": [{"name": "CD"}]
  // }
  auto doc = QJsonDocument::fromJson(albumStr);
  if (doc.isNull()) {
    return;
  }
  auto map = doc.object();
  if (map.isEmpty()) {
    return;
  }

  QRegExp discTrackPosRe(QLatin1String("(\\d+)-(\\d+)"));
  QRegExp yearRe(QLatin1String("^\\d{4}-\\d{2}"));
  QList<ExtraArtist> trackExtraArtists;
  ImportTrackDataVector trackDataVector(trackDataModel()->getTrackData());
  FrameCollection framesHdr;
  const bool standardTags = m_importer->getStandardTags();
  if (standardTags) {
    framesHdr.setAlbum(map.value(QLatin1String("title")).toString().trimmed());
    framesHdr.setArtist(getArtistString(map.value(QLatin1String("artists"))
                                        .toArray()));

    // The year can be found in "released".
    QString released(map.value(QLatin1String("released")).toString());
    if (yearRe.indexIn(released) == 0) {
      released.truncate(4);
    }
    framesHdr.setYear(released.toInt());

    // The genre can be found in "genre" or "style".
    // All genres found are checked for an ID3v1 number, starting with those
    // in the style field.
    const auto genreList = map.value(QLatin1String("styles")).toArray() +
        map.value(QLatin1String("genres")).toArray();
    QStringList genres, customGenres;
    for (const auto& val : genreList) {
      QString genre = val.toString().trimmed();
      if (!genre.isEmpty()) {
        int genreNum = Genres::getNumber(genre);
        if (genreNum != 255) {
          genres.append(QString::fromLatin1(Genres::getName(genreNum)));
        } else {
          customGenres.append(genre);
        }
      }
    }
    genres.append(customGenres);
    if (!genres.isEmpty()) {
      framesHdr.setGenre(genres.join(Frame::stringListSeparator()));
    }
  }

  trackDataVector.setCoverArtUrl(QUrl());
  const bool coverArt = m_importer->getCoverArt();
  if (coverArt) {
    // Cover art can be found in "images"
    auto images = map.value(QLatin1String("images")).toArray();
    if (!images.isEmpty()) {
      trackDataVector.setCoverArtUrl(
            QUrl(images.first().toObject().value(QLatin1String("uri"))
                 .toString()));
    }
  }

  const bool additionalTags = m_importer->getAdditionalTags();
  if (additionalTags) {
    // Publisher can be found in "label"
    auto labels = map.value(QLatin1String("labels")).toArray();
    if (!labels.isEmpty()) {
      auto firstLabelMap = labels.first().toObject();
      framesHdr.setValue(Frame::FT_Publisher,
          fixUpArtist(firstLabelMap.value(QLatin1String("name")).toString()));
      QString catNo = firstLabelMap.value(QLatin1String("catno"))
          .toString().trimmed();
      if (!catNo.isEmpty() && catNo.toLower() != QLatin1String("none")) {
        framesHdr.setValue(Frame::FT_CatalogNumber, catNo);
      }
    }
    // Media can be found in "formats"
    auto formats = map.value(QLatin1String("formats")).toArray();
    if (!formats.isEmpty()) {
      framesHdr.setValue(Frame::FT_Media,
                         formats.first().toObject().value(QLatin1String("name"))
                         .toString().trimmed());
    }
    // Credits can be found in "extraartists"
    const auto extraartists = map.value(QLatin1String("extraartists")).toArray();
    if (!extraartists.isEmpty()) {
      for (const auto& val : extraartists) {
        ExtraArtist extraArtist(val.toObject());
        if (extraArtist.hasTrackRestriction()) {
          trackExtraArtists.append(extraArtist);
        } else {
          extraArtist.addToFrames(framesHdr);
        }
      }
    }
    // Release country can be found in "country"
    QString country(map.value(QLatin1String("country")).toString().trimmed());
    if (!country.isEmpty()) {
      framesHdr.setValue(Frame::FT_ReleaseCountry, country);
    }
  }

  FrameCollection frames(framesHdr);
  ImportTrackDataVector::iterator it = trackDataVector.begin();
  bool atTrackDataListEnd = (it == trackDataVector.end());
  int trackNr = 1;
  const auto trackList = map.value(QLatin1String("tracklist")).toArray();

  // Check if all positions are empty.
  bool allPositionsEmpty = true;
  for (const auto& val : trackList) {
    if (!val.toObject().value(QLatin1String("position")).toString().isEmpty()) {
      allPositionsEmpty = false;
      break;
    }
  }

  for (const auto& val : trackList) {
    auto track = val.toObject();

    QString position(track.value(QLatin1String("position")).toString());
    bool ok;
    int pos = position.toInt(&ok);
    if (!ok) {
      if (discTrackPosRe.exactMatch(position)) {
        if (additionalTags) {
          frames.setValue(Frame::FT_Disc, discTrackPosRe.cap(1));
        }
        pos = discTrackPosRe.cap(2).toInt();
      } else {
        pos = trackNr;
      }
    }
    QString title(track.value(QLatin1String("title")).toString().trimmed());

    const QStringList durationHms = track.value(QLatin1String("duration"))
        .toString().split(QLatin1Char(':'));
    int duration = 0;
    for (const auto& val : durationHms) {
      duration *= 60;
      duration += val.toInt();
    }
    if (!allPositionsEmpty && position.isEmpty()) {
      if (additionalTags) {
        framesHdr.setValue(Frame::FT_Subtitle, title);
      }
    } else if (!title.isEmpty() || duration != 0) {
      if (standardTags) {
        frames.setTrack(pos);
        frames.setTitle(title);
      }
      const auto artists(track.value(QLatin1String("artists")).toArray());
      if (!artists.isEmpty()) {
        if (standardTags) {
          frames.setArtist(getArtistString(artists));
        }
        if (additionalTags) {
          frames.setValue(Frame::FT_AlbumArtist, framesHdr.getArtist());
        }
      }
      if (additionalTags) {
        const auto extraartists(track.value(QLatin1String("extraartists")).toArray());
        if (!extraartists.isEmpty()) {
          for (const auto& val : extraartists) {
            ExtraArtist extraArtist(val.toObject());
            extraArtist.addToFrames(frames);
          }
        }
      }
      for (const auto& extraArtist : trackExtraArtists) {
        extraArtist.addToFrames(frames, position);
      }

      if (atTrackDataListEnd) {
        ImportTrackData trackData;
        trackData.setFrameCollection(frames);
        trackData.setImportDuration(duration);
        trackDataVector.append(trackData);
      } else {
        while (!atTrackDataListEnd && !it->isEnabled()) {
          ++it;
          atTrackDataListEnd = (it == trackDataVector.end());
        }
        if (!atTrackDataListEnd) {
          (*it).setFrameCollection(frames);
          (*it).setImportDuration(duration);
          ++it;
          atTrackDataListEnd = (it == trackDataVector.end());
        }
      }
      ++trackNr;
    }
    frames = framesHdr;
  }
  // handle redundant tracks
  frames.clear();
  while (!atTrackDataListEnd) {
    if (it->isEnabled()) {
      if ((*it).getFileDuration() == 0) {
        it = trackDataVector.erase(it);
      } else {
        (*it).setFrameCollection(frames);
        (*it).setImportDuration(0);
        ++it;
      }
    } else {
      ++it;
    }
    atTrackDataListEnd = (it == trackDataVector.end());
  }
  trackDataModel()->setTrackData(trackDataVector);
}

void DiscogsImporter::JsonImpl::sendFindQuery(
  const ServerImporterConfig*,
  const QString& artist, const QString& album)
{
  // Query looks like this:
  // http://api.discogs.com//database/search?type=release&title&q=amon+amarth+avenger
  m_importer->sendRequest(QString::fromLatin1(m_discogsServer),
              QLatin1String("/database/search?type=release&title&q=") +
              encodeUrlQuery(artist + QLatin1Char(' ') + album), QLatin1String("https"),
              m_discogsHeaders);
}

void DiscogsImporter::JsonImpl::sendTrackListQuery(
  const ServerImporterConfig*, const QString& cat, const QString& id)
{
  // Query looks like this:
  // http://api.discogs.com/releases/761529
  m_importer->sendRequest(QString::fromLatin1(m_discogsServer), QLatin1Char('/') +
              QString::fromLatin1(QUrl::toPercentEncoding(cat)) +
              QLatin1Char('/') + id, QLatin1String("https"), m_discogsHeaders);
}


/**
 * Constructor.
 *
 * @param netMgr network access manager
 * @param trackDataModel track data to be filled with imported values
 */
DiscogsImporter::DiscogsImporter(QNetworkAccessManager* netMgr,
                                 TrackDataModel* trackDataModel)
  : ServerImporter(netMgr, trackDataModel),
    m_htmlImpl(new HtmlImpl(this)), m_jsonImpl(new JsonImpl(this)),
    m_impl(m_htmlImpl)
{
  setObjectName(QLatin1String("DiscogsImporter"));
}

/**
 * Destructor.
 */
DiscogsImporter::~DiscogsImporter()
{
  m_impl = nullptr;
  delete m_jsonImpl;
  delete m_htmlImpl;
}

/**
 * Name of import source.
 * @return name.
 */
const char* DiscogsImporter::name() const {
  return QT_TRANSLATE_NOOP("@default", "Discogs");
}

/** anchor to online help, 0 to disable */
const char* DiscogsImporter::helpAnchor() const { return "import-discogs"; }

/** configuration, 0 if not used */
ServerImporterConfig* DiscogsImporter::config() const {
  return &DiscogsConfig::instance();
}

/** additional tags option, false if not used */
bool DiscogsImporter::additionalTags() const { return true; }

/**
 * Process finished findCddbAlbum request.
 *
 * @param searchStr search data received
 */
void DiscogsImporter::parseFindResults(const QByteArray& searchStr)
{
  m_impl->parseFindResults(searchStr);
}

/**
 * Parse result of album request and populate m_trackDataModel with results.
 *
 * @param albumStr album data received
 */
void DiscogsImporter::parseAlbumResults(const QByteArray& albumStr)
{
  m_impl->parseAlbumResults(albumStr);
}

/**
 * Send a query command to search on the server.
 *
 * @param cfg      import source configuration
 * @param artist   artist to search
 * @param album    album to search
 */
void DiscogsImporter::sendFindQuery(
  const ServerImporterConfig* cfg,
  const QString& artist, const QString& album)
{
  m_impl = selectImpl(cfg);
  m_impl->sendFindQuery(cfg, artist, album);
}

/**
 * Send a query command to fetch the track list
 * from the server.
 *
 * @param cfg      import source configuration
 * @param cat      category
 * @param id       ID
 */
void DiscogsImporter::sendTrackListQuery(
  const ServerImporterConfig* cfg, const QString& cat, const QString& id)
{
  m_impl = selectImpl(cfg);
  m_impl->sendTrackListQuery(cfg, cat, id);
}

/**
 * Set token to access Discogs API.
 * You have to create an account on Discogs and then generate a token
 * (Settings/Developers, Generate new token). The token can then be used for
 * the "Discogs Auth Flow" in the header "Authorization: Discogs token=value"
 * If a token is found in the configuration, the importer using the Discogs
 * API is used, else the HTML importer.
 * @param cfg configuration which can contain token
 */
DiscogsImporter::BaseImpl* DiscogsImporter::selectImpl(
    const ServerImporterConfig* cfg) const
{
  if (cfg) {
    QByteArray token = cfg->property("token").toByteArray();
    if (!token.isEmpty()) {
      m_jsonImpl->headers()["Authorization"] = "Discogs token=" + token;
      return m_jsonImpl;
    }
  }
  return m_htmlImpl;
}
