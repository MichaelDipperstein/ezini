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
static char *SkipWS(const char *str);
static char *DupStr(const char *src);
static char *GetLine(FILE *fp);

/***************************************************************************
*                                FUNCTIONS
***************************************************************************/

/***************************************************************************
*   Function   : ParseIni
*   Description: This function parses an INI file, searching for (section,
*                key, value) triples passes their values to a callback
*                function every time one is discovered.
*   Parameters : iniFile - A pointer to an opened file stream to be parsed
*                callback - A pointer to the callback function to be called
*                           when a (section, key, value) triple is
*                           discovered
*                userData - A pointer to data to be passed to the callback
*                           function
*   Effects    : The provided file is parsed for (section, key, value)
*                triples until EOF is encountered or an error occurs.
*                The file is left open either way.
*   Returned   : 0 for success, Non-zero on error.  Error type is contained
*                in errno.
***************************************************************************/
int ParseIni(FILE *iniFile, parse_cb callback, void *userData)
{
    char *line;
    char *ptr;
    ini_entry_t entry;
    int retval;

    if (NULL == iniFile)
    {
        errno = ENOENT;
        return -1;
    }

    if (NULL == callback)
    {
        errno = EINVAL;
        return -1;
    }

    line = NULL;
    entry.section = NULL;
    entry.key = NULL;
    entry.value = NULL;
    retval = 0;

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

        if (*ptr == '[')
        {
            /* possible new section */
            char *end;

            end = strchr(ptr, ']');

            if (NULL == end)
            {
                retval = -1;
                errno = EILSEQ;
                goto end;
            }

            /* we have the full string for a new section, trim white space */
            ptr = SkipWS(ptr + 1);

            while (isspace(*(end - 1)))
            {
                end--;
            }

            *end = '\0';

            free(entry.section);
            entry.section = DupStr(ptr);
            free(line);
            continue;
        }

        /* the only other allowable lines are of the form key = value */
        entry.key = ptr;
        ptr++;

        while (*ptr != ' ' && *ptr !='\t' && *ptr != '=')
        {
            if (*ptr == '\0')
            {
                retval = -1;
                errno = EILSEQ;
                goto end;
            }

            ptr++;
        }

        *ptr = '\0';
        ptr++;

        while (*ptr == ' ' || *ptr =='\t' || *ptr == '=')
        {
            if (*ptr == '\0')
            {
                retval = -1;
                errno = EILSEQ;
                goto end;
            }

            ptr++;
        }

        /* we found the start of value, trim trailing white space */
        entry.value = ptr;
        ptr = entry.value + strlen(entry.value) - 1;

        while (isspace(*ptr))
        {
            ptr--;
        }

        *(ptr + 1) = '\0';
        retval = callback(userData, entry);

        if (retval != 0)
        {
            /* callback failed or indicated that we should stop */
            goto end;
        }

        free(line);
    }


end:
    free(line);
    free(entry.section);
    return retval;
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

