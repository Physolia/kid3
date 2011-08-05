/**
 * \file scriptinterface.cpp
 * D-Bus script adaptor.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 20 Dec 2007
 *
 * Copyright (C) 2007-2011  Urs Fleisch
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

#include "scriptinterface.h"
#ifdef HAVE_QTDBUS
#include <QDBusMessage>
#include <QDBusConnection>
#include <QFileInfo>
#include <QCoreApplication>
#include "kid3mainwindow.h"
#include "kid3application.h"
#include "kid3form.h"
#include "taggedfile.h"
#include "frametablemodel.h"
#include "filefilter.h"
#include "pictureframe.h"
#include "fileproxymodel.h"
#include "modeliterator.h"
#include "filelist.h"

/**
 * Constructor.
 *
 * @param parent parent object
 * @param app application
 */
ScriptInterface::ScriptInterface(Kid3MainWindow* parent, Kid3Application* app) :
  QDBusAbstractAdaptor(parent), m_mainWin(parent), m_app(app)
{
  setObjectName("ScriptInterface");
  setAutoRelaySignals(true);
}

/**
 * Destructor.
 */
ScriptInterface::~ScriptInterface()
{
}

/**
 * Open file or directory.
 *
 * @param path path to file or directory
 *
 * @return true if ok.
 */
bool ScriptInterface::openDirectory(const QString& path)
{
  return m_app->openDirectory(path, true);
}

/**
 * Save all modified files.
 *
 * @return true if ok,
 *         else the error message is available using getErrorMessage().
 */
bool ScriptInterface::save()
{
  QStringList errorFiles = m_app->saveDirectory();
  if (errorFiles.isEmpty()) {
    m_errorMsg.clear();
    return true;
  } else {
    m_errorMsg = "Error while writing file:\n" + errorFiles.join("\n");
    return false;
  }
}

/**
 * Get a detailed error message provided by some methods.
 *
 * @return detailed error message.
 */
QString ScriptInterface::getErrorMessage() const
{
  return m_errorMsg;
}

/**
 * Revert changes in the selected files.
 */
void ScriptInterface::revert()
{
  m_app->revertFileModifications();
}

/**
 * Import tags from a file.
 *
 * @param tagMask tag bit (1 for tag 1, 2 for tag 2)
 * @param path    path of file
 * @param fmtIdx  index of format
 *
 * @return true if ok.
 */
bool ScriptInterface::importFromFile(int tagMask, const QString& path,
                                     int fmtIdx)
{
  return m_app->importTags(TrackData::tagVersionCast(tagMask), path, fmtIdx);
}

/**
 * Download album cover art into the picture frame of the selected files.
 *
 * @param url           URL of picture file or album art resource
 * @param allFilesInDir true to add the image to all files in the directory
 */
void ScriptInterface::downloadAlbumArt(const QString& url, bool allFilesInDir)
{
  m_app->downloadImage(url, allFilesInDir
    ? Kid3Application::ImageForAllFilesInDirectory
    : Kid3Application::ImageForSelectedFiles);
}

/**
 * Export tags to a file.
 *
 * @param tagMask tag bit (1 for tag 1, 2 for tag 2)
 * @param path    path of file
 * @param fmtIdx  index of format
 *
 * @return true if ok.
 */
bool ScriptInterface::exportToFile(int tagMask, const QString& path, int fmtIdx)
{
  return m_app->exportTags(TrackData::tagVersionCast(tagMask), path, fmtIdx);
}

/**
 * Create a playlist.
 *
 * @return true if ok.
 */
bool ScriptInterface::createPlaylist()
{
  return m_app->writePlaylist();
}

/**
 * Quit the application.
 */
void ScriptInterface::quit()
{
  selectAll();
  revert();
  QCoreApplication::quit();
}

/**
 * Select all files.
 */
void ScriptInterface::selectAll()
{
  m_app->selectAllFiles();
}

/**
 * Deselect all files.
 */
void ScriptInterface::deselectAll()
{
  m_app->deselectAllFiles();
}

/**
 * Set the first file as the current file.
 *
 * @return true if there is a first file.
 */
bool ScriptInterface::firstFile()
{
  return m_app->firstFile(false);
}

/**
 * Set the previous file as the current file.
 *
 * @return true if there is a previous file.
 */
bool ScriptInterface::previousFile()
{
  return m_app->previousFile(false);
}

