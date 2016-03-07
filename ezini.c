/***************************************************************************
*                  INI File section, key, and value parser
*
*   File    : ezini.c
*   Purpose : This file implements a library function that parses an INI
*             file for section, key, value triples and calling a callback
*             function as triples are discovered.
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

/***************************************************************************
*                             INCLUDED FILES
***************************************************************************/
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "ezini.h"

/***************************************************************************
*                               PROTOTYPES
***************************************************************************/
static void FreeEntry(ini_entry_t *entry);

static char *SkipWS(const char *str);
static char *DupStr(const char *src);
static char *GetLine(FILE *fp);
static int CompairEntry(const void *p1, const void *p2);

/***************************************************************************
*                                FUNCTIONS
***************************************************************************/

/***************************************************************************
*   Function   : MakeINI
*   Description: This function creates the specified INI file from the list
*                of entries passed as an argument.  It will start from
*                wherever the file is open to.
*   Parameters : iniFile - A pointer to the INI file to be made.  The file
*                          must be opened for writing.
*                entries - A pointer to an array of ini_entry_t type
*                          elements that will be used to create the INI file
*                count - The number of elements in the array pointed to
*                        by entries
*   Effects    : The specified file is created and the (section, key, value)
*                triples in the entries array are written to the file.  It
*                will start writing from the current location in the file.
*   Returned   : 0 for success, Non-zero on error.  Error type is contained
*                in errno.
***************************************************************************/
int MakeINI(FILE *iniFile, ini_entry_t *entries, const size_t count)
{
    char *section;
    size_t i;

    if (NULL == iniFile)
    {
        errno = EINVAL;
        return -1;
    }

    if (NULL == entries)
    {
        errno = EINVAL;
        return -1;
    }

    /* sort entries by section */
    qsort((void *)entries, count, sizeof(ini_entry_t), CompairEntry);

    section = entries[0].section;
    fprintf(iniFile, "[%s]\n", section);

    for (i = 0; i < count; i++)
    {
        if (0 != strcmp(section, entries[i].section))
        {
            section = entries[i].section;
            fprintf(iniFile, "\n[%s]\n", section);
        }

        fprintf(iniFile, "%s = %s\n", entries[i].key, entries[i].value);
    }

    return 0;
}

/***************************************************************************
*   Function   : GetEntry
*   Description: This function parses an INI file stream passed as an input,
*                searching for the next (section, key, value) triple.  The
*                resulting triple will be used to populate the entry structure
*                passed as a parameter.
*   Parameters : iniFile - A pointer to the INI file to be parsed.  It must
*                          be opened for reading.
*                entry - A pointer to the entry structure used to store the
*                        discovered (section, key, value) triple.
*   Effects    : The specified file is read until it discovers a 
*   Returned   : 1 when an entry is found
*                0 when no more entries can be found
*                -1 for an error.  Error type is contained in errno.
***************************************************************************/
int GetEntry(FILE *iniFile, ini_entry_t *entry)
{
    char *line;
    char *ptr;

    if (NULL == iniFile)
    {
        errno = EINVAL;
        return -1;
    }

    if (NULL == entry)
    {
        errno = EINVAL;
        return -1;
    }

    /* handle section names, comments, and blank lines */
    while ((line = GetLine(iniFile)) != NULL)
    {
        /* skip leading spaces and blank lines */
        ptr = SkipWS(line);

        /* skip blank lines and lines starting with ';' or '#' */
        if (*ptr == '\0' || *ptr == ';' || *ptr == '#')
        {
            free(line);
            continue;
        }
        else if (*ptr == '[')
        {
            /* possible new section */
            char *end;

            end = strchr(ptr, ']');

            if (NULL == end)
            {
                FreeEntry(entry);
                errno = EILSEQ;
                return -1;
            }

            /* we have the full string for a new section, trim white space */
            ptr = SkipWS(ptr + 1);

            while (isspace(*(end - 1)))
            {
                end--;
            }

            *end = '\0';

            free(entry->section);
            entry->section = DupStr(ptr);
            free(line);
        }
        else
        {
            /* this line should be key = value */
            break;
        }
    }

    /* we either have a non-section line or nothing left to get */
    if (NULL == line)
    {
        /* nothing left to get */
        FreeEntry(entry);
        return 0;
    }

    /* the only other allowable lines are of the form key = value */
    entry->key = ptr;
    ptr++;

    while (*ptr != '=')
    {
        if (*ptr == '\0')
        {
            /* didn't find '=' */
            entry->key = NULL;
            FreeEntry(entry);
            free(line);
            errno = EILSEQ;
            return -1;
        }

        ptr++;
    }

    /* we found the '=' separating key and value trim white space */
    entry->value = ptr + 1;
    ptr--;
    
    while (isspace(*ptr))
    {
        ptr--;
    }

    *(ptr + 1) = '\0';
    entry->key = DupStr(entry->key);

    ptr = entry->value;

    /* now skip white space after '=' */
    while (*ptr == ' ' || *ptr =='\t')
    {
        if (*ptr == '\0')
        {
            FreeEntry(entry);
            free(line);
            errno = EILSEQ;
            return -1;
        }

        ptr++;
    }

    /* we found the start of value, trim trailing white space */
    entry->value = ptr;
    ptr = entry->value + strlen(entry->value) - 1;

    while (isspace(*ptr))
    {
        ptr--;
    }

    *(ptr + 1) = '\0';
    entry->value = DupStr(entry->value);

    free(line);
    return 1;
}

