/**
 * \file id3form.h
 * GUI for kid3, originally generated by QT Designer.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 8 Apr 2003
 *
 * Copyright (C) 2003-2008  Urs Fleisch
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

#ifndef ID3FORM_H
#define ID3FORM_H

#include <QSplitter>
#include <QLabel>
#include <QComboBox>
#include <QLineEdit>
#include "filelist.h"
#include "dirlist.h"
#include "picturelabel.h"
#include "taggedfile.h"
#include "qtcompatmac.h"

class QCheckBox;
class QPushButton;
class QToolButton;
class QSpinBox;
class FormatConfig;
class QGridLayout;
class QGroupBox;
class QPixmap;
class QListWidget;
class QListWidgetItem;
class FrameList;
class FrameTable;

/**
 * Main widget.
 */
class Id3Form : public QSplitter
{
Q_OBJECT

public:
	/** 
	 * Constructs an Id3Form as a child of 'parent', with the 
	 * name 'name' and widget flags set to 'f'.
	 * @param parent parent widget
	 */
	Id3Form(QWidget* parent = 0);

	/**
	 * Destructor.
	 */
	virtual ~Id3Form();

	/**
	 * Enable or disable controls requiring ID3v1 tags.
	 *
	 * @param enable true to enable
	 */
	void enableControlsV1(bool enable);

	/**
	 * Display the format of tag 1.
	 *
	 * @param str string describing format, e.g. "ID3v1.1"
	 */
	void setTagFormatV1(const QString& str);

	/**
	 * Display the format of tag 2.
	 *
	 * @param str string describing format, e.g. "ID3v2.4"
	 */
	void setTagFormatV2(const QString& str);

	/**
	 * Adjust the size of the right half box.
	 */
	void adjustRightHalfBoxSize();

	/**
	 * Hide or show file controls.
	 *
	 * @param hide true to hide, false to show
	 */
	void hideFile(bool hide);

	/**
	 * Hide or show tag 1 controls.
	 *
	 * @param hide true to hide, false to show
	 */
	void hideV1(bool hide);

	/**
	 * Hide or show tag 2 controls.
	 *
	 * @param hide true to hide, false to show
	 */
	void hideV2(bool hide);

	/**
	 * Hide or show picture.
	 *
	 * @param hide true to hide, false to show
	 */
	void hidePicture(bool hide);

	/**
	 * Save the local settings to the configuration.
	 */
	void saveConfig();

	/**
	 * Read the local settings from the configuration.
	 */
	void readConfig();

	/**
	 * Init GUI.
	 */
	void initView();

	/**
	 * Get current filename format string.
	 * @return filename format.
	 */
	QString getFilenameFormat() const { return m_formatComboBox->currentText(); }

	/**
	 * Set current filename format string.
	 * @param fmt filename format
	 */
	void setFilenameFormat(const QString& fmt) { m_formatComboBox->setEditText(fmt); }

	/**
	 * Get current from filename format string.
	 * @return filename format.
	 */
	QString getFromFilenameFormat() const {
		return m_formatFromFilenameComboBox->currentText();
	}

	/**
	 * Set current from filename format string.
	 * @param fmt filename format
	 */
	void setFromFilenameFormat(const QString& fmt) {
		m_formatFromFilenameComboBox->setEditText(fmt);
	}

	/**
	 * Get filename.
	 */
	QString getFilename() const { return m_nameLineEdit->text(); }

	/**
	 * Set filename.
	 * @param fn filename
	 */
	void setFilename(const QString& fn) { m_nameLineEdit->setText(fn); }

	/**
	 * Check if the filename line edit is enabled.
	 * @return true if the filename line edit is enabled.
	 */
	bool isFilenameEditEnabled() const { return m_nameLineEdit->isEnabled(); }

	/**
	 * Enable or disable the filename line edit.
	 * @param en true to enable
	 */
	void setFilenameEditEnabled(bool en) { m_nameLineEdit->setEnabled(en); }

	/**
	 * Mark the filename as changed.
	 * @param en true to mark as changed
	 */
	void markChangedFilename(bool en);