/**
 * Set the next file as the current file.
 *
 * @return true if there is a next file.
 */
bool ScriptInterface::nextFile()
{
  return m_app->nextFile(false);
}

/**
 * Select the first file.
 *
 * @return true if there is a first file.
 */
bool ScriptInterface::selectFirstFile()
{
  return m_app->firstFile(true);
}

/**
 * Select the previous file.
 *
 * @return true if there is a previous file.
 */
bool ScriptInterface::selectPreviousFile()
{
  return m_app->previousFile(true);
}

/**
 * Select the next file.
 *
 * @return true if there is a next file.
 */
bool ScriptInterface::selectNextFile()
{
  return m_app->nextFile(true);
}

/**
 * Select the current file.
 *
 * @return true if there is a current file.
 */
bool ScriptInterface::selectCurrentFile()
{
 return m_app->selectCurrentFile(true);
}

/**
 * Expand the current file item if it is a directory.
 * A file list item is a directory if getFileName() returns a name with
 * '/' as the last character.
 * The directory is fetched but not expanded in the GUI. To expand it in the
 * GUI, call nextFile() or selectNextFile() after expandDirectory().
 *
 * @return true if current file item is a directory.
 */
bool ScriptInterface::expandDirectory()
{
  QModelIndex index(m_app->getFileSelectionModel()->currentIndex());
  if (!FileProxyModel::getPathIfIndexOfDir(index).isNull()) {
    m_app->fetchDirectory(index);
    return true;
  }
  return false;
}

/**
 * Apply the file name format.
 */
void ScriptInterface::applyFilenameFormat()
{
  m_app->applyFilenameFormat();
}

/**
 * Apply the tag format.
 */
void ScriptInterface::applyTagFormat()
{
  m_app->applyId3Format();
}

/**
 * Set the directory name from the tags.
 *
 * @param tagMask tag mask (bit 0 for tag 1, bit 1 for tag 2)
 * @param format  directory name format
 * @param create  true to create, false to rename
 *
 * @return true if ok,
 *         else the error message is available using getErrorMessage().
 */
bool ScriptInterface::setDirNameFromTag(int tagMask, const QString& format,
                                        bool create)
{
  if (m_app->renameDirectory(TrackData::tagVersionCast(tagMask), format,
                             create, &m_errorMsg)) {
    m_errorMsg.clear();
    return true;
  } else {
    m_errorMsg = "Error while renaming:\n" + m_errorMsg;
    return false;
  }
}

/**
 * Set subsequent track numbers in the selected files.
 *
 * @param tagMask      tag mask (bit 0 for tag 1, bit 1 for tag 2)
 * @param firstTrackNr number to use for first file
 */
void ScriptInterface::numberTracks(int tagMask, int firstTrackNr)
{
  m_app->numberTracks(firstTrackNr, 0, TrackData::tagVersionCast(tagMask));
}

/**
 * Filter the files.
 *
 * @param expression filter expression
 */
void ScriptInterface::filter(const QString& expression)
{
  m_app->applyFilter(expression);
}

#ifdef HAVE_TAGLIB
/**
 * Convert ID3v2.3 tags to ID3v2.4.
 */
void ScriptInterface::convertToId3v24()
{
  m_app->convertToId3v24();
}
#endif

#if defined HAVE_TAGLIB && defined HAVE_ID3LIB
/**
 * Convert ID3v2.4 tags to ID3v2.3.
 */
void ScriptInterface::convertToId3v23()
{
  m_app->convertToId3v23();
}
#endif

/**
 * Get path of directory.
 *
 * @return absolute path of directory.
 */
QString ScriptInterface::getDirectoryName()
{
  return m_app->getDirPath();
}

/**
 * Get name of current file.
 *
 * @return absolute file name, ends with "/" if it is a directory.
 */
QString ScriptInterface::getFileName()
{
  return m_app->getFileNameOfSelectedFile();
}

/**
 * Set name of selected file.
 * The file will be renamed when the directory is saved.
 *
 * @param name file name.
 */
void ScriptInterface::setFileName(const QString& name)
{
  m_app->setFileNameOfSelectedFile(name);
}

/**
 * Set format to use when setting the filename from the tags.
 *
 * @param format file name format
 * @see setFileNameFromTag()
 */
void ScriptInterface::setFileNameFormat(const QString& format)
{
  m_app->setTagsToFilenameFormat(format);
}

