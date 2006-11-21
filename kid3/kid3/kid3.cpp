/**
 * \file kid3.cpp
 * Kid3 application.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 9 Jan 2003
 */

#include "config.h"
#include <qdir.h>
#include <qprinter.h>
#include <qpainter.h>
#include <qurl.h>
#include <qtextstream.h>
#include <qcursor.h>
#include <qmessagebox.h>
#include <qpushbutton.h>
#include "qtcompatmac.h"
#if QT_VERSION >= 0x040000
#include <Q3ProgressBar>
#include <Q3GroupBox>
#include <QCloseEvent>
#include <Q3ValueList>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <Q3PopupMenu>
#else
#include <qprogressbar.h>
#include <qgroupbox.h>
#endif

#ifdef CONFIG_USE_KDE
#include <kapp.h>
#include <kurl.h>
#include <kmenubar.h>
#include <kstatusbar.h>
#include <kconfig.h>
#include <kstdaction.h>
#include <kaction.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <kkeydialog.h>
#else
#include <qapplication.h>
#include <qmenubar.h>
#include <qaction.h>
#include <qstatusbar.h>
#include <qmessagebox.h>
#include <qfiledialog.h>
#endif

#include "kid3.h"
#include "id3form.h"
#include "genres.h"
#include "framelist.h"
#include "configdialog.h"
#include "importdialog.h"
#include "exportdialog.h"
#include "numbertracksdialog.h"
#include "standardtags.h"
#include "rendirdialog.h"
#include "dirlist.h"
#ifdef HAVE_ID3LIB
#include "mp3file.h"
#endif
#ifdef HAVE_VORBIS
#include "oggfile.h"
#endif
#ifdef HAVE_FLAC
#include "flacfile.h"
#endif
#ifdef HAVE_TAGLIB
#include "taglibfile.h"
#endif

#ifdef KID3_USE_KCONFIGDIALOG
#include <kconfigskeleton.h>
#endif

#ifndef CONFIG_USE_KDE
#include <qdialog.h>
#include <qtextbrowser.h>
#include <qtextcodec.h>

class BrowserDialog : public QDialog {
public:
	BrowserDialog(QWidget *parent, QString &caption);
	~BrowserDialog();
	void goToAnchor(const QString& anchor);

private:
	QTextBrowser *m_textBrowser;
	QString m_filename;
};

BrowserDialog::BrowserDialog(QWidget *parent, QString &caption)
	: QDialog(parent, "browser")
{
	setCaption(caption);
	QVBoxLayout *vlayout = new QVBoxLayout(this);
	if (!vlayout) {
		return ;
	}
	vlayout->setSpacing(6);
	vlayout->setMargin(6);

	m_textBrowser = new QTextBrowser(this, "textBrowser");
	m_filename = QDir::currentDirPath() + QDir::separator() + "kid3_";
	QString lang((QString(QTextCodec::locale())).left(2));
	if (!QFile::exists(m_filename + lang + ".html")) {
		lang = "en";
	}
	m_filename += lang + ".html";
#if QT_VERSION >= 0x040000
	m_textBrowser->setSource(QUrl::fromLocalFile(m_filename));
#else
	m_textBrowser->setSource(m_filename);
#endif
	vlayout->addWidget(m_textBrowser);

	QHBoxLayout *hlayout = new QHBoxLayout(vlayout);
	QSpacerItem *hspacer = new QSpacerItem(16, 0, QSizePolicy::Expanding,
	                                       QSizePolicy::Minimum);
	QPushButton *backButton = new QPushButton(i18n("&Back"), this);
	QPushButton *forwardButton = new QPushButton(i18n("&Forward"), this);
	QPushButton *closeButton = new QPushButton(i18n("&Close"), this);
	if (hlayout && backButton && forwardButton && closeButton) {
		hlayout->addWidget(backButton);
		hlayout->addWidget(forwardButton);
		hlayout->addItem(hspacer);
		hlayout->addWidget(closeButton);
		closeButton->setDefault(true);
		backButton->setEnabled(false);
		forwardButton->setEnabled(false);
		connect(backButton, SIGNAL(clicked()), m_textBrowser, SLOT(backward()));
		connect(forwardButton, SIGNAL(clicked()), m_textBrowser, SLOT(forward()));
		connect(m_textBrowser, SIGNAL(backwardAvailable(bool)), backButton, SLOT(setEnabled(bool)));
		connect(m_textBrowser, SIGNAL(forwardAvailable(bool)), forwardButton, SLOT(setEnabled(bool)));
		connect(closeButton, SIGNAL(clicked()), this, SLOT(accept()));
	}
	resize(500, 500);
}

BrowserDialog::~BrowserDialog()
{}

void BrowserDialog::goToAnchor(const QString& anchor)
{
#if QT_VERSION >= 0x040000
	QUrl url = QUrl::fromLocalFile(m_filename);
	url.setFragment(anchor);
	m_textBrowser->setSource(url);
#else
	if (!anchor.isEmpty()) {
		m_textBrowser->setSource(m_filename + '#' + anchor);
	} else {
		m_textBrowser->setSource(m_filename);
	}
#endif
}

BrowserDialog* Kid3App::s_helpBrowser = 0;
#endif


MiscConfig Kid3App::s_miscCfg("General Options");
ImportConfig Kid3App::s_genCfg("General Options");
FormatConfig Kid3App::s_fnFormatCfg("FilenameFormat");
FormatConfig Kid3App::s_id3FormatCfg("Id3Format");
FreedbConfig Kid3App::s_freedbCfg("Freedb");
DiscogsConfig Kid3App::s_discogsCfg("Discogs");
MusicBrainzConfig Kid3App::s_musicBrainzCfg("MusicBrainz");

/**
 * Constructor.
 *
 * @param name name
 */

Kid3App::Kid3App() :
	m_importDialog(0), m_exportDialog(0), m_numberTracksDialog(0)
{
	copytags = new StandardTags();
	initStatusBar();
	setModified(false);
	doc_dir = QString::null;
	initView();
	initActions();
	FrameList::setListBox(view->framesListBox);
	s_fnFormatCfg.setAsFilenameFormatter();

	resize(sizeHint());
#ifdef CONFIG_USE_KDE
	config=kapp->config();
#else
	config = new Kid3Settings();
	config->setPath("kid3.sourceforge.net", "Kid3", Kid3Settings::User);
	config->beginGroup("/kid3");
#endif
	readOptions();
}

/**
 * Destructor.
 */

Kid3App::~Kid3App()
{
	delete m_importDialog;
	delete m_numberTracksDialog;
#ifndef CONFIG_USE_KDE
	delete s_helpBrowser;
	s_helpBrowser = 0;
#endif
}

/**
 * Init menu and toolbar actions.
 */

