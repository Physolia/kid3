/**
 * \file importconfig.cpp
 * Configuration for import dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 17 Sep 2003
 *
 * Copyright (C) 2003-2018  Urs Fleisch
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

#include "importconfig.h"
#include <QString>
#include "isettings.h"
#include "config.h"

namespace {

/**
 * Convert tag version to import destination value in configuration.
 * @param tagVersion tag version
 * @return value used in configuration, kept for backwards compatibility.
 */
inline int tagVersionToImportDestCfg(Frame::TagVersion tagVersion) {
  return static_cast<int>(tagVersion) - 1;
}

/**
 * Convert import destination value in configuration to tag version.
 * @param importDest value used in configuration, kept for backwards
 *                   compatibility.
 * @return tag version.
 */
inline Frame::TagVersion importDestCfgToTagVersion(int importDest) {
  return Frame::tagVersionCast(importDest + 1);
}

}

int ImportConfig::s_index = -1;

/**
 * Constructor.
 */
ImportConfig::ImportConfig()
  : StoredConfig<ImportConfig>(QLatin1String("Import")), m_importServer(0),
    m_importDest(Frame::TagV1), m_importFormatIdx(0),
    m_maxTimeDifference(3),
    m_importVisibleColumns(0x2000000000ULL),
    m_importTagsIdx(0),
    m_pictureSourceIdx(0),
    m_enableTimeDifferenceCheck(true)
{
  /**
   * Preset import format regular expressions.
   * The following codes are used before the () expressions.
   * %s %{title} title (song)
   * %l %{album} album
   * %a %{artist} artist
   * %c %{comment} comment
   * %y %{year} year
   * %t %{track} track, at least two digits
   * %T %{tracknumber} track number
   * %g %{genre} genre
   * %d %{duration} duration mm:ss
   * %D %{seconds} duration in seconds
   * %f %{file} file name
   * %p %{filepath} absolute file path
   * %u %{url} URL
   * %n %{tracks} number of tracks
   */
  m_importFormatNames.append(QLatin1String("CSV unquoted"));
  m_importFormatHeaders.append(QLatin1String(""));
  m_importFormatTracks.append(QLatin1String(
    R"(%{track}([^\r\n\t]*)\t%{title}([^\r\n\t]*)\t%{artist}([^\r\n\t]*)\t)"
    R"(%{album}([^\r\n\t]*)\t%{year}([^\r\n\t]*)\t%{genre}([^\r\n\t]*)\t)"
    R"(%{comment}([^\r\n\t]*)\t(?:\d+:)?%{duration}(\d+:\d+))"));

  m_importFormatNames.append(QLatin1String("CSV quoted"));
  m_importFormatHeaders.append(QLatin1String(""));
  // Using a raw string literal here causes clang to issue
  // "Unbalanced opening parenthesis in C++ code".
  m_importFormatTracks.append(QLatin1String(
    "\"?%{track}([^\\r\\n\\t\"]*)\"?\\t\"?%{title}([^\\r\\n\\t\"]*)\"?\\t\"?"
    "%{artist}([^\\r\\n\\t\"]*)\"?\\t\"?%{album}([^\\r\\n\\t\"]*)\"?"
    "\\t\"?%{year}([^\\r\\n\\t\"]*)\"?\\t\"?%{genre}([^\\r\\n\\t\"]*)\"?"
    "\\t\"?%{comment}([^\\r\\n\\t\"]*)\"?\\t\"?(?:\\d+:)?%{duration}(\\d+:\\d+)"));

  m_importFormatNames.append(QLatin1String("CSV more unquoted"));
  m_importFormatHeaders.append(QLatin1String(""));
  m_importFormatTracks.append(QLatin1String(
    "%{track}([^\\r\\n\\t]*)\\t%{title}([^\\r\\n\\t]*)\\t%{artist}([^\\r\\n\\t]*)\\t"
    "%{album}([^\\r\\n\\t]*)\\t%{year}([^\\r\\n\\t]*)\\t%{genre}([^\\r\\n\\t]*)\\"
    "t%{comment}([^\\r\\n\\t]*)\\t(?:\\d+:)?%{duration}(\\d+:\\d+)(?:\\.\\d+)?\\t"
    "%{album artist}([^\\r\\n\\t]*)\\t%{arranger}([^\\r\\n\\t]*)\\t"
    "%{author}([^\\r\\n\\t]*)\\t%{bpm}([^\\r\\n\\t]*)\\t"
    "%{composer}([^\\r\\n\\t]*)\\t%{conductor}([^\\r\\n\\t]*)\\t"
    "%{copyright}([^\\r\\n\\t]*)\\t%{disc number}([^\\r\\n\\t]*)\\t"
    "%{encoded-by}([^\\r\\n\\t]*)\\t%{grouping}([^\\r\\n\\t]*)\\t%{isrc}([^\\r\\n\\t]*)\\t"
    "%{language}([^\\r\\n\\t]*)\\t%{lyricist}([^\\r\\n\\t]*)\\t%{lyrics}([^\\r\\n\\t]*)\\t"
    "%{media}([^\\r\\n\\t]*)\\t%{original album}([^\\r\\n\\t]*)\\t"
    "%{original artist}([^\\r\\n\\t]*)\\t%{original date}([^\\r\\n\\t]*)\\t"
    "%{part}([^\\r\\n\\t]*)\\t%{performer}([^\\r\\n\\t]*)\\t"
    "%{publisher}([^\\r\\n\\t]*)\\t%{remixer}([^\\r\\n\\t]*)\\t"
    "%{subtitle}([^\\r\\n\\t]*)\\t%{website}([^\\r\\n\\t]*)"));

  m_importFormatNames.append(QLatin1String("CSV more quoted"));
  m_importFormatHeaders.append(QLatin1String(""));
  m_importFormatTracks.append(QLatin1String(
    "\"?%{track}([^\\r\\n\\t\"]*)\"?\\t\"?%{title}([^\\r\\n\\t\"]*)\"?\\t"
    "\"?%{artist}([^\\r\\n\\t\"]*)\"?\\t\"?%{album}([^\\r\\n\\t\"]*)\"?\\t"
    "\"?%{year}([^\\r\\n\\t\"]*)\"?\\t\"?%{genre}([^\\r\\n\\t\"]*)\"?\\t"
    "\"?%{comment}([^\\r\\n\\t\"]*)\"?\\t"
    "\"?(?:\\d+:)?%{duration}(\\d+:\\d+)(?:\\.\\d+)?\"?\\t"
    "\"?%{album artist}([^\\r\\n\\t\"]*)\"?\\t\"?%{arranger}([^\\r\\n\\t\"]*)\"?\\t"
    "\"?%{author}([^\\r\\n\\t\"]*)\"?\\t\"?%{bpm}([^\\r\\n\\t\"]*)\"?\\t"
    "\"?%{composer}([^\\r\\n\\t\"]*)\"?\\t\"?%{conductor}([^\\r\\n\\t\"]*)\"?\\t"
    "\"?%{copyright}([^\\r\\n\\t\"]*)\"?\\t\"?%{disc number}([^\\r\\n\\t\"]*)\"?\\t"
    "\"?%{encoded-by}([^\\r\\n\\t\"]*)\"?\\t\"?%{grouping}([^\\r\\n\\t\"]*)\"?\\t"
    "\"?%{isrc}([^\\r\\n\\t\"]*)\"?\\t"
    "\"?%{language}([^\\r\\n\\t\"]*)\"?\\t\"?%{lyricist}([^\\r\\n\\t\"]*)\"?\\t"
    "\"?%{lyrics}([^\\r\\n\\t\"]*)\"?\\t"
    "\"?%{media}([^\\r\\n\\t\"]*)\"?\\t\"?%{original album}([^\\r\\n\\t\"]*)\"?\\t"
    "\"?%{original artist}([^\\r\\n\\t\"]*)\"?\\t\"?%{original date}([^\\r\\n\\t\"]*)\"?\\t"
    "\"?%{part}([^\\r\\n\\t\"]*)\"?\\t\"?%{performer}([^\\r\\n\\t\"]*)\"?\\t"
    "\"?%{publisher}([^\\r\\n\\t\"]*)\"?\\t\"?%{remixer}([^\\r\\n\\t\"]*)\"?\\t"
    "\"?%{subtitle}([^\\r\\n\\t\"]*)\"?\\t\"?%{website}([^\\r\\n\\t\"]*)"));

  m_importFormatNames.append(QLatin1String("freedb HTML text"));
  m_importFormatHeaders.append(QLatin1String(
    R"(%{artist}(\S[^\r\n/]*\S)\s*/\s*%{album}(\S[^\r\n]*\S)[\r\n]+\s*)"
    R"(tracks:\s+\d+.*year:\s*%{year}([^\r\n\t]*)?.*)"
    R"(genre:\s*%{genre}(\S[^\r\n]*\S)?[\r\n])"));
  m_importFormatTracks.append(QLatin1String(
    R"([\r\n]%{track}(\d+)[\.\s]+%{duration}(\d+:\d+)\s+%{title}(\S[^\r\n]*\S))"));

  m_importFormatNames.append(QLatin1String("freedb HTML source"));
  m_importFormatHeaders.append(QLatin1String(
    R"(<[^>]+>%{artist}([^<\s][^\r\n/]*\S)\s*/\s*)"
    R"(%{album}(\S[^\r\n]*[^\s>])<[^>]+>[\r\n]+\s*)"
    R"(tracks:\s+\d+.*year:\s*%{year}([^\r\n\t]*)?.*)"
    R"(genre:\s*%{genre}(\S[^\r\n>]*\S)?<[^>]+>[\r\n])"));
  m_importFormatTracks.append(QLatin1String(
    R"(<td[^>]*>\s*%{track}(\d+).</td><td[^>]*>\s*%{duration}(\d+:\d+)</td>)"
    R"(<td[^>]*>(?:<[^>]+>)?%{title}([^<\r\n]+))"));

  m_importFormatNames.append(QLatin1String("Title"));
  m_importFormatHeaders.append(QLatin1String(""));
  m_importFormatTracks.append(QLatin1String(R"(\s*%{title}(\S[^\r\n]*\S)\s*)"));

  m_importFormatNames.append(QLatin1String("Track Title"));
  m_importFormatHeaders.append(QLatin1String(""));
  m_importFormatTracks.append(QLatin1String(
    R"(\s*%{track}(\d+)[\.\s]+%{title}(\S[^\r\n]*\S)\s*)"));

  m_importFormatNames.append(QLatin1String("Track Title Time"));
  m_importFormatHeaders.append(QLatin1String(""));
  m_importFormatTracks.append(QLatin1String(
    R"(\s*%{track}(\d+)[\.\s]+%{title}(\S[^\r\n]*\S)\s+%{duration}(\d+:\d+)\s*)"));

  m_importFormatNames.append(QLatin1String("Custom Format"));
  m_importFormatHeaders.append(QLatin1String(""));
  m_importFormatTracks.append(QLatin1String(""));

  m_importTagsNames.append(QLatin1String("Artist to Album Artist"));
  m_importTagsSources.append(QLatin1String("%{artist}"));
  m_importTagsExtractions.append(QLatin1String("%{albumartist}(.+)"));

  m_importTagsNames.append(QLatin1String("Album Artist to Artist"));
  m_importTagsSources.append(QLatin1String("%{albumartist}"));
  m_importTagsExtractions.append(QLatin1String("%{artist}(.+)"));

  m_importTagsNames.append(QLatin1String("Artist to Composer"));
  m_importTagsSources.append(QLatin1String("%{artist}"));
  m_importTagsExtractions.append(QLatin1String("%{composer}(.+)"));

  m_importTagsNames.append(QLatin1String("Artist to Conductor"));
  m_importTagsSources.append(QLatin1String("%{artist}"));
  m_importTagsExtractions.append(QLatin1String("%{conductor}(.+)"));

  m_importTagsNames.append(QLatin1String("Track Number from Title"));
  m_importTagsSources.append(QLatin1String("%{title}"));
  m_importTagsExtractions.append(
        QLatin1String(R"(\s*%{track}(\d+)[\.\s]+%{title}(\S.*\S)\s*)"));

  m_importTagsNames.append(QLatin1String("Track Number to Title"));
  m_importTagsSources.append(QLatin1String("%{track} %{title}"));
  m_importTagsExtractions.append(QLatin1String("%{title}(.+)"));

  m_importTagsNames.append(QLatin1String("Subtitle from Title"));
  m_importTagsSources.append(QLatin1String("%{title}"));
  m_importTagsExtractions.append(QLatin1String("%{subtitle}(.+) - "));

  m_importTagsNames.append(QLatin1String("Title Annotation to Comment"));
  m_importTagsSources.append(QLatin1String("%{title}"));
  m_importTagsExtractions.append(
        QLatin1String("%{title}(.+) +\\(%{comment}((?:Bonus|Remix)[^)]*)\\)"));

  m_importTagsNames.append(QLatin1String("Modification Date to Date"));
  m_importTagsSources.append(QLatin1String("%{modificationdate}"));
  m_importTagsExtractions.append(QLatin1String(R"(%{date}(\d\d\d\d-\d\d-\d\d))"));

  m_importTagsNames.append(QLatin1String("Filename to Original Filename"));
  m_importTagsSources.append(QLatin1String("%{file}"));
  m_importTagsExtractions.append(QLatin1String("%{TOFN}(.+)"));

  m_importTagsNames.append(QLatin1String("Custom Format"));
  m_importTagsSources.append(QLatin1String(""));
  m_importTagsExtractions.append(QLatin1String(""));

  m_pictureSourceNames.append(QLatin1String("Google Images"));
  m_pictureSourceUrls.append(QLatin1String(
    "http://www.google.com/search?tbm=isch&q=%u{artist}%20%u{album}"));
  m_pictureSourceNames.append(QLatin1String("Yahoo Images"));
  m_pictureSourceUrls.append(QLatin1String(
    "http://images.search.yahoo.com/search/images?ei=UTF-8&p=%u{artist}%20%u{album}"));
  m_pictureSourceNames.append(QLatin1String("Amazon"));
  m_pictureSourceUrls.append(QLatin1String(
    "http://www.amazon.com/s?search-alias=aps&field-keywords=%u{artist}+%u{album}"));
  m_pictureSourceNames.append(QLatin1String("Amazon.co.uk"));
  m_pictureSourceUrls.append(QLatin1String(
    "http://www.amazon.co.uk/s?search-alias=aps&field-keywords=%u{artist}+%u{album}"));
  m_pictureSourceNames.append(QLatin1String("Amazon.de"));
  m_pictureSourceUrls.append(QLatin1String(
    "http://www.amazon.de/s?search-alias=aps&field-keywords=%u{artist}+%u{album}"));
  m_pictureSourceNames.append(QLatin1String("Amazon.fr"));
  m_pictureSourceUrls.append(QLatin1String(
    "http://www.amazon.fr/s?search-alias=aps&field-keywords=%u{artist}+%u{album}"));
  m_pictureSourceNames.append(QLatin1String("MusicBrainz"));
  m_pictureSourceUrls.append(QLatin1String(
    "http://musicbrainz.org/search/textsearch.html?query=%u{artist}+%u{album}&type=release"));
  m_pictureSourceNames.append(QLatin1String("Discogs"));
  m_pictureSourceUrls.append(QLatin1String(
    "http://www.discogs.com/search?q=%u{artist}+%u{album}"));
  m_pictureSourceNames.append(QLatin1String("CD Universe"));
  m_pictureSourceUrls.append(QLatin1String(
    "http://www.cduniverse.com/sresult.asp?HT_Search_Info=%u{artist}+%u{album}"));
  m_pictureSourceNames.append(QLatin1String("Coveralia"));
  m_pictureSourceUrls.append(QLatin1String(
    "http://www.coveralia.com/mostrar.php?bus=%u{artist}%20%u{album}&bust=2"));
  m_pictureSourceNames.append(QLatin1String("SlothRadio"));
  m_pictureSourceUrls.append(QLatin1String(
    "http://www.slothradio.com/covers/?artist=%u{artist}&album=%u{album}"));
  m_pictureSourceNames.append(QLatin1String("Albumart"));
  m_pictureSourceUrls.append(QLatin1String(
    "http://www.albumart.org/index.php?srchkey=%u{artist}+%u{album}&searchindex=Music"));
  m_pictureSourceNames.append(QLatin1String("Custom Source"));
  m_pictureSourceUrls.append(QLatin1String(""));

  m_matchPictureUrlMap.append({
    {QLatin1String("https?://www.google.(?:[^/]+)/.*imgurl=([^&]+)&.*"),
     QLatin1String("\\1")},
    {QLatin1String("http://images.search.yahoo.com/.*&imgurl=([^&]+)&.*"),
     QLatin1String("http%3A%2F%2F\\1")},
    {QLatin1String(
     "http://(?:www.)?amazon.(?:com|co.uk|de|fr).*/(?:dp|ASIN|images|product|-)/([A-Z0-9]+).*"),
     QLatin1String("http://images.amazon.com/images/P/\\1.01._SCLZZZZZZZ_.jpg")},
    {QLatin1String(
     "http://musicbrainz.org/misc/redirects/.*&asin=([A-Z0-9]+).*"),
     QLatin1String("http://images.amazon.com/images/P/\\1.01._SCLZZZZZZZ_.jpg")},
    {QLatin1String("(http://.*4shared.com/img/.*)"),
     QLatin1String("\\1.jpg")}
  });
}

