/**
 * \brief INI File handling library header
 * \file ezini.h
 * \author Michael Dipperstein (mdipper@alumni.cs.ucsb.edu)
 * \date November 22, 2015
 *
 * This file implements a set of library functions that maybe be used
 * to create, update, and/or parse INI files.
 *
 * \copyright Copyright (C) 2015 by Michael Dipperstein
 * (mdipper@alumni.cs.ucsb.edu)
 *
 * \par
 * This file is part of the ezini library.
 *
 * \license The ezini library is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * \par
 * The ezini library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser
 * General Public License for more details.
 *
 * \par
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#ifndef EZINI_H
#define EZINI_H

/**
 * \mainpage EZIni - INI File handling library
 *
 * These pages provide documentation for EZIni, an INI File handling library.
 *
 * \copyright Copyright (C) 2015 by Michael Dipperstein
 * (mdipper@alumni.cs.ucsb.edu)
 *
 * \license The ezini library is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * \par
 * The ezini library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser
 * General Public License for more details.
 *
 * \par
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/***************************************************************************
*                            TYPE DEFINITIONS
***************************************************************************/

/**
 * \struct ini_entry_t
 * \brief A structure containing the section, key, and value of INI.
 * file entry
 */
typedef struct
{
    char *section;      /*!< pointer to a NULL terminated string with the
                            section name */
    char *key;          /*!< pointer to a a NULL terminated string with entry
                            key name */
    char *value;        /*!< pointer to a NULL terminated string with entry value.
                            Use ASCII strings to represent numbers */
} ini_entry_t;

/**
 * \struct ini_entry_list_t
 * \brief A structure for sorted lists of entries.
 */
typedef struct ini_entry_list_t
{
    ini_entry_t entry;              /*!< An INI entry structure
                                        (not a pointer) */
    struct ini_entry_list_t *next;  /*!< A pointer to the next entry in
                                        the list */
} ini_entry_list_t;

/**
 * \typedef struct ini_entry_list_t
 * \brief A shortcut for struct ini_entry_list_t
 */

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