void Kid3App::initActions()
{
#ifdef CONFIG_USE_KDE
	fileOpen = KStdAction::open(
	    this, SLOT(slotFileOpen()), actionCollection());
	fileOpenRecent = KStdAction::openRecent(
	    this, SLOT(slotFileOpenRecent(const KURL&)), actionCollection());
	fileRevert = KStdAction::revert(
	    this, SLOT(slotFileRevert()), actionCollection());
	fileSave = KStdAction::save(
	    this, SLOT(slotFileSave()), actionCollection());
	fileQuit = KStdAction::quit(
	    this, SLOT(slotFileQuit()), actionCollection());
#if KDE_VERSION < 0x30200
	viewToolBar = KStdAction::showToolbar(
	    this, SLOT(slotViewToolBar()), actionCollection());
	viewStatusBar = KStdAction::showStatusbar(
	    this, SLOT(slotViewStatusBar()), actionCollection());
	viewToolBar->setStatusText(i18n("Enables/disables the toolbar"));
	viewStatusBar->setStatusText(i18n("Enables/disables the statusbar"));
#else
	setStandardToolBarMenuEnabled(true);
	createStandardStatusBarAction();
#endif
	settingsShortcuts = KStdAction::keyBindings(
		this, SLOT(slotSettingsShortcuts()), actionCollection());
	settingsConfigure = KStdAction::preferences(
	    this, SLOT(slotSettingsConfigure()), actionCollection());

	fileOpen->setStatusText(i18n("Opens a directory"));
	fileOpenRecent->setStatusText(i18n("Opens a recently used directory"));
	fileRevert->setStatusText(
	    i18n("Reverts the changes of all or the selected files"));
	fileSave->setStatusText(i18n("Saves the changed files"));
	fileQuit->setStatusText(i18n("Quits the application"));
	settingsShortcuts->setStatusText(i18n("Configure Shortcuts"));
	settingsConfigure->setStatusText(i18n("Preferences dialog"));

	new KAction(i18n("&Import..."), 0, this,
		    SLOT(slotImport()), actionCollection(),
		    "import");
	new KAction(i18n("Import from &freedb.org..."), 0, this,
		    SLOT(slotImportFreedb()), actionCollection(),
		    "import_freedb");
	new KAction(i18n("Import from &Discogs..."), 0, this,
		    SLOT(slotImportDiscogs()), actionCollection(),
		    "import_discogs");
	new KAction(i18n("Import from MusicBrainz &release..."), 0, this,
		    SLOT(slotImportMusicBrainzRelease()), actionCollection(),
		    "import_musicbrainzrelease");
#ifdef HAVE_TUNEPIMP
	new KAction(i18n("Import from &MusicBrainz fingerprint..."), 0, this,
		    SLOT(slotImportMusicBrainz()), actionCollection(),
		    "import_musicbrainz");
#endif
	new KAction(i18n("&Export..."), 0, this,
		    SLOT(slotExport()), actionCollection(),
		    "export");
	new KAction(i18n("&Create Playlist"), 0, this,
		    SLOT(slotCreatePlaylist()), actionCollection(),
		    "create_playlist");
	new KAction(i18n("Apply &Filename Format"), 0, this,
		    SLOT(slotApplyFilenameFormat()), actionCollection(),
		    "apply_filename_format");
	new KAction(i18n("Apply &Tag Format"), 0, this,
		    SLOT(slotApplyId3Format()), actionCollection(),
		    "apply_id3_format");
	new KAction(i18n("&Rename Directory..."), 0, this,
		    SLOT(slotRenameDirectory()), actionCollection(),
		    "rename_directory");
	new KAction(i18n("&Number Tracks..."), 0, this,
		    SLOT(slotNumberTracks()), actionCollection(),
		    "number_tracks");
	settingsShowHideV1 =
		new KAction(i18n("Hide Tag &1"), 0, this,
								SLOT(slotSettingsShowHideV1()), actionCollection(),
								"hide_v1");
	settingsShowHideV2 =
		new KAction(i18n("Hide Tag &2"), 0, this,
								SLOT(slotSettingsShowHideV2()), actionCollection(),
								"hide_v2");

	new KAction(i18n("Select &All"), KShortcut("Alt+A"), this,
		    SLOT(slotSelectAll()), actionCollection(),
		    "select_all");
	new KAction(i18n("&Next File"), KShortcut("Alt+Down"), this,
		    SLOT(slotNextFile()), actionCollection(),
		    "next_file");
	new KAction(i18n("&Previous File"), KShortcut("Alt+Up"), this,
		    SLOT(slotPreviousFile()), actionCollection(),
		    "previous_file");
	new KAction(i18n("Tag 1") + ": " + i18n("From Filename"), 0, view, SLOT(fromFilenameV1()),
				actionCollection(), "v1_from_filename");
	new KAction(i18n("Tag 1") + ": " + i18n("From Tag 2"), 0, view, SLOT(fromID3V1()),
				actionCollection(), "v1_from_v2");
	new KAction(i18n("Tag 1") + ": " + i18n("Copy"), 0, view, SLOT(copyV1()),
				actionCollection(), "v1_copy");
	new KAction(i18n("Tag 1") + ": " + i18n("Paste"), 0, view, SLOT(pasteV1()),
				actionCollection(), "v1_paste");
	new KAction(i18n("Tag 1") + ": " + i18n("Remove"), 0, view, SLOT(removeV1()),
				actionCollection(), "v1_remove");
	new KAction(i18n("Tag 2") + ": " + i18n("From Filename"), 0, view, SLOT(fromFilenameV2()),
				actionCollection(), "v2_from_filename");
	new KAction(i18n("Tag 2") + ": " + i18n("From Tag 1"), 0, view, SLOT(fromID3V2()),
				actionCollection(), "v2_from_v1");
	new KAction(i18n("Tag 2") + ": " + i18n("Copy"), 0, view, SLOT(copyV2()),
				actionCollection(), "v2_copy");
	new KAction(i18n("Tag 2") + ": " + i18n("Paste"), 0, view, SLOT(pasteV2()),
				actionCollection(), "v2_paste");
	new KAction(i18n("Tag 2") + ": " + i18n("Remove"), 0, view, SLOT(removeV2()),
				actionCollection(), "v2_remove");
	new KAction(i18n("Frames:") + " " + i18n("Edit"), 0, view, SLOT(editFrame()),
				actionCollection(), "frames_edit");
	new KAction(i18n("Frames:") + " " + i18n("Add"), 0, view, SLOT(addFrame()),
				actionCollection(), "frames_add");
	new KAction(i18n("Frames:") + " " + i18n("Delete"), 0, view, SLOT(deleteFrame()),
				actionCollection(), "frames_delete");
	new KAction(i18n("Filename") + ": " + i18n("From Tag 1"), 0, view, SLOT(fnFromID3V1()),
				actionCollection(), "filename_from_v1");
	new KAction(i18n("Filename") + ": " + i18n("From Tag 2"), 0, view, SLOT(fnFromID3V2()),
				actionCollection(), "filename_from_v2");
	new KAction(i18n("Filename") + ": " + i18n("Focus"), 0, view->nameLineEdit, SLOT(setFocus()),
				actionCollection(), "filename_focus");
	new KAction(i18n("Tag 1") + ": " + i18n("Focus"), 0, view->titleV1LineEdit, SLOT(setFocus()),
				actionCollection(), "v1_focus");
	new KAction(i18n("Tag 2") + ": " + i18n("Focus"), 0, view->titleV2LineEdit, SLOT(setFocus()),
				actionCollection(), "v2_focus");

	createGUI();

#else
	fileOpen = new QAction(this);
	if (fileOpen) {
		fileOpen->setText(i18n("Opens a directory"));
		fileOpen->setMenuText(i18n("&Open..."));
		fileOpen->setAccel(Qt::CTRL + Qt::Key_O);
		connect(fileOpen, SIGNAL(activated()),
			this, SLOT(slotFileOpen()));
	}
	fileSave = new QAction(this);
	if (fileSave) {
		fileSave->setText(i18n("Saves the changed files"));
		fileSave->setMenuText(i18n("&Save"));
		fileSave->setAccel(Qt::CTRL + Qt::Key_S);
		connect(fileSave, SIGNAL(activated()),
			this, SLOT(slotFileSave()));
	}
	fileRevert = new QAction(this);
	if (fileRevert) {
		fileRevert->setText(
		    i18n("Reverts the changes of all or the selected files"));
		fileRevert->setMenuText(i18n("Re&vert"));
		connect(fileRevert, SIGNAL(activated()),
			this, SLOT(slotFileRevert()));
	}
	fileImport = new QAction(this);
	if (fileImport) {
		fileImport->setText(i18n("Import from file or clipboard"));
		fileImport->setMenuText(i18n("&Import..."));
		connect(fileImport, SIGNAL(activated()),
			this, SLOT(slotImport()));
	}
	fileImportFreedb = new QAction(this);
	if (fileImportFreedb) {
		fileImportFreedb->setText(i18n("Import from freedb.org"));
		fileImportFreedb->setMenuText(i18n("Import from &freedb.org..."));
		connect(fileImportFreedb, SIGNAL(activated()),
			this, SLOT(slotImportFreedb()));
	}
	fileImportDiscogs = new QAction(this);
	if (fileImportDiscogs) {
		fileImportDiscogs->setText(i18n("Import from Discogs"));
		fileImportDiscogs->setMenuText(i18n("Import from &Discogs..."));
		connect(fileImportDiscogs, SIGNAL(activated()),
			this, SLOT(slotImportDiscogs()));
	}
	fileImportMusicBrainzRelease = new QAction(this);
	if (fileImportMusicBrainzRelease) {
		fileImportMusicBrainzRelease->setText(i18n("Import from MusicBrainz release"));
		fileImportMusicBrainzRelease->setMenuText(i18n("Import from MusicBrainz &release..."));
		connect(fileImportMusicBrainzRelease, SIGNAL(activated()),
			this, SLOT(slotImportMusicBrainzRelease()));
	}
#ifdef HAVE_TUNEPIMP
	fileImportMusicBrainz = new QAction(this);
	if (fileImportMusicBrainz) {
		fileImportMusicBrainz->setText(i18n("Import from MusicBrainz fingerprint"));
		fileImportMusicBrainz->setMenuText(i18n("Import from &MusicBrainz fingerprint..."));
		connect(fileImportMusicBrainz, SIGNAL(activated()),
			this, SLOT(slotImportMusicBrainz()));
	}
#endif
	fileExport = new QAction(this);
	if (fileExport) {
		fileExport->setText(i18n("Export to file or clipboard"));
		fileExport->setMenuText(i18n("&Export..."));
		connect(fileExport, SIGNAL(activated()),
			this, SLOT(slotExport()));
	}
	fileCreatePlaylist = new QAction(this);
	if (fileCreatePlaylist) {
		fileCreatePlaylist->setText(i18n("Create M3U Playlist"));
		fileCreatePlaylist->setMenuText(i18n("&Create Playlist"));
		connect(fileCreatePlaylist, SIGNAL(activated()),
			this, SLOT(slotCreatePlaylist()));
	}
	fileQuit = new QAction(this);
	if (fileQuit) {
		fileQuit->setText(i18n("Quits the application"));
		fileQuit->setMenuText(i18n("&Quit"));
		fileQuit->setAccel(Qt::CTRL + Qt::Key_Q);
		connect(fileQuit, SIGNAL(activated()),
			this, SLOT(slotFileQuit()));
	}
	helpHandbook = new QAction(this);
	if (helpHandbook) {
		helpHandbook->setText(i18n("Kid3 Handbook"));
		helpHandbook->setMenuText(i18n("Kid3 &Handbook"));
		connect(helpHandbook, SIGNAL(activated()),
			this, SLOT(slotHelpHandbook()));
	}
	helpAbout = new QAction(this);
	if (helpAbout) {
		helpAbout->setText(i18n("About Kid3"));
		helpAbout->setMenuText(i18n("&About Kid3"));
		connect(helpAbout, SIGNAL(activated()),
			this, SLOT(slotHelpAbout()));
	}
	helpAboutQt = new QAction(this);
	if (helpAboutQt) {
		helpAboutQt->setText(i18n("About Qt"));
		helpAboutQt->setMenuText(i18n("About &Qt"));
		connect(helpAboutQt, SIGNAL(activated()),
			this, SLOT(slotHelpAboutQt()));
	}
	toolsApplyFilenameFormat = new QAction(this);
	if (toolsApplyFilenameFormat) {
		toolsApplyFilenameFormat->setText(i18n("Apply Filename Format"));
		toolsApplyFilenameFormat->setMenuText(i18n("Apply &Filename Format"));
		connect(toolsApplyFilenameFormat, SIGNAL(activated()),
			this, SLOT(slotApplyFilenameFormat()));
	}
	toolsApplyId3Format = new QAction(this);
	if (toolsApplyId3Format) {
		toolsApplyId3Format->setText(i18n("Apply Tag Format"));
		toolsApplyId3Format->setMenuText(i18n("Apply &Tag Format"));
		connect(toolsApplyId3Format, SIGNAL(activated()),
			this, SLOT(slotApplyId3Format()));
	}
	toolsRenameDirectory = new QAction(this);
	if (toolsRenameDirectory) {
		toolsRenameDirectory->setText(i18n("Rename Directory"));
		toolsRenameDirectory->setMenuText(i18n("&Rename Directory..."));
		connect(toolsRenameDirectory, SIGNAL(activated()),
			this, SLOT(slotRenameDirectory()));
	}
	toolsNumberTracks = new QAction(this);
	if (toolsNumberTracks) {
		toolsNumberTracks->setText(i18n("Number Tracks"));
		toolsNumberTracks->setMenuText(i18n("&Number Tracks..."));
		connect(toolsNumberTracks, SIGNAL(activated()),
			this, SLOT(slotNumberTracks()));
	}
	settingsShowHideV1 = new QAction(this);
	if (settingsShowHideV1) {
		settingsShowHideV1->setText(i18n("Hide Tag 1"));
		settingsShowHideV1->setMenuText(i18n("Hide Tag &1"));
		connect(settingsShowHideV1, SIGNAL(activated()),
			this, SLOT(slotSettingsShowHideV1()));
	}
	settingsShowHideV2 = new QAction(this);
	if (settingsShowHideV2) {
		settingsShowHideV2->setText(i18n("Hide Tag 2"));
		settingsShowHideV2->setMenuText(i18n("Hide Tag &2"));
		connect(settingsShowHideV2, SIGNAL(activated()),
			this, SLOT(slotSettingsShowHideV2()));
	}
	settingsConfigure = new QAction(this);
	if (settingsConfigure) {
		settingsConfigure->setText(i18n("Configure Kid3"));
		settingsConfigure->setMenuText(i18n("&Configure Kid3..."));
		connect(settingsConfigure, SIGNAL(activated()),
			this, SLOT(slotSettingsConfigure()));
	}
	menubar = new QMenuBar(this);
	fileMenu = new Q3PopupMenu(this);
	toolsMenu = new Q3PopupMenu(this);
	settingsMenu = new Q3PopupMenu(this);
	helpMenu = new Q3PopupMenu(this);
	if (menubar && fileMenu && toolsMenu && settingsMenu && helpMenu) {
		fileOpen->addTo(fileMenu);
		fileMenu->insertSeparator();
		fileSave->addTo(fileMenu);
		fileRevert->addTo(fileMenu);
		fileMenu->insertSeparator();
		fileImport->addTo(fileMenu);
		fileImportFreedb->addTo(fileMenu);
		fileImportDiscogs->addTo(fileMenu);
		fileImportMusicBrainzRelease->addTo(fileMenu);
#ifdef HAVE_TUNEPIMP
		fileImportMusicBrainz->addTo(fileMenu);
#endif
		fileExport->addTo(fileMenu);
		fileCreatePlaylist->addTo(fileMenu);
		fileMenu->insertSeparator();
		fileQuit->addTo(fileMenu);
		menubar->insertItem((i18n("&File")), fileMenu);

		toolsApplyFilenameFormat->addTo(toolsMenu);
		toolsApplyId3Format->addTo(toolsMenu);
		toolsRenameDirectory->addTo(toolsMenu);
		toolsNumberTracks->addTo(toolsMenu);
		menubar->insertItem((i18n("&Tools")), toolsMenu);

		settingsShowHideV1->addTo(settingsMenu);
		settingsShowHideV2->addTo(settingsMenu);
		settingsMenu->insertSeparator();
		settingsConfigure->addTo(settingsMenu);
		menubar->insertItem(i18n("&Settings"), settingsMenu);

		helpHandbook->addTo(helpMenu);
		helpAbout->addTo(helpMenu);
		helpAboutQt->addTo(helpMenu);
		menubar->insertItem(i18n("&Help"), helpMenu);
	}
	setCaption("Kid3");
#endif
}

