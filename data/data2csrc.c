/***
 * data2csrc.c -- Read data and output c source to stdout
 *
 * Copyright (c) 2007 Thomas Perl <thpinfo.com/about>
 * Website: http://thpinfo.com/
 *
 * License: This code is in the public domain.
 ***/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main( int argc, char** argv) {
    FILE *fp;
    int c, x = 0, i;
    char filename[FILENAME_MAX], varname[FILENAME_MAX];

    if( argc != 3 && argc != 2) {
        fprintf( stderr, "Usage: %s /path/to/infile.dat [name_of_variable]\n", argv[0]);
        exit(1);
    }

    strcpy( filename, argv[1]);

    if( argc == 2) {
        /* Determine variable name based on filename */
        strcpy( varname, argv[1]);
        for( i=0; i<strlen( varname); i++) {
            if( varname[i] == '.') varname[i] = '\0';
            if( varname[i] == '-') varname[i] = '_';
        }
    } else {
        strcpy( varname, argv[2]);
    }

    fp = fopen( filename, "rb");
    if( fp == NULL) { fprintf( stderr, "Cannot open file: %s\n", argv[1]); exit(1); }
    printf( "/**\n * Automatically generated from \"%s\" by %s.\n **/\n", filename, argv[0]);
    printf( "const char %s[] = {", varname);
    while( (c = fgetc( fp)) != EOF) {
        if( x > 0) putchar(',');
        if( x++ % 20 == 0) putchar('\n');
        printf( "%d", c);
    }
    printf( "\n};\n\n");
    fclose( fp);
    return 0;
}