	/**
	 * Set preview picture data.
	 * @param data picture data, 0 if no picture is available
	 */
	void setPictureData(const QByteArray* data) { m_pictureLabel->setData(data); }

	/**
	 * Set details info text.
	 *
	 * @param info detail information
	 */
	void setDetailInfo(const TaggedFile::DetailInfo& info);

	/**
	 * Fill directory list.
	 * @param index index of directory in filesystem model
	 * @return false if name is not directory path, else true.
	 */
	bool readDirectoryList(const QModelIndex& index) { return m_dirListBox->readDir(index); }

	/**
	 * Fill file list.
	 * @param dir      path of directory
	 * @param fileName name of file to select (optional, else empty)
	 * @return false if name is not directory path, else true.
	 */
	bool readFileList(const QString& dir, const QString& fileName = QString()) {
		return m_fileListBox->readDir(dir, fileName);
	}

	/**
	 * Get the first item in the filelist.
	 * @return first file.
	 */
	FileListItem* firstFile() { return m_fileListBox->first(); }

	/**
	 * Get the next item in the filelist.
	 * @return next file.
	 */
	FileListItem* nextFile() { return m_fileListBox->next(); }

	/**
	 * Get the next item in the filelist.
	 * @return next file.
	 */
	FileListItem* currentFile() { return m_fileListBox->current(); }

	/**
	 * Get the first item in the the current directory.
	 * @return first file.
	 */
	FileListItem* firstFileInDir() { return m_fileListBox->firstInDir(); }

	/**
	 * Get the next item in the current directory.
	 * @return next file.
	 */
	FileListItem* nextFileInDir() { return m_fileListBox->nextInDir(); }

	/**
	 * Get the first file or directory item in the filelist.
	 *
	 * @return first file.
	 */
	FileListItem* firstFileOrDir() { return m_fileListBox->firstFileOrDir(); }

	/**
	 * Get the next file or directory item in the filelist.
	 *
	 * @return next file.
	 */
	FileListItem* nextFileOrDir() { return m_fileListBox->nextFileOrDir(); }

	/**
	 * Get information about directory.
	 * @return directory information.
	 */
	const DirInfo* getDirInfo() const { return m_fileListBox->getDirInfo(); }

	/**
	 * Refresh text of all files in file list and check if any file is modified.
	 * @return true if a file is modified.
	 */
	bool updateModificationState() { return m_fileListBox->updateModificationState(); }

	/**
	 * Get file list.
	 * @return file list.
	 */
	FileList* getFileList() { return m_fileListBox; }

	/**
	 * Get frame list.
	 * @return frame list.
	 */
	FrameList* getFrameList() { return m_framelist; }

	/**
	 * Get tag 1 frame table.
	 * @return frame table.
	 */
	FrameTable* frameTableV1() { return m_framesV1Table; }

	/**
	 * Get tag 2 frame table.
	 * @return frame table.
	 */
	FrameTable* frameTableV2() { return m_framesV2Table; }

public slots:
	/**
	 * Button ID3v1 From Filename.
	 */
	void fromFilenameV1();

	/**
	 * Button ID3v2 From Filename.
	 */
	void fromFilenameV2();

	/**
	 * Button ID3v2 From ID3v1.
	 */
	void fromID3V2();

	/**
	 * Button ID3v1 From ID3v2.
	 */
	void fromID3V1();

	/**
	 * Button ID3v1 Copy.
	 */
	void copyV1();

	/**
	 * Button ID3v2 Copy.
	 */
	void copyV2();

	/**
	 * Button ID3v2 Remove.
	 */
	void removeV2();

	/**
	 * Button ID3v1 Paste.
	 */
	void pasteV1();

	/**
	 * Button ID3v2 Paste.
	 */
	void pasteV2();

	/**
	 * Button ID3v1 Remove.
	 */
	void removeV1();

	/**
	 * File list box file selected
	 */
	void fileSelected();

