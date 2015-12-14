/**
 * \file taggedfile.cpp
 * Handling of tagged files.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 25 Sep 2005
 *
 * Copyright (C) 2005-2013  Urs Fleisch
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

#include "taggedfile.h"
#include <QDir>
#include <QString>
#include <QRegExp>
#include <sys/stat.h>
#include "tagconfig.h"
#include "genres.h"
#include "modeliterator.h"
#include "saferename.h"
#include "fileproxymodel.h"

/**
 * Constructor.
 *
 * @param idx index in file proxy model
 */
TaggedFile::TaggedFile(const QPersistentModelIndex& idx) :
  m_index(idx), m_changedFramesV1(0), m_changedFramesV2(0), m_truncation(0),
  m_changedV1(false), m_changedV2(false),
  m_modified(false), m_marked(false)
{
  Q_ASSERT(m_index.model()->metaObject() == &FileProxyModel::staticMetaObject);
  if (const FileProxyModel* model = getFileProxyModel()) {
    m_newFilename = model->fileName(m_index);
    m_filename = m_newFilename;
  }
}

/**
 * Destructor.
 */
TaggedFile::~TaggedFile()
{
}

/**
 * Get file proxy model.
 * @return file proxy model.
 */
const FileProxyModel* TaggedFile::getFileProxyModel() const
{
  // The validity of this cast is checked in the constructor.
  return static_cast<const FileProxyModel*>(m_index.model());
}

/**
 * Get directory name.
 *
 * @return directory name
 */
QString TaggedFile::getDirname() const
{
  if (const FileProxyModel* model = getFileProxyModel()) {
    return model->filePath(m_index.parent());
  }
  return QString();
}

/**
 * Set file name.
 *
 * @param fn file name
 */
void TaggedFile::setFilename(const QString& fn)
{
  m_newFilename = fn;
  m_revertedFilename.clear();
  updateModifiedState();
}

/**
 * Get current path to file.
 * @return absolute path.
 */
QString TaggedFile::currentFilePath() const
{
  if (const FileProxyModel* model = getFileProxyModel()) {
    return model->filePath(m_index);
  }
  return QString();
}

/**
 * Get features supported.
 * @return bit mask with Feature flags set.
 */
int TaggedFile::taggedFileFeatures() const
{
  return 0;
}

/**
 * Get currently active tagged file features.
 * @return active tagged file features.
 * @see setActiveTaggedFileFeatures()
 */
int TaggedFile::activeTaggedFileFeatures() const
{
  return 0;
}

/**
 * Activate some features provided by the tagged file.
 * For example, if the TF_ID3v24 feature is provided, it can be set, so that
 * writeTags() will write ID3v2.4.0 tags. If the feature is deactivated by
 * passing 0, tags in the default format will be written again.
 *
 * @param features bit mask with some of the Feature flags which are
 * provided by this file, as returned by taggedFileFeatures(), 0 to disable
 * special features.
 */
void TaggedFile::setActiveTaggedFileFeatures(int features)
{
  Q_UNUSED(features)
}

/**
 * Get ID3v1 title.
 *
 * @return string,
 *         "" if the field does not exist,
 *         QString::null if the tags do not exist.
 */
QString TaggedFile::getTitleV1() const
{
  return QString();
}

/**
 * Get ID3v1 artist.
 *
 * @return string,
 *         "" if the field does not exist,
 *         QString::null if the tags do not exist.
 */
QString TaggedFile::getArtistV1() const
{
  return QString();
}

/**
 * Get ID3v1 album.
 *
 * @return string,
 *         "" if the field does not exist,
 *         QString::null if the tags do not exist.
 */
QString TaggedFile::getAlbumV1() const
{
  return QString();
}

/**
 * Get ID3v1 comment.
 *
 * @return string,
 *         "" if the field does not exist,
 *         QString::null if the tags do not exist.
 */
QString TaggedFile::getCommentV1() const
{
  return QString();
}

/**
 * Get ID3v1 year.
 *
 * @return number,
 *         0 if the field does not exist,
 *         -1 if the tags do not exist.
 */
int TaggedFile::getYearV1() const
{
  return -1;
}

/**
 * Get ID3v1 track.
 *
 * @return number,
 *         0 if the field does not exist,
 *         -1 if the tags do not exist.
 */
int TaggedFile::getTrackNumV1() const
{
  return -1;
}

/**
 * Get ID3v1 genre.
 *
 * @return string,
 *         "" if the field does not exist,
 *         QString::null if the tags do not exist.
 */
QString TaggedFile::getGenreV1() const
{
  return QString();
}

/**
 * Remove ID3v1 frames.
 *
 * @param flt filter specifying which frames to remove
 */
