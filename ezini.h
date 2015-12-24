/***************************************************************************
*                  INI File section, key, and value parser
*
*   File    : ezini.c
*   Purpose : This file provides type definitions and prototypes for a
*             library parses INI file for section, key, value triples.
*   Author  : Michael Dipperstein
*   Date    : November 22, 2015
*
****************************************************************************
*
* ezini: INI File section, key, and value parser
* Copyright (C) 2015 by Michael Dipperstein (mdipper@alumni.cs.ucsb.edu)
*
* This file is part of the ezini library.
*
* The ezini library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License as
* published by the Free Software Foundation; either version 3 of the
* License, or (at your option) any later version.
*
* The ezini library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser
* General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
***************************************************************************/
#ifndef EZINI_H
#define EZINI_H

/***************************************************************************
*                            TYPE DEFINITIONS
***************************************************************************/

/*section, key, and value of INI file entry */
typedef struct
{
    char *section;
    char *key;
    char *value;
} ini_entry_t;

/* format of the callback called when INI entry is read */
typedef int (*parse_cb)(void *userData, const ini_entry_t entry);

/***************************************************************************
*                               PROTOTYPES
***************************************************************************/
int ParseINI(const char *iniFile, parse_cb callback, void *userData);
int MakeINI(const char *iniFile, ini_entry_t *entries, const size_t count);

#endif  /* ndef EZINI_H */
