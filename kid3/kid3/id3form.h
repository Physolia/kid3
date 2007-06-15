/**
 * \file id3form.h
 * GUI for kid3, originally generated by QT Designer.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 8 Apr 2003
 */

#ifndef ID3FORM_H
#define ID3FORM_H

#include <qsplitter.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include "filelist.h"
#include "dirlist.h"
#include "standardtags.h"
#include "qtcompatmac.h"

class QCheckBox;
class QPushButton;
class QSpinBox;
class Kid3ScrollView;
class FormatConfig;
class Q3VBoxLayout;
class QGridLayout;
class Q3GroupBox;
class Q3ListBox;
class Q3ListBoxItem;
class Q3ScrollView;
class Q3VBox;
class Q3HBox;

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
	 * @param name   Qt name
	 */
	Id3Form(QWidget* parent = 0, const char* name = 0);

	/**
	 * Get filter from ID3v1 check boxes.
	 *
	 * @return filter.
	 */
	StandardTagsFilter getFilterFromID3V1();

	/**
	 * Get filter from ID3v2 check boxes.
	 *
	 * @return filter.
	 */
	StandardTagsFilter getFilterFromID3V2();

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
	 * Set details info text.
	 * @param text details text
	 */
	void setDetailInfo(const QString& text) { m_detailsLabel->setText(text); }

	/**
	 * Fill directory list.
	 * @param dir path of directory
	 * @return false if name is not directory path, else true.
	 */
	bool readDirectoryList(const QString& dir) { return m_dirListBox->readDir(dir); }

	/**
	 * Fill file list.
	 * @param dir path of directory
	 * @return false if name is not directory path, else true.
	 */
	bool readFileList(const QString& dir) { return m_fileListBox->readDir(dir); }

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
	 * Refresh text of all files in file list and check if any file is modified.
	 * @return true if a file is modified.
	 */
	bool updateModificationState() { return m_fileListBox->updateModificationState(); }

	/**
	 * Set the custom genres configuration from the entries in the combo box.
	 */
	void customGenresComboBoxToConfig() const;

	/**
	 * Set the custom genres combo box from the configuration.
	 */
	void customGenresConfigToComboBox();

	/**
	 * Mark truncated ID3v1 fields.
	 *
	 * @param flags truncation flags
	 * @see StandardTags::TruncationFlag.
	 */
	void markTruncatedFields(unsigned flags);

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
	 * Get standard tags from the ID3v1 controls.
	 *
	 * @param st standard tags to store result
	 */
	void getStandardTagsV1(StandardTags* st);

	/**
	 * Get standard tags from the ID3v2 controls.
	 *
	 * @param st standard tags to store result
	 */
	void getStandardTagsV2(StandardTags* st);

	/**
	 * Set ID3v1 standard tags controls.
	 *
	 * @param st standard tags to set
	 */
	void setStandardTagsV1(const StandardTags* st);

	/**
	 * Set ID3v2 standard tags controls.
	 *
	 * @param st standard tags to set
	 */
	void setStandardTagsV2(const StandardTags* st);

	/**
	 * Set all ID3v1 and ID3v2 check boxes on or off.
	 *
	 * @param val true to set check boxes on.
	 */
	void setAllCheckBoxes(bool val);

	/**
	 * Get number of files selected in file list box.
	 *
	 * @return number of files selected.
	 */
	int numFilesSelected();

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
	 * Album V1 line edit is changed.
	 * @param txt contents of line edit
	 */
	void albumV1LineEditChanged(const QString& txt);

	/**
	 * Artist V1 line edit is changed.
	 * @param txt contents of line edit
	 */
	void artistV1LineEditChanged(const QString& txt);

	/**
	 * Title V1 line edit is changed.
	 * @param txt contents of line edit
	 */
	void titleV1LineEditChanged(const QString& txt);

	/**
	 * Album V2 line edit is changed.
	 * @param txt contents of line edit
	 */
	void albumV2LineEditChanged(const QString& txt);

	/**
	 * Artist V2 line edit is changed.
	 * @param txt contents of line edit
	 */
	void artistV2LineEditChanged(const QString& txt);

	/**
	 * Title V2 line edit is changed.
	 * @param txt contents of line edit
	 */
	void titleV2LineEditChanged(const QString& txt);

	/**
	 * Directory list box directory selected.
	 *
	 * @param item selected item
	 */
#if QT_VERSION >= 0x040000
	void dirSelected(Q3ListBoxItem* item);
#else
	void dirSelected(QListBoxItem* item);
#endif

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
	 * Select next file.
	 */
	void selectNextFile();

	/**
	 * Select previous file.
	 */
	void selectPreviousFile();

signals:
	/**
	 * Emitted when some of the selected files have been renamed.
	 */
	void selectedFilesRenamed();

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

	/**
	 * Set ID3v1 genre controls.
	 *
	 * @param genreStr genre string
	 */
	void setGenreV1(const QString& genreStr);

	/**
	 * Set ID3v2 genre controls.
	 *
	 * @param genreStr genre string
	 */
	void setGenreV2(const QString& genreStr);

	FileList* m_fileListBox;
	QComboBox* m_genreV1ComboBox;
	QComboBox* m_genreV2ComboBox;
	QComboBox* m_formatComboBox;
	QLineEdit* m_nameLineEdit;
	QLineEdit* m_titleV1LineEdit;
	QLineEdit* m_titleV2LineEdit;
	QLabel* m_detailsLabel;
	Q3ListBox* m_framesListBox;
	DirList* m_dirListBox;
	QSplitter* m_vSplitter;
	Q3GroupBox* m_idV1GroupBox;
	Q3GroupBox* m_idV2GroupBox;
	QPushButton* m_fnV1Button;
	QLineEdit* m_albumV1LineEdit;
	QLineEdit* m_artistV1LineEdit;
	QCheckBox* m_albumV1CheckBox;
	QCheckBox* m_yearV1CheckBox;
	QSpinBox* m_yearV1SpinBox;
	QCheckBox* m_trackV1CheckBox;
	QSpinBox* m_trackV1SpinBox;
	QCheckBox* m_titleV1CheckBox;
	QCheckBox* m_genreV1CheckBox;
	QCheckBox* m_commentV1CheckBox;
	QCheckBox* m_artistV1CheckBox;
	QLineEdit* m_commentV1LineEdit;
	QCheckBox* m_commentV2CheckBox;
	QLineEdit* m_albumV2LineEdit;
	QLineEdit* m_artistV2LineEdit;
	QCheckBox* m_genreV2CheckBox;
	QLineEdit* m_commentV2LineEdit;
	QCheckBox* m_yearV2CheckBox;
	QPushButton* m_id3V2PushButton;
	QSpinBox* m_yearV2SpinBox;
	QCheckBox* m_trackV2CheckBox;
	QSpinBox* m_trackV2SpinBox;
	QCheckBox* m_artistV2CheckBox;
	QCheckBox* m_titleV2CheckBox;
	QCheckBox* m_albumV2CheckBox;
	Q3VBox* m_rightHalfVBox;

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
};

#endif // ID3FORM_H