void TaggedFile::deleteFramesV1(const FrameFilter& flt)
{
  if (flt.isEnabled(Frame::FT_Title))   setTitleV1(QLatin1String(""));
  if (flt.isEnabled(Frame::FT_Artist))  setArtistV1(QLatin1String(""));
  if (flt.isEnabled(Frame::FT_Album))   setAlbumV1(QLatin1String(""));
  if (flt.isEnabled(Frame::FT_Comment)) setCommentV1(QLatin1String(""));
  if (flt.isEnabled(Frame::FT_Date))    setYearV1(0);
  if (flt.isEnabled(Frame::FT_Track))   setTrackNumV1(0);
  if (flt.isEnabled(Frame::FT_Genre))   setGenreV1(QLatin1String(""));
}

/**
 * Set ID3v1 title.
 *
 * @param str string to set, "" to remove field.
 */
void TaggedFile::setTitleV1(const QString&)
{
}

/**
 * Set ID3v1 artist.
 *
 * @param str string to set, "" to remove field.
 */
void TaggedFile::setArtistV1(const QString&)
{
}

/**
 * Set ID3v1 album.
 *
 * @param str string to set, "" to remove field.
 */
void TaggedFile::setAlbumV1(const QString&)
{
}

/**
 * Set ID3v1 comment.
 *
 * @param str string to set, "" to remove field.
 */
void TaggedFile::setCommentV1(const QString&)
{
}

/**
 * Set ID3v1 year.
 *
 * @param num number to set, 0 to remove field.
 */
void TaggedFile::setYearV1(int)
{
}

/**
 * Set ID3v1 track.
 *
 * @param num number to set, 0 to remove field.
 */
void TaggedFile::setTrackNumV1(int)
{
}

/**
 * Set ID3v1 genre as text.
 *
 * @param str string to set, "" to remove field, QString::null to ignore.
 */
void TaggedFile::setGenreV1(const QString&)
{
}

/**
 * Check if file has an ID3v1 tag.
 *
 * @return true if a V1 tag is available.
 * @see isTagInformationRead()
 */
bool TaggedFile::hasTagV1() const
{
  return false;
}

/**
 * Check if ID3v1 tags are supported by the format of this file.
 *
 * @return true if V1 tags are supported.
 */
bool TaggedFile::isTagV1Supported() const
{
  return false;
}

/**
 * Get absolute filename.
 *
 * @return absolute file path.
 */
QString TaggedFile::getAbsFilename() const
{
  QDir dir(getDirname());
  return QDir::cleanPath(dir.absoluteFilePath(m_newFilename));
}

/**
 * Mark filename as unchanged.
 */
void TaggedFile::markFilenameUnchanged()
{
  m_filename = m_newFilename;
  m_revertedFilename.clear();
  updateModifiedState();
}

/**
 * Revert modification of filename.
 */
void TaggedFile::revertChangedFilename()
{
  m_revertedFilename = m_newFilename;
  m_newFilename = m_filename;
  updateModifiedState();
}

/**
 * Undo reverted modification of filename.
 * When writeTags() fails because the file is not writable, the filename is
 * reverted using revertChangedFilename() so that the file permissions can be
 * changed using the real filename. After changing the permissions, this
 * function can be used to change the filename back before saving the file.
 */
void TaggedFile::undoRevertChangedFilename()
{
  if (!m_revertedFilename.isEmpty()) {
    m_newFilename = m_revertedFilename;
    m_revertedFilename.clear();
    updateModifiedState();
  }
}

/**
 * Mark tag 1 as changed.
 *
 * @param type type of changed frame
 */
void TaggedFile::markTag1Changed(Frame::Type type)
{
  m_changedV1 = true;
  if (static_cast<unsigned>(type) < sizeof(m_changedFramesV1) * 8) {
    m_changedFramesV1 |= (1ULL << type);
  }
  updateModifiedState();
}

/**
 * Mark tag 1 as unchanged.
 */
void TaggedFile::markTag1Unchanged() {
  m_changedV1 = false;
  m_changedFramesV1 = 0;
  clearTrunctionFlags();
  updateModifiedState();
}

/**
 * Mark tag 2 as changed.
 *
 * @param type type of changed frame
 */
void TaggedFile::markTag2Changed(Frame::Type type)
{
  m_changedV2 = true;
  if (static_cast<unsigned>(type) < sizeof(m_changedFramesV2) * 8) {
    m_changedFramesV2 |= (1ULL << type);
  }
  updateModifiedState();
}

/**
 * Mark tag 2 as unchanged.
 */
void TaggedFile::markTag2Unchanged()
{
  m_changedV2 = false;
  m_changedFramesV2 = 0;
  updateModifiedState();
}

/**
 * Set the mask of the frame types changed in tag 2.
 * @param mask mask of frame types
 */
void TaggedFile::setChangedFramesV2(quint64 mask) {
  m_changedFramesV2 = mask;
  m_changedV2 = mask != 0;
  updateModifiedState();
}

void TaggedFile::updateModifiedState()
{
  bool modified = m_changedV1 || m_changedV2 || m_newFilename != m_filename;
  if (m_modified != modified) {
    m_modified = modified;
    if (const FileProxyModel* model = getFileProxyModel()) {
      const_cast<FileProxyModel*>(model)->notifyModificationChanged(
            m_index, m_modified);
    }
  }
}

