/**
 * \file configdialogpages.h
 * Pages for configuration dialog.
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

#pragma once

#include <QObject>
#include "kid3api.h"

class QCheckBox;
class QString;
class QWidget;
class QComboBox;
class QLineEdit;
class QSpinBox;
class QStringListModel;
class QStandardItemModel;
class FormatBox;
class ConfigTable;
class CommandsTableModel;
class CheckableStringListModel;
class StarRatingMappingsModel;
class FormatConfig;
class TagConfig;
class FileConfig;
class UserActionsConfig;
class GuiConfig;
class NetworkConfig;
class ImportConfig;
class PlaylistConfig;
class IPlatformTools;

/**
 * Pages for configuration dialog.
 */
class KID3_GUI_EXPORT ConfigDialogPages : public QObject {
  Q_OBJECT
public:
  /**
   * Constructor.
   */
  explicit ConfigDialogPages(IPlatformTools* platformTools, QObject* parent = nullptr);

  /**
   * Destructor.
   */
  virtual ~ConfigDialogPages() override = default;

  /**
   * Create page with tags settings.
   * @return tags page.
   */
  QWidget* createTagsPage();

  /**
   * Create page with files settings.
   * @return files page.
   */
  QWidget* createFilesPage();

  /**
   * Create page with actions settings.
   * @return actions page.
   */
  QWidget* createActionsPage();

  /**
   * Create page with network settings.
   * @return network page.
   */
  QWidget* createNetworkPage();

  /**
   * Create page with plugins settings.
   * @return plugins page.
   */
  QWidget* createPluginsPage();

  /**
   * Set values in pages from current configuration.
   */
  void setConfig();

  /**
   * Get values from pages and store them in the current configuration.
   */
  void getConfig() const;

public slots:
  /**
   * Set values in pages from default configuration.
   */
  void setDefaultConfig();

private slots:
  void editFormatsFromTag();
  void editFormatsToTag();
  void editPlaylistFormats();
  void onCustomFramesEditModelChanged();

private:
  /**
   * Set values in dialog from given configurations.
   */
  void setConfigs(
      const FormatConfig& fnCfg, const FormatConfig& id3Cfg,
      const TagConfig& tagCfg, const FileConfig& fileCfg,
      const UserActionsConfig& userActionsCfg, const GuiConfig& guiCfg,
      const NetworkConfig& networkCfg, const ImportConfig& importCfg,
      const PlaylistConfig& playlistCfg);

  void getQuickAccessFramesConfig(QList<int>& frameTypes,
                                  quint64& frameMask) const;
  void setQuickAccessFramesConfig(const QList<int>& types,
                                  quint64 frameMask);

  IPlatformTools* m_platformTools;
  /** Load last-opened files checkbox */
  QCheckBox* m_loadLastOpenedFileCheckBox;
  /** Preserve timestamp checkbox */
  QCheckBox* m_preserveTimeCheckBox;
  /** Mark changes checkbox */
  QCheckBox* m_markChangesCheckBox;
  /** Filename for cover lineedit */
  QLineEdit* m_coverFileNameLineEdit;
  /** File list name filter combo box */
  QComboBox* m_nameFilterComboBox;
  /** Include folders lineedit */
  QLineEdit* m_includeFoldersLineEdit;
  /** Exclude folders lineedit */
  QLineEdit* m_excludeFoldersLineEdit;
  /** Show hidden files checkbox */
  QCheckBox* m_showHiddenFilesCheckBox;
  /** File text encoding combo box */
  QComboBox* m_fileTextEncodingComboBox;
  /** Mark truncated fields checkbox */
  QCheckBox* m_markTruncationsCheckBox;
  /** ID3v1 text encoding combo box */
  QComboBox* m_textEncodingV1ComboBox;
  /** Use track/total number of tracks format checkbox */
  QCheckBox* m_totalNumTracksCheckBox;
  /** Comment field name combo box */
  QComboBox* m_commentNameComboBox;
  /** Picture field name combo box */
  QComboBox* m_pictureNameComboBox;
  /** Mark if larger check box */
  QCheckBox* m_markOversizedPicturesCheckBox;
  /** Maximum picture size spin box */
  QSpinBox* m_maximumPictureSizeSpinBox;
  /** Genre as text instead of numeric string checkbox */
  QCheckBox* m_genreNotNumericCheckBox;
  /** WAV files with lowercase id3 chunk checkbox */
  QCheckBox* m_lowercaseId3ChunkCheckBox;
  /** Mark standard violations check box */
  QCheckBox* m_markStandardViolationsCheckBox;
  /** ID3v2 text encoding combo box */
  QComboBox* m_textEncodingComboBox;
  /** ID3v2 version combo box */
  QComboBox* m_id3v2VersionComboBox;
  /** Number of digits in track number spin box */
  QSpinBox* m_trackNumberDigitsSpinBox;
  /** Filename Format box */
  FormatBox* m_fnFormatBox;
  /** ID3 Format box */
  FormatBox* m_tagFormatBox;
  /** Only custom genres checkbox */
  QCheckBox* m_onlyCustomGenresCheckBox;
  /** Model with list of custom genres */
  QStringListModel* m_genresEditModel;
  /** Model with list of custom frame names */
  QStringListModel* m_customFramesEditModel;
  /** Model with standard tags selection */
  QStandardItemModel* m_quickAccessTagsModel;
  /** Model with star rating mappings */
  StarRatingMappingsModel* m_starRatingMappingsModel;
  /** Track field name combo box */
  QComboBox* m_trackNameComboBox;
  /** Play on double click checkbox */
  QCheckBox* m_playOnDoubleClickCheckBox;
  /** Select file on play checkbox */
  QCheckBox* m_selectFileOnPlayCheckBox;
  /** Commands table */
  ConfigTable* m_commandsTable;
  /** Commands table model */
  CommandsTableModel* m_commandsTableModel;
  /** Browser line edit */
  QLineEdit* m_browserLineEdit;
  /** Use proxy check box */
  QCheckBox* m_proxyCheckBox;
  /** Proxy line edit */
  QLineEdit* m_proxyLineEdit;
  /** Use proxy authentication check box */
  QCheckBox* m_proxyAuthenticationCheckBox;
  /** Proxy user name line edit */
  QLineEdit* m_proxyUserNameLineEdit;
  /** Proxy password line edit */
  QLineEdit* m_proxyPasswordLineEdit;
  /** Model with enabled metadata plugins */
  CheckableStringListModel* m_enabledMetadataPluginsModel;
  /** Model with enabled plugins */
  CheckableStringListModel* m_enabledPluginsModel;
  /** Available formats for tag to filename */
  QStringList m_toFilenameFormats;
  /** Available formats for tag from filename */
  QStringList m_fromFilenameFormats;
  /** Available playlist file name formats */
  QStringList m_playlistFileNameFormats;
};