/**
 * Init status bar.
 */

void Kid3App::initStatusBar()
{
	statusBar()->message(i18n("Ready."));
}

/**
 * Init GUI.
 */

void Kid3App::initView()
{ 
	view = new id3Form(this, "id3Form");
	if (view) {
		setCentralWidget(view);	
		QStringList strList;
		for (const char** sl = Genres::strList; *sl != 0; ++sl) {
			strList += *sl;
		}
		view->genreV1ComboBox->QCM_addItems(strList);
		view->genreV2ComboBox->QCM_addItems(strList);
		view->formatComboBox->setEditable(TRUE);
		strList.clear();
		for (const char** sl = MiscConfig::defaultFnFmtList; *sl != 0; ++sl) {
			strList += *sl;
		}
		view->formatComboBox->QCM_addItems(strList);
	}
}

/**
 * Open directory.
 *
 * @param dir     directory or file path
 * @param confirm if true ask if there are unsaved changes
 */

void Kid3App::openDirectory(QString dir, bool confirm)
{
	if (confirm && !saveModified()) {
		return;
	}
	if (dir.isNull() || dir.isEmpty()) {
		return;
	}
	QFileInfo file(dir);
	if (!file.isDir()) {
		dir = file.dirPath(TRUE);
	}

	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	slotStatusMsg(i18n("Opening directory..."));
	if (view->mp3ListBox->readDir(dir)) {
		view->m_dirListBox->readDir(dir);
		setModified(false);
#ifdef CONFIG_USE_KDE
		KURL url;
		url.setPath(dir);
		fileOpenRecent->addURL(url);
		setCaption(dir, false);
#else
		setCaption(dir + " - Kid3");
#endif
		doc_dir = dir;
	}
	slotStatusMsg(i18n("Ready."));
	QApplication::restoreOverrideCursor();
}

/**
 * Save application options.
 */

void Kid3App::saveOptions()
{
#ifdef CONFIG_USE_KDE
	fileOpenRecent->saveEntries(config, "Recent Files");
#else
	s_miscCfg.windowWidth = size().width();
	s_miscCfg.windowHeight = size().height();
#endif
	s_miscCfg.splitterSizes = view->sizes();
	s_miscCfg.m_vSplitterSizes = view->m_vSplitter->sizes();
	s_miscCfg.formatItem = view->formatComboBox->currentItem();
	s_miscCfg.formatText = view->formatComboBox->currentText();

	s_miscCfg.m_customGenres.clear();
	int idx, numGenres = view->genreV2ComboBox->count();
	for (idx = Genres::count + 1; idx < numGenres; ++idx) {
		s_miscCfg.m_customGenres.append(view->genreV2ComboBox->text(idx));
	}

	s_miscCfg.writeToConfig(config);
	s_fnFormatCfg.writeToConfig(config);
	s_id3FormatCfg.writeToConfig(config);
	s_genCfg.writeToConfig(config);
	s_freedbCfg.writeToConfig(config);
	s_discogsCfg.writeToConfig(config);
#ifdef HAVE_TUNEPIMP
	s_musicBrainzCfg.writeToConfig(config);
#endif
}

/**
 * Load application options.
 */

void Kid3App::readOptions()
{
	s_miscCfg.readFromConfig(config);
	s_fnFormatCfg.readFromConfig(config);
	s_id3FormatCfg.readFromConfig(config);
	s_genCfg.readFromConfig(config);
	s_freedbCfg.readFromConfig(config);
	s_discogsCfg.readFromConfig(config);
#ifdef HAVE_TUNEPIMP
	s_musicBrainzCfg.readFromConfig(config);
#endif
	updateHideV1();
	updateHideV2();
#ifdef CONFIG_USE_KDE
	setAutoSaveSettings();
	fileOpenRecent->loadEntries(config,"Recent Files");
#if KDE_VERSION < 0x30200
	viewToolBar->setChecked(!toolBar("mainToolBar")->isHidden());
	viewStatusBar->setChecked(!statusBar()->isHidden());
#endif
#else
	if (s_miscCfg.windowWidth != -1 && s_miscCfg.windowHeight != -1) {
		resize(s_miscCfg.windowWidth, s_miscCfg.windowHeight);
	}
#endif
	if (!s_miscCfg.splitterSizes.empty()) {
		view->setSizes(s_miscCfg.splitterSizes);
	}
	if (!s_miscCfg.m_vSplitterSizes.empty()) {
		view->m_vSplitter->setSizes(s_miscCfg.m_vSplitterSizes);
	} else {
		// no values in configuration => set defaults
		// the window height is a bit too large, but works
		int sumSizes = size().height();
		if (sumSizes > 70) {
			Q3ValueList<int> sizes;
			sizes.append(sumSizes - 70);
			sizes.append(70);
			view->m_vSplitter->setSizes(sizes);
		}
	}
	view->formatComboBox->setCurrentItem(s_miscCfg.formatItem);
	view->formatComboBox->setCurrentText(s_miscCfg.formatText);
	view->genreV2ComboBox->insertStringList(s_miscCfg.m_customGenres);
}

