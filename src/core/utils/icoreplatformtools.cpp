/**
 * \file icoreplatformtools.cpp
 * Interface for GUI independent platform specific tools.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 06 Apr 2013
 *
 * Copyright (C) 2013-2018  Urs Fleisch
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

#include "icoreplatformtools.h"
#include <QString>

/**
 * Destructor.
 */
ICorePlatformTools::~ICorePlatformTools()
{
}

/**
 * Display dialog to select an existing file.
 * This default implementation only displays a warning, it is only supported
 * when having a GUI.
 * @param parent parent widget
 * @param caption caption
 * @param dir directory
 * @param filter filter
 * @param selectedFilter the selected filter is returned here
 * @return selected file, empty if canceled.
 */
QString ICorePlatformTools::getOpenFileName(QWidget* parent,
    const QString& caption, const QString& dir, const QString& filter,
    QString* selectedFilter)
{
  Q_UNUSED(parent)
  Q_UNUSED(selectedFilter)
  qWarning("getOpenFileName(%s, %s, %s) not implemented without GUI.",
           qPrintable(caption), qPrintable(dir), qPrintable(filter));
  return QString();
}

/**
 * Display dialog to select a file to save.
 * This default implementation only displays a warning, it is only supported
 * when having a GUI.
 * @param parent parent widget
 * @param caption caption
 * @param dir directory
 * @param filter filter
 * @param selectedFilter the selected filter is returned here
 * @return selected file, empty if canceled.
 */
QString ICorePlatformTools::getSaveFileName(QWidget* parent,
    const QString& caption, const QString& dir, const QString& filter,
    QString* selectedFilter)
{
  Q_UNUSED(parent)
  Q_UNUSED(selectedFilter)
  qWarning("getSaveFileName(%s, %s, %s) not implemented without GUI.",
           qPrintable(caption), qPrintable(dir), qPrintable(filter));
  return QString();
}

/**
 * Display dialog to select an existing directory.
 * This default implementation only displays a warning, it is only supported
 * when having a GUI.
 * @param parent parent widget
 * @param caption caption
 * @param startDir start directory
 * @return selected directory, empty if canceled.
 */
QString ICorePlatformTools::getExistingDirectory(QWidget* parent,
    const QString& caption, const QString& startDir)
{
  Q_UNUSED(parent)
  qWarning("getExistingDirectory(%s, %s) not implemented without GUI.",
           qPrintable(caption), qPrintable(startDir));
  return QString();
}

/**
 * Check if platform has a graphical user interface.
 * @return true if platform has GUI.
 */
bool ICorePlatformTools::hasGui() const
{
  return false;
}

/**
 * Construct a name filter string suitable for file dialogs.
 * This function can be used to implement fileDialogNameFilter()
 * for QFileDialog.
 * @param nameFilters list of description, filter pairs, e.g.
 * [("Images", "*.jpg *.jpeg *.png"), ("All Files", "*")].
 * @return name filter string.
 */
QString ICorePlatformTools::qtFileDialogNameFilter(
    const QList<QPair<QString, QString> >& nameFilters)
{
  QString filter;
  for (auto it = nameFilters.constBegin(); it != nameFilters.constEnd(); ++it) {
    if (!filter.isEmpty()) {
      filter += QLatin1String(";;");
    }
    filter += it->first;
    filter += QLatin1String(" (");
    filter += it->second;
    filter += QLatin1Char(')');
  }
  return filter;
}

/**
 * Get file pattern part of m_nameFilter.
 * This function can be used to implement getNameFilterPatterns()
 * for QFileDialog.
 * @param nameFilter name filter string
 * @return file patterns, e.g. "*.mp3".
 */
QString ICorePlatformTools::qtNameFilterPatterns(const QString& nameFilter)
{
  int start = nameFilter.indexOf(QLatin1Char('(')),
      end = nameFilter.indexOf(QLatin1Char(')'));
  return start != -1 && end != -1 && end > start
      ? nameFilter.mid(start + 1, end - start - 1)
      : QString();
}
