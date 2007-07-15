/**
 * \file configdialog.cpp
 * Configuration dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 17 Sep 2003
 */

#include "configdialog.h"
#include "config.h"
#ifdef CONFIG_USE_KDE
#include <kconfig.h>
#endif

#include <qlayout.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qstring.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qtabwidget.h>
#include "qtcompatmac.h"
#if QT_VERSION >= 0x040000
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#else
#include <qgroupbox.h>
#endif

#include "formatconfig.h"
#include "formatbox.h"
#include "miscconfig.h"
#include "stringlistedit.h"
#include "commandstable.h"
#include "kid3.h"

/**
 * Constructor.
 *
 * @param parent  parent widget
 * @param caption dialog title
 */
#ifdef KID3_USE_KCONFIGDIALOG
ConfigDialog::ConfigDialog(QWidget* parent, QString& caption,
													 KConfigSkeleton* configSkeleton) :
	KConfigDialog(parent, "configure", configSkeleton,
								IconList, Ok | Cancel | Help, Ok, true)
#else
ConfigDialog::ConfigDialog(QWidget* parent, QString& caption) :
	QDialog(parent)
#endif
{
	QCM_setWindowTitle(caption);
	QVBoxLayout* vlayout = new QVBoxLayout(this);
	vlayout->setSpacing(6);
	vlayout->setMargin(6);
#ifndef KID3_USE_KCONFIGDIALOG
	QTabWidget* tabWidget = new QTabWidget(this);
#endif

	QWidget* tagsPage = new QWidget;
	if (tagsPage) {
		QVBoxLayout* vlayout = new QVBoxLayout(tagsPage);
		if (vlayout) {
			vlayout->setMargin(6);
			vlayout->setSpacing(6);
#if QT_VERSION >= 0x040000
			QGroupBox* v1GroupBox = new QGroupBox(i18n("ID3v1"), tagsPage);
#else
			QGroupBox* v1GroupBox = new QGroupBox(1, Qt::Horizontal, i18n("ID3v1"), tagsPage);
#endif
			if (v1GroupBox) {
				m_markTruncationsCheckBox = new QCheckBox(i18n("&Mark truncated fields"), v1GroupBox);
#if QT_VERSION >= 0x040000
				QHBoxLayout* hbox = new QHBoxLayout;
				hbox->setMargin(2);
				hbox->addWidget(m_markTruncationsCheckBox);
				v1GroupBox->setLayout(hbox);
#endif
				vlayout->addWidget(v1GroupBox);
			}
#if QT_VERSION >= 0x040000
			QGroupBox* v2GroupBox = new QGroupBox(i18n("ID3v2"), tagsPage);
#else
			QGroupBox* v2GroupBox = new QGroupBox(1, Qt::Horizontal, i18n("ID3v2"), tagsPage);
#endif
			if (v2GroupBox) {
				m_totalNumTracksCheckBox = new QCheckBox(i18n("Use &track/total number of tracks format"), v2GroupBox);
#if defined HAVE_ID3LIB && defined HAVE_TAGLIB
				QLabel* id3v2VersionLabel = new QLabel(i18n("&Version used for new tags:"), v2GroupBox);
				m_id3v2VersionComboBox = new QComboBox(v2GroupBox);
				if (id3v2VersionLabel && m_id3v2VersionComboBox) {
					m_id3v2VersionComboBox->QCM_insertItem(MiscConfig::ID3v2_3_0, i18n("ID3v2.3.0 (id3lib)"));
					m_id3v2VersionComboBox->QCM_insertItem(MiscConfig::ID3v2_4_0, i18n("ID3v2.4.0 (TagLib)"));
					m_id3v2VersionComboBox->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum));
					id3v2VersionLabel->setBuddy(m_id3v2VersionComboBox);
				}
#endif
#if QT_VERSION >= 0x040000
				QVBoxLayout* vbox = new QVBoxLayout;
				vbox->setMargin(2);
				vbox->addWidget(m_totalNumTracksCheckBox);
#if defined HAVE_ID3LIB && defined HAVE_TAGLIB
				vbox->addWidget(id3v2VersionLabel);
				vbox->addWidget(m_id3v2VersionComboBox);
#endif
				v2GroupBox->setLayout(vbox);
#endif
				vlayout->addWidget(v2GroupBox);
			}
#ifdef HAVE_VORBIS
#if QT_VERSION >= 0x040000
			QGroupBox* vorbisGroupBox = new QGroupBox(i18n("Ogg/Vorbis"), tagsPage);
#else
			QGroupBox* vorbisGroupBox = new QGroupBox(2, Qt::Horizontal, i18n("Ogg/Vorbis"), tagsPage);
#endif
			if (vorbisGroupBox) {
				QLabel* commentNameLabel = new QLabel(i18n("Comment field &name:"), vorbisGroupBox);
				m_commentNameComboBox = new QComboBox(vorbisGroupBox);
				if (commentNameLabel && m_commentNameComboBox) {
					m_commentNameComboBox->setEditable(true);
					QStringList items;
					items += "COMMENT";
					items += "DESCRIPTION";
					m_commentNameComboBox->QCM_addItems(items);
					m_commentNameComboBox->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum));
					commentNameLabel->setBuddy(m_commentNameComboBox);
				}
