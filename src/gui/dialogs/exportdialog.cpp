/**
 * \file exportdialog.cpp
 * Export dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 10 May 2006
 *
 * Copyright (C) 2006-2018  Urs Fleisch
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

#include "exportdialog.h"
#include <QGuiApplication>
#include <QClipboard>
#include <QLayout>
#include <QPushButton>
#include <QLabel>
#include <QCheckBox>
#include <QSpinBox>
#include <QString>
#include <QTextEdit>
#include <QTableView>
#include <QComboBox>
#include <QDir>
#include <QUrl>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include "taggedfile.h"
#include "genres.h"
#include "exportconfig.h"
#include "importconfig.h"
#include "contexthelp.h"
#include "textexporter.h"
#include "texttablemodel.h"
#include "formatlistedit.h"
#include "iplatformtools.h"

/**
 * Constructor.
 *
 * @param platformTools platform tools
 * @param parent       parent widget
 * @param textExporter text exporter to use
 */
ExportDialog::ExportDialog(IPlatformTools* platformTools,
                           QWidget* parent, TextExporter* textExporter)
  : QDialog(parent), m_platformTools(platformTools),
    m_textExporter(textExporter), m_textTableModel(new TextTableModel(this))
{
  setObjectName(QLatin1String("ExportDialog"));
  setModal(true);
  setWindowTitle(tr("Export"));
  setSizeGripEnabled(true);

  auto vlayout = new QVBoxLayout(this);
  m_edit = new QTextEdit(this);
  m_edit->setAcceptRichText(false);
  vlayout->addWidget(m_edit);

  m_table = new QTableView(this);
  m_table->setModel(m_textTableModel);
  m_table->hide();
  vlayout->addWidget(m_table);

  QString formatToolTip = ImportTrackData::getFormatToolTip();
  m_formatListEdit = new FormatListEdit(
        {tr("&Format:"), tr("H&eader:"), tr("&Tracks:"), tr("F&ooter:")},
        {QString(), formatToolTip, formatToolTip, formatToolTip},
        this);
  connect(m_formatListEdit, &FormatListEdit::formatChanged,
          this, &ExportDialog::showPreview);
  vlayout->addWidget(m_formatListEdit);

  auto butlayout = new QHBoxLayout;
  m_fileButton = new QPushButton(tr("To F&ile..."), this);
  m_fileButton->setAutoDefault(false);
  butlayout->addWidget(m_fileButton);
  connect(m_fileButton, &QAbstractButton::clicked, this, &ExportDialog::slotToFile);

  m_clipButton = new QPushButton(tr("To Clip&board"), this);
  m_clipButton->setAutoDefault(false);
  butlayout->addWidget(m_clipButton);
  connect(m_clipButton, &QAbstractButton::clicked, this, &ExportDialog::slotToClipboard);

  auto butspacer = new QSpacerItem(16, 0, QSizePolicy::Expanding,
                                           QSizePolicy::Minimum);
  butlayout->addItem(butspacer);

  QLabel* srcLabel = new QLabel(tr("&Source:"), this);
  butlayout->addWidget(srcLabel);
  m_srcComboBox = new QComboBox(this);
  m_srcComboBox->setEditable(false);
  FOR_ALL_TAGS(tagNr) {
    m_srcComboBox->addItem(tr("Tag %1").arg(Frame::tagNumberToString(tagNr)),
                           Frame::tagVersionFromNumber(tagNr));
  }
  srcLabel->setBuddy(m_srcComboBox);
  butlayout->addWidget(m_srcComboBox);
  connect(m_srcComboBox, static_cast<void (QComboBox::*)(int)>(
            &QComboBox::activated),
          this, &ExportDialog::onSrcComboBoxActivated);

  vlayout->addLayout(butlayout);

  auto hlayout = new QHBoxLayout;
  QPushButton* helpButton = new QPushButton(tr("&Help"), this);
  helpButton->setAutoDefault(false);
  hlayout->addWidget(helpButton);
  connect(helpButton, &QAbstractButton::clicked, this, &ExportDialog::showHelp);

  QPushButton* saveButton = new QPushButton(tr("&Save Settings"), this);
  saveButton->setAutoDefault(false);
  hlayout->addWidget(saveButton);
  connect(saveButton, &QAbstractButton::clicked, this, &ExportDialog::saveConfig);

  auto hspacer = new QSpacerItem(16, 0, QSizePolicy::Expanding,
                                         QSizePolicy::Minimum);
  hlayout->addItem(hspacer);

  QPushButton* closeButton = new QPushButton(tr("&Close"), this);
  closeButton->setAutoDefault(false);
  hlayout->addWidget(closeButton);
  connect(closeButton, &QAbstractButton::clicked, this, &QDialog::accept);

  vlayout->addLayout(hlayout);
}

