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

/* section, key, and value of INI file entry */
typedef struct
{
    char *section;
    char *key;
    char *value;
} ini_entry_t;

/* structure for sorted list of entries */
typedef struct ini_entry_list_t
{
    ini_entry_t entry;
    struct ini_entry_list_t *next;
} ini_entry_list_t;

/***************************************************************************
*                               PROTOTYPES
***************************************************************************/

/* add entry to a sorted entry list */
int AddEntryToList(ini_entry_list_t **list, const char *section,
    const char *key, const char *value);

/* frees all of the entries in an entry list */
void FreeEntryList(ini_entry_list_t **list);

/* create/add entries to an INI file from a sorted entry list */
int MakeINIFile(const char *iniFile, const ini_entry_list_t *list);
int AddEntryToFile(const char *iniFile, const ini_entry_list_t *list);

/* remove a single entry from an INI file */
int DeleteEntryFromFile(const char *iniFile, const char *section,
    const char *key);

/***************************************************************************
* get the next entry in INI file.
* returns:  1 if an entry is found
*           0 if there are no more entries
*           -1 on error
***************************************************************************/
int GetEntryFromFile(FILE *iniFile, ini_entry_t *entry);

#endif  /* ndef EZINI_H */