#ifdef CONFIG_USE_KDE
/**
 * Saves the window properties to the session config file.
 *
 * @param _cfg application configuration
 */

void Kid3App::saveProperties(KConfig *_cfg)
{
	if (_cfg) { // otherwise KDE 3.0 compiled program crashes with KDE 3.1
		_cfg->writeEntry("dirname", doc_dir);
	}
}

/**
 * Reads the session config file and restores the application's state.
 *
 * @param _cfg application configuration
 */

void Kid3App::readProperties(KConfig* _cfg)
{
	openDirectory(_cfg->readEntry("dirname", ""));
}

#else /* CONFIG_USE_KDE */

/**
 * Window is closed.
 *
 * @param ce close event
 */

void Kid3App::closeEvent(QCloseEvent *ce)
{
	if (queryClose()) {
		ce->accept();
	}
	else {
		ce->ignore();
	}
}
#endif /* CONFIG_USE_KDE */

/**
 * Save all changed files.
 *
 * @return true
 */

bool Kid3App::saveDirectory(void)
{
	QString errorFiles;
	int numFiles = 0, totalFiles = 0;
	bool renamed = FALSE;
	FileListItem* mp3file = view->mp3ListBox->first();
	// Get number of files to be saved to display correct progressbar
	while (mp3file != 0) {
		if (mp3file->getFile()->isChanged()) {
			++totalFiles;
		}
		mp3file = view->mp3ListBox->next();
	}
	Q3ProgressBar *progress = new Q3ProgressBar();
	statusBar()->addWidget(progress, 0, true);
	progress->setTotalSteps(totalFiles);
	progress->setProgress(numFiles);
#ifdef CONFIG_USE_KDE
	kapp->processEvents();
#else
	qApp->processEvents();
#endif
	mp3file = view->mp3ListBox->first();
	while (mp3file != 0) {
		if (!mp3file->getFile()->writeTags(FALSE, &renamed, s_miscCfg.m_preserveTime)) {
			errorFiles.append(mp3file->getFile()->getFilename());
			errorFiles.append('\n');
		}
		mp3file = view->mp3ListBox->next();
		++numFiles;
		progress->setProgress(numFiles);
	}
	statusBar()->removeWidget(progress);
	delete progress;
	if (renamed) {
		view->mp3ListBox->readDir(doc_dir);
		setModified(false);
	}
	else {
		updateModificationState();
	}
	if (!errorFiles.isEmpty()) {
		QMessageBox::warning(0, i18n("File Error"),
							 i18n("Error while writing file:\n") +
							 errorFiles,
							 QMessageBox::Ok, QCM_NoButton);
	}
	return true;
}

/**
 * If anything was modified, save after asking user.
 *
 * @return FALSE if user canceled.
 */

bool Kid3App::saveModified()
{
	bool completed=true;

	if(isModified() && !doc_dir.isEmpty())
	{
		Kid3App *win=(Kid3App *) parent();
#ifdef CONFIG_USE_KDE
		const int Yes = KMessageBox::Yes;
		const int No = KMessageBox::No;
		const int Cancel = KMessageBox::Cancel;
		int want_save = KMessageBox::warningYesNoCancel(
		    win,
		    i18n("The current directory has been modified.\n"
			 "Do you want to save it?"),
		    i18n("Warning"));
#else
		const int Yes = QMessageBox::Yes;
		const int No = QMessageBox::No;
		const int Cancel = QMessageBox::Cancel;
		int want_save = QMessageBox::warning(
			win,
			i18n("Warning - Kid3"),
			i18n("The current directory has been modified.\n"
			     "Do you want to save it?"),
			QMessageBox::Yes | QMessageBox::Default,
			QMessageBox::No,
			QMessageBox::Cancel | QMessageBox::Escape);
#endif
		switch(want_save)
		{
		case Yes:
			saveDirectory();
			completed=true;
			break;

		case No:
			setModified(false);
			completed=true;
			break;

		case Cancel:
			completed=false;
			break;

		default:
			completed=false;
			break;
		}
	}

	return completed;
}

/**
 * Free allocated resources.
 * Our destructor may not be called, so cleanup is done here.
 */
void Kid3App::cleanup()
{
#ifndef CONFIG_USE_KDE
#ifdef _MSC_VER
	// A _BLOCK_TYPE_IS_VALID assertion pops up if config is deleted
	// on Windows, MSVC 2005, Qt 4.1.2
	config->sync();
#else
	delete config;
#endif
#endif
		delete copytags;
#ifdef HAVE_ID3LIB
		Mp3File::staticCleanup();
#endif
#ifdef HAVE_VORBIS
		OggFile::staticCleanup();
#endif
#ifdef HAVE_FLAC
		FlacFile::staticCleanup();
#endif
#ifdef HAVE_TAGLIB
		TagLibFile::staticCleanup();
#endif
}

/**
 * Update modification state before closing.
 * Called on closeEvent() of window.
 * If anything was modified, save after asking user.
 * Save options before closing.
 * This method is called by closeEvent(), which occurs when the
 * window is closed or slotFileQuit() (Quit menu) is selected.
 *
 * @return false if user canceled,
 *         true will quit the application.
 */

bool Kid3App::queryClose()
{
	updateCurrentSelection();
	if (saveModified()) {
		saveOptions();
		cleanup();
		return true;
	}
	return false;
}

/**
 * Request new directory and open it.
 */

void Kid3App::slotFileOpen()
{
	updateCurrentSelection();
	if(saveModified()) {
		QString dir, filter, flt;
#ifdef CONFIG_USE_KDE
		flt = "*.mp3 *.ogg *.flac *.mpc *.MP3 *.OGG *.FLAC *.MPC *.Mp3 *.Ogg *.Flac *.Mpc *.mP3 *.ogG *.oGg *.oGG *.OgG *.OGg *.flaC *.flAc *.flAC *.FlaC *.FlAc *.mpC *.mPc *.mPC *.MpC *.MPc|MP3, OGG, FLAC, MPC (*.mp3, *.ogg, *.flac *.mpc)\n";
#if defined HAVE_ID3LIB || defined HAVE_TAGLIB
		flt += "*.mp3 *.MP3 *.Mp3 *.mP3|MP3 (*.mp3)\n";
#endif
#if defined HAVE_VORBIS || defined HAVE_TAGLIB
		flt += "*.ogg *.OGG *.Ogg *.ogG *.oGg *.oGG *.OgG *.OGg|OGG (*.ogg)\n";
#endif
#if defined HAVE_FLAC || defined HAVE_TAGLIB
		flt += "*.flac *.FLAC *.Flac *.flaC *.flAc *.flAC *.FlaC *.FlAc|FLAC (*.flac)\n";
#endif
#ifdef HAVE_TAGLIB
		flt += "*.mpc *.MPC *.Mpc *.mpC *.mPc *.mPC *.MpC *.MPc|MPC (*.mpc)\n";
#endif
		flt += ("*|All Files (*)");
		KFileDialog diag(
		    QString::null,
		    flt,
		    this, "filedialog", TRUE);
		diag.setCaption(i18n("Open"));
		if (diag.exec() == QDialog::Accepted) {
			dir = diag.selectedFile();
			filter = diag.currentFilter();
		}
#else
		flt = "MP3, OGG, FLAC, MPC (*.mp3 *.ogg *.flac *.mpc *.MP3 *.OGG *.FLAC *.MPC *.Mp3 *.Ogg *.Flac *.Mpc *.mP3 *.ogG *.oGg *.oGG *.OgG *.OGg *.flaC *.flAc *.flAC *.FlaC *.FlAc *.mpC *.mPc *.mPC *.MpC *.MPc);;";
#if defined HAVE_ID3LIB || defined HAVE_TAGLIB
		flt += "MP3 (*.mp3 *.MP3 *.Mp3 *.mP3);;";
#endif
#if defined HAVE_VORBIS || defined HAVE_TAGLIB
		flt += "OGG (*.ogg *.OGG *.Ogg *.ogG *.oGg *.oGG *.OgG *.OGg);;";
#endif
#if defined HAVE_FLAC || defined HAVE_TAGLIB
		flt += "FLAC (*.flac *.FLAC *.Flac *.flaC *.flAc *.flAC *.FlaC *.FlAc);;";
#endif
#ifdef HAVE_TAGLIB
		flt += "MPC (*.mpc *.MPC *.Mpc *.mpC *.mPc *.mPC *.MpC *.MPc);;";
#endif
		flt += i18n("All Files (*)");
		dir = QFileDialog::getOpenFileName(
		    QString::null, flt,
		    this, 0, QString::null, &filter);
#endif
		if (!dir.isEmpty()) {
			int start = filter.find('('), end = filter.find(')');
			if (start != -1 && end != -1 && end > start) {
				filter = filter.mid(start + 1, end - start - 1);
			}
			s_miscCfg.nameFilter = filter;
			openDirectory(dir);
		}
	}
}

#ifdef CONFIG_USE_KDE
/**
 * Open recent directory.
 *
 * @param url URL of directory to open
 */

void Kid3App::slotFileOpenRecent(const KURL& url)
{
	updateCurrentSelection();
	QString dir = url.path();
	openDirectory(dir, true);
}
#else /* CONFIG_USE_KDE */
void Kid3App::slotFileOpenRecent(const KURL&) {}
#endif /* CONFIG_USE_KDE */