#if QT_VERSION >= 0x040000
				QHBoxLayout* hbox = new QHBoxLayout;
				hbox->setMargin(2);
				hbox->addWidget(commentNameLabel);
				hbox->addWidget(m_commentNameComboBox);
				vorbisGroupBox->setLayout(hbox);
#endif
				vlayout->addWidget(vorbisGroupBox);
			}
#endif
			QHBoxLayout* hlayout = new QHBoxLayout;
			if (hlayout) {
#if QT_VERSION >= 0x040000
				QGroupBox* genresGroupBox = new QGroupBox(i18n("Custom &Genres"), tagsPage);
#else
				QGroupBox* genresGroupBox = new QGroupBox(1, Qt::Horizontal, i18n("Custom &Genres"), tagsPage);
#endif
				if (genresGroupBox) {
					m_onlyCustomGenresCheckBox = new QCheckBox(i18n("&Show only custom genres"), genresGroupBox);
					m_genresEdit = new StringListEdit(genresGroupBox);
#if QT_VERSION >= 0x040000
					QVBoxLayout* vbox = new QVBoxLayout;
					vbox->setMargin(2);
					vbox->addWidget(m_onlyCustomGenresCheckBox);
					vbox->addWidget(m_genresEdit);
					genresGroupBox->setLayout(vbox);
#endif
					hlayout->addWidget(genresGroupBox);
				}
				QString id3FormatTitle(i18n("&Tag Format"));
				m_id3FormatBox = new FormatBox(id3FormatTitle, tagsPage);
				if (m_id3FormatBox) {
					hlayout->addWidget(m_id3FormatBox);
				}
				vlayout->addLayout(hlayout);
			}
		}
#ifdef KID3_USE_KCONFIGDIALOG
		addPage(tagsPage, i18n("Tags"), "package_multimedia");
#else
		tabWidget->addTab(tagsPage, i18n("&Tags"));
