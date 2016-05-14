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

static int PopulateEntry(ini_entry_t *entry, const char *section,
    const char *key, const char *value);

/***************************************************************************
*                                FUNCTIONS
***************************************************************************/

/***************************************************************************
*   Function   : AddEntryToList
*   Description: This function adds a (section, key, value) entry to an
*                entry list.  The entry will be inserted alphebetcially by
*                section name then key.  If an entry containing the same
*                section name and key already exists, the new value will
*                overwrite the old value.
*   Parameters : list - A pointer to an ini_entry_list_t pointer that
*                       points to the head of an entry list.  Pass a pointer
*                       to an ini_entry_list_t pointing to NULL if the
*                       list needs to be created.
*                section - A NULL terminated string containing the name of
*                          the section for the entry.
*                key - A NULL terminated string containing the name of the
*                      key for the entry.
*                value - A NULL terminated string containing the value of
*                       the key for the entry.  All values must be
*                       represented as strings.  They may be converted
*                       to/from strings by the calling program
*   Effects    : An entry structure containing copies of the (section, key,
*                value) entry is added to the list passed as a parameter.
*                memory will be dynamically allocated as needed.
*   Returned   : 0 for success.  -1 for an error.  Error type is contained
*                in errno.
***************************************************************************/
int AddEntryToList(ini_entry_list_t **list, const char *section,
    const char *key, const char *value)
{
    ini_entry_list_t *here;
    ini_entry_list_t *prev;
    ini_entry_list_t *tmp;
    ini_entry_t entry;
    int result;

    /* handle empty list */
    if (NULL == *list)
    {
        *list = (ini_entry_list_t *)malloc(sizeof(ini_entry_list_t));

        if (NULL == *list)
        {
            return -1;
        }

        /* copy triple into entry */
        if (0 != PopulateEntry(&((*list)->entry), section, key, value))
        {
            return -1;
        }

        (*list)->next = NULL;

        return 0;
    }

    /* make entry to be inserted.  it's easier to work with. */
    if (0 != PopulateEntry(&entry, section, key, value))
    {
        return -1;
    }


    /* find where to insert entry into non-empty list */
    here = *list;
    prev = NULL;
    result = 0;

    while (NULL != here)
    {
        result = CompairEntry(&entry, &(here->entry));

        if (result <= 0)
        {
            break;
        }

        prev = here;
        here = here->next;
    }

    /* we found where we need to insert the entry */
    if (0 == result)
    {
        /* section and key match, just replace the value */
        free(here->entry.value);
        here->entry.value = DupStr(value);
        return 0;
    }

   tmp = (ini_entry_list_t *)malloc(sizeof(ini_entry_list_t));

    if (NULL == tmp)
    {
        FreeEntry(&entry);
        return -1;
    }

    tmp->entry.section = entry.section;
    tmp->entry.key = entry.key;
    tmp->entry.value = entry.value;
    tmp->next = here;

    if (NULL == prev)
    {
        /* insert at the head of the list */
        *list = tmp;
    }
    else
    {
        prev->next = tmp;
    }

    return 0;
}

/***************************************************************************
*   Function   : FreeEntryList
*   Description: This function steps head to tail through an entry list,
*                freeing the memembers of each entry, then the entry
*                itself.
*   Parameters : list - A pointer to an ini_entry_list_t pointer that
*                       points to the head of an entry list.
*   Effects    : All of the memory allocated for all of the entries in
*                an entry list will be freeded.
*   Returned   : NONE
***************************************************************************/
void FreeEntryList(ini_entry_list_t **list)
{
    ini_entry_list_t *here;
    ini_entry_list_t *next;

    if (NULL == *list)
    {
        return;
    }

    here = *list;

    /* delete list head to tail */
    do
    {
        next = here->next;
        FreeEntry(&(here->entry));
        free(here);
        here = next;
    } while (here != NULL);
}