/**
 * Persist configuration.
 *
 * @param config configuration
 */
void ImportConfig::writeToConfig(ISettings* config) const
{
  config->beginGroup(m_group);
  config->setValue(QLatin1String("ImportServer"),
                   QVariant(m_importServer));
  config->setValue(QLatin1String("ImportDestination"),
                   QVariant(tagVersionToImportDestCfg(m_importDest)));
  config->setValue(QLatin1String("ImportFormatNames"),
                   QVariant(m_importFormatNames));
  config->setValue(QLatin1String("ImportFormatHeaders"),
                   QVariant(m_importFormatHeaders));
  config->setValue(QLatin1String("ImportFormatTracks"),
                   QVariant(m_importFormatTracks));
  config->setValue(QLatin1String("ImportFormatIdx"),
                   QVariant(m_importFormatIdx));
  config->setValue(QLatin1String("EnableTimeDifferenceCheck"),
                   QVariant(m_enableTimeDifferenceCheck));
  config->setValue(QLatin1String("MaxTimeDifference"),
                   QVariant(m_maxTimeDifference));
#ifdef Q_OS_MAC
  // Convince Mac OS X to store a 64-bit value.
  config->setValue(QLatin1String("ImportVisibleColumns"),
                   QVariant(m_importVisibleColumns | (Q_UINT64_C(1) << 63)));
#else
  config->setValue(QLatin1String("ImportVisibleColumns"),
                   QVariant(m_importVisibleColumns));
#endif
  config->setValue(QLatin1String("ImportTagsNames"),
                   QVariant(m_importTagsNames));
  config->setValue(QLatin1String("ImportTagsSources"),
                   QVariant(m_importTagsSources));
  config->setValue(QLatin1String("ImportTagsExtractions"),
                   QVariant(m_importTagsExtractions));
  config->setValue(QLatin1String("ImportTagsIdx"),
                   QVariant(m_importTagsIdx));

  config->setValue(QLatin1String("PictureSourceNames"),
                   QVariant(m_pictureSourceNames));
  config->setValue(QLatin1String("PictureSourceUrls"),
                   QVariant(m_pictureSourceUrls));
  config->setValue(QLatin1String("PictureSourceIdx"),
                   QVariant(m_pictureSourceIdx));
  QStringList keys, values;
  for (auto it = m_matchPictureUrlMap.constBegin();
       it != m_matchPictureUrlMap.constEnd();
       ++it) {
    keys.append(it->first);
    values.append(it->second);
  }
  config->setValue(QLatin1String("MatchPictureUrlMapKeys"), QVariant(keys));
  config->setValue(QLatin1String("MatchPictureUrlMapValues"), QVariant(values));

  config->setValue(QLatin1String("DisabledPlugins"),
                   QVariant(m_disabledPlugins));
  config->endGroup();
  config->beginGroup(m_group, true);
  config->setValue(QLatin1String("BrowseCoverArtWindowGeometry"),
                   QVariant(m_browseCoverArtWindowGeometry));
  config->setValue(QLatin1String("ImportWindowGeometry"),
                   QVariant(m_importWindowGeometry));
  config->endGroup();
}

