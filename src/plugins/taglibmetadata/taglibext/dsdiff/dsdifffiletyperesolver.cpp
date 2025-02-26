/***************************************************************************
    copyright            : (C) 2006 by Martin Aumueller
    email                : aumuell@reserv.at
    copyright            : (C) 2022 by Thomas Hou
    email                : hytcqq@gmail.com
***************************************************************************/

/***************************************************************************
 *   This library is free software; you can redistribute it and/or modify  *
 *   it  under the terms of the GNU Lesser General Public License version  *
 *   2.1 as published by the Free Software Foundation.                     *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful, but   *
 *   WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,            *
 *   MA  02110-1301  USA                                                   *
 ***************************************************************************/

#if defined __GNUC__ && (__GNUC__ * 100 + __GNUC_MINOR__) >= 407
/** Defined if GCC is used and supports diagnostic pragmas */
#define GCC_HAS_DIAGNOSTIC_PRAGMA
#endif

#ifdef GCC_HAS_DIAGNOSTIC_PRAGMA
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
#endif

#include "dsdifffiletyperesolver.h"
#include "dsdifffile.h"

#include <cstring>

#if (((TAGLIB_MAJOR_VERSION) << 16) + ((TAGLIB_MINOR_VERSION) << 8) + (TAGLIB_PATCH_VERSION)) > 0x010400  && defined _WIN32

TagLib::File *DSDIFFFileTypeResolver::createFile(TagLib::FileName fileName,
        bool readProperties,
        TagLib::AudioProperties::ReadStyle propertiesStyle) const
{
    const wchar_t* wstr = static_cast<const wchar_t*>(fileName);
    const char* str = static_cast<const char*>(fileName);
    const wchar_t* wext;
    const char* ext;
    if ((wstr && (wext = wcsrchr(fileName, L'.')) != 0 && !_wcsicmp(wext, L".dff")) ||
        (str  && (ext  = strrchr(fileName,  '.')) != 0 && !_stricmp(ext, ".dff")))
    {
        return new DSDIFFFile(fileName, readProperties, propertiesStyle);
    }

    return 0;
}

#else

TagLib::File *DSDIFFFileTypeResolver::createFile(const char *fileName,
        bool readProperties,
        TagLib::AudioProperties::ReadStyle propertiesStyle) const
{
    const char *ext = strrchr(fileName, '.');
    if(ext && !strcasecmp(ext, ".dff"))
    {
        return new DSDIFFFile(fileName, readProperties, propertiesStyle);
    }

    return nullptr;
}

#endif

#ifdef GCC_HAS_DIAGNOSTIC_PRAGMA
#pragma GCC diagnostic pop
#endif
