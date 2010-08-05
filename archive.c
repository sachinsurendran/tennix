
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


#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <arpa/inet.h>

#include "archive.h"

#ifdef TENNIXAR_STANDALONE
int main(int argc, char* argv[])
{
    TennixArchive *tnxar;
    char *data;
    FILE *fp;
    char *filename;
    char *bn = (char*)(argv[0]);
    int len, i;
    struct stat st;

    if(strcmp(bn, "archive") == 0) {
        if (argc < 2) {
            fprintf(stderr, "Usage: %s archive.tnx file1 [....]\n", bn);
            exit(EXIT_FAILURE);
        } else if (argc == 2) {
            fprintf(stderr, "Refusing to create an empty archive.\n");
            exit(EXIT_FAILURE);
        }

        if (stat(argv[1], &st) != -1) {
            fprintf(stderr, "File %s already exists. Aborting.\n", argv[1]);
            exit(EXIT_FAILURE);
        }
        tnxar = tnxar_create();
        
        fprintf(stderr, "Creating %s with %d files\n", argv[1], argc-2);
        for (i=2; i<argc; i++) {
            fp = fopen(argv[i], "rb");
            fseek(fp, 0, SEEK_END);
            len = ftell(fp);
            fseek(fp, 0, SEEK_SET);
            data = (char*)malloc(len);
            fread(data, len, 1, fp);
            fclose(fp);
            tnxar_append(tnxar, (char*)(argv[i]), data, len);
        }
        tnxar_build(tnxar, argv[1]);
    } else if(strcmp(bn, "dump") == 0) {
        if (argc < 2) {
            fprintf(stderr, "Usage: %s archive.tnx\n", bn);
            exit(EXIT_FAILURE);
        }
        tnxar = tnxar_open(argv[1]);
        assert(tnxar != NULL);
        tnxar_dump(tnxar);
        tnxar_close(tnxar);
    } else if(strcmp(bn, "extract") == 0) {
        if (argc < 2 || argc > 3) {
            fprintf(stderr, "Usage: %s archive.tnx [file]\n", bn);
            exit(EXIT_FAILURE);
        }
        tnxar = tnxar_open(argv[1]);
        assert(tnxar != NULL);
        if (argc == 2) {
            while (!tnxar_eof(tnxar)) {
                filename = tnxar_get_current_filename(tnxar);
                fprintf(stderr, "Extracting: %s", filename);
                data = tnxar_read_current(tnxar);
                len = tnxar_size_current(tnxar);
                fprintf(stderr, " (%d bytes)", len);
                fp = fopen(filename, "wb");
                fputc('.', stderr);
                fwrite(data, len, 1, fp);
                fputc('.', stderr);
                fclose(fp);
                fprintf(stderr, ".OK\n");
                free(data);
                tnxar_next(tnxar);
            }
        } else if (argc == 3) {
            filename = argv[2];
            if (tnxar_set_current_filename(tnxar, filename) != 0) {
                fprintf(stderr, "Extracting: %s", filename);
                data = tnxar_read_current(tnxar);
                len = tnxar_size_current(tnxar);
                fprintf(stderr, " (%d bytes)", len);
                fp = fopen(filename, "wb");
                fputc('.', stderr);
                fwrite(data, len, 1, fp);
                fputc('.', stderr);
                fclose(fp);
                fprintf(stderr, ".OK\n");
                free(data);
            } else {
                fprintf(stderr, "File not found in %s: %s\n", argv[1], filename);
                tnxar_close(tnxar);
                exit(EXIT_FAILURE);
            }
        }
        tnxar_close(tnxar);
    }

    return EXIT_SUCCESS;
}
#endif