/**
 * Read persisted configuration.
 *
 * @param config configuration
 */
void ImportConfig::readFromConfig(ISettings* config)
{
  QStringList names, headers, tracks;
  QStringList tagsNames, tagsSources, tagsExtractions;
  QStringList picNames, picUrls;

  config->beginGroup(m_group);
  m_importServer = config->value(QLatin1String("ImportServer"),
                                 m_importServer).toInt();
  m_importDest = importDestCfgToTagVersion(
    config->value(QLatin1String("ImportDestination"),
                  tagVersionToImportDestCfg(m_importDest)).toInt());
  names = config->value(QLatin1String("ImportFormatNames"),
                        m_importFormatNames).toStringList();
  headers = config->value(QLatin1String("ImportFormatHeaders"),
                          m_importFormatHeaders).toStringList();
  tracks = config->value(QLatin1String("ImportFormatTracks"),
                         m_importFormatTracks).toStringList();
  m_importFormatIdx = config->value(QLatin1String("ImportFormatIdx"),
                                    m_importFormatIdx).toInt();
  m_enableTimeDifferenceCheck =
      config->value(QLatin1String("EnableTimeDifferenceCheck"),
                    m_enableTimeDifferenceCheck).toBool();
  m_maxTimeDifference = config->value(QLatin1String("MaxTimeDifference"),
                                      m_maxTimeDifference).toInt();
  m_importVisibleColumns = config->value(QLatin1String("ImportVisibleColumns"),
                                         m_importVisibleColumns).toULongLong();
#ifdef Q_OS_MAC
  m_importVisibleColumns &= ~(Q_UINT64_C(1) << 63);
#endif
  tagsNames = config->value(QLatin1String("ImportTagsNames"),
                            m_importTagsNames).toStringList();
  tagsSources = config->value(QLatin1String("ImportTagsSources"),
                              m_importTagsSources).toStringList();
  tagsExtractions = config->value(QLatin1String("ImportTagsExtractions"),
                                  m_importTagsExtractions).toStringList();
  m_importTagsIdx = config->value(QLatin1String("ImportTagsIdx"),
                                  m_importTagsIdx).toInt();

  picNames = config->value(QLatin1String("PictureSourceNames"),
                           m_pictureSourceNames).toStringList();
  picUrls = config->value(QLatin1String("PictureSourceUrls"),
                          m_pictureSourceUrls).toStringList();
  m_pictureSourceIdx = config->value(
    QLatin1String("PictureSourceIdx"), m_pictureSourceIdx).toInt();
  QStringList keys = config->value(QLatin1String("MatchPictureUrlMapKeys"),
                                   QStringList()).toStringList();
  QStringList values = config->value(QLatin1String("MatchPictureUrlMapValues"),
                                     QStringList()).toStringList();
  if (!keys.empty() && !values.empty()) {
    m_matchPictureUrlMap.clear();
    for (auto itk = keys.constBegin(), itv = values.constBegin();
         itk != keys.constEnd() && itv != values.constEnd();
         ++itk, ++itv) {
      m_matchPictureUrlMap.append({*itk, *itv});
    }
  }

  m_disabledPlugins = config->value(QLatin1String("DisabledPlugins"),
                                 m_disabledPlugins).toStringList();
  config->endGroup();
  config->beginGroup(m_group, true);
  m_browseCoverArtWindowGeometry = config->value(
        QLatin1String("BrowseCoverArtWindowGeometry"),
        m_browseCoverArtWindowGeometry).toByteArray();
  m_importWindowGeometry = config->value(QLatin1String("ImportWindowGeometry"),
                                         m_importWindowGeometry).toByteArray();
  config->endGroup();

  // KConfig seems to strip empty entries from the end of the string lists,
  // so we have to append them again.
  const int numNames = names.size();
  while (headers.size() < numNames) headers.append(QLatin1String(""));
  while (tracks.size() < numNames) tracks.append(QLatin1String(""));
  const int numPicNames = picNames.size();
  while (picUrls.size() < numPicNames) picUrls.append(QLatin1String(""));

  /* Use defaults if no configuration found */
  for (auto namesIt = names.constBegin(), headersIt = headers.constBegin(),
         tracksIt = tracks.constBegin();
       namesIt != names.constEnd() && headersIt != headers.constEnd() &&
         tracksIt != tracks.constEnd();
       ++namesIt, ++headersIt, ++tracksIt) {
    int idx = m_importFormatNames.indexOf(*namesIt);
    if (idx >= 0) {
      m_importFormatHeaders[idx] = *headersIt;
      m_importFormatTracks[idx] = *tracksIt;
    } else if (!(*namesIt).isEmpty()) {
      m_importFormatNames.append(*namesIt);
      m_importFormatHeaders.append(*headersIt);
      m_importFormatTracks.append(*tracksIt);
    }
  }

  for (auto tagsNamesIt = tagsNames.constBegin(), sourcesIt = tagsSources.constBegin(),
         extractionsIt = tagsExtractions.constBegin();
       tagsNamesIt != tagsNames.constEnd() && sourcesIt != tagsSources.constEnd() &&
         extractionsIt != tagsExtractions.constEnd();
       ++tagsNamesIt, ++sourcesIt, ++extractionsIt) {
    int idx = m_importTagsNames.indexOf(*tagsNamesIt);
    if (idx >= 0) {
      m_importTagsSources[idx] = *sourcesIt;
      m_importTagsExtractions[idx] = *extractionsIt;
    } else if (!(*tagsNamesIt).isEmpty()) {
      m_importTagsNames.append(*tagsNamesIt);
      m_importTagsSources.append(*sourcesIt);
      m_importTagsExtractions.append(*extractionsIt);
    }
  }

  for (auto picNamesIt = picNames.constBegin(), picUrlsIt = picUrls.constBegin();
       picNamesIt != picNames.constEnd() && picUrlsIt != picUrls.constEnd();
       ++picNamesIt, ++picUrlsIt) {
    int idx = m_pictureSourceNames.indexOf(*picNamesIt);
    if (idx >= 0) {
      m_pictureSourceUrls[idx] = *picUrlsIt;
    } else if (!(*picNamesIt).isEmpty()) {
      m_pictureSourceNames.append(*picNamesIt);
      m_pictureSourceUrls.append(*picUrlsIt);
    }
  }

  if (m_importFormatIdx >= static_cast<int>(m_importFormatNames.size()))
    m_importFormatIdx = 0;
  if (m_importTagsIdx >= static_cast<int>(m_importTagsNames.size()))
    m_importTagsIdx = 0;
  if (m_pictureSourceIdx >=  static_cast<int>(m_pictureSourceNames.size()))
    m_pictureSourceIdx = 0;

  // Replace mappings which do no longer work.
  if (m_pictureSourceUrls.removeOne(QLatin1String(
      "http://cdbaby.com/found?artist=%u{artist}&album=%u{album}"))) {
    m_pictureSourceNames.removeOne(QLatin1String("CD Baby"));
  }
  if (m_pictureSourceUrls.removeOne(QLatin1String(
      "http://www.jamendo.com/en/search/all/%u{artist}%20%u{album}"))) {
    m_pictureSourceNames.removeOne(QLatin1String("Jamendo"));
  }
  m_pictureSourceUrls.replaceInStrings(
        QLatin1String("http://images.google.com/images?q=%u{artist}%20%u{album}"),
        QLatin1String("http://www.google.com/search?tbm=isch&q=%u{artist}%20%u{album}"));
  for (auto it = m_matchPictureUrlMap.begin(); it != m_matchPictureUrlMap.end();) {
    if (it->first == QLatin1String(
          "http://images.google.com/.*imgurl=([^&]+)&.*") ||
        it->first == QLatin1String(
          "http://www.google.com/.*imgurl=([^&]+)&.*") ||
        it->first == QLatin1String(
          "http://www.google.(?:[^/]+)/.*imgurl=([^&]+)&.*")) {
      *it = {QLatin1String("https?://www.google.(?:[^/]+)/.*imgurl=([^&]+)&.*"),
             QLatin1String("\\1")};
    } else if (it->first == QLatin1String(
                 "http://rds.yahoo.com/.*&imgurl=([^&]+)&.*")) {
      *it = {QLatin1String(
             "http://images.search.yahoo.com/.*&imgurl=([^&]+)&.*"),
             QLatin1String("http%3A%2F%2F\\1")};
    } else if (it->first == QLatin1String(
                 "http://rds.yahoo.com/.*%26imgurl=((?:[^%]|%(?!26))+).*") ||
               it->first == QLatin1String(
                 R"(http://cdbaby.com/cd/(\w)(\w)(\w+))") ||
               it->first == QLatin1String(
                 "http://www.jamendo.com/en/album/(\\d+)")) {
      it = m_matchPictureUrlMap.erase(it);
      continue;
    }
    ++it;
  }
}

