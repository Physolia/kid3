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

#include <qvariant.h>
#include <qsplitter.h>
#include "filelist.h"
#include "standardtags.h"

class QVBoxLayout;
class QGridLayout;
class QCheckBox;
class QComboBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QListBox;
class QListBoxItem;
class QPushButton;
class QSpinBox;
class QSplitter;
class QScrollView;
class QVBox;
class QHBox;
class Kid3ScrollView;
class FormatConfig;
class DirList;

/**
 * Main widget.
 */
class id3Form : public QSplitter
{
Q_OBJECT

public:
	/** 
	 * Constructs an id3Form as a child of 'parent', with the 
	 * name 'name' and widget flags set to 'f'.
	 * @param parent parent widget
	 * @param name   Qt name
	 */
	id3Form(QWidget* parent = 0, const char* name = 0);

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

	FileList *mp3ListBox;
	QComboBox* genreV1ComboBox;
	QComboBox* genreV2ComboBox;
	QComboBox* formatComboBox;
	QLineEdit* nameLineEdit;
	QLabel* detailsLabel;
	QListBox* framesListBox;
	QLineEdit* titleV1LineEdit;
	QLineEdit* titleV2LineEdit;
	QSplitter* m_vSplitter;
	DirList* m_dirListBox;

public slots:
	/**
	 * Button ID3v1 From Filename.
	 */
	virtual void fromFilenameV1();
	/**
	 * Button ID3v2 From Filename.
	 */
	virtual void fromFilenameV2();
	/**
	 * Button ID3v2 From ID3v1.
	 */
	virtual void fromID3V2();
	/**
	 * Button ID3v1 From ID3v2.
	 */
	virtual void fromID3V1();
	/**
	 * Button ID3v1 Copy.
	 */
	virtual void copyV1();
	/**
	 * Button ID3v2 Copy.
	 */
	virtual void copyV2();
	/**
	 * Button ID3v2 Remove.
	 */
	virtual void removeV2();
	/**
	 * Button ID3v1 Paste.
	 */
	virtual void pasteV1();
	/**
	 * Button ID3v2 Paste.
	 */
	virtual void pasteV2();
	/**
	 * Button ID3v1 Remove.
	 */
	virtual void removeV1();
	/**
	 * File list box file selected
	 */
	virtual void fileSelected(void);
	/**
	 * Get standard tags from the ID3v1 controls.
	 *
	 * @param st standard tags to store result
	 */
	virtual void getStandardTagsV1(StandardTags * st);
	/**
	 * Get standard tags from the ID3v2 controls.
	 *
	 * @param st standard tags to store result
	 */
	virtual void getStandardTagsV2(StandardTags * st);
	/**
	 * Set ID3v1 standard tags controls.
	 *
	 * @param st standard tags to set
	 */
	virtual void setStandardTagsV1(const StandardTags * st);
	/**
	 * Set ID3v2 standard tags controls.
	 *
	 * @param st standard tags to set
	 */
	virtual void setStandardTagsV2(const StandardTags * st);
	/**
	 * Set all ID3v1 and ID3v2 check boxes on or off.
	 *
	 * @param val TRUE to set check boxes on.
	 */
	virtual void setAllCheckBoxes(bool val);
	/**
	 * Get number of files selected in file list box.
	 *
	 * @return number of files selected.
	 */
	virtual int numFilesSelected();
	/**
	 * Frame list button Edit.
	 */
	virtual void editFrame(void);
	/**
	 * Frame list button Add.
	 */
	virtual void addFrame(void);
	/**
	 * Frame list button Delete.
	 */
	virtual void deleteFrame(void);
	/**
	 * Set filename according to ID3v1 tags.
	 */
	virtual void fnFromID3V1(void);
	/**
	 * Set filename according to ID3v1 tags.
	 */
	virtual void fnFromID3V2(void);