/**
 * Notify model about changes in extra model data, e.g. the information on
 * which the TaggedFileIconProvider depends.
 *
 * This method shall be called when such data changes, e.g. at the end of
 * readTags() implementations.
 *
 * @param priorIsTagInformationRead prior value returned by
 * isTagInformationRead()
 */
void TaggedFile::notifyModelDataChanged(bool priorIsTagInformationRead) const
{
  if (isTagInformationRead() != priorIsTagInformationRead) {
    if (const FileProxyModel* model = getFileProxyModel()) {
      const_cast<FileProxyModel*>(model)->notifyModelDataChanged(m_index);
    }
  }
}

/**
 * Notify model about changes in the truncation state.
 *
 * This method shall be called when truncation is checked.
 *
 * @param priorTruncation prior value of m_truncation != 0
 */
void TaggedFile::notifyTruncationChanged(bool priorTruncation) const
{
  bool currentTruncation = m_truncation != 0;
  if (currentTruncation != priorTruncation) {
    if (const FileProxyModel* model = getFileProxyModel()) {
      const_cast<FileProxyModel*>(model)->notifyModelDataChanged(m_index);
    }
  }
}

/**
 * Remove artist part from album string.
 * This is used when only the album is needed, but the regexp in
 * getTagsFromFilename() matched a "artist - album" string.
 *
 * @param album album string
 *
 * @return album with artist removed.
 */
static QString removeArtist(const QString& album)
{
  QString str(album);
  int pos = str.indexOf(QLatin1String(" - "));
  if (pos != -1) {
    str.remove(0, pos + 3);
  }
  return str;
}

/**
 * Get tags from filename.
 * Supported formats:
 * album/track - artist - song
 * artist - album/track song
 * /artist - album - track - song
 * album/artist - track - song
 * artist/album/track song
 * album/artist - song
 *
 * @param frames frames to put result
 * @param fmt format string containing the following codes:
 *            %s title (song)
 *            %l album
 *            %a artist
 *            %c comment
 *            %y year
 *            %t track
 */