void ImportConfig::setAvailablePlugins(const QStringList& availablePlugins)
{
  if (m_availablePlugins != availablePlugins) {
    m_availablePlugins = availablePlugins;
    emit availablePluginsChanged(m_availablePlugins);
  }
}

void ImportConfig::setImportServer(int importServer)
{
  if (m_importServer != importServer) {
    m_importServer = importServer;
    emit importServerChanged(m_importServer);
  }
}

void ImportConfig::setImportDest(Frame::TagVersion importDest)
{
  if (m_importDest != importDest) {
    m_importDest = importDest;
    emit importDestChanged(m_importDest);
  }
}

void ImportConfig::setImportFormatNames(const QStringList& importFormatNames)
{
  if (m_importFormatNames != importFormatNames) {
    m_importFormatNames = importFormatNames;
    emit importFormatNamesChanged(m_importFormatNames);
  }
}

void ImportConfig::setImportFormatHeaders(const QStringList& importFormatHeaders)
{
  if (m_importFormatHeaders != importFormatHeaders) {
    m_importFormatHeaders = importFormatHeaders;
    emit importFormatHeadersChanged(m_importFormatHeaders);
  }
}

void ImportConfig::setImportFormatTracks(const QStringList& importFormatTracks)
{
  if (m_importFormatTracks != importFormatTracks) {
    m_importFormatTracks = importFormatTracks;
    emit importFormatTracksChanged(m_importFormatTracks);
  }
}