/**
 * Revert file modifications.
 * Acts on selected files or all files if no file is selected.
 */

void Kid3App::slotFileRevert()
{
	FileListItem* mp3file = view->mp3ListBox->first();
	bool no_selection = view->numFilesSelected() == 0;
	while (mp3file != 0) {
		if (no_selection || mp3file->isInSelection()) {
			mp3file->getFile()->readTags(TRUE);
		}
		mp3file = view->mp3ListBox->next();
	}
	if (!no_selection) {
		StandardTags st; // empty
		view->setStandardTagsV1(&st);
		view->setStandardTagsV2(&st);
		view->nameLineEdit->setEnabled(FALSE);
		fileSelected();
	}
	else {
		updateModificationState();
	}
}

/**
 * Save modified files.
 */

void Kid3App::slotFileSave()
{
	updateCurrentSelection();
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	slotStatusMsg(i18n("Saving directory..."));

	saveDirectory();
	slotStatusMsg(i18n("Ready."));
	QApplication::restoreOverrideCursor();
	updateGuiControls();
}

/**
 * Quit application.
 */

void Kid3App::slotFileQuit()
{
	slotStatusMsg(i18n("Exiting..."));
	close(); /* this will lead to call of closeEvent(), queryClose() */
}

#ifdef CONFIG_USE_KDE
#if KDE_VERSION < 0x30200
/**
 * Turn tool bar on or off.
 */

void Kid3App::slotViewToolBar()
{
	slotStatusMsg(i18n("Toggling toolbar..."));
	if(!viewToolBar->isChecked()) {
		toolBar("mainToolBar")->hide();
	}
	else {
		toolBar("mainToolBar")->show();
	}		
	slotStatusMsg(i18n("Ready."));
}

/**
 * Turn status bar on or off.
 */

void Kid3App::slotViewStatusBar()
{
	slotStatusMsg(i18n("Toggle the statusbar..."));
	if(!viewStatusBar->isChecked()) {
		statusBar()->hide();
	}
	else {
		statusBar()->show();
	}
	slotStatusMsg(i18n("Ready."));
}
#else

void Kid3App::slotViewToolBar() {}
void Kid3App::slotViewStatusBar() {}

#endif

/**
 * Shortcuts configuration.
 */
void Kid3App::slotSettingsShortcuts()
{
	KKeyDialog::configure(actionCollection(), this);
}

/**
 * Display help for a topic.
 *
 * @param anchor anchor in help document
 */
void Kid3App::displayHelp(const QString& anchor)
{
	kapp->invokeHelp(anchor, QString::null, "");
}

void Kid3App::slotHelpHandbook() {}
void Kid3App::slotHelpAbout() {}
void Kid3App::slotHelpAboutQt() {}

#else /* CONFIG_USE_KDE */

void Kid3App::slotViewToolBar() {}
void Kid3App::slotViewStatusBar() {}
void Kid3App::slotSettingsShortcuts() {}

/**
 * Display help for a topic.
 *
 * @param anchor anchor in help document
 */
void Kid3App::displayHelp(const QString& anchor)
{
	if (!s_helpBrowser) {
		QString caption(i18n("Kid3 Handbook"));
		s_helpBrowser =
			new BrowserDialog(NULL, caption);
	}
	if (s_helpBrowser) { 
		s_helpBrowser->goToAnchor(anchor);
		s_helpBrowser->setModal(!anchor.isEmpty());
		if (!s_helpBrowser->isShown()) {
			s_helpBrowser->show();
		}
	}
}

/**
 * Display handbook.
 */

void Kid3App::slotHelpHandbook()
{
	displayHelp();
}

/**
 * Display "About" dialog.
 */

void Kid3App::slotHelpAbout()
{
	QMessageBox::about(
		(Kid3App*)parent(), "Kid3",
		"Kid3 " VERSION
		"\n(c) 2003-2006 Urs Fleisch\nufleisch@users.sourceforge.net");
}

/**
 * Display "About Qt" dialog.
 */

void Kid3App::slotHelpAboutQt()
{
	QMessageBox::aboutQt((Kid3App*)parent(), "Kid3");
}
#endif /* CONFIG_USE_KDE */

/**
 * Select all files.
 */
void Kid3App::slotSelectAll()
{
	view->mp3ListBox->selectAll(true);
}

/**
 * Select next file.
 */
void Kid3App::slotNextFile()
{
	int ci = view->mp3ListBox->currentItem();
	if (ci >= 0 && ci < (int)view->mp3ListBox->count() - 1) {
		++ci;
		view->mp3ListBox->clearSelection();
		view->mp3ListBox->setCurrentItem(ci);
		view->mp3ListBox->setSelected(ci, true);
	}
}

/**
 * Select previous file.
 */
void Kid3App::slotPreviousFile()
{
	int ci = view->mp3ListBox->currentItem();
	if (ci > 0) {
		--ci;
		view->mp3ListBox->clearSelection();
		view->mp3ListBox->setCurrentItem(ci);
		view->mp3ListBox->setSelected(ci, true);
	}
}

/**
 * Change status message.
 *
 * @param text message
 */

void Kid3App::slotStatusMsg(const QString &text)
{
	statusBar()->message(text);
	// processEvents() is necessary to make the change of the status bar
	// visible when it is changed back again in the same function,
	// i.e. in the same call from the Qt main event loop.
#ifdef CONFIG_USE_KDE
	kapp->processEvents();
#else
	qApp->processEvents();
#endif
}

/**
 * Create playlist.
 */

void Kid3App::slotCreatePlaylist(void)
{
	QDir dir(view->mp3ListBox->getAbsDirname());
	QString fn = view->mp3ListBox->getAbsDirname() + QDir::separator() + dir.dirName() + ".m3u";
	QFile file(fn);
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	slotStatusMsg(i18n("Creating playlist..."));
	if (file.open(QCM_WriteOnly)) {
		QTextStream stream(&file);
		FileListItem* mp3file = view->mp3ListBox->first();
		while (mp3file != 0) {
			stream << mp3file->getFile()->getFilename() << "\n";
			mp3file = view->mp3ListBox->next();
		}
		file.close();
	}
	slotStatusMsg(i18n("Ready."));
	QApplication::restoreOverrideCursor();
}

/**
 * Update track data and create import dialog.
 */
void Kid3App::setupImportDialog()
{
	m_trackDataList.clear();
	FileListItem* mp3file = view->mp3ListBox->first();
	bool firstTrack = true;
	while (mp3file != 0) {
		mp3file->getFile()->readTags(false);
		if (firstTrack) {
			StandardTags st;
			mp3file->getFile()->getStandardTagsV2(&st);
			if (st.artist.isEmpty() && st.album.isEmpty()) {
				mp3file->getFile()->getStandardTagsV1(&st);
			}
			m_trackDataList.artist = st.artist;
			m_trackDataList.album = st.album;
			firstTrack = false;
		}
		m_trackDataList.push_back(ImportTrackData(mp3file->getFile()->getAbsFilename(),
                                            mp3file->getFile()->getDuration()));
		mp3file = view->mp3ListBox->next();
	}

	if (!m_importDialog) {
		QString caption(i18n("Import"));
		m_importDialog =
			new ImportDialog(NULL, caption, m_trackDataList);
	}
	if (m_importDialog) {
		m_importDialog->clear();
	}
}

/**
 * Execute the import dialog.
 */
void Kid3App::execImportDialog()
{
	if (m_importDialog &&
			m_importDialog->exec() == QDialog::Accepted) {
		slotStatusMsg(i18n("Import..."));
		ImportTrackDataVector::const_iterator it = m_trackDataList.begin();
		StandardTags st;
		bool destV1 = m_importDialog->getDestV1();
		StandardTagsFilter flt(destV1 ?
													 view->getFilterFromID3V1() :
													 view->getFilterFromID3V2());
		bool no_selection = view->numFilesSelected() == 0;
		FileListItem* mp3file = view->mp3ListBox->first();
		while (mp3file != 0) {
			mp3file->getFile()->readTags(false);
			if (destV1) {
				mp3file->getFile()->getStandardTagsV1(&st);
			} else {
				mp3file->getFile()->getStandardTagsV2(&st);
			}
			if (it != m_trackDataList.end()) {
				(*it).copyActiveTags(st);
				++it;
			} else {
				break;
			}
			formatStandardTagsIfEnabled(&st);
			if (destV1) {
				mp3file->getFile()->setStandardTagsV1(&st, flt);
			} else {
				mp3file->getFile()->setStandardTagsV2(&st, flt);
			}
			mp3file = view->mp3ListBox->next();
		}
		if (!no_selection) {
			StandardTags st; // empty
			view->setStandardTagsV1(&st);
			view->setStandardTagsV2(&st);
			view->nameLineEdit->setEnabled(FALSE);
			fileSelected();
		}
		else {
			updateModificationState();
		}
		slotStatusMsg(i18n("Ready."));
		QApplication::restoreOverrideCursor();
	}
}

/**
 * Import.
 */

void Kid3App::slotImport(void)
{
	setupImportDialog();
	if (m_importDialog) {
		m_importDialog->setAutoStartSubDialog(ImportDialog::ASD_None);
		execImportDialog();
	}
}

/**
 * Import from freedb.org.
 */
void Kid3App::slotImportFreedb()
{
	setupImportDialog();
	if (m_importDialog) {
		m_importDialog->setAutoStartSubDialog(ImportDialog::ASD_Freedb);
		execImportDialog();
	}
}