TennixArchive* tnxar_open(char* filename)
{
    int i;

    TennixArchive *tnxar = (TennixArchive*)malloc(sizeof(TennixArchive));
    assert(tnxar != NULL);

    tnxar->fp = fopen(filename, "rb");
    if (tnxar->fp == NULL) {
        free(tnxar);
        return NULL;
    }

    tnxar->offset = sizeof(TennixArchiveHeader)*fread(&(tnxar->header), sizeof(TennixArchiveHeader), 1, tnxar->fp);
    assert(tnxar->offset == sizeof(TennixArchiveHeader));
    assert(strcmp(tnxar->header.header, TENNIX_ARCHIVE_HEADER) == 0);
    assert(tnxar->header.versionmajor == TENNIX_ARCHIVE_VERSIONMAJOR);
    assert(tnxar->header.versionminor == TENNIX_ARCHIVE_VERSIONMINOR);

    tnxar->items = (TennixArchiveItem*)calloc(tnxar->header.items, sizeof(TennixArchiveItem));
    assert(tnxar->items != NULL);
    tnxar->offset += sizeof(TennixArchiveItem)*fread(tnxar->items, sizeof(TennixArchiveItem), tnxar->header.items, tnxar->fp);
    assert(tnxar->offset == sizeof(TennixArchiveHeader) + tnxar->header.items*sizeof(TennixArchiveItem));

    tnxar_xormem((char*)(tnxar->items), tnxar->header.items*sizeof(TennixArchiveItem), tnxar->header.key);

    for (i=0; i<tnxar->header.items; i++) {
        /* convert offset + length from network byte order */
        tnxar->items[i].offset = ntohl(tnxar->items[i].offset);
        tnxar->items[i].length = ntohl(tnxar->items[i].length);
    }

    tnxar->current_item = 0;

    return tnxar;
}


#ifdef TENNIXAR_STANDALONE
void tnxar_dump(TennixArchive* tnxar)
{
    fprintf(stderr, "Tennix Archive\n");
    fprintf(stderr, "Header: %s\n", tnxar->header.header);
    fprintf(stderr, "Version: %d.%d\n", tnxar->header.versionmajor, tnxar->header.versionminor);
    fprintf(stderr, "Master key: %d\n", tnxar->header.key);
    fprintf(stderr, "Items: %d\n", tnxar->header.items);
    for (tnxar->current_item = 0; tnxar->current_item < tnxar->header.items; tnxar->current_item++) {
        fprintf(stderr, "===========\n");
        fprintf(stderr, "File: %s (#%d)\n", tnxar->items[tnxar->current_item].filename, tnxar->current_item);
        fprintf(stderr, "Size: %d\n", tnxar->items[tnxar->current_item].length);
        fprintf(stderr, "Offset: %d\n", tnxar->items[tnxar->current_item].offset);
        fprintf(stderr, "Key: %d\n", tnxar->items[tnxar->current_item].key);
    }
}
#endif

int tnxar_set_current_filename(TennixArchive* tnxar, char* filename)
{
    int i;

    for (i=0; i<tnxar->header.items; i++) {
        if (strcmp(tnxar->items[i].filename, filename) == 0) {
            tnxar->current_item = i;
            return 1;
        }
    }

    return 0;
}

char* tnxar_get_current_filename(TennixArchive* tnxar)
{
    return tnxar->items[tnxar->current_item].filename;
}

char* tnxar_read_current(TennixArchive* tnxar)
{
    int size = tnxar_size_current(tnxar);
    char* data = (char*)malloc(size);
    fseek(tnxar->fp, tnxar->items[tnxar->current_item].offset, SEEK_SET);
    assert(fread(data, size, 1, tnxar->fp) == 1);
    tnxar_xormem(data, size, tnxar->items[tnxar->current_item].key);
    return data;
}

size_t tnxar_size_current(TennixArchive* tnxar)
{
    return tnxar->items[tnxar->current_item].length;
}

int tnxar_eof(TennixArchive* tnxar)
{
    return tnxar->current_item >= tnxar->header.items;
}

void tnxar_next(TennixArchive* tnxar)
{
    tnxar->current_item++;
}