void ImportConfig::setImportFormatIndex(int importFormatIdx)
{
  if (m_importFormatIdx != importFormatIdx) {
    m_importFormatIdx = importFormatIdx;
    emit importFormatIndexChanged(m_importFormatIdx);
  }
}

void ImportConfig::setMaxTimeDifference(int maxTimeDifference)
{
  if (m_maxTimeDifference != maxTimeDifference) {
    m_maxTimeDifference = maxTimeDifference;
    emit maxTimeDifferenceChanged(m_maxTimeDifference);
  }
}

void ImportConfig::setImportVisibleColumns(quint64 importVisibleColumns)
{
  if (m_importVisibleColumns != importVisibleColumns) {
    m_importVisibleColumns = importVisibleColumns;
    emit importVisibleColumnsChanged(m_importVisibleColumns);
  }
}

void ImportConfig::setImportWindowGeometry(const QByteArray& importWindowGeometry)
{
  if (m_importWindowGeometry != importWindowGeometry) {
    m_importWindowGeometry = importWindowGeometry;
    emit importWindowGeometryChanged(m_importWindowGeometry);
  }
}

void ImportConfig::setImportTagsNames(const QStringList& importTagsNames)
{
  if (m_importTagsNames != importTagsNames) {
    m_importTagsNames = importTagsNames;
    emit importTagsNamesChanged(m_importTagsNames);
  }
}