void TaggedFile::getTagsFromFilename(FrameCollection& frames, const QString& fmt)
{
  QRegExp re;
  QString fn(getAbsFilename());

  // construct regular expression from format string

  // if the format does not contain a '_', they are replaced by spaces
  // in the filename.
  QString fileName(fn);
  if (!fmt.contains(QLatin1Char('_'))) {
    fileName.replace(QLatin1Char('_'), QLatin1Char(' '));
  }

  // escape regexp characters
  QString pattern;
  const int fmtLen = fmt.length();
  static const QString escChars(QLatin1String("+?.*^$()[]{}|\\"));
  for (int i = 0; i < fmtLen; ++i) {
    const QChar ch = fmt.at(i);
    if (escChars.contains(ch)) {
      pattern += QLatin1Char('\\');
    }
    pattern += ch;
  }
  // and finally a dot followed by 2 to 4 characters for the extension
  pattern += QLatin1String("\\..{2,4}$");

  static const struct {
    const char* from;
    const char* to;
  } codeToName[] = {
    { "%s", "%\\{title\\}" },
    { "%l", "%\\{album\\}" },
    { "%a", "%\\{artist\\}" },
    { "%c", "%\\{comment\\}" },
    { "%y", "%\\{date\\}" },
    { "%t", "%\\{track number\\}" },
    { "%g", "%\\{genre\\}" },
    { "%\\{year\\}", "%\\{date\\}" },
    { "%\\{track\\}", "%\\{track number\\}" },
    { "%\\{tracknumber\\}", "%\\{track number\\}" },
    { "%\\{discnumber\\}", "%\\{disc number\\}" }
  };
  int percentIdx = 0, nr = 1;
  for (unsigned i = 0; i < sizeof(codeToName) / sizeof(codeToName[0]); ++i) {
    pattern.replace(QString::fromLatin1(codeToName[i].from), QString::fromLatin1(codeToName[i].to));
  }

  QMap<QString, int> codePos;
  while (((percentIdx = pattern.indexOf(QLatin1String("%\\{"), percentIdx)) >= 0) &&
         (percentIdx < static_cast<int>(pattern.length()) - 1)) {
    int closingBracePos = pattern.indexOf(QLatin1String("\\}"), percentIdx + 3);
    if (closingBracePos > percentIdx + 3) {
      QString code =
        pattern.mid(percentIdx + 3, closingBracePos - percentIdx - 3);
      codePos[code] = nr++;
      if (code == QLatin1String("track number") || code == QLatin1String("date") || code == QLatin1String("disc number") ||
          code == QLatin1String("bpm")) {
        pattern.replace(percentIdx, closingBracePos - percentIdx + 2, QLatin1String("(\\d{1,4})"));
        percentIdx += 9;
      } else {
        pattern.replace(percentIdx, closingBracePos - percentIdx + 2, QLatin1String("([^-_\\./ ](?:[^/]*[^-_/ ])?)"));
        percentIdx += 23;
      }
    } else {
      percentIdx += 3;
    }
  }

  re.setPattern(pattern);
  if (re.indexIn(fileName) != -1) {
    for (QMap<QString, int>::iterator it = codePos.begin();
         it != codePos.end();
         ++it) {
      QString name = it.key();
      QString str = re.cap(*it);
      if (!str.isEmpty()) {
        if (name == QLatin1String("track number") && str.length() == 2 && str[0] == QLatin1Char('0')) {
          // remove leading zero
          str = str.mid(1);
        }
        if (name != QLatin1String("ignore"))
          frames.setValue(Frame::ExtendedType(name), str);
      }
    }
    return;
  }

  // album/track - artist - song
  re.setPattern(QLatin1String("([^/]+)/(\\d{1,3})[-_\\. ]+([^-_\\./ ][^/]+)[_ ]-[_ ]([^-_\\./ ][^/]+)\\..{2,4}$"));
  if (re.indexIn(fn) != -1) {
    frames.setAlbum(removeArtist(re.cap(1)));
    frames.setTrack(re.cap(2).toInt());
    frames.setArtist(re.cap(3));
    frames.setTitle(re.cap(4));
    return;
  }

  // artist - album (year)/track song
  re.setPattern(QLatin1String("([^/]+)[_ ]-[_ ]([^/]+)[_ ]\\((\\d{4})\\)/(\\d{1,3})[-_\\. ]+([^-_\\./ ][^/]+)\\..{2,4}$"));
  if (re.indexIn(fn) != -1) {
    frames.setArtist(re.cap(1));
    frames.setAlbum(re.cap(2));
    frames.setYear(re.cap(3).toInt());
    frames.setTrack(re.cap(4).toInt());
    frames.setTitle(re.cap(5));
    return;
  }

  // artist - album/track song
  re.setPattern(QLatin1String("([^/]+)[_ ]-[_ ]([^/]+)/(\\d{1,3})[-_\\. ]+([^-_\\./ ][^/]+)\\..{2,4}$"));
  if (re.indexIn(fn) != -1) {
    frames.setArtist(re.cap(1));
    frames.setAlbum(re.cap(2));
    frames.setTrack(re.cap(3).toInt());
    frames.setTitle(re.cap(4));
    return;
  }
  // /artist - album - track - song
  re.setPattern(QLatin1String("/([^/]+[^-_/ ])[_ ]-[_ ]([^-_/ ][^/]+[^-_/ ])[-_\\. ]+(\\d{1,3})[-_\\. ]+([^-_\\./ ][^/]+)\\..{2,4}$"));
  if (re.indexIn(fn) != -1) {
    frames.setArtist(re.cap(1));
    frames.setAlbum(re.cap(2));
    frames.setTrack(re.cap(3).toInt());
    frames.setTitle(re.cap(4));
    return;
  }
  // album/artist - track - song
  re.setPattern(QLatin1String("([^/]+)/([^/]+[^-_\\./ ])[-_\\. ]+(\\d{1,3})[-_\\. ]+([^-_\\./ ][^/]+)\\..{2,4}$"));
  if (re.indexIn(fn) != -1) {
    frames.setAlbum(removeArtist(re.cap(1)));
    frames.setArtist(re.cap(2));
    frames.setTrack(re.cap(3).toInt());
    frames.setTitle(re.cap(4));
    return;
  }
  // artist/album/track song
  re.setPattern(QLatin1String("([^/]+)/([^/]+)/(\\d{1,3})[-_\\. ]+([^-_\\./ ][^/]+)\\..{2,4}$"));
  if (re.indexIn(fn) != -1) {
    frames.setArtist(re.cap(1));
    frames.setAlbum(re.cap(2));
    frames.setTrack(re.cap(3).toInt());
    frames.setTitle(re.cap(4));
    return;
  }
  // album/artist - song
  re.setPattern(QLatin1String("([^/]+)/([^/]+[^-_/ ])[_ ]-[_ ]([^-_/ ][^/]+)\\..{2,4}$"));
  if (re.indexIn(fn) != -1) {
    frames.setAlbum(removeArtist(re.cap(1)));
    frames.setArtist(re.cap(2));
    frames.setTitle(re.cap(3));
    return;
  }
}

/**
 * Format a time string "h:mm:ss".
 * If the time is less than an hour, the hour is not put into the
 * string and the minute is not padded with zeroes.
 *
 * @param seconds time in seconds
 *
 * @return string with the time in hours, minutes and seconds.
 */
QString TaggedFile::formatTime(unsigned seconds)
{
  unsigned hours = seconds / 3600;
  seconds %= 3600;
  unsigned minutes = seconds / 60;
  seconds %= 60;
  QString timeStr;
  if (hours > 0) {
    timeStr.sprintf("%u:%02u:%02u", hours, minutes, seconds);
  } else {
    timeStr.sprintf("%u:%02u", minutes, seconds);
  }
  return timeStr;
}