#endif
	}

	QWidget* filesPage = new QWidget;
	if (filesPage) {
		QVBoxLayout* vlayout = new QVBoxLayout(filesPage);
		if (vlayout) {
			vlayout->setMargin(6);
			vlayout->setSpacing(6);
#if QT_VERSION >= 0x040000
			QGroupBox* saveGroupBox = new QGroupBox(i18n("Save"), filesPage);
#else
			QGroupBox* saveGroupBox = new QGroupBox(1, Qt::Horizontal, i18n("Save"), filesPage);
#endif
			if (saveGroupBox) {
				m_preserveTimeCheckBox = new QCheckBox(i18n("&Preserve file timestamp"), saveGroupBox);
#if QT_VERSION >= 0x040000
				QHBoxLayout* hbox = new QHBoxLayout;
				hbox->setMargin(2);
				hbox->addWidget(m_preserveTimeCheckBox);
				saveGroupBox->setLayout(hbox);
#endif
				vlayout->addWidget(saveGroupBox);
			}
			QString fnFormatTitle(i18n("&Filename Format"));
			m_fnFormatBox = new FormatBox(fnFormatTitle, filesPage);
			if (m_fnFormatBox) {
				vlayout->addWidget(m_fnFormatBox);
			}
		}
#ifdef KID3_USE_KCONFIGDIALOG
		addPage(filesPage, i18n("Files"), "package_system");
#else
		tabWidget->addTab(filesPage, i18n("&Files"));
#endif
	}

	QWidget* actionsPage = new QWidget;
	if (actionsPage) {
		QVBoxLayout* vlayout = new QVBoxLayout(actionsPage);
		if (vlayout) {
			vlayout->setMargin(6);
			vlayout->setSpacing(6);
#if QT_VERSION >= 0x040000
			QGroupBox* browserGroupBox = new QGroupBox(i18n("Browser"), actionsPage);
#else
			QGroupBox* browserGroupBox = new QGroupBox(2, Qt::Horizontal, i18n("Browser"), actionsPage);
#endif
			if (browserGroupBox) {
				QLabel* browserLabel = new QLabel(i18n("Web &browser:"), browserGroupBox);
				m_browserLineEdit = new QLineEdit(browserGroupBox);
				if (browserLabel && m_browserLineEdit) {
					browserLabel->setBuddy(m_browserLineEdit);
				}
#if QT_VERSION >= 0x040000
				QHBoxLayout* hbox = new QHBoxLayout;
				hbox->setMargin(2);
				hbox->addWidget(browserLabel);
				hbox->addWidget(m_browserLineEdit);
				browserGroupBox->setLayout(hbox);
#endif
				vlayout->addWidget(browserGroupBox);
			}

#if QT_VERSION >= 0x040000
			QGroupBox* commandsGroupBox = new QGroupBox(i18n("Context &Menu Commands"), actionsPage);
#else
			QGroupBox* commandsGroupBox = new QGroupBox(1, Qt::Horizontal, i18n("Context &Menu Commands"), actionsPage);
#endif
			if (commandsGroupBox) {
				m_commandsTable = new CommandsTable(commandsGroupBox);
#if QT_VERSION >= 0x040000
				QHBoxLayout* hbox = new QHBoxLayout;
				hbox->setMargin(2);
				hbox->addWidget(m_commandsTable);
				commandsGroupBox->setLayout(hbox);
#endif
				vlayout->addWidget(commandsGroupBox);
			}
		}
#ifdef KID3_USE_KCONFIGDIALOG
		addPage(actionsPage, i18n("User Actions"), "package_utilities");
#else
		tabWidget->addTab(actionsPage, i18n("&User Actions"));
#endif
	}

	QWidget* networkPage = new QWidget;
	if (networkPage) {
		QVBoxLayout* vlayout = new QVBoxLayout(networkPage);
		if (vlayout) {
			vlayout->setMargin(6);
			vlayout->setSpacing(6);
#if QT_VERSION >= 0x040000
			QGroupBox* proxyGroupBox = new QGroupBox(i18n("Proxy"), networkPage);
#else
			QGroupBox* proxyGroupBox = new QGroupBox(2, Qt::Horizontal, i18n("Proxy"), networkPage);
#endif
			if (proxyGroupBox) {
				m_proxyCheckBox = new QCheckBox(i18n("&Proxy:"), proxyGroupBox);
				m_proxyLineEdit = new QLineEdit(proxyGroupBox);
#if QT_VERSION >= 0x040000
				QHBoxLayout* hbox = new QHBoxLayout;
				hbox->setMargin(2);
				hbox->addWidget(m_proxyCheckBox);
				hbox->addWidget(m_proxyLineEdit);
				proxyGroupBox->setLayout(hbox);
#endif
				vlayout->addWidget(proxyGroupBox);
			}

			QSpacerItem* vspacer = new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
			vlayout->addItem(vspacer);
		}
#ifdef KID3_USE_KCONFIGDIALOG
		addPage(networkPage, i18n("Network"), "package_network");
#else
		tabWidget->addTab(networkPage, i18n("&Network"));
#endif
	}

#ifndef KID3_USE_KCONFIGDIALOG
	vlayout->addWidget(tabWidget);
	QHBoxLayout* hlayout = new QHBoxLayout;
	QSpacerItem* hspacer = new QSpacerItem(16, 0, QSizePolicy::Expanding,
	                                       QSizePolicy::Minimum);
	QPushButton* helpButton = new QPushButton(i18n("&Help"), this);
	QPushButton* okButton = new QPushButton(i18n("&OK"), this);
	QPushButton* cancelButton = new QPushButton(i18n("&Cancel"), this);
	if (hlayout && helpButton && okButton && cancelButton) {
		hlayout->addWidget(helpButton);
		hlayout->addItem(hspacer);
		hlayout->addWidget(okButton);
		hlayout->addWidget(cancelButton);
		okButton->setDefault(true);
		connect(helpButton, SIGNAL(clicked()), this, SLOT(slotHelp()));
		connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
		connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
		vlayout->addLayout(hlayout);
	}