void ImportConfig::setImportTagsSources(const QStringList& importTagsSources)
{
  if (m_importTagsSources != importTagsSources) {
    m_importTagsSources = importTagsSources;
    emit importTagsSourcesChanged(m_importTagsSources);
  }
}

void ImportConfig::setImportTagsExtractions(const QStringList& importTagsExtractions)
{
  if (m_importTagsExtractions != importTagsExtractions) {
    m_importTagsExtractions = importTagsExtractions;
    emit importTagsExtractionsChanged(m_importTagsExtractions);
  }
}

void ImportConfig::setImportTagsIndex(int importTagsIdx)
{
  if (m_importTagsIdx != importTagsIdx) {
    m_importTagsIdx = importTagsIdx;
    emit importTagsIndexChanged(m_importTagsIdx);
  }
}

void ImportConfig::setPictureSourceNames(const QStringList& pictureSourceNames)
{
  if (m_pictureSourceNames != pictureSourceNames) {
    m_pictureSourceNames = pictureSourceNames;
    emit pictureSourceNamesChanged(m_pictureSourceNames);
  }
}

void ImportConfig::setPictureSourceUrls(const QStringList& pictureSourceUrls)
{
  if (m_pictureSourceUrls != pictureSourceUrls) {
    m_pictureSourceUrls = pictureSourceUrls;
    emit pictureSourceUrlsChanged(m_pictureSourceUrls);
  }
}