/**
 * Rename a file.
 * This methods takes care of case insensitive filesystems.
 *
 * @param fnOld old filename
 * @param fnNew new filename
 *
 * @return true if ok.
 */
bool TaggedFile::renameFile(const QString& fnOld, const QString& fnNew) const
{
  QString dirname = getDirname();
  if (fnNew.toLower() == fnOld.toLower()) {
    // If the filenames only differ in case, the new file is reported to
    // already exist on case insensitive filesystems (e.g. Windows),
    // so it is checked if the new file is really the old file by
    // comparing inodes and devices. If the files are not the same,
    // another file would be overwritten and an error is reported.
    if (QFile::exists(dirname + QDir::separator() + fnNew)) {
      struct stat statOld, statNew;
      if (::stat((dirname + QDir::separator() + fnOld).toLatin1().data(), &statOld) == 0 &&
          ::stat((dirname + QDir::separator() + fnNew).toLatin1().data(), &statNew) == 0 &&
          !(statOld.st_ino == statNew.st_ino &&
            statOld.st_dev == statNew.st_dev)) {
        qDebug("rename(%s, %s): %s already exists", fnOld.toLatin1().data(),
               fnNew.toLatin1().data(), fnNew.toLatin1().data());
        return false;
      }
    }

    // if the filenames only differ in case, first rename to a
    // temporary filename, so that it works also with case
    // insensitive filesystems (e.g. Windows).
    QString temp_filename(fnNew);
    temp_filename.append(QLatin1String("_CASE"));
    if (!Utils::safeRename(dirname, fnOld, temp_filename)) {
      qDebug("rename(%s, %s) failed", fnOld.toLatin1().data(),
             temp_filename.toLatin1().data());
      return false;
    }
    if (!Utils::safeRename(dirname, temp_filename, fnNew)) {
      qDebug("rename(%s, %s) failed", temp_filename.toLatin1().data(),
             fnNew.toLatin1().data());
      return false;
    }
  } else if (QFile::exists(dirname + QDir::separator() + fnNew)) {
    qDebug("rename(%s, %s): %s already exists", fnOld.toLatin1().data(),
           fnNew.toLatin1().data(), fnNew.toLatin1().data());
    return false;
  } else if (!Utils::safeRename(dirname, fnOld, fnNew)) {
    qDebug("rename(%s, %s) failed", fnOld.toLatin1().data(),
           fnNew.toLatin1().data());
    return false;
  }
  return true;
}

/**
 * Get field name for comment from configuration.
 *
 * @return field name.
 */
QString TaggedFile::getCommentFieldName() const
{
  return TagConfig::instance().commentName();
}

/**
 * Split a track string into number and total.
 *
 * @param str track
 * @param total the total is returned here if found, else 0
 *
 * @return number, 0 if parsing failed, -1 if str is null
 */
int TaggedFile::splitNumberAndTotal(const QString& str, int* total)
{
  if (total)
    *total = 0;
  if (str.isNull())
    return -1;

  int slashPos = str.indexOf(QLatin1Char('/'));
  if (slashPos == -1)
    return str.toInt();

  if (total)
    *total = str.mid(slashPos + 1).toInt();
  return str.left(slashPos).toInt();
}

/**
 * Get the total number of tracks in the directory.
 *
 * @return total number of tracks, -1 if unavailable.
 */
int TaggedFile::getTotalNumberOfTracksInDir() const {
  int numTracks = -1;
  QModelIndex parentIdx = m_index.parent();
  if (parentIdx.isValid()) {
    numTracks = 0;
    TaggedFileOfDirectoryIterator it(parentIdx);
    while (it.hasNext()) {
      it.next();
      ++numTracks;
    }
  }
  return numTracks;
}

/**
 * Get the total number of tracks if it is enabled.
 *
 * @return total number of tracks,
 *         -1 if disabled or unavailable.
 */
int TaggedFile::getTotalNumberOfTracksIfEnabled() const
{
  return TagConfig::instance().enableTotalNumberOfTracks()
      ? getTotalNumberOfTracksInDir() : -1;
}

/**
 * Format track number/total number of tracks with configured digits.
 *
 * @param num track number, <= 0 if empty
 * @param numTracks total number of tracks, <= 0 to disable
 *
 * @return formatted "track/total" string.
 */
QString TaggedFile::trackNumberString(int num, int numTracks) const
{
  int numDigits = getTrackNumberDigits();
  QString str;
  if (num != 0) {
    if (numDigits > 0) {
      str.sprintf("%0*d", numDigits, num);
    } else {
      str.setNum(num);
    }
    if (numTracks > 0) {
      str += QLatin1Char('/');
      if (numDigits > 0) {
        str += QString().sprintf("%0*d", numDigits, numTracks);
      } else {
        str += QString::number(numTracks);
      }
    }
  } else {
    str = QLatin1String("");
  }
  return str;
}

