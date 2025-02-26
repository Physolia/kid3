/**
 * \file saferename.cpp
 * Safely rename a file.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 16 Feb 2012
 *
 * Copyright (C) 2012-2018  Urs Fleisch
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

#include "saferename.h"
#include <QDir>
#include "formatconfig.h"

#ifdef Q_OS_WIN32

bool Utils::hasIllegalFileNameCharacters(const QString& fileName)
{
  static const char illegalChars[] = "<>:\"|?*";
  QString fileNameWithoutDrive(
      (QDir::isAbsolutePath(fileName) && fileName.mid(1, 2) == QLatin1String(":/"))
        ? fileName.mid(3) : fileName);
  for (const char* chPtr = illegalChars; *chPtr; ++chPtr) {
    if (fileNameWithoutDrive.contains(QLatin1Char(*chPtr))) {
      return true;
    }
  }
  return false;
}

#else

bool Utils::hasIllegalFileNameCharacters(const QString&)
{
  return false;
}

#endif

bool Utils::safeRename(const QString& oldName, const QString& newName)
{
  if (hasIllegalFileNameCharacters(newName))
    return false;

  return QDir().rename(oldName, newName);
}

bool Utils::safeRename(const QString& dirPath,
                       const QString& oldName, const QString& newName)
{
  if (hasIllegalFileNameCharacters(newName))
    return false;

  return QDir(dirPath).rename(oldName, newName);
}

bool Utils::replaceIllegalFileNameCharacters(
    QString& fileName, const QString& defaultReplacement,
    const char* illegalChars)
{
  if (!illegalChars) {
#ifdef Q_OS_WIN32
    illegalChars = "<>:\"|?*\\/";
#else
    illegalChars = "/";
#endif
  }
  QMap<QChar, QString> replaceMap;
  bool changed = false;
  for (const char* ic = illegalChars; *ic; ++ic) {
    QChar illegalChar = QLatin1Char(*ic);
    if (fileName.contains(illegalChar)) {
      if (!changed) {
        const FormatConfig& fnCfg = FilenameFormatConfig::instance();
        if (fnCfg.strRepEnabled()) {
          const auto constStrRepMap = fnCfg.strRepMap();
          for (const auto& keyVal : constStrRepMap) {
            if (keyVal.first.length() == 1) {
              replaceMap.insert(keyVal.first.at(0), keyVal.second);
            }
          }
        }
        changed = true;
      }
      QString replacement = replaceMap.value(illegalChar, defaultReplacement);
      fileName.replace(illegalChar, replacement);
    }
  }
  return changed;
}