void ImportConfig::setPictureSourceIndex(int pictureSourceIdx)
{
  if (m_pictureSourceIdx != pictureSourceIdx) {
    m_pictureSourceIdx = pictureSourceIdx;
    emit pictureSourceIndexChanged(m_pictureSourceIdx);
  }
}

void ImportConfig::setBrowseCoverArtWindowGeometry(const QByteArray& browseCoverArtWindowGeometry)
{
  if (m_browseCoverArtWindowGeometry != browseCoverArtWindowGeometry) {
    m_browseCoverArtWindowGeometry = browseCoverArtWindowGeometry;
    emit browseCoverArtWindowGeometryChanged(m_browseCoverArtWindowGeometry);
  }
}

void ImportConfig::setMatchPictureUrlMap(const QList<QPair<QString, QString>>& matchPictureUrlMap)
{
  if (m_matchPictureUrlMap != matchPictureUrlMap) {
    m_matchPictureUrlMap = matchPictureUrlMap;
    emit matchPictureUrlMapChanged(m_matchPictureUrlMap);
  }
}

QStringList ImportConfig::matchPictureUrlStringList() const
{
  QStringList lst;
  QList<QPair<QString, QString>> urlMap = matchPictureUrlMap();
  for (auto it = urlMap.constBegin(); it != urlMap.constEnd(); ++it) {
    lst.append(it->first);
    lst.append(it->second);
  }
  return lst;
}

void ImportConfig::setMatchPictureUrlStringList(const QStringList& lst)
{
  QList<QPair<QString, QString>> urlMap;
  auto it = lst.constBegin();
  while (it != lst.constEnd()) {
    QString key = *it++;
    if (it != lst.constEnd()) {
      urlMap.append({key, *it++});
    }
  }
  setMatchPictureUrlMap(urlMap);
}

void ImportConfig::setImportDir(const QString& importDir)
{
  if (m_importDir != importDir) {
    m_importDir = importDir;
    emit importDirChanged(m_importDir);
  }
}

void ImportConfig::setDisabledPlugins(const QStringList& disabledPlugins)
{
  if (m_disabledPlugins != disabledPlugins) {
    m_disabledPlugins = disabledPlugins;
    emit disabledPluginsChanged(m_disabledPlugins);
  }
}

void ImportConfig::setEnableTimeDifferenceCheck(bool enableTimeDifferenceCheck)
{
  if (m_enableTimeDifferenceCheck != enableTimeDifferenceCheck) {
    m_enableTimeDifferenceCheck = enableTimeDifferenceCheck;
    emit enableTimeDifferenceCheckChanged(m_enableTimeDifferenceCheck);
  }
}