/***************************************************************************
*   Function   : MakeINIFile
*   Description: This function creates the specified INI file from the list
*                of entries passed as an argument.  Any existing INI file
*                with the same name in the same path will be overwritten.
*   Parameters : iniFile - The name of the INI file to be created
*                list - A pointer to a sorted list of enteries.
*   Effects    : The specified file is created and the (section, key, value)
*                triples in the entry list are written to the file.  If
*                the specified file already exists, it will be overwritten.
*                If the entry list is not sorted, multiple sections with
*                the same name may be created.
*   Returned   : 0 for success, Non-zero on error.  Error type is contained
*                in errno.
***************************************************************************/
int MakeINIFile(const char *iniFile, const ini_entry_list_t *list)
{
    char *section;
    FILE *fp;

    if (NULL == iniFile)
    {
        errno = EINVAL;
        return -1;
    }

    if (NULL == list)
    {
        errno = EINVAL;
        return -1;
    }

    fp = fopen(iniFile, "w");

    if (NULL == fp)
    {
        return -1;
    }

    section = list->entry.section;
    fprintf(fp, "[%s]\n", section);

    while (NULL != list)
    {
        if (0 != strcmp(section, list->entry.section))
        {
            section = list->entry.section;
            fprintf(fp, "\n[%s]\n", section);
        }

        fprintf(fp, "%s = %s\n", list->entry.key, list->entry.value);

        list = list->next;
    }

    fclose(fp);
    return 0;
}
/***************************************************************************
*   Function   : AddEntryToFile
*   Description: This function adds (section, key, value) entries in an
*                entry list to an INI file.  The entries will be inserted
*                alphebetcially by section name then key.  If an entry
*                containing the same section name and key already exists,
*                the new value will overwrite the old value.
*   Parameters : iniFile - The name of the INI file to be modified.
*                list - A pointer to list of sorted list of enteries to
*                       be added to the INI File.
*   Effects    : The INI file will be re-written containg the rusults of
*                adding the enteries in the entry list to the entries
*                already contained in the INI file.
*   Returned   : 0 for success.  -1 for an error.  Error type is contained
*                in errno.
***************************************************************************/
int AddEntryToFile(const char *iniFile, const ini_entry_list_t *list)
{
    ini_entry_t entry;
    ini_entry_list_t *merged;
    const ini_entry_list_t *here;
    int result;
    FILE *fp;

    if (NULL == iniFile)
    {
        errno = EINVAL;
        return -1;
    }

    if (NULL == list)
    {
        errno = EINVAL;
        return -1;
    }

    fp = fopen(iniFile, "r");

    if (NULL == fp)
    {
        return -1;
    }

    merged = NULL;
    entry.section = NULL;
    entry.key = NULL;
    entry.value = NULL;

    /* read ini file back into an entry list */
    while ((result = GetEntryFromFile(fp, &entry)) > 0)
    {
        AddEntryToList(&merged, entry.section, entry.key, entry.value);
    }

    fclose(fp);

    if (result < 0)
    {
        FreeEntryList(&merged);
        return -1;
    }

    /* add entries passed into this function to entries from INI file */
    here = list;

    while (NULL != here)
    {
        result = AddEntryToList(&merged, here->entry.section, here->entry.key,
            here->entry.value);

        if (result != 0)
        {
            FreeEntryList(&merged);
            return -1;
        }

        here = here->next;
    }

    /* re-write INI file from merged entry list */
    result = MakeINIFile(iniFile, merged);
    FreeEntryList(&merged);

    return result;
}

