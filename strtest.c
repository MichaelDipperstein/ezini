/**
 * \brief A Test Program using the ezini INI file handling library
 * \file strtest.c
 * \author Michael Dipperstein (mdipperstein@gmail.com)
 * \date November 22, 2015
 *
 * This file test the INI parser on a variety of (section, key, value)
 * strings.
 *
 * \copyright Copyright (C) 2015, 2019 by Michael Dipperstein
 * (mdipperstein@gmail.com)
 *
 * \par
 * This file is part of the ezini library.
 *
 * \license
 * The ezini library is free software; you can redistribute it
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

/**
 * \defgroup tests INI File Parsing Tests
 * \brief This module contains code testing the the ezini INI file handling
 * library parser on a variety of (section, key, value) strings.
 * @{
 */

/***************************************************************************
*                             INCLUDED FILES
***************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ezini.h"

/*!
  \def strtest_main
  \brief Hack so that Doxygen doesn't confuse this with sample.c's main
*/
#define strtest_main main

/***************************************************************************
*                            TYPE DEFINITIONS
***************************************************************************/

/***************************************************************************
*                               PROTOTYPES
***************************************************************************/

/***************************************************************************
*                                FUNCTIONS
***************************************************************************/

/**
 * \fn int strtest_main(int argc, char *argv[])
 *
 * \brief This function calls GetEntryFromFile() to read the test_strs.ini
 * file and prints the entries as they are discovered.
 *
 * \param argc Not Used
 *
 * \param argv Not Used
 *
 * \effects
 * The (section, key, value) triples in test_strs.ini are printed.
 *
 * \returns 0 (regardless of results)
 */
int strtest_main(int argc, char *argv[])
{
    int result;
    FILE *fp;
    ini_entry_t entry;

    ((void)(argc));
    ((void)(argv));

    printf("Reading test_strs.ini\n");
    printf("=====================\n");
    fp = fopen("test_strs.ini", "r");

    /* initialize entry structure before reading first entry */
    entry.section = NULL;
    entry.key = NULL;
    entry.value = NULL;

    /* read one entry at a time */
    while ((result = GetEntryFromFile(fp, &entry)) > 0)
    {
        printf("%s\n", entry.section);
        printf("\t%s\n", entry.key);
        printf("\t%s\n", entry.value);
    }

    fclose(fp);

    if (result < 0)
    {
        printf("Error getting entry from test_strs.ini\n");
    }

    return 0;
}

/**@}*/
