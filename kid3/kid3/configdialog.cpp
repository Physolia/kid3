/**
 * \file configdialog.cpp
 * Configuration dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 17 Sep 2003
 */

#include "configdialog.h"
#ifdef CONFIG_USE_KDE
#include <klocale.h>
#include <kconfig.h>
#else
#define i18n(s) tr(s)
#define I18N_NOOP(s) QT_TR_NOOP(s)
#endif

#include <qlayout.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qstring.h>
#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qcombobox.h>

#include "formatconfig.h"
#include "formatbox.h"
#include "miscconfig.h"
#include "commandstable.h"

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
								IconList, Ok | Cancel, Ok, true)
#else
ConfigDialog::ConfigDialog(QWidget* parent, QString& caption) :
	QTabDialog(parent, "configure", true)
#endif
{
	setCaption(caption);
	QWidget* generalPage = new QWidget(this);
	if (generalPage) {
		QVBoxLayout* vlayout = new QVBoxLayout(generalPage, 6, 6);
		if (vlayout) {
			QGroupBox* saveGroupBox = new QGroupBox(1, Horizontal, i18n("Save"), generalPage);
			if (saveGroupBox) {
				m_preserveTimeCheckBox = new QCheckBox(i18n("&Preserve file timestamp"), saveGroupBox);
				vlayout->addWidget(saveGroupBox);
			}
			QGroupBox* v2GroupBox = new QGroupBox(1, Horizontal, i18n("ID3v2.3"), generalPage);
			if (v2GroupBox) {
				m_totalNumTracksCheckBox = new QCheckBox(i18n("Use &track/total number of tracks format"), v2GroupBox);
				vlayout->addWidget(v2GroupBox);
			}
#ifdef HAVE_VORBIS
			QGroupBox* vorbisGroupBox = new QGroupBox(2, Horizontal, i18n("Ogg/Vorbis"), generalPage);
			if (vorbisGroupBox) {
				QLabel* commentNameLabel = new QLabel(i18n("Comment Field &Name:"), vorbisGroupBox);
				m_commentNameComboBox = new QComboBox(true, vorbisGroupBox);
				if (commentNameLabel && m_commentNameComboBox) {
					static const char* items[] = { "COMMENT", "DESCRIPTION", 0 };
					m_commentNameComboBox->insertStrList(items);
#if QT_VERSION >= 0x030100
					m_commentNameComboBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
#else
					m_commentNameComboBox->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum));
#endif
					commentNameLabel->setBuddy(m_commentNameComboBox);
				}
				vlayout->addWidget(vorbisGroupBox);
			}
#endif
#if QT_VERSION >= 300
			QGroupBox* commandsGroupBox = new QGroupBox(1, Horizontal, i18n("Context &Menu Commands"), generalPage);
			if (commandsGroupBox) {
				m_commandsTable = new CommandsTable(commandsGroupBox, "commandsTable");
				vlayout->addWidget(commandsGroupBox);
			}
#endif
			QSpacerItem* vspacer = new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
			vlayout->addItem(vspacer);
		}
#ifdef KID3_USE_KCONFIGDIALOG
		addPage(generalPage, i18n("General"), "package_system");
#else
		addTab(generalPage, i18n("&General"));
#endif
	}

	QWidget* formatPage = new QWidget(this);
	if (formatPage) {
		QVBoxLayout* vlayout = new QVBoxLayout(formatPage, 6, 6);
		if (vlayout) {
			formatEditingCheckBox = new QCheckBox(formatPage, "formatEditingCheckBox");
			formatEditingCheckBox->setText(i18n("Format while &editing:"));
			vlayout->addWidget(formatEditingCheckBox);

			QHBoxLayout *fmtLayout = new QHBoxLayout(vlayout);
			QString fnFormatTitle(i18n("&Filename Format"));
			fnFormatBox = new FormatBox(fnFormatTitle, formatPage, "fnFormatBox");
			QString id3FormatTitle(i18n("&ID3 Format"));
			id3FormatBox = new FormatBox(id3FormatTitle, formatPage, "id3FormatBox");
			if (fmtLayout && fnFormatBox && id3FormatBox) {
				fmtLayout->addWidget(fnFormatBox);
				fmtLayout->addWidget(id3FormatBox);
			}
		}
#ifdef KID3_USE_KCONFIGDIALOG
		addPage(formatPage, i18n("Format"), "package_editors");
#else
		addTab(formatPage, i18n("F&ormat"));
#endif
	}
#ifndef KID3_USE_KCONFIGDIALOG
	setOkButton();
	setCancelButton();
#endif
#if QT_VERSION < 300
	resize(415, 488);
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
void ConfigDialog::setConfig(const FormatConfig *fnCfg,
							 const FormatConfig *id3Cfg,
							 const MiscConfig *miscCfg)
{
	fnFormatBox->fromFormatConfig(fnCfg);
	id3FormatBox->fromFormatConfig(id3Cfg);
	formatEditingCheckBox->setChecked(miscCfg->formatWhileEditing);
	m_totalNumTracksCheckBox->setChecked(miscCfg->m_enableTotalNumberOfTracks);
	m_preserveTimeCheckBox->setChecked(miscCfg->m_preserveTime);
#if QT_VERSION >= 300
	m_commandsTable->setCommandList(miscCfg->m_contextMenuCommands);
#endif
#ifdef HAVE_VORBIS
#if QT_VERSION >= 300
	m_commentNameComboBox->setCurrentText(miscCfg->m_commentName);
#else
	m_commentNameComboBox->setEditText(miscCfg->m_commentName);
#endif
#endif
}

/**
 * Get values from dialog and store them in the current configuration.
 *
 * @param fnCfg   filename format configuration
 * @param fnCfg   ID3 format configuration
 * @param miscCfg misc. configuration
 */
void ConfigDialog::getConfig(FormatConfig *fnCfg,
							 FormatConfig *id3Cfg,
							 MiscConfig *miscCfg) const
{
	fnFormatBox->toFormatConfig(fnCfg);
	id3FormatBox->toFormatConfig(id3Cfg);
	miscCfg->formatWhileEditing = formatEditingCheckBox->isChecked();
	miscCfg->m_enableTotalNumberOfTracks = m_totalNumTracksCheckBox->isChecked();
	miscCfg->m_preserveTime = m_preserveTimeCheckBox->isChecked();
#if QT_VERSION >= 300
	m_commandsTable->getCommandList(miscCfg->m_contextMenuCommands);
#endif
#ifdef HAVE_VORBIS
	miscCfg->m_commentName = m_commentNameComboBox->currentText();
#endif
}