/**
 * Set the file names of the selected files from the tags.
 *
 * @param tagMask tag bit (1 for tag 1, 2 for tag 2)
 * @see setFileNameFormat()
 */
void ScriptInterface::setFileNameFromTag(int tagMask)
{
  m_app->getFilenameFromTags(TrackData::tagVersionCast(tagMask));
}

/**
 * Get value of frame.
 * To get binary data like a picture, the name of a file to write can be
 * added after the @a name, e.g. "Picture:/path/to/file".
 *
 * @param tagMask tag bit (1 for tag 1, 2 for tag 2)
 * @param name    name of frame (e.g. "Artist")
 */
QString ScriptInterface::getFrame(int tagMask, const QString& name)
{
  QString frameName(name);
  QString dataFileName;
  int colonIndex = frameName.indexOf(':');
  if (colonIndex != -1) {
    dataFileName = frameName.mid(colonIndex + 1);
    frameName.truncate(colonIndex);
  }
  FrameTableModel* ft = (tagMask & 2) ? m_app->frameModelV2() :
    m_app->frameModelV1();
  FrameCollection::const_iterator it = ft->frames().findByName(frameName);
  if (it != ft->frames().end()) {
    if (!dataFileName.isEmpty()) {
      PictureFrame::writeDataToFile(*it, dataFileName);
    }
    return it->getValue();
  } else {
    return "";
  }
}

/**
 * Set value of frame.
 * For tag 2 (@a tagMask 2), if no frame with @a name exists, a new frame
 * is added, if @a value is empty, the frame is deleted.
 * To add binary data like a picture, a file can be added after the
 * @a name, e.g. "Picture:/path/to/file".
 *
 * @param tagMask tag bit (1 for tag 1, 2 for tag 2)
 * @param name    name of frame (e.g. "Artist")
 * @param value   value of frame
 */
bool ScriptInterface::setFrame(int tagMask, const QString& name,
                           const QString& value)
{
  QString frameName(name);
  QString dataFileName;
  int colonIndex = frameName.indexOf(':');
  if (colonIndex != -1) {
    dataFileName = frameName.mid(colonIndex + 1);
    frameName.truncate(colonIndex);
  }
  FrameTableModel* ft = (tagMask & 2) ? m_app->frameModelV2() :
    m_app->frameModelV1();
  FrameCollection frames(ft->frames());
  FrameCollection::iterator it = frames.findByName(frameName);
  if (it != frames.end()) {
    if (it->getType() == Frame::FT_Picture && !dataFileName.isEmpty() &&
        (tagMask & 2) != 0) {
      m_app->deleteFrame(it->getName());
      PictureFrame frame;
      PictureFrame::setDescription(frame, value);
      PictureFrame::setDataFromFile(frame, dataFileName);
      PictureFrame::setMimeTypeFromFileName(frame, dataFileName);
      m_app->addFrame(&frame);
    } else if (value.isEmpty() && (tagMask & 2) != 0) {
      m_app->deleteFrame(it->getName());
    } else {
      Frame& frame = const_cast<Frame&>(*it);
      frame.setValueIfChanged(value);
      ft->transferFrames(frames);
    }
    return true;
  } else if (tagMask & 2) {
    Frame::Type type = Frame::getTypeFromName(frameName);
    Frame frame(type, value, frameName, -1);
    if (type == Frame::FT_Picture && !dataFileName.isEmpty()) {
      PictureFrame::setFields(frame);
      PictureFrame::setDescription(frame, value);
      PictureFrame::setDataFromFile(frame, dataFileName);
      PictureFrame::setMimeTypeFromFileName(frame, dataFileName);
    }
    m_app->addFrame(&frame);
    return true;
  }
  return false;
}

/**
 * Get all frames of a tag.
 *
 * @param tagMask tag bit (1 for tag 1, 2 for tag 2)
 *
 * @return list with alternating frame names and values.
 */
QStringList ScriptInterface::getTag(int tagMask)
{
  QStringList lst;
  FrameTableModel* ft = (tagMask & 2) ? m_app->frameModelV2() :
    m_app->frameModelV1();
  for (FrameCollection::const_iterator it = ft->frames().begin();
       it != ft->frames().end();
       ++it) {
    lst << it->getName();
    lst << it->getValue();
  }
  return lst;
}

