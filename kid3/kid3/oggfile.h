/**
 * \file oggfile.h
 * Handling of Ogg files.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 26 Sep 2005
 */

#ifndef OGGFILE_H
#define OGGFILE_H

#include "config.h"
#if defined HAVE_VORBIS || defined HAVE_FLAC

#include "taggedfile.h"
#include "qtcompatmac.h"
#if QT_VERSION >= 0x040000
#include <Q3ValueList>
#else
#include <qvaluelist.h>
#endif

class OggFrameList;

 /** List box item containing OGG file */
class OggFile : public TaggedFile {
public:
	/**
	 * Constructor.
	 *
	 * @param dn directory name
	 * @param fn filename
	 */
	OggFile(const QString& dn, const QString& fn);

	/**
	 * Destructor.
	 */
	virtual ~OggFile();

	/**
	 * Read tags from file.
	 *
	 * @param force true to force reading even if tags were already read.
	 */
	virtual void readTags(bool force);

	/**
	 * Write tags to file and rename it if necessary.
	 *
	 * @param force   true to force writing even if file was not changed.
	 * @param renamed will be set to true if the file was renamed,
	 *                i.e. the file name is no longer valid, else *renamed
	 *                is left unchanged
	 * @param preserve true to preserve file time stamps
	 *
	 * @return true if ok, false if the file could not be written or renamed.
	 */
	virtual bool writeTags(bool force, bool* renamed, bool preserve);

	/**
	 * Remove all ID3v2 tags.
	 *
	 * @param flt filter specifying which fields to remove
	 */
	virtual void removeTagsV2(const StandardTagsFilter& flt);

	/**
	 * Get ID3v2 title.
	 *
	 * @return string,
	 *         "" if the field does not exist,
	 *         QString::null if the tags do not exist.
	 */
	virtual QString getTitleV2();

	/**
	 * Get ID3v2 artist.
	 *
	 * @return string,
	 *         "" if the field does not exist,
	 *         QString::null if the tags do not exist.
	 */
	virtual QString getArtistV2();

	/**
	 * Get ID3v2 album.
	 *
	 * @return string,
	 *         "" if the field does not exist,
	 *         QString::null if the tags do not exist.
	 */
	virtual QString getAlbumV2();

	/**
	 * Get ID3v2 comment.
	 *
	 * @return string,
	 *         "" if the field does not exist,
	 *         QString::null if the tags do not exist.
	 */
	virtual QString getCommentV2();

	/**
	 * Get ID3v2 year.
	 *
	 * @return number,
	 *         0 if the field does not exist,
	 *         -1 if the tags do not exist.
	 */
	virtual int getYearV2();

	/**
	 * Get ID3v2 track.
	 *
	 * @return number,
	 *         0 if the field does not exist,
	 *         -1 if the tags do not exist.
	 */
	virtual int getTrackNumV2();

	/**
	 * Get ID3v2 genre.
	 *
	 * @return number,
	 *         0xff if the field does not exist,
	 *         -1 if the tags do not exist.
	 */
	virtual int getGenreNumV2();

	/**
	 * Get ID3v2 genre as text.
	 *
	 * @return string,
	 *         "" if the field does not exist,
	 *         QString::null if the tags do not exist.
	 */
	virtual QString getGenreV2();

	/**
	 * Set ID3v2 title.
	 *
	 * @param str string to set, "" to remove field.
	 */
	virtual void setTitleV2(const QString& str);

	/**
	 * Set ID3v2 artist.
	 *
	 * @param str string to set, "" to remove field.
	 */
	virtual void setArtistV2(const QString& str);

	/**
	 * Set ID3v2 album.
	 *
	 * @param str string to set, "" to remove field.
	 */
	virtual void setAlbumV2(const QString& str);

	/**
	 * Set ID3v2 comment.
	 *
	 * @param str string to set, "" to remove field.
	 */
	virtual void setCommentV2(const QString& str);

	/**
	 * Set ID3v2 year.
	 *
	 * @param num number to set, 0 to remove field.
	 */
	virtual void setYearV2(int num);

	/**
	 * Set ID3v2 track.
	 *
	 * @param num number to set, 0 to remove field.
	 */
	virtual void setTrackNumV2(int num);

	/**
	 * Set ID3v2 genre.
	 *
	 * @param num number to set, 0xff to remove field.
	 */
	virtual void setGenreNumV2(int num);