/**
 * Import from Discogs.
 */
void Kid3App::slotImportDiscogs()
{
	setupImportDialog();
	if (m_importDialog) {
		m_importDialog->setAutoStartSubDialog(ImportDialog::ASD_Discogs);
		execImportDialog();
	}
}

/**
 * Import from MusicBrainz release database.
 */
void Kid3App::slotImportMusicBrainzRelease()
{
	setupImportDialog();
	if (m_importDialog) {
		m_importDialog->setAutoStartSubDialog(ImportDialog::ASD_MusicBrainzRelease);
		execImportDialog();
	}
}

/**
 * Import from MusicBrainz.
 */
void Kid3App::slotImportMusicBrainz()
{
#ifdef HAVE_TUNEPIMP
	setupImportDialog();
	if (m_importDialog) {
		m_importDialog->setAutoStartSubDialog(ImportDialog::ASD_MusicBrainz);
		execImportDialog();
	}
#endif
}

/**
 * Set data to be exported.
 *
 * @param src ExportDialog::SrcV1 to export ID3v1,
 *            ExportDialog::SrcV2 to export ID3v2
 */
void Kid3App::setExportData(int src)
{
	if (m_exportDialog) {
		ImportTrackDataVector trackDataVector;
		FileListItem* mp3file = view->mp3ListBox->first();
		bool firstTrack = true;
		while (mp3file != 0) {
			mp3file->getFile()->readTags(false);
			ImportTrackData trackData(mp3file->getFile()->getAbsFilename(),
																mp3file->getFile()->getDuration());
			if (src == ExportDialog::SrcV1) {
				mp3file->getFile()->getStandardTagsV1(&trackData);
			} else {
				mp3file->getFile()->getStandardTagsV2(&trackData);
			}
			if (firstTrack) {
				trackDataVector.artist = trackData.artist;
				trackDataVector.album = trackData.album;
				firstTrack = false;
			}
			trackDataVector.push_back(trackData);
			mp3file = view->mp3ListBox->next();
		}
		m_exportDialog->setExportData(trackDataVector);
	}
}

/**
 * Export.
 */
void Kid3App::slotExport()
{
	m_exportDialog = new ExportDialog(0);
	if (m_exportDialog) {
		m_exportDialog->readConfig();
		setExportData(s_genCfg.m_exportSrcV1 ?
									ExportDialog::SrcV1 : ExportDialog::SrcV2);
		connect(m_exportDialog, SIGNAL(exportDataRequested(int)),
						this, SLOT(setExportData(int)));
		m_exportDialog->exec();
		delete m_exportDialog;
		m_exportDialog = 0;
	}
}

/**
 * Show or hide the ID3V1.1 controls according to the settings and
 * set the menu entries appropriately.
 */
void Kid3App::updateHideV1()
{
	if (s_miscCfg.m_hideV1) {
		view->idV1GroupBox->hide();
#ifdef CONFIG_USE_KDE
		settingsShowHideV1->setText(i18n("Show Tag &1"));
#else
		settingsShowHideV1->setText(i18n("Show Tag 1"));
		settingsShowHideV1->setMenuText(i18n("Show Tag &1"));
#endif

	} else {
		view->idV1GroupBox->show();
#ifdef CONFIG_USE_KDE
		settingsShowHideV1->setText(i18n("Hide Tag &1"));
#else
		settingsShowHideV1->setText(i18n("Hide Tag 1"));
		settingsShowHideV1->setMenuText(i18n("Hide Tag &1"));
#endif
	}
#if QT_VERSION >= 0x040000
	view->adjustRightHalfBoxSize();
#endif
}

/**
 * Show or hide the ID3V2.3 controls according to the settings and
 * set the menu entries appropriately.
 */
void Kid3App::updateHideV2()
{
	if (s_miscCfg.m_hideV2) {
		view->idV2GroupBox->hide();
#ifdef CONFIG_USE_KDE
		settingsShowHideV2->setText(i18n("Show Tag &2"));
#else
		settingsShowHideV2->setText(i18n("Show Tag 2"));
		settingsShowHideV2->setMenuText(i18n("Show Tag &2"));
#endif

	} else {
		view->idV2GroupBox->show();
#ifdef CONFIG_USE_KDE
		settingsShowHideV2->setText(i18n("Hide Tag &2"));
#else
		settingsShowHideV2->setText(i18n("Hide Tag 2"));
		settingsShowHideV2->setMenuText(i18n("Hide Tag &2"));
#endif
	}
#if QT_VERSION >= 0x040000
	view->adjustRightHalfBoxSize();
#endif
}

/**
 * Show or hide ID3v1.1 controls.
 */
void Kid3App::slotSettingsShowHideV1()
{
	s_miscCfg.m_hideV1 = !s_miscCfg.m_hideV1;
	updateHideV1();
}

/**
 * Show or hide ID3v2.3 controls.
 */
void Kid3App::slotSettingsShowHideV2()
{
	s_miscCfg.m_hideV2 = !s_miscCfg.m_hideV2;
	updateHideV2();
}

/**
 * Preferences.
 */

void Kid3App::slotSettingsConfigure(void)
{
	QString caption(i18n("Configure - Kid3"));
#ifdef KID3_USE_KCONFIGDIALOG
	KConfigSkeleton* configSkeleton = new KConfigSkeleton;
	ConfigDialog* dialog =
		new ConfigDialog(NULL, caption, configSkeleton);
#else
	ConfigDialog *dialog =
		new ConfigDialog(NULL, caption);
#endif
	if (dialog) {
		dialog->setConfig(&s_fnFormatCfg, &s_id3FormatCfg, &s_miscCfg);
		if (dialog->exec() == QDialog::Accepted) {
			dialog->getConfig(&s_fnFormatCfg, &s_id3FormatCfg, &s_miscCfg);
			s_fnFormatCfg.writeToConfig(config);
			s_id3FormatCfg.writeToConfig(config);
			s_miscCfg.writeToConfig(config);
#ifdef CONFIG_USE_KDE
			config->sync();
#endif
		}
	}
#ifdef KID3_USE_KCONFIGDIALOG
	delete configSkeleton;
#endif
}

/**
 * Apply filename format.
 */
void Kid3App::slotApplyFilenameFormat(void)
{
	StandardTags st;
	if (view->numFilesSelected() == 1) {
		updateCurrentSelection();
	}
	FileListItem* mp3file = view->mp3ListBox->first();
	bool no_selection = view->numFilesSelected() == 0;
	while (mp3file != 0) {
		if (no_selection || mp3file->isInSelection()) {
			mp3file->getFile()->readTags(false);
			QString str;
			str = mp3file->getFile()->getFilename();
			s_fnFormatCfg.formatString(str);
			mp3file->getFile()->setFilename(str);
		}
		mp3file = view->mp3ListBox->next();
	}
	updateGuiControls();
}

/**
 * Apply ID3 format.
 */
void Kid3App::slotApplyId3Format(void)
{
	StandardTags st;
	if (view->numFilesSelected() == 1) {
		updateCurrentSelection();
	}
	StandardTagsFilter fltV1(view->getFilterFromID3V1());
	StandardTagsFilter fltV2(view->getFilterFromID3V2());
	FileListItem* mp3file = view->mp3ListBox->first();
	bool no_selection = view->numFilesSelected() == 0;
	while (mp3file != 0) {
		if (no_selection || mp3file->isInSelection()) {
			mp3file->getFile()->readTags(false);
			mp3file->getFile()->getStandardTagsV1(&st);
			s_id3FormatCfg.formatStandardTags(st);
			mp3file->getFile()->setStandardTagsV1(&st, fltV1);
			mp3file->getFile()->getStandardTagsV2(&st);
			s_id3FormatCfg.formatStandardTags(st);
			mp3file->getFile()->setStandardTagsV2(&st, fltV2);
		}
		mp3file = view->mp3ListBox->next();
	}
	updateGuiControls();
}

/**
 * Rename directory.
 */
void Kid3App::slotRenameDirectory(void)
{
	if (saveModified() && view->mp3ListBox->first()) {
		QString caption(i18n("Rename Directory"));
		RenDirDialog *dialog =
			new RenDirDialog(NULL, caption, view->mp3ListBox->first()->getFile());
		if (dialog) {
			if (dialog->exec() == QDialog::Accepted) {
				FileListItem* mp3file = view->mp3ListBox->first();
				QString errorMsg;
				bool again = false;
				while (mp3file &&
					   dialog->performAction(mp3file->getFile(), again, &errorMsg)) {
					mp3file = view->mp3ListBox->next();
				}
				openDirectory(dialog->getNewDirname());
				if (again) {
					mp3file = view->mp3ListBox->first();
					while (mp3file &&
						   dialog->performAction(mp3file->getFile(), again, &errorMsg)) {
						mp3file = view->mp3ListBox->next();
					}
					openDirectory(dialog->getNewDirname());
				}
				if (!errorMsg.isEmpty()) {
					QMessageBox::warning(0, i18n("File Error"),
										 i18n("Error while renaming:\n") +
										 errorMsg,
										 QMessageBox::Ok, QCM_NoButton);
				}
			}
		}
	}
}

/**
 * Number tracks.
 */