/**
 * Get technical information about file.
 * Properties are Format, Bitrate, Samplerate, Channels, Duration,
 * Channel Mode, VBR, Tag 1, Tag 2.
 * Properties which are not available are omitted.
 *
 * @return list with alternating property names and values.
 */
QStringList ScriptInterface::getInformation()
{
  QStringList lst;
  QModelIndex index = m_app->getFileSelectionModel()->currentIndex();
  if (TaggedFile* taggedFile = FileProxyModel::getTaggedFileOfIndex(index)) {
    TaggedFile::DetailInfo info;
    taggedFile->getDetailInfo(info);
    if (info.valid) {
      lst << "Format" << info.format;
      if (info.bitrate > 0 && info.bitrate < 999) {
        lst << "Bitrate" << QString::number(info.bitrate);
      }
      if (info.sampleRate > 0) {
        lst << "Samplerate" << QString::number(info.sampleRate);
      }
      if (info.channels > 0) {
        lst << "Channels" << QString::number(info.channels);
      }
      if (info.duration > 0) {
        lst << "Duration" << QString::number(info.duration);
      }
      if (info.channelMode == TaggedFile::DetailInfo::CM_Stereo ||
          info.channelMode == TaggedFile::DetailInfo::CM_JointStereo) {
        lst << "Channel Mode" <<
          (info.channelMode == TaggedFile::DetailInfo::CM_Stereo ?
           "Stereo" : "Joint Stereo");
      }
      if (info.vbr) {
        lst << "VBR" << "1";
      }
    }
    QString tag1 = taggedFile->getTagFormatV1();
    if (!tag1.isEmpty()) {
      lst << "Tag 1" << tag1;
    }
    QString tag2 = taggedFile->getTagFormatV2();
    if (!tag2.isEmpty()) {
      lst << "Tag 2" << tag2;
    }
  }
  return lst;
}

/**
 * Set tag from file name.
 *
 * @param tagMask tag bit (1 for tag 1, 2 for tag 2)
 */
void ScriptInterface::setTagFromFileName(int tagMask)
{
  if (tagMask & 1) {
    m_app->getTagsFromFilenameV1();
  } else if (tagMask & 2) {
    m_app->getTagsFromFilenameV2();
  }
}

/**
 * Set tag from other tag.
 *
 * @param tagMask tag bit (1 for tag 1, 2 for tag 2)
 */
void ScriptInterface::setTagFromOtherTag(int tagMask)
{
  if (tagMask & 1) {
    m_app->copyV2ToV1();
  } else if (tagMask & 2) {
    m_app->copyV1ToV2();
  }
}

/**
 * Copy tag.
 *
 * @param tagMask tag bit (1 for tag 1, 2 for tag 2)
 */
void ScriptInterface::copyTag(int tagMask)
{
  if (tagMask & 1) {
    m_app->copyTagsV1();
  } else if (tagMask & 2) {
    m_app->copyTagsV2();
  }
}

/**
 * Paste tag.
 *
 * @param tagMask tag bit (1 for tag 1, 2 for tag 2)
 */
void ScriptInterface::pasteTag(int tagMask)
{
  if (tagMask & 1) {
    m_app->pasteTagsV1();
  } else if (tagMask & 2) {
    m_app->pasteTagsV2();
  }
}

/**
 * Remove tag.
 *
 * @param tagMask tag bit (1 for tag 1, 2 for tag 2)
 */
void ScriptInterface::removeTag(int tagMask)
{
  if (tagMask & 1) {
    m_app->removeTagsV1();
  } else if (tagMask & 2) {
    m_app->removeTagsV2();
  }
}

/**
 * Hide or show tag in GUI.
 *
 * @param tagMask tag bit (1 for tag 1, 2 for tag 2)
 * @param hide    true to hide tag
 */
void ScriptInterface::hideTag(int tagMask, bool hide)
{
  if (tagMask & 1) {
    m_mainWin->m_form->hideV1(hide);
  } else if (tagMask & 2) {
    m_mainWin->m_form->hideV2(hide);
  }
}

/**
 * Reparse the configuration.
 * Automated configuration changes are possible by modifying
 * the configuration file and then reparsing the configuration.
 */
void ScriptInterface::reparseConfiguration()
{
  m_mainWin->readOptions();
}

#ifdef HAVE_PHONON
/**
 * Play selected audio files.
 */
void ScriptInterface::playAudio()
{
  m_app->playAudio();
}
#endif

#endif // HAVE_QTDBUS