	/**
	 * Set ID3v2 genre as text.
	 *
	 * @param str string to set, "" to remove field, QString::null to ignore.
	 */
	virtual void setGenreV2(const QString& str);

	/**
	 * Check if tag information has already been read.
	 *
	 * @return true if information is available,
	 *         false if the tags have not been read yet, in which case
	 *         hasTagV1() and hasTagV2() do not return meaningful information.
	 */
	virtual bool isTagInformationRead() const;

	/**
	 * Check if file has an ID3v2 tag.
	 *
	 * @return true if a V2 tag is available.
	 * @see isTagInformationRead()
	 */
	virtual bool hasTagV2() const;

	/**
	 * Get technical detail information.
	 *
	 * @return string with detail information,
	 *         "" if no information available.
	 */
	virtual QString getDetailInfo() const;

	/**
	 * Get duration of file.
	 *
	 * @return duration in seconds,
	 *         0 if unknown.
	 */
	virtual unsigned getDuration() const;

	/**
	 * Get frame list for this type of tagged file.
	 *
	 * @return frame list.
	 */
	virtual FrameList* getFrameList() const;

	/**
	 * Get file extension including the dot.
	 *
	 * @return file extension ".ogg".
	 */
	virtual QString getFileExtension() const;

	/**
	 * Get the format of tag 2.
	 *
	 * @return "Vorbis".
	 */
	virtual QString getTagFormatV2() const;

	/**
	 * Clean up static resources.
	 */
	static void staticCleanup();

	friend class OggFrameList;

protected:
	/** Vorbis comment field. */
	class CommentField {
	public:
		/** Constructor. */
		CommentField(const QString& name = QString::null,
								 const QString& value = QString::null) :
			m_name(name), m_value(value) {}
		/** Destructor. */
		~CommentField() {}
		/**
		 * Get name.
		 * @return name.
		 */
		QString getName() const { return m_name; }
		/**
		 * Get value.
		 * @return value.
		 */
		QString getValue() const { return m_value; }
		/**
		 * Set value.
		 * @param value value
		 */
		void setValue(const QString& value) { m_value = value; }

	private:
		QString m_name;
		QString m_value;
	};

	/** Vorbis comment list. */
	class CommentList : public Q3ValueList<CommentField> {
	public:
		/** Constructor. */
		CommentList() {}
		/** Destructor. */
		~CommentList() {}
		/**
		 * Get value.
		 * @param name name
		 * @return value, "" if not found.
		 */
		QString getValue(const QString& name) const;
		/**
		 * Set value.
		 * @param name name
		 * @param value value
		 * @return true if value was changed.
		 */
		bool setValue(const QString& name, const QString& value);
	};

	/**
	 * Get text field.
	 *
	 * @param name name
	 * @return value, "" if not found,
	 *         QString::null if the tags have not been read yet.
	 */
	QString getTextField(const QString& name) const;

	/**
	 * Set text field.
	 * If value is null or the tags have not been read yet, nothing is changed.
	 * If value is different from the current value, changedV2 is set.
	 *
	 * @param name name
	 * @param value value, "" to remove, QString::null to do nothing
	 */
	void setTextField(const QString& name, const QString& value);

	/** Comments of this file. */
	CommentList m_comments;
	/** true if file has been read. */
	bool m_fileRead;
	/** true if comments are changed. */
	bool m_changed;

private:
	OggFile(const OggFile&);
	OggFile& operator=(const OggFile&);

#ifdef HAVE_VORBIS
	/** Information about Ogg/Vorbis file. */
	struct FileInfo {
		/**
		 * Read information about an Ogg/Vorbis file.
		 * @param fn file name
		 * @return true if ok.
		 */
		bool read(const char* fn);

		bool valid;      /**< true if read() was successful */
		int version;     /**< vorbis encoder version */
		int channels;    /**< number of channels */
		long sampleRate; /**< sample rate in Hz */
		long bitrate;    /**< bitrate in bits/s */
		long duration;   /**< duration in seconds */
	};

	/** Info about file. */
	FileInfo m_fileInfo;

	/** Frame list for Ogg files. */
	static OggFrameList* s_oggFrameList;
#endif // HAVE_VORBIS
};

#endif // HAVE_VORBIS || define HAVE_FLAC

#endif // OGGFILE_H