void Kid3App::slotNumberTracks()
{
	if (!m_numberTracksDialog) {
		m_numberTracksDialog = new NumberTracksDialog(0);
	}
	if (m_numberTracksDialog) {
		if (m_numberTracksDialog->exec() == QDialog::Accepted) {
			int nr = m_numberTracksDialog->getStartNumber();
			bool destV1 = m_numberTracksDialog->getDestV1();

			updateCurrentSelection();
			FileListItem *mp3file = view->mp3ListBox->first();
			bool no_selection = view->numFilesSelected() == 0;
			while (mp3file != 0) {
				if (no_selection || mp3file->isInSelection()) {
					mp3file->getFile()->readTags(false);
					if (destV1) {
						int oldnr = mp3file->getFile()->getTrackNumV1();
						if (nr != oldnr) {
							mp3file->getFile()->setTrackNumV1(nr);
						}
					} else {
						int oldnr = mp3file->getFile()->getTrackNumV2();
						if (nr != oldnr) {
							mp3file->getFile()->setTrackNumV2(nr);
						}
					}
					++nr;
				}
				mp3file = view->mp3ListBox->next();
			}
			updateGuiControls();
		}
	}
}

/**
 * Open directory on drop.
 *
 * @param txt URL of directory or file in directory
 */

void Kid3App::openDrop(QString txt)
{
	int lfPos = txt.find('\n');
	if (lfPos > 0 && lfPos < (int)txt.length() - 1) {
		txt.truncate(lfPos + 1);
	}
	QUrl url(txt);
	if (url.hasPath()) {
		QString dir = url.path().stripWhiteSpace();
#if defined _WIN32 || defined WIN32
		// There seems to be problems with filenames on Win32,
		// so correct
		if (dir[0] == '/' && dir[1] == '/' && dir[3] == '|') {
			dir[3] = ':';
			dir.remove(0, 2);
		} else if (dir[0] == '/' && dir[2] == ':') {
			dir.remove(0, 1);
		}
#endif
		updateCurrentSelection();
		openDirectory(dir, true);
	}
}

/**
 * Set tags in file to tags in GUI controls.
 *
 * @param mp3file file
 */

void Kid3App::updateTags(TaggedFile *mp3file)
{
	StandardTags st;
	StandardTagsFilter flt;
	flt.setAllTrue();
	view->getStandardTagsV1(&st);
	mp3file->setStandardTagsV1(&st, flt);
	view->getStandardTagsV2(&st);
	mp3file->setStandardTagsV2(&st, flt);
	if (view->nameLineEdit->isEnabled()) {
		mp3file->setFilename(view->nameLineEdit->text());
	}
}

/**
 * Update modification state, caption and listbox entries.
 */

void Kid3App::updateModificationState(void)
{
	setModified(view->mp3ListBox->updateModificationState());
#ifdef CONFIG_USE_KDE
	setCaption(doc_dir, isModified());
#else
	QString cap(doc_dir);
	if (isModified()) {
		cap += i18n(" [modified]");
	}
	if (!cap.isEmpty()) {
		cap += " - ";
	}
	cap += "Kid3";
	setCaption(cap);
#endif
}

/**
 * Update files of current selection.
 */

void Kid3App::updateCurrentSelection(void)
{
	FileListItem* mp3file = view->mp3ListBox->first();
	while (mp3file != 0) {
		if (mp3file->isInSelection()) {
			updateTags(mp3file->getFile());
		}
		mp3file = view->mp3ListBox->next();
	}
	updateModificationState();
}

/**
 * Update GUI controls from the tags in the files.
 * The new selection is stored and the GUI controls and frame list
 * updated accordingly (filtered for multiple selection).
 */
void Kid3App::updateGuiControls()
{
	StandardTags tags_v1, tags_v2;
	FileListItem* mp3file = view->mp3ListBox->first();
	FileListItem* singleItem = 0;
	TaggedFile* single_v2_file = 0;
	TaggedFile* firstMp3File = 0;
	int num_files_selected = 0;
	bool tagV1Supported = false;

	while (mp3file != 0) {
		if (mp3file->isSelected()) {
			StandardTags filetags;
			mp3file->setInSelection(TRUE);
			mp3file->getFile()->readTags(FALSE);
			mp3file->getFile()->getStandardTagsV1(&filetags);
			if (num_files_selected == 0) {
				tags_v1 = filetags;
			}
			else {
				tags_v1.filterDifferent(filetags);
			}
			mp3file->getFile()->getStandardTagsV2(&filetags);
			if (num_files_selected == 0) {
				tags_v2 = filetags;
				single_v2_file = mp3file->getFile();
				singleItem = mp3file;
				firstMp3File = mp3file->getFile();
			}
			else {
				tags_v2.filterDifferent(filetags);
				single_v2_file = 0;
				singleItem = 0;
			}
			++num_files_selected;

			if (mp3file->getFile()->isTagV1Supported()) {
				tagV1Supported = true;
			}
		}
		else {
			mp3file->setInSelection(FALSE);
		}
		mp3file = view->mp3ListBox->next();
	}

#if defined HAVE_ID3LIB && defined HAVE_TAGLIB
	if (single_v2_file) {
		if (dynamic_cast<Mp3File*>(single_v2_file) != 0 &&
				!single_v2_file->isChanged() &&
				single_v2_file->isTagInformationRead() && single_v2_file->hasTagV2() &&
				single_v2_file->getTagFormatV2() == QString::null) {
			TagLibFile* tagLibFile;
			if (singleItem &&
					(tagLibFile = new TagLibFile(
						single_v2_file->getDirname(),
						single_v2_file->getFilename())) != 0) {
				singleItem->setFile(tagLibFile);
				single_v2_file = tagLibFile;
				single_v2_file->readTags(false);
				single_v2_file->getStandardTagsV1(&tags_v1);
				single_v2_file->getStandardTagsV2(&tags_v2);
			}
		}
	}
#endif

	view->setStandardTagsV1(&tags_v1);
	view->setStandardTagsV2(&tags_v2);
	view->setAllCheckBoxes(num_files_selected == 1);
	updateModificationState();
	if (single_v2_file) {
		FrameList* framelist = single_v2_file->getFrameList();
		if (framelist) {
			framelist->setTags(single_v2_file);
		}
		view->nameLineEdit->setEnabled(TRUE);
		view->nameLineEdit->setText(single_v2_file->getFilename());
		view->detailsLabel->setText(single_v2_file->getDetailInfo());
		view->setTagFormatV1(single_v2_file->getTagFormatV1());
		view->setTagFormatV2(single_v2_file->getTagFormatV2());
	}
	else {
		FrameList* framelist;
		if (firstMp3File && (framelist = firstMp3File->getFrameList()) != 0) {
			framelist->clear();
		}
		view->nameLineEdit->setEnabled(FALSE);
		view->detailsLabel->setText("");
		view->setTagFormatV1(QString::null);
		view->setTagFormatV2(QString::null);
	}

	if (num_files_selected == 0) {
		tagV1Supported = true;
	}
	view->enableControlsV1(tagV1Supported);
}

/**
 * Process change of selection.
 * The files of the current selection are updated.
 * The new selection is stored and the GUI controls and frame list
 * updated accordingly (filtered for multiple selection).
 */
void Kid3App::fileSelected(void)
{
	updateCurrentSelection();
	updateGuiControls();
}

/**
 * Copy a set of standard tags into copy buffer.
 *
 * @param st tags to copy
 */

void Kid3App::copyTags(const StandardTags *st)
{
	*copytags = *st;
}

/**
 * Paste from copy buffer to standard tags.
 *
 * @param st tags to fill from data in copy buffer.
 */

void Kid3App::pasteTags(StandardTags *st)
{
	if (!copytags->title.isNull())
		st->title = copytags->title;
	if (!copytags->artist.isNull())
		st->artist = copytags->artist;
	if (!copytags->album.isNull())
		st->album = copytags->album;
	if (!copytags->comment.isNull())
		st->comment = copytags->comment;
	if (copytags->year >= 0)
		st->year = copytags->year;
	if (copytags->track >= 0)
		st->track = copytags->track;
	if (copytags->genre >= 0) {
		st->genre = copytags->genre;
		st->genreStr = copytags->genreStr;
	}
}

/**
 * Paste from copy buffer to ID3v1 tags.
 */
void Kid3App::pasteTagsV1()
{
	updateCurrentSelection();
	StandardTags st;
	StandardTagsFilter flt(view->getFilterFromID3V1());
	FileListItem* mp3file = view->mp3ListBox->first();
	while (mp3file != 0) {
		if (mp3file->isInSelection()) {
			mp3file->getFile()->getStandardTagsV1(&st);
			pasteTags(&st);
			formatStandardTagsIfEnabled(&st);
			mp3file->getFile()->setStandardTagsV1(&st, flt);
		}
		mp3file = view->mp3ListBox->next();
	}
	// update controls with filtered data
	updateGuiControls();
}

/**
 * Paste from copy buffer to ID3v2 tags.
 */
void Kid3App::pasteTagsV2()
{
	updateCurrentSelection();
	StandardTags st;
	StandardTagsFilter flt(view->getFilterFromID3V2());
	FileListItem* mp3file = view->mp3ListBox->first();
	while (mp3file != 0) {
		if (mp3file->isInSelection()) {
			mp3file->getFile()->getStandardTagsV2(&st);
			pasteTags(&st);
			formatStandardTagsIfEnabled(&st);
			mp3file->getFile()->setStandardTagsV2(&st, flt);
		}
		mp3file = view->mp3ListBox->next();
	}
	// update controls with filtered data
	updateGuiControls();
}

/**
 * Set ID3v1 tags according to filename.
 * If a single file is selected the tags in the GUI controls
 * are set, else the tags in the multiple selected files.
 */