/**
 * Format the track number (digits, total number of tracks) if enabled.
 *
 * @param value    string containing track number, will be modified
 * @param addTotal true to add total number of tracks if enabled
 *                 "/t" with t = total number of tracks will be appended
 *                 if enabled and value contains a number
 */
void TaggedFile::formatTrackNumberIfEnabled(QString& value, bool addTotal) const
{
  int numDigits = getTrackNumberDigits();
  int numTracks = addTotal ? getTotalNumberOfTracksIfEnabled() : -1;
  if (numTracks > 0 || numDigits > 1) {
    bool ok;
    int trackNr = value.toInt(&ok);
    if (ok && trackNr > 0) {
      if (numTracks > 0) {
        value.sprintf("%0*d/%0*d", numDigits, trackNr, numDigits, numTracks);
      } else {
        value.sprintf("%0*d", numDigits, trackNr);
      }
    }
  }
}

/**
 * Get the number of track number digits configured.
 *
 * @return track number digits,
 *         1 if invalid or unavailable.
 */
int TaggedFile::getTrackNumberDigits() const
{
  int numDigits = TagConfig::instance().trackNumberDigits();
  if (numDigits < 1 || numDigits > 5)
    numDigits = 1;
  return numDigits;
}

/**
 * Remove ID3v2 frames.
 *
 * @param flt filter specifying which frames to remove
 */
void TaggedFile::deleteFramesV2(const FrameFilter& flt)
{
  if (flt.isEnabled(Frame::FT_Title))   setTitleV2(QLatin1String(""));
  if (flt.isEnabled(Frame::FT_Artist))  setArtistV2(QLatin1String(""));
  if (flt.isEnabled(Frame::FT_Album))   setAlbumV2(QLatin1String(""));
  if (flt.isEnabled(Frame::FT_Comment)) setCommentV2(QLatin1String(""));
  if (flt.isEnabled(Frame::FT_Date))    setYearV2(0);
  if (flt.isEnabled(Frame::FT_Track))   setTrackV2(QLatin1String(""));
  if (flt.isEnabled(Frame::FT_Genre))   setGenreV2(QLatin1String(""));
}

/**
 * Get the format of tag 1.
 *
 * @return string describing format of tag 1,
 *         e.g. "ID3v1.1", "ID3v2.3", "Vorbis", "APE",
 *         QString::null if unknown.
 */
QString TaggedFile::getTagFormatV1() const
{
  return QString();
}

/**
 * Get the format of tag 2.
 *
 * @return string describing format of tag 2,
 *         e.g. "ID3v1.1", "ID3v2.3", "Vorbis", "APE",
 *         QString::null if unknown.
 */
QString TaggedFile::getTagFormatV2() const
{
  return QString();
}

/**
 * Check if a string has to be truncated.
 *
 * @param str  string to be checked
 * @param flag flag to be set if string has to be truncated
 * @param len  maximum length of string
 *
 * @return str truncated to len characters if necessary, else QString::null.
 */
QString TaggedFile::checkTruncation(
  const QString& str, quint64 flag, int len)
{
  bool priorTruncation = m_truncation != 0;
  QString result;
  if (static_cast<int>(str.length()) > len) {
    result = str;
    result.truncate(len);
    m_truncation |= flag;
  } else {
    m_truncation &= ~flag;
  }
  notifyTruncationChanged(priorTruncation);
  return result;
}

/**
 * Check if a number has to be truncated.
 *
 * @param val  value to be checked
 * @param flag flag to be set if number has to be truncated
 * @param max  maximum value
 *
 * @return val truncated to max if necessary, else -1.
 */
int TaggedFile::checkTruncation(int val, quint64 flag,
                                int max)
{
  bool priorTruncation = m_truncation != 0;
  int result;
  if (val > max) {
    m_truncation |= flag;
    result = max;
  } else {
    m_truncation &= ~flag;
    result = -1;
  }
  notifyTruncationChanged(priorTruncation);
  return result;
}

/**
 * Get a specific frame from the tags 1.
 *
 * @param type  frame type
 * @param frame the frame is returned here
 *
 * @return true if ok.
 */
bool TaggedFile::getFrameV1(Frame::Type type, Frame& frame)
{
  int n = -1;
  bool number = false;

  switch (type) {
    case Frame::FT_Album:
      frame.m_value = getAlbumV1();
      break;
    case Frame::FT_Artist:
      frame.m_value = getArtistV1();
      break;
    case Frame::FT_Comment:
      frame.m_value = getCommentV1();
      break;
    case Frame::FT_Date:
      n = getYearV1();
      number = true;
      break;
    case Frame::FT_Genre:
      frame.m_value = getGenreV1();
      break;
    case Frame::FT_Title:
      frame.m_value = getTitleV1();
      break;
    case Frame::FT_Track:
      n = getTrackNumV1();
      number = true;
      break;
    default:
      // maybe handled in a subclass
      return false;
  }
  if (number) {
    if (n == -1) {
      frame.m_value = QString();
    } else if (n == 0) {
      frame.m_value = QLatin1String("");
    } else {
      frame.m_value.setNum(n);
    }
  }
  frame.setType(type);
  return true;
}