	/**
	 * Get number of files selected in file list box.
	 *
	 * @return number of files selected.
	 */
	int numFilesSelected();

	/**
	 * Get the number of files or directories selected in the file list box.
	 *
	 * @return number of files or directories selected.
	 */
	int numFilesOrDirsSelected();

	/**
	 * Frame list button Edit.
	 */
	void editFrame();

	/**
	 * Frame list button Add.
	 */
	void addFrame();

	/**
	 * Frame list button Delete.
	 */
	void deleteFrame();

	/**
	 * Set filename according to ID3v1 tags.
	 */
	void fnFromID3V1();

	/**
	 * Set filename according to ID3v1 tags.
	 */
	void fnFromID3V2();

	/**
	 * Filename line edit is changed.
	 * @param txt contents of line edit
	 */
	void nameLineEditChanged(const QString& txt);

	/**
	 * Directory list box directory selected.
	 *
	 * @param index selected item
	 */
	void dirSelected(const QModelIndex& index);

	/**
	 * Set focus on filename controls.
	 */
	void setFocusFilename();

	/**
	 * Set focus on tag 1 controls.
	 */
	void setFocusV1();

	/**
	 * Set focus on tag 2 controls.
	 */
	void setFocusV2();

	/**
	 * Select all files.
	 */
	void selectAllFiles();

	/**
	 * Deselect all files.
	 */
	void deselectAllFiles();

	/**
	 * Select first file.
	 *
	 * @return true if a file exists.
	 */
	bool selectFirstFile();

	/**
	 * Select next file.
	 *
	 * @return true if a next file exists.
	 */
	bool selectNextFile();

	/**
	 * Select previous file.
	 *
	 * @return true if a previous file exists.
	 */
	bool selectPreviousFile();

signals:
	/**
	 * Emitted when some of the selected files have been renamed.
	 */
	void selectedFilesRenamed();

	/**
	 * Emitted when the window is resized.
	 */
	void windowResized();

private:
	/**
	 * Format string within line edit.
	 *
	 * @param le   line edit
	 * @param txt  text in line edit
	 * @param fcfg format configuration
	 */
	void formatLineEdit(QLineEdit* le, const QString& txt,
						const FormatConfig* fcfg);

	FileList* m_fileListBox;
	QComboBox* m_formatComboBox;
	QComboBox* m_formatFromFilenameComboBox;
	QLabel* m_nameLabel;
	QLineEdit* m_nameLineEdit;
	QListWidget* m_framesListBox;
	DirList* m_dirListBox;
	FrameList* m_framelist;
	FrameTable* m_framesV1Table;
	FrameTable* m_framesV2Table;
	QSplitter* m_vSplitter;
	QWidget* m_fileWidget;
	QWidget* m_tag1Widget;
	QWidget* m_tag2Widget;
	QToolButton* m_fileButton;
	QToolButton* m_tag1Button;
	QToolButton* m_tag2Button;
	QLabel* m_fileLabel;
	QLabel* m_tag1Label;
	QLabel* m_tag2Label;
	QPushButton* m_fnV1Button;
	QPushButton* m_toTagV1Button;
	QPushButton* m_id3V2PushButton;
	QWidget* m_rightHalfVBox;
	PictureLabel* m_pictureLabel;

	/** Collapse pixmap, will be allocated in constructor */
	static QPixmap* s_collapsePixmap;
	/** Expand pixmap, will be allocated in constructor */
	static QPixmap* s_expandPixmap;

private slots:
	/**
	 * Accept drag.
	 *
	 * @param ev drag event.
	 */
	void dragEnterEvent(QDragEnterEvent* ev);

	/**
	 * Handle drop event.
	 *
	 * @param ev drop event.
	 */
	void dropEvent(QDropEvent* ev);

	/**
	 * Toggle visibility of file controls.
	 */
	void showHideFile();

	/**
	 * Toggle visibility of tag 1 controls.
	 */
	void showHideTag1();

	/**
	 * Toggle visibility of tag 2 controls.
	 */
	void showHideTag2();
};

#endif // ID3FORM_H