void Kid3App::getTagsFromFilenameV1(void)
{
	updateCurrentSelection();
	StandardTags st;
	FileListItem* mp3file = view->mp3ListBox->first();
	bool multiselect = view->numFilesSelected() > 1;
	StandardTagsFilter flt(view->getFilterFromID3V1());
	while (mp3file != 0) {
		if (mp3file->isInSelection()) {
			if (!multiselect && view->nameLineEdit->isEnabled()) {
				mp3file->getFile()->setFilename(
					view->nameLineEdit->text());
			}
			mp3file->getFile()->getStandardTagsV1(&st);
			mp3file->getFile()->getTagsFromFilename(&st, view->formatComboBox->currentText());
			formatStandardTagsIfEnabled(&st);
			mp3file->getFile()->setStandardTagsV1(&st, flt);
		}
		mp3file = view->mp3ListBox->next();
	}
	// update controls with filtered data
	updateGuiControls();
}

/**
 * Set ID3v2 tags according to filename.
 * If a single file is selected the tags in the GUI controls
 * are set, else the tags in the multiple selected files.
 */

void Kid3App::getTagsFromFilenameV2(void)
{
	updateCurrentSelection();
	StandardTags st;
	FileListItem* mp3file = view->mp3ListBox->first();
	bool multiselect = view->numFilesSelected() > 1;
	StandardTagsFilter flt(view->getFilterFromID3V2());
	while (mp3file != 0) {
		if (mp3file->isInSelection()) {
			if (!multiselect && view->nameLineEdit->isEnabled()) {
				mp3file->getFile()->setFilename(
					view->nameLineEdit->text());
			}
			mp3file->getFile()->getStandardTagsV2(&st);
			mp3file->getFile()->getTagsFromFilename(&st, view->formatComboBox->currentText());
			formatStandardTagsIfEnabled(&st);
			mp3file->getFile()->setStandardTagsV2(&st, flt);
		}
		mp3file = view->mp3ListBox->next();
	}
	// update controls with filtered data
	updateGuiControls();
}

/**
 * Set filename according to tags.
 * If a single file is selected the tags in the GUI controls
 * are used, else the tags in the multiple selected files.
 *
 * @param tag_version 1=ID3v1, 2=ID3v2
 */

void Kid3App::getFilenameFromTags(int tag_version)
{
	updateCurrentSelection();
	StandardTags st;
	FileListItem* mp3file = view->mp3ListBox->first();
	bool multiselect = view->numFilesSelected() > 1;
	while (mp3file != 0) {
		if (mp3file->isInSelection()) {
			if (tag_version == 2) {
				mp3file->getFile()->getStandardTagsV2(&st);
			}
			else {
				mp3file->getFile()->getStandardTagsV1(&st);
			}
			mp3file->getFile()->getFilenameFromTags(
				&st, view->formatComboBox->currentText());
			formatFileNameIfEnabled(mp3file->getFile());
			if (!multiselect) {
				view->nameLineEdit->setText(
					mp3file->getFile()->getFilename());
			}
		}
		mp3file = view->mp3ListBox->next();
	}
	// update controls with filtered data
	updateGuiControls();
}

/**
 * Copy ID3v1 tags to ID3v2 tags of selected files.
 */

void Kid3App::copyV1ToV2(void)
{
	updateCurrentSelection();
	StandardTags st;
	StandardTagsFilter flt(view->getFilterFromID3V2());
	FileListItem* mp3file = view->mp3ListBox->first();
	while (mp3file != 0) {
		if (mp3file->isInSelection()) {
			mp3file->getFile()->getStandardTagsV1(&st);
			formatStandardTagsIfEnabled(&st);
			mp3file->getFile()->setStandardTagsV2(&st, flt);
		}
		mp3file = view->mp3ListBox->next();
	}
	// update controls with filtered data
	updateGuiControls();
}

/**
 * Copy ID3v2 tags to ID3v1 tags of selected files.
 */

void Kid3App::copyV2ToV1(void)
{
	updateCurrentSelection();
	StandardTags st;
	StandardTagsFilter flt(view->getFilterFromID3V1());
	FileListItem* mp3file = view->mp3ListBox->first();
	while (mp3file != 0) {
		if (mp3file->isInSelection()) {
			mp3file->getFile()->getStandardTagsV2(&st);
			formatStandardTagsIfEnabled(&st);
			mp3file->getFile()->setStandardTagsV1(&st, flt);
		}
		mp3file = view->mp3ListBox->next();
	}
	// update controls with filtered data
	updateGuiControls();
}

/**
 * Remove ID3v1 tags in selected files.
 */

void Kid3App::removeTagsV1(void)
{
	updateCurrentSelection();
	StandardTagsFilter flt(view->getFilterFromID3V1());
	FileListItem *mp3file = view->mp3ListBox->first();
	while (mp3file != 0) {
		if (mp3file->isInSelection()) {
			mp3file->getFile()->removeTagsV1(flt);
		}
		mp3file = view->mp3ListBox->next();
	}
	updateGuiControls();
}

/**
 * Remove ID3v2 tags in selected files.
 */

void Kid3App::removeTagsV2(void)
{
	updateCurrentSelection();
	StandardTagsFilter flt(view->getFilterFromID3V2());
	FileListItem* mp3file = view->mp3ListBox->first();
	while (mp3file != 0) {
		if (mp3file->isInSelection()) {
			mp3file->getFile()->removeTagsV2(flt);
		}
		mp3file = view->mp3ListBox->next();
	}
	updateGuiControls();
}

/**
 * Update ID3v2 tags in GUI controls from file displayed in frame list.
 *
 * @param taggedFile the selected file
 */
void Kid3App::updateAfterFrameModification(TaggedFile* taggedFile)
{
	if (taggedFile) {
		StandardTags st;
		taggedFile->getStandardTagsV2(&st);
		view->setStandardTagsV2(&st);
		updateModificationState();
	}
}

/**
 * Get the selected file together with its frame list.
 * If multiple files are selected, 0 is returned for both parameters.
 *
 * @param taggedFile the file is returned here,
 *                   0 if not exactly one file is selected
 * @param framelist  the frame list is returned here,
 *                   0 if not exactly one file is selected
 */
void Kid3App::getSelectedFileWithFrameList(
	TaggedFile*& taggedFile, FrameList*& framelist)
{
	taggedFile = 0;
	framelist = 0;
	if (view->numFilesSelected() != 1) {
		return;
	}
	FileListItem* mp3file = view->mp3ListBox->first();
	while (mp3file != 0) {
		if (mp3file->isInSelection()) {
			taggedFile = mp3file->getFile();
			framelist = mp3file->getFile()->getFrameList();
			return;
		}
		mp3file = view->mp3ListBox->next();
	}
}

/**
 * Edit selected frame.
 */
void Kid3App::editFrame(void)
{
	FrameList* framelist;
	TaggedFile* taggedFile;
	updateCurrentSelection();
	getSelectedFileWithFrameList(taggedFile, framelist);
	if (taggedFile && framelist && framelist->editFrame()) {
		updateAfterFrameModification(taggedFile);
	}
}

/**
 * Delete selected frame.
 */
void Kid3App::deleteFrame(void)
{
	FrameList* framelist;
	TaggedFile* taggedFile;
	updateCurrentSelection();
	getSelectedFileWithFrameList(taggedFile, framelist);
	if (taggedFile && framelist && framelist->deleteFrame()) {
		updateAfterFrameModification(taggedFile);
	}
}

/**
 * Select a frame type and add such a frame to frame list.
 */
void Kid3App::addFrame(void)
{
	FrameList* framelist;
	TaggedFile* taggedFile;
	int id;
	updateCurrentSelection();
	getSelectedFileWithFrameList(taggedFile, framelist);
	if (taggedFile && framelist &&
			(id = framelist->selectFrameId()) != -1 &&
			framelist->addFrame(id, true)) {
		updateAfterFrameModification(taggedFile);
	} else if (!taggedFile && !framelist) {
		// multiple files selected
		FileListItem* mp3file = view->mp3ListBox->first();
		bool firstFile = true;
		while (mp3file != 0) {
			if (mp3file->isInSelection()) {
				if (firstFile) {
					firstFile = false;
					taggedFile = mp3file->getFile();
					framelist = mp3file->getFile()->getFrameList();
					framelist->setTags(taggedFile);
					if ((id = framelist->selectFrameId()) != -1 &&
							framelist->addFrame(id, true)) {
						framelist->copyFrame();
					} else {
						break;
					}
				} else {
					if (mp3file->getFile()->getFrameList() == framelist) {
						framelist->setTags(mp3file->getFile());
						framelist->pasteFrame();
					}
				}
			}
			mp3file = view->mp3ListBox->next();
		}
		if (taggedFile && framelist) {
//			framelist->setTags(taggedFile);
		}
		updateModificationState();
	}
}

/**
 * Format a filename if format while editing is switched on.
 *
 * @param taggedFile file to modify
 */
void Kid3App::formatFileNameIfEnabled(TaggedFile* taggedFile) const
{
	if (s_fnFormatCfg.m_formatWhileEditing) {
		QString fn(taggedFile->getFilename());
		s_fnFormatCfg.formatString(fn);
		taggedFile->setFilename(fn);
	}
}

/**
 * Format tags if format while editing is switched on.
 *
 * @param st standard tags
 */
void Kid3App::formatStandardTagsIfEnabled(StandardTags* st) const
{
	if (s_id3FormatCfg.m_formatWhileEditing) {
		s_id3FormatCfg.formatStandardTags(*st);
	}
}
