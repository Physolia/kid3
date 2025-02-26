/**
 * \file textexporter.cpp
 * Export tags as text.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 22 Jul 2011
 *
 * Copyright (C) 2011-2018  Urs Fleisch
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

#include "textexporter.h"
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QTextStream>
#include "exportconfig.h"
#include "importconfig.h"
#include "fileconfig.h"

/**
 * Constructor.
 * @param parent parent object
 */
TextExporter::TextExporter(QObject* parent) : QObject(parent)
{
  setObjectName(QLatin1String("TextExporter"));
}

/**
 * Destructor.
 */
TextExporter::~TextExporter()
{
  // not inline or default to silence weak-vtables warning
}

/**
 * Update text from tags.
 *
 * @param headerFormat header format
 * @param trackFormat track format
 * @param trailerFormat trailer format
 */
void TextExporter::updateText(
  const QString& headerFormat, const QString& trackFormat,
  const QString& trailerFormat)
{
  m_text.clear();
  const int numTracks = m_trackDataVector.size();
  int trackNr = 0;
  for (auto it = m_trackDataVector.constBegin();
       it != m_trackDataVector.constEnd();
       ++it) {
    if (trackNr == 0 && !headerFormat.isEmpty()) {
      m_text.append((*it).formatString(headerFormat));
      m_text.append(QLatin1Char('\n'));
    }
    if (!trackFormat.isEmpty()) {
      m_text.append((*it).formatString(trackFormat));
      m_text.append(QLatin1Char('\n'));
    }
    if (trackNr == numTracks - 1 && !trailerFormat.isEmpty()) {
      m_text.append((*it).formatString(trailerFormat));
      m_text.append(QLatin1Char('\n'));
    }
    ++trackNr;
  }
}

/**
 * Update text from tags using formats from the configuration.
 *
 * int fmtIdx index of format
 */
void TextExporter::updateTextUsingConfig(int fmtIdx)
{
  const ExportConfig& exportCfg = ExportConfig::instance();
  const QStringList headerFmts = exportCfg.exportFormatHeaders();
  const QStringList trackFmts = exportCfg.exportFormatTracks();
  const QStringList trailerFmts = exportCfg.exportFormatTrailers();
  if (fmtIdx < headerFmts.size() && fmtIdx < trackFmts.size() &&
      fmtIdx < trailerFmts.size()) {
    updateText(headerFmts.at(fmtIdx), trackFmts.at(fmtIdx),
               trailerFmts.at(fmtIdx));
  }
}

/**
 * Export to a file.
 *
 * @param fn file name
 *
 * @return true if ok.
 */
bool TextExporter::exportToFile(const QString& fn)
{
  if (!fn.isEmpty()) {
    QFile file(fn);
    if (file.open(QIODevice::WriteOnly)) {
      ImportConfig::instance().setImportDir(QFileInfo(file).dir().path());
      QTextStream stream(&file);
      QString codecName = FileConfig::instance().textEncoding();
      if (codecName != QLatin1String("System")) {
#if QT_VERSION >= 0x060000
        if (auto encoding = QStringConverter::encodingForName(codecName.toLatin1())) {
          stream.setEncoding(*encoding);
        }
#else
        stream.setCodec(codecName.toLatin1());
#endif
      }
      stream << m_text;
      file.close();
      return true;
    }
  }
  return false;
}