/**
 * Set a frame in the tags 1.
 *
 * @param frame frame to set.
 *
 * @return true if ok.
 */
bool TaggedFile::setFrameV1(const Frame& frame)
{
  int n = -1;
  if (frame.getType() == Frame::FT_Date ||
      frame.getType() == Frame::FT_Track) {
    if (frame.isInactive()) {
      n = -1;
    } else if (frame.isEmpty()) {
      n = 0;
    } else {
      n = Frame::numberWithoutTotal(frame.m_value);
    }
  }
  switch (frame.getType()) {
    case Frame::FT_Album:
      setAlbumV1(frame.m_value);
      break;
    case Frame::FT_Artist:
      setArtistV1(frame.m_value);
      break;
    case Frame::FT_Comment:
      setCommentV1(frame.m_value);
      break;
    case Frame::FT_Date:
      setYearV1(n);
      break;
    case Frame::FT_Genre:
      setGenreV1(frame.m_value);
      break;
    case Frame::FT_Title:
      setTitleV1(frame.m_value);
      break;
    case Frame::FT_Track:
      setTrackNumV1(n);
      break;
    default:
      // maybe handled in a subclass
      return false;
  }
  return true;
}

/**
 * Get a specific frame from the tags 2.
 *
 * @param type  frame type
 * @param frame the frame is returned here
 *
 * @return true if ok.
 */
bool TaggedFile::getFrameV2(Frame::Type type, Frame& frame)
{
  int n = -1;
  bool number = false;

  switch (type) {
    case Frame::FT_Album:
      frame.m_value = getAlbumV2();
      break;
    case Frame::FT_Artist:
      frame.m_value = getArtistV2();
      break;
    case Frame::FT_Comment:
      frame.m_value = getCommentV2();
      break;
    case Frame::FT_Date:
      n = getYearV2();
      number = true;
      break;
    case Frame::FT_Genre:
      frame.m_value = getGenreV2();
      break;
    case Frame::FT_Title:
      frame.m_value = getTitleV2();
      break;
    case Frame::FT_Track:
      frame.m_value = getTrackV2();
      break;
    default:
      // maybe handled in a subclass
      return false;
  }
  if (number) {
    if (n == -1) {
      frame.m_value = QString();
    } else if (n == 0) {
      frame.m_value = QLatin1String("");
    } else {
      frame.m_value.setNum(n);
    }
  }
  frame.setType(type);
  return true;
}

/**
 * Set a frame in the tags 2.
 *
 * @param frame frame to set
 *
 * @return true if ok.
 */
bool TaggedFile::setFrameV2(const Frame& frame)
{
  int n = -1;
  if (frame.getType() == Frame::FT_Date) {
    if (frame.isInactive()) {
      n = -1;
    } else if (frame.isEmpty()) {
      n = 0;
    } else {
      n = Frame::numberWithoutTotal(frame.m_value);
    }
  }
  switch (frame.getType()) {
    case Frame::FT_Album:
      setAlbumV2(frame.m_value);
      break;
    case Frame::FT_Artist:
      setArtistV2(frame.m_value);
      break;
    case Frame::FT_Comment:
      setCommentV2(frame.m_value);
      break;
    case Frame::FT_Date:
      setYearV2(n);
      break;
    case Frame::FT_Genre:
      setGenreV2(frame.m_value);
      break;
    case Frame::FT_Title:
      setTitleV2(frame.m_value);
      break;
    case Frame::FT_Track:
      setTrackV2(frame.m_value);
      break;
    default:
      // maybe handled in a subclass
      return false;
  }
  return true;
}

/**
 * Add a frame in the tags 2.
 *
 * @param frame frame to add, a field list may be added by this method
 *
 * @return true if ok.
 */
bool TaggedFile::addFrameV2(Frame& frame)
{
  return TaggedFile::setFrameV2(frame);
}

/**
 * Delete a frame in the tags 2.
 *
 * @param frame frame to delete.
 *
 * @return true if ok.
 */
bool TaggedFile::deleteFrameV2(const Frame& frame)
{
  Frame emptyFrame(frame);
  emptyFrame.setValue(QLatin1String(""));
  return setFrameV2(emptyFrame);
}

/**
 * Get all frames in tag 1.
 *
 * @param frames frame collection to set.
 */
void TaggedFile::getAllFramesV1(FrameCollection& frames)
{
  frames.clear();
  Frame frame;
  for (int i = Frame::FT_FirstFrame; i <= Frame::FT_LastV1Frame; ++i) {
    if (getFrameV1(static_cast<Frame::Type>(i), frame)) {
      frames.insert(frame);
    }
  }
}

/**
 * Set frames in tag 1.
 *
 * @param frames      frame collection
 * @param onlyChanged only frames with value marked as changed are set
 */
void TaggedFile::setFramesV1(const FrameCollection& frames, bool onlyChanged)
{
  for (FrameCollection::const_iterator it = frames.begin();
       it != frames.end();
       ++it) {
    if (!onlyChanged || it->isValueChanged()) {
        setFrameV1(*it);
    }
  }
}

