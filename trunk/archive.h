#ifndef __ARCHIVE_H
#define __ARCHIVE_H

/**
 *
 * Tennix Archive File Format
 * Copyright (C) 2009 Thomas Perl <thp@thpinfo.com>
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, 
 * MA  02110-1301, USA.
 *
 **/


#include <stdint.h>

#define TENNIX_ARCHIVE_HEADER "thpinfo.com/2009/tennix/afmt "
#define TENNIX_ARCHIVE_HEADER_LEN 30

#define TENNIX_ARCHIVE_VERSIONMAJOR 1
#define TENNIX_ARCHIVE_VERSIONMINOR 0

#define TENNIX_ARCHIVE_ITEM_MAXNAME 86


/* architecture-independent (in-file) structs */

struct _TennixArchiveItem {
    char filename[TENNIX_ARCHIVE_ITEM_MAXNAME];
    uint32_t offset; /* network byte order */
    uint32_t length; /* network byte order */
    uint8_t key;
};

typedef struct _TennixArchiveItem TennixArchiveItem;

struct _TennixArchiveHeader {
    char header[TENNIX_ARCHIVE_HEADER_LEN];
    uint8_t versionmajor; /* major file version */
    uint8_t versionminor; /* minor file version */
    uint8_t key;
    uint8_t items; /* maximum 255 files per archive */
};

typedef struct _TennixArchiveHeader TennixArchiveHeader;



/* architecture-dependent (in-memory) structs */

struct _TennixArchive {
    FILE *fp;
    TennixArchiveHeader header;
    TennixArchiveItem *items;
    char** blobs;
    size_t offset;
    int current_item;
};

typedef struct _TennixArchive TennixArchive;

/* reading existing archives */
TennixArchive* tnxar_open(char* filename);
int tnxar_set_current_filename(TennixArchive* tnxar, char* filename);
char* tnxar_get_current_filename(TennixArchive* tnxar);
char* tnxar_read_current(TennixArchive* tnxar);
size_t tnxar_size_current(TennixArchive* tnxar);
int tnxar_eof(TennixArchive* tnxar);
void tnxar_next(TennixArchive* tnxar);
void tnxar_close(TennixArchive* tnxar);

/* utility functions */
void tnxar_xormem(char* mem, uint32_t length, char key);


#ifdef TENNIXAR_STANDALONE
/* for debugging */
void tnxar_dump(TennixArchive* tnxar);

/* creating new archives */
TennixArchive* tnxar_create();
void tnxar_append(TennixArchive *tnxar, char* filename, char* mem, uint32_t length);
void tnxar_build(TennixArchive *tnxar, char* filename);
#endif

#endif