/***************************************************************************
*   Function   : FreeEntry
*   Description: This function frees the allocated memory that is pointed to
*                by the members of an ini_entry_t structure.
*   Parameters : entry - pointer to the entry structure containing the
*                        members pointing to the memory to be freed.
*   Effects    : Dynamically allocated memory is freed.
*   Returned   : None
***************************************************************************/
static void FreeEntry(ini_entry_t *entry)
{
    free(entry->section);
    free(entry->key);
    free(entry->value);
}


/***************************************************************************
*   Function   : SkipWS
*   Description: This function returns a pointer to the first non-space
*                in the string passed as a parameter.
*   Parameters : str - string being searched
*   Effects    : None
*   Returned   : A pointer to the first non-space in str.
***************************************************************************/
static char *SkipWS(const char *str)
{
    char *c;

    c = (char *)str;
    while(isspace(*c))
    {
        c++;
    }

    return c;
}

/***************************************************************************
*   Function   : DupStr
*   Description: This function returns a copy of the string passed as a
*                parameter.  The memory for the copy is allocated by
*                malloc and must be freed by the caller.
*   Parameters : str - string being copied
*   Effects    : None
*   Returned   : A copy of str in malloced memory is returned on success.
*                NULL is returned on failure.
***************************************************************************/
static char *DupStr(const char *src)
{
    char *dest;

    if (NULL == src)
    {
        return NULL;
    }

    dest = (char *)malloc(strlen(src) + 1);

    if (NULL != dest)
    {
        strcpy(dest, src);
    }

    return dest;
}

/***************************************************************************
*   Function   : GetLine
*   Description: This function returns a NULL terminated array of char
*                containing the next line in the file passed as an argument.
*                The memory for the string returned is allocated by malloc
*                and must be freed by the caller.
*   Parameters : fp - pointer to the file being read
*   Effects    : One line is read from fp and copied into a dynamically
*                allocated string.
*   Returned   : A NULL terminated array of char containing the next line
*                in fp is retured.  The array must be free by the caller.
*                NULL is returned at end of file.
***************************************************************************/
static char *GetLine(FILE *fp)
{
    char *line;         /* string to read line into */
    char *next;         /* where to write the next characters into */
    const size_t chunkSize = 32;
    size_t lineSize;

    if ((NULL == fp) || feof(fp))
    {
        return NULL;
    }

    lineSize = chunkSize;
    line = (char *)malloc(lineSize * sizeof(char));

    if (NULL == line)
    {
        /* allocation failed */
        return NULL;
    }

    line[0] = '\0';
    next = line;

    while (NULL != fgets(next, lineSize - strlen(line), fp))
    {
        if ('\n' == line[strlen(line) - 1])
        {
            /* we got to the EOL strip off the trailing '\n' and exit */
            line[strlen(line) - 1] = '\0';
            break;
        }
        else
        {
            /* there's still more on this line */
            lineSize += chunkSize;
            line = (char *)realloc(line, lineSize);

            if (NULL == line)
            {
                return NULL;
            }

            next = line + strlen(line);
        }
    }

    return line;
}

/***************************************************************************
*   Function   : CompairEntry
*   Description: This function compares the entries pointed to by the void
*                pointers p1 and p2 and returns an integer less than, equal
*                to, or greater than zero if p1 is less than, equal to or
*                greater than p2.
*   Parameters : str - string being copied
*   Effects    : None
*   Returned   : An integer less than, equal to, or greater than zero if
*                p1.section is found, respectively, to be less than, to
*                match, or be greater than p2.section.
***************************************************************************/
static int CompairEntry(const void *p1, const void *p2)
{
    char *s1;
    char *s2;

    s1 = ((ini_entry_t *)p1)->section;
    s2 = ((ini_entry_t *)p2)->section;

    return strcmp(s1, s2);
}