/**
 * Get all frames in tag 2.
 * This generic implementation only supports the standard tags and should
 * be reimplemented in derived classes.
 *
 * @param frames frame collection to set.
 */
void TaggedFile::getAllFramesV2(FrameCollection& frames)
{
  frames.clear();
  Frame frame;
  for (int i = Frame::FT_FirstFrame; i <= Frame::FT_LastV1Frame; ++i) {
    if (getFrameV2(static_cast<Frame::Type>(i), frame)) {
      frames.insert(frame);
    }
  }
}

/**
 * Update marked property of frame.
 * If the frame is a picture and its size exceeds the configured
 * maximum size, the frame is marked. This method should be called in
 * reimplementations of getAllFramesV2().
 *
 * @param frame frame to check
 * @see resetMarkedState()
 */
void TaggedFile::updateMarkedState(Frame& frame)
{
  if (frame.getType() == Frame::FT_Picture) {
    const TagConfig& tagCfg = TagConfig::instance();
    if (tagCfg.markOversizedPictures()) {
      QVariant data = frame.getField(frame, Frame::ID_Data);
      if (!data.isNull()) {
        if (data.toByteArray().size() > tagCfg.maximumPictureSize()) {
          frame.setMarked(true);
          m_marked = true;
          return;
        }
      }
    }
    frame.setMarked(false);
  }
}

/**
 * Close any file handles which are held open by the tagged file object.
 * The default implementation does nothing. If a concrete subclass holds
 * any file handles open, it has to close them in this method. This method
 * can be used before operations which require that a file is not open,
 * e.g. file renaming on Windows.
 */
void TaggedFile::closeFileHandle()
{
}

/**
 * Add a suitable field list for the frame if missing.
 * If a frame is created, its field list is empty. This method will create
 * a field list appropriate for the frame type and tagged file type if no
 * field list exists. The default implementation does nothing.
 * @param frame frame where field list is added
 */
void TaggedFile::addFieldList(Frame&) const
{
}

/**
 * Set frames in tag 2.
 *
 * @param frames      frame collection
 * @param onlyChanged only frames with value marked as changed are set
 */
void TaggedFile::setFramesV2(const FrameCollection& frames, bool onlyChanged)
{
  bool myFramesValid = false;
  FrameCollection myFrames;

  for (FrameCollection::const_iterator it = frames.begin();
       it != frames.end();
       ++it) {
    if (!onlyChanged || it->isValueChanged()) {
      if (it->getIndex() != -1) {
        // The frame has an index, so the original tag can be modified
        setFrameV2(*it);
      } else {
        // The frame does not have an index
        if (it->getType() <= Frame::FT_LastV1Frame) {
          // Standard tags can be handled with the basic method
          TaggedFile::setFrameV2(*it);
        } else {
          // The frame has to be looked up and modified
          if (!myFramesValid) {
            getAllFramesV2(myFrames);
            myFramesValid = true;
          }
          FrameCollection::iterator myIt = myFrames.find(*it);
          if (myIt != myFrames.end() && myIt->getIndex() != -1) {
            Frame myFrame(*it);
            myFrame.setIndex(myIt->getIndex());
            setFrameV2(myFrame);
          } else {
            // Such a frame does not exist, add a new one.
            Frame addedFrame(*it);
            addFrameV2(addedFrame);
            Frame myFrame(*it);
            myFrame.setIndex(addedFrame.getIndex());
            setFrameV2(myFrame);
          }
        }
      }
    }
  }
}


/**
 * Constructor.
 */
TaggedFile::DetailInfo::DetailInfo() :
  channelMode(CM_None), channels(0), sampleRate(0), bitrate(0), duration(0),
  valid(false), vbr(false)
{
}

/**
 * Destructor.
 */
TaggedFile::DetailInfo::~DetailInfo()
{
}

/**
 * Get string representation of detail information.
 * @return information summary as string.
 */
QString TaggedFile::DetailInfo::toString() const
{
  QString str;
  if (valid) {
    str = format;
    str += QLatin1Char(' ');
    if (bitrate > 0 && bitrate < 999) {
      if (vbr) str += QLatin1String("VBR ");
      str += QString::number(bitrate);
      str += QLatin1String(" kbps ");
    }
    if (sampleRate > 0) {
      str += QString::number(sampleRate);
      str += QLatin1String(" Hz ");
    }
    switch (channelMode) {
      case TaggedFile::DetailInfo::CM_Stereo:
        str += QLatin1String("Stereo ");
        break;
      case TaggedFile::DetailInfo::CM_JointStereo:
        str += QLatin1String("Joint Stereo ");
        break;
      default:
        if (channels > 0) {
          str += QString::number(channels);
          str += QLatin1String(" Channels ");
        }
    }
    if (duration > 0) {
      str += TaggedFile::formatTime(duration);
    }
  }
  return str;
}