/**
 * Export to a file.
 */
void ExportDialog::slotToFile()
{
  QString fileName = m_platformTools->getSaveFileName(this, QString(),
      ImportConfig::instance().importDir(), QString(), nullptr);
  if (!fileName.isEmpty()) {
    if (!m_textExporter->exportToFile(fileName)) {
      QMessageBox::warning(
        nullptr, tr("File Error"),
        tr("Error while writing file:\n") + fileName,
        QMessageBox::Ok, QMessageBox::NoButton);
    }
  }
}

/**
 * Export to clipboard.
 */
void ExportDialog::slotToClipboard()
{
  QGuiApplication::clipboard()->setText(m_textExporter->getText(),
                                        QClipboard::Clipboard);
}

/**
 * Show exported text as preview in editor.
 */
void ExportDialog::showPreview()
{
  m_textExporter->updateText(m_formatListEdit->getCurrentFormat(1),
                             m_formatListEdit->getCurrentFormat(2),
                             m_formatListEdit->getCurrentFormat(3));
  QString text(m_textExporter->getText());
  if (m_textTableModel->setText(
        text, !m_formatListEdit->getCurrentFormat(1).isEmpty())) {
    m_table->resizeColumnsToContents();
    m_table->show();
    m_edit->hide();
  } else {
    m_edit->setPlainText(text);
    m_table->hide();
    m_edit->show();
  }
}

/**
 * Set the format combo box and line edits from the configuration.
 */
void ExportDialog::setFormatFromConfig()
{
  const ExportConfig& exportCfg = ExportConfig::instance();
  m_formatListEdit->setFormats(
        {exportCfg.exportFormatNames(), exportCfg.exportFormatHeaders(),
         exportCfg.exportFormatTracks(), exportCfg.exportFormatTrailers()},
        exportCfg.exportFormatIndex());
}

/**
 * Read the local settings from the configuration.
 */
void ExportDialog::readConfig()
{
  m_srcComboBox->setCurrentIndex(
      m_srcComboBox->findData(ExportConfig::instance().exportSource()));

  setFormatFromConfig();

  if (!ExportConfig::instance().exportWindowGeometry().isEmpty()) {
    restoreGeometry(ExportConfig::instance().exportWindowGeometry());
  }
}

/**
 * Save the local settings to the configuration.
 */
void ExportDialog::saveConfig()
{
  ExportConfig& exportCfg = ExportConfig::instance();
  exportCfg.setExportSource(Frame::tagVersionCast(
    m_srcComboBox->itemData(m_srcComboBox->currentIndex()).toInt()));
  int idx;
  QList<QStringList> formats = m_formatListEdit->getFormats(&idx);
  exportCfg.setExportFormatIndex(idx);
  exportCfg.setExportFormatNames(formats.at(0));
  exportCfg.setExportFormatHeaders(formats.at(1));
  exportCfg.setExportFormatTracks(formats.at(2));
  exportCfg.setExportFormatTrailers(formats.at(3));
  exportCfg.setExportWindowGeometry(saveGeometry());

  setFormatFromConfig();
}

/**
 * Show help.
 */
void ExportDialog::showHelp()
{
  ContextHelp::displayHelp(QLatin1String("export"));
}

/**
 * Called when the source combo box selection is changed.
 * @param index combo box index
 */
void ExportDialog::onSrcComboBoxActivated(int index)
{
  m_textExporter->readTagsInTrackData(
        Frame::tagVersionCast(m_srcComboBox->itemData(index).toInt()));
  showPreview();
}