/***************************************************************************
*   Function   : DeleteEntryFromFile
*   Description: This function deletes all entries from an INI file that
*                match the section and key passed as an argument.
*                NOTE: There will never be more than one matching entry in
*                      INI files created by this library.
*   Parameters : iniFile - The name of the INI file containing the entry
*                          to be deleted.
*                section - A pointer to a null terminated string containing
*                         the name of the section of the entry to be
*                         deleted.
*                key - A pointer to a null terminated string containing the
*                      name of the key of the entry to be deleted.
*   Effects    : The INI file will be re-written without any entries that
*                match the section and key to be deleted.
*   Returned   : 0 for success.  -1 for an error.  Error type is contained
*                in errno.
***************************************************************************/
int DeleteEntryFromFile(const char *iniFile, const char *section,
    const char *key)
{
    ini_entry_t entry;
    ini_entry_list_t *list;
    int result;
    FILE *fp;

    if (NULL == iniFile)
    {
        errno = EINVAL;
        return -1;
    }

    if (NULL == section)
    {
        errno = EINVAL;
        return -1;
    }

    if (NULL == key)
    {
        errno = EINVAL;
        return -1;
    }

    fp = fopen(iniFile, "r");

    if (NULL == fp)
    {
        return -1;
    }

    list = NULL;
    entry.section = NULL;
    entry.key = NULL;
    entry.value = NULL;

    /* read ini file back into a structure */
    while ((result = GetEntryFromFile(fp, &entry)) > 0)
    {
        if (0 != strcmp(entry.section, section))
        {
            /* this isn't one we're supposed to delete */
            AddEntryToList(&list, entry.section, entry.key, entry.value);
        }
        else if (0 != strcmp(entry.key, key))
        {
            /* this isn't one we're supposed to delete */
            AddEntryToList(&list, entry.section, entry.key, entry.value);
        }
    }

    fclose(fp);

    if (result < 0)
    {
        FreeEntryList(&list);
        return -1;
    }

    result = MakeINIFile(iniFile, list);
    FreeEntryList(&list);

    return result;
}

/***************************************************************************
*   Function   : GetEntryFromFile
*   Description: This function parses an INI file stream passed as an input,
*                searching for the next (section, key, value) triple.  The
*                resulting triple will be used to populate the entry
*                structure passed as a parameter.
*   Parameters : iniFile - A pointer to the INI file to be parsed.  It must
*                          be opened for reading.
*                entry - A pointer to the entry structure used to store the
*                        discovered (section, key, value) triple.
*   Effects    : The specified file is read until it discovers an entry
*   Returned   : 1 when an entry is found
*                0 when no more entries can be found
*                -1 for an error.  Error type is contained in errno.
***************************************************************************/
int GetEntryFromFile(FILE *iniFile, ini_entry_t *entry)
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

    entry->section = NULL;
    entry->key = NULL;
    entry->value = NULL;
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
*   Parameters : p1 - A pointer to the first entry
*                p2 - A pointer to the second entry
*   Effects    : None
*   Returned   : An integer less than, equal to, or greater than zero if
*                p1 is found, respectively, to be less than, to match, or be
*                greater than p2.
*
*                NOTE: Entries are compared by section, if the sections are
*                equal, their keys are compared.  Their values are never
*                compared.
***************************************************************************/
static int CompairEntry(const void *p1, const void *p2)
{
    char *s1;
    char *s2;
    int result;

    s1 = ((ini_entry_t *)p1)->section;
    s2 = ((ini_entry_t *)p2)->section;
    result = strcmp(s1, s2);

    if (0 != result)
    {
        return result;
    }

    s1 = ((ini_entry_t *)p1)->key;
    s2 = ((ini_entry_t *)p2)->key;
    result = strcmp(s1, s2);
    return result;
}

/***************************************************************************
*   Function   : PopulateEntry
*   Description: This function compares the entries pointed to by the void
*                pointers p1 and p2 and returns an integer less than, equal
*                to, or greater than zero if p1 is less than, equal to or
*                greater than p2.
*   Parameters : p1 - A pointer to the first entry
*                p2 - A pointer to the second entry
*   Effects    : None
*   Returned   : An integer less than, equal to, or greater than zero if
*                p1 is found, respectively, to be less than, to match, or be
*                greater than p2.
***************************************************************************/
static int PopulateEntry(ini_entry_t *entry, const char *section,
    const char *key, const char *value)
{
    if (NULL == entry)
    {
        return -1;
    }

    entry->section = DupStr(section);

    if (NULL == entry->section)
    {
        return -1;
    }

    entry->key = DupStr(key);

    if (NULL == entry->key)
    {
        free(entry->section);
        return -1;
    }

    entry->value = DupStr(value);

    if (NULL == entry->value)
    {
        free(entry->section);
        free(entry->key);
        return -1;
    }

    return 0;
}
