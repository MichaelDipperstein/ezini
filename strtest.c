/***************************************************************************
*                  Test Program using ezini INI File parser
*
*   File    : strtest.c
*   Purpose : To test the INI parser on a variety of (section, key, value)
*             strings
*   Author  : Michael Dipperstein
*   Date    : November 22, 2015
*
****************************************************************************
*
* sample: Sample usage of the ezini INI File parser
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

/***************************************************************************
*                             INCLUDED FILES
***************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ezini.h"

/***************************************************************************
*                            TYPE DEFINITIONS
***************************************************************************/

/***************************************************************************
*                               PROTOTYPES
***************************************************************************/

/***************************************************************************
*                                FUNCTIONS
***************************************************************************/

/***************************************************************************
*   Function   : main
*   Description: This function calls GetEntryFromFile to read the
*                test_strs.ini file and prints the entries as they are
*                discovered.
*   Parameters : argc - not used
*                argc - not used
*   Effects    : The (section, key, value) triples in test_strs.ini are
*                printed.
*   Returned   : 0
***************************************************************************/
int main(int argc, char *argv[])
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