void tnxar_close(TennixArchive *tnxar)
{
    assert(tnxar != NULL);

    fclose(tnxar->fp);
    tnxar->fp = NULL;

    free(tnxar->items);
    tnxar->items = NULL;

    free(tnxar);
}

void tnxar_xormem(char* mem, uint32_t length, char key)
{
   char *i = mem, *end = mem+length;

    for(; i != end; i++) {
        *i ^= key;
    }
}

#ifdef TENNIXAR_STANDALONE
TennixArchive* tnxar_create()
{
    TennixArchive *tnxar = (TennixArchive*)calloc(sizeof(TennixArchive), 1);
    assert(tnxar != NULL);

    strcpy(tnxar->header.header, TENNIX_ARCHIVE_HEADER);
    tnxar->header.items = 0;
}

void tnxar_append(TennixArchive* tnxar, char* filename, char* mem, uint32_t length)
{
    int i;
    TennixArchiveItem *item;

    assert(tnxar != NULL);

    tnxar->header.items++;
    tnxar->items = (TennixArchiveItem*)realloc(tnxar->items, sizeof(TennixArchiveItem)*tnxar->header.items);
    tnxar->blobs = (char**)realloc(tnxar->blobs, sizeof(char*)*tnxar->header.items);

    item = &(tnxar->items[tnxar->header.items-1]);
    tnxar->blobs[tnxar->header.items-1] = mem;
    for (i=0; i<TENNIX_ARCHIVE_ITEM_MAXNAME; i++) {
        item->filename[i] = mem[(i*2)%length];
    }
    strcpy(item->filename, filename);
    item->length = length;
}

void tnxar_build(TennixArchive *tnxar, char* filename)
{
    int i;
    size_t offset = 0;
    size_t *memsize = NULL;
    assert(tnxar != NULL);

    memsize = (size_t*)calloc(tnxar->header.items, sizeof(size_t));

    tnxar->fp = fopen(filename, "wb");

    offset += sizeof(TennixArchiveHeader) + tnxar->header.items*sizeof(TennixArchiveItem);

    tnxar->header.versionmajor = TENNIX_ARCHIVE_VERSIONMAJOR;
    tnxar->header.versionminor = TENNIX_ARCHIVE_VERSIONMINOR;

    tnxar->header.key = (0xaa + 0x77*tnxar->header.items*3) % 0xff;

    fprintf(stderr, "Packing: ");
    for (i=0; i<tnxar->header.items; i++) {
        fprintf(stderr, "%s", tnxar->items[i].filename);
        tnxar->items[i].offset = htonl(offset); /* network byte order */
        tnxar->items[i].key = 0xaa ^ ((i<<2)%0x100);
        tnxar_xormem(tnxar->blobs[i], tnxar->items[i].length, tnxar->items[i].key);
        memsize[i] = tnxar->items[i].length;
        offset += tnxar->items[i].length;
        tnxar->items[i].length = htonl(tnxar->items[i].length); /* network byte order */
        tnxar_xormem((char*)(tnxar->items + i), sizeof(TennixArchiveItem), tnxar->header.key);
        if (i != tnxar->header.items-1) {
            fprintf(stderr, ", ");
        }
    }
    fputc('\n', stderr);

    fprintf(stderr, "Writing: %s", filename);
    fputc('.', stderr);
    fwrite(&(tnxar->header), sizeof(TennixArchiveHeader), 1, tnxar->fp);
    fputc('.', stderr);
    fwrite(tnxar->items, sizeof(TennixArchiveItem), tnxar->header.items, tnxar->fp);
    fputc('.', stderr);
    for (i=0; i<tnxar->header.items; i++) {
        fwrite(tnxar->blobs[i], memsize[i], 1, tnxar->fp);
        free(tnxar->blobs[i]);
    }
    fputc('.', stderr);
    fprintf(stderr, "OK\n");

    free(memsize);
    free(tnxar->blobs);
    fclose(tnxar->fp);
}
#endif