#endif
}

/**
 * Destructor.
 */
ConfigDialog::~ConfigDialog()
{}

/**
 * Set values in dialog from current configuration.
 *
 * @param fnCfg   filename format configuration
 * @param fnCfg   ID3 format configuration
 * @param miscCfg misc. configuration
 */
void ConfigDialog::setConfig(const FormatConfig* fnCfg,
							 const FormatConfig* id3Cfg,
							 const MiscConfig* miscCfg)
{
	m_fnFormatBox->fromFormatConfig(fnCfg);
	m_id3FormatBox->fromFormatConfig(id3Cfg);
	m_markTruncationsCheckBox->setChecked(miscCfg->m_markTruncations);
	m_totalNumTracksCheckBox->setChecked(miscCfg->m_enableTotalNumberOfTracks);
	m_preserveTimeCheckBox->setChecked(miscCfg->m_preserveTime);
	m_onlyCustomGenresCheckBox->setChecked(miscCfg->m_onlyCustomGenres);
	m_genresEdit->setStrings(miscCfg->m_customGenres);
	m_commandsTable->setCommandList(miscCfg->m_contextMenuCommands);
#ifdef HAVE_VORBIS
#if QT_VERSION >= 0x040000
	int idx = m_commentNameComboBox->findText(miscCfg->m_commentName);
	if (idx >= 0) {
		m_commentNameComboBox->setCurrentIndex(idx);
	} else {
		m_commentNameComboBox->addItem(miscCfg->m_commentName);
		m_commentNameComboBox->setCurrentIndex(m_commentNameComboBox->count() - 1);
	}
#else
	m_commentNameComboBox->setCurrentText(miscCfg->m_commentName);
#endif
#endif
#if defined HAVE_ID3LIB && defined HAVE_TAGLIB
	m_id3v2VersionComboBox->QCM_setCurrentIndex(miscCfg->m_id3v2Version);
#endif
	m_browserLineEdit->setText(miscCfg->m_browser);
	m_proxyCheckBox->setChecked(miscCfg->m_useProxy);
	m_proxyLineEdit->setText(miscCfg->m_proxy);
}

/**
 * Get values from dialog and store them in the current configuration.
 *
 * @param fnCfg   filename format configuration
 * @param fnCfg   ID3 format configuration
 * @param miscCfg misc. configuration
 */
void ConfigDialog::getConfig(FormatConfig* fnCfg,
							 FormatConfig* id3Cfg,
							 MiscConfig* miscCfg) const
{
	m_fnFormatBox->toFormatConfig(fnCfg);
	m_id3FormatBox->toFormatConfig(id3Cfg);
	miscCfg->m_markTruncations = m_markTruncationsCheckBox->isChecked();
	miscCfg->m_enableTotalNumberOfTracks = m_totalNumTracksCheckBox->isChecked();
	miscCfg->m_preserveTime = m_preserveTimeCheckBox->isChecked();
	miscCfg->m_onlyCustomGenres = m_onlyCustomGenresCheckBox->isChecked();
	m_genresEdit->getStrings(miscCfg->m_customGenres);
	m_commandsTable->getCommandList(miscCfg->m_contextMenuCommands);
#ifdef HAVE_VORBIS
	miscCfg->m_commentName = m_commentNameComboBox->currentText();
#endif
#if defined HAVE_ID3LIB && defined HAVE_TAGLIB
	miscCfg->m_id3v2Version = m_id3v2VersionComboBox->QCM_currentIndex();
#endif
	miscCfg->m_browser = m_browserLineEdit->text();
	miscCfg->m_useProxy = m_proxyCheckBox->isChecked();
	miscCfg->m_proxy = m_proxyLineEdit->text();
}

/**
 * Show help.
 */
void ConfigDialog::slotHelp()
{
	Kid3App::displayHelp("configure-kid3");
}
