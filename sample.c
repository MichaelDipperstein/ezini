/***************************************************************************
*                 Sample Program using ezini INI File parser
*
*   File    : sample.c
*   Purpose : To demonstrate the usage of the ezini INI file parser
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
typedef struct my_struct_t
{
    int     myInt;
    float   myFloat;
    char    myString[10];
} my_struct_t;

/***************************************************************************
*                               PROTOTYPES
***************************************************************************/
static int PopulateMyStruct(my_struct_t *my_struct, const ini_entry_t *entry);

/***************************************************************************
*                                FUNCTIONS
***************************************************************************/

/***************************************************************************
*   Function   : main
*   Description: This function calls GetEntry to read the test_strs.ini
*                file and prints the entries as they are discovered.  Then
*                it creates test_struct.ini and calls GetEntry to read it.
*                PopulateMyStruct is called to load the entry values into
*                an array of my_struct_t.  The contents of the populated
*                struct array are printed.
*   Parameters : argc - not used
*                argc - not used
*   Effects    : The (section, key, value) triples in test_strs.ini are
*                printed.  test_struct.ini is created and it's enteries are
*                used to populate my_struct_t, a two element array of
*                my_struct_t.  Its contents are printed.
*   Returned   : 0
***************************************************************************/
int main(int argc, char *argv[])
{
    my_struct_t my_structs[2];
    int i;
    int result;
    FILE *fp;

    ini_entry_t entries[6];
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
    while ((result = GetEntry(fp, &entry)) > 0)
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

    /* build list of entries for MakeINI */
    entries[0].section = "struct 1";
    entries[0].key = "int field";
    entries[0].value = "123";

    entries[1].section = "struct 2";
    entries[1].key = "str field";
    entries[1].value = "string2";

    entries[2].section = "struct 1";
    entries[2].key = "float field";
    entries[2].value = "456.789";

    entries[3].section = "struct 2";
    entries[3].key = "float field";
    entries[3].value = "987.654";

    entries[4].section = "struct 1";
    entries[4].key = "str field";
    entries[4].value = "string1";

    entries[5].section = "struct 2";
    entries[5].key = "int field";
    entries[5].value = "321";

    printf("\nWriting test_struct.ini\n");
    printf("=======================\n");

    /* now create the ini */
    fp = fopen("test_struct.ini", "w");

    if (0 != MakeINI(fp, entries, 6))
    {
        printf("Error making test_struct.ini file\n");
    }

    fclose(fp);


    printf("\nReading test_struct.ini\n");
    printf("=======================\n");
    fp = fopen("test_struct.ini", "r");

    /* initialize entry structure before reading first entry */
    entry.section = NULL;
    entry.key = NULL;
    entry.value = NULL;

    /* read ini file back into a structure */
    while ((result = GetEntry(fp, &entry)) > 0)
    {
        PopulateMyStruct(my_structs, &entry);
    }

    fclose(fp);

    if (result < 0)
    {
        printf("Error getting entry from test_struct.ini\n");
    }

    for (i = 0; i < 2; i++)
    {
        printf("struct %d\n", (i + 1));
        printf("\tmyInt %d\n", my_structs[i].myInt);
        printf("\tmyFloat %f\n", my_structs[i].myFloat);
        printf("\tmyString %s\n", my_structs[i].myString);
    }

    return 0;
}


/***************************************************************************
*   Function   : PopulateMyStruct
*   Description: This function stores section, key, value entries into an
*                array of my_struct_t.  The section number is used as the
*                array index.  key is the name of struct the field that the
*                value is to be stored in.  Values are converted to the
*                correct type based on the type of the field that they are
*                stored into.
*   Parameters : my_struct - A pointer to the array of my_struct_t that
*                            entries will be stored into.
*                entry - The section, key, value strings discovered by
*                        GetEntry.
*   Effects    : The (section, key, value) strings are used to fill the
*                my_struct_t type array passed as my_struct.
*   Returned   : 0 on success and -1 on failure.
***************************************************************************/
static int PopulateMyStruct(my_struct_t *my_struct, const ini_entry_t *entry)
{
    my_struct_t *ptr;

    /* use the XXX in struct XXX section as index into my_struct */
    if (strncmp("struct ", entry->section, 7) == 0)
    {
        int offset;

        offset = atoi(entry->section + 7);

        if (offset < 1)
        {
            return -1;      /* invalid number in struct_XXX */
        }

        offset--;
        ptr = my_struct + offset;
    }
    else
    {
        return -1;      /* unexpected section */
    }

    if (strcmp("int field", entry->key) == 0)
    {
        ptr->myInt = atoi(entry->value);
    }
    else if (strcmp("float field", entry->key) == 0)
    {
        ptr->myFloat = atof(entry->value);
    }
    else if (strcmp("str field", entry->key) == 0)
    {
        strncpy(ptr->myString, entry->value, 10);
    }
    else
    {
        return -1;      /* unexpected key */
    }

    return 0;
}
