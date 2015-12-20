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
int MyCallback1(void *userData, const ini_entry_t entry);
int MyCallback2(void *userData, const ini_entry_t entry);

/***************************************************************************
*                                FUNCTIONS
***************************************************************************/

/***************************************************************************
*   Function   : main
*   Description: This function calls ParseINI to parse the test_strs.ini
*                file and uses MyCallback1 to fill a buffer with (section,
*                key, value) triples that are found.  The buffer is then
*                printed.  Then it calls ParseINI to parse the
*                test_struct.ini file and uses MyCallback2 to populate an
*                array of my_struct_t.  The contents of the populated struct
*                array are printed.
*   Parameters : argc - not used
*                argc - not used
*   Effects    : The (section, key, value) triples in test_strs.ini are
*                printed and the contents of a populated my_struct_t two
*                element array of my_struct_t are printed.
*   Returned   : 0
***************************************************************************/
int main(int argc, char *argv[])
{
    FILE *iniFile;
    char *buffer;
    my_struct_t my_structs[2];
    int i;

    ((void)(argc));
    ((void)(argv));
    buffer = NULL;


    /* process ini file with string data using callback 1 */
    iniFile = fopen("test_strs.ini", "r");

    if (NULL == iniFile)
    {
        perror("Error opening test_strs.ini");
    }

    if (0 != ParseIni(iniFile, &MyCallback1, (void *)&buffer))
    {
        printf("Error test_struct.ini file\n");
    }

    fclose(iniFile);
    printf("%s", buffer);
    free(buffer);

    /* process ini file with struct data using callback 2 */
    iniFile = fopen("test_struct.ini", "r");

    if (NULL == iniFile)
    {
        perror("Error opening test_struct.ini");
    }

    if (0 != ParseIni(iniFile, &MyCallback2, (void *)my_structs))
    {
        printf("Error parsing test_struct.ini file\n");
    }

    fclose(iniFile);
    printf("\n");

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
*   Function   : MyCallback1
*   Description: This is a callback function for ParseINI.  It stores all
*                of the section, key, value entries into one giant
*                dynamically allocated array of char pointed to by userData.
*                The function that called ParseINI is responsible for
*                freeing userData when it is done with it.
*   Parameters : userData - A pointer to the pointer to the array of char
*                           that will contain all of the section, key, value
*                           entries
*                entry - The section, key, value strings discovered by
*                        ParseINI
*   Effects    : The (section, key, value) strings are concatenated to the
*                dynamically allocated array pointed to by *userData.  The
*                array is grown to accommodate size of the strings, which
*                are space delimited and followed by a '\n'.
*   Returned   : 0 on success and -1 on failure.
***************************************************************************/
int MyCallback1(void *userData, const ini_entry_t entry)
{
    size_t len;
    char *buffer;

    buffer = *((char **)userData);
    len = strlen(entry.section) + strlen(entry.key) + strlen(entry.value) + 3;

    if (NULL == buffer)
    {
        buffer = malloc(len * sizeof(char) + 1);
    }
    else
    {
        len += strlen(*((char **)userData)) + 1;
        buffer = realloc(buffer, len * sizeof(char));
    }

    if (NULL == buffer)
    {
        *((char **)userData) = buffer;
        return -1;
    }

    strcat(buffer, entry.section);
    strcat(buffer, " ");
    strcat(buffer, entry.key);
    strcat(buffer, " ");
    strcat(buffer, entry.value);
    strcat(buffer, "\n");

    *((char **)userData) = buffer;
    return 0;
}

/***************************************************************************
*   Function   : MyCallback2
*   Description: This is a callback function for ParseINI.  It stores
*                section, key, value entries into an array of my_struct_t.
*                The section number is used as the array index.  key is the
*                name of struct the field that the value is to be stored in.
*                Values are converted to the correct type based on the type
*                of the field that they are stored into.
*   Parameters : userData - A pointer to the pointer to the array of
*                           my_struct_t that entries will be store into.
*                entry - The section, key, value strings discovered by
*                        ParseINI
*   Effects    : The (section, key, value) strings are used to fill the
*                my_struct_t type array passed as userData.
*   Returned   : 0 on success and -1 on failure.
***************************************************************************/
int MyCallback2(void *userData, const ini_entry_t entry)
{
    my_struct_t *ptr;

    /* use the XXX in struct_XXX section as index into userData */
    if (strncmp("struct_", entry.section, 7) == 0)
    {
        int offset;

        offset = atoi(entry.section + 7);

        if (offset < 1)
        {
            return -1;      /* invalid number in struct_XXX */
        }

        offset--;
        ptr = ((my_struct_t *)userData) + offset;
    }
    else
    {
        return -1;      /* unexpected section */
    }

    if (strcmp("int_field", entry.key) == 0)
    {
        ptr->myInt = atoi(entry.value);
    }
    else if (strcmp("float_field", entry.key) == 0)
    {
        ptr->myFloat = atof(entry.value);
    }
    else if (strcmp("str_field", entry.key) == 0)
    {
        strncpy(ptr->myString, entry.value, 10);
    }
    else
    {
        return -1;      /* unexpected key */
    }

    return 0;
}