	/**
	 * Filename line edit is changed.
	 * @param txt contents of line edit
	 */
	virtual void nameLineEditChanged(const QString &txt);
	/**
	 * Album V1 line edit is changed.
	 * @param txt contents of line edit
	 */
	virtual void albumV1LineEditChanged(const QString &txt);
	/**
	 * Artist V1 line edit is changed.
	 * @param txt contents of line edit
	 */
	virtual void artistV1LineEditChanged(const QString &txt);
	/**
	 * Title V1 line edit is changed.
	 * @param txt contents of line edit
	 */
	virtual void titleV1LineEditChanged(const QString &txt);
	/**
	 * Album V2 line edit is changed.
	 * @param txt contents of line edit
	 */
	virtual void albumV2LineEditChanged(const QString &txt);
	/**
	 * Artist V2 line edit is changed.
	 * @param txt contents of line edit
	 */
	virtual void artistV2LineEditChanged(const QString &txt);
	/**
	 * Title V2 line edit is changed.
	 * @param txt contents of line edit
	 */
	virtual void titleV2LineEditChanged(const QString &txt);
	/**
	 * Directory list box directory selected.
	 *
	 * @param item selected item
	 */
	virtual void dirSelected(QListBoxItem* item);

private:
	/**
	 * Format string within line edit.
	 *
	 * @param le   line edit
	 * @param txt  text in line edit
	 * @param fcfg format configuration
	 */
	void formatLineEdit(QLineEdit *le, const QString &txt,
						const FormatConfig *fcfg);

	QGroupBox* filenameGroupBox;
	QLabel* nameLabel;
	QPushButton* fnV1Button;
	QPushButton* fnV2Button;
	QLabel* formatLabel;
	QLabel* infoLabel;
	QGroupBox* idV1GroupBox;
	QLineEdit* albumV1LineEdit;
	QPushButton* copyV1PushButton;
	QPushButton* filenameV1PushButton;
	QLineEdit* artistV1LineEdit;
	QPushButton* removeV1PushButton;
	QPushButton* pasteV1PushButton;
	QCheckBox* albumV1CheckBox;
	QCheckBox* yearV1CheckBox;
	QSpinBox* yearV1SpinBox;
	QCheckBox* trackV1CheckBox;
	QSpinBox* trackV1SpinBox;
	QPushButton* id3V1PushButton;
	QCheckBox* titleV1CheckBox;
	QCheckBox* genreV1CheckBox;
	QCheckBox* commentV1CheckBox;
	QCheckBox* artistV1CheckBox;
	QLineEdit* commentV1LineEdit;
	QGroupBox* idV2GroupBox;
	QPushButton* copyV2PushButton;
	QCheckBox* commentV2CheckBox;
	QLineEdit* albumV2LineEdit;
	QLineEdit* artistV2LineEdit;
	QPushButton* pasteV2PushButton;
	QCheckBox* genreV2CheckBox;
	QLineEdit* commentV2LineEdit;
	QPushButton* filenameV2PushButton;
	QPushButton* removeV2PushButton;
	QCheckBox* yearV2CheckBox;
	QPushButton* id3V2PushButton;
	QSpinBox* yearV2SpinBox;
	QCheckBox* trackV2CheckBox;
	QSpinBox* trackV2SpinBox;
	QCheckBox* artistV2CheckBox;
	QCheckBox* titleV2CheckBox;
	QCheckBox* albumV2CheckBox;
	QPushButton* editFramesPushButton;
	QPushButton* framesAddPushButton;
	QPushButton* deleteFramesPushButton;
	QLabel* framesTextLabel;
	Kid3ScrollView *scrollView;
	QVBox* rightHalfVBox;
	QHBox* trackV1HBox;
	QHBox* trackV2HBox;
	QVBox* frameButtonVBox;
	QGridLayout* filenameGroupBoxLayout;
	QGridLayout* idV1GroupBoxLayout;
	QGridLayout* idV2GroupBoxLayout;

private slots:
	/**
	 * Accept drag.
	 *
	 * @param ev drag event.
	 */
	void dragEnterEvent(QDragEnterEvent * ev);
	/**
	 * Handle drop event.
	 *
	 * @param ev drop event.
	 */
	void dropEvent(QDropEvent * ev);
};

#endif // ID3FORM_H
