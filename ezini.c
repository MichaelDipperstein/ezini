/**
 * \brief INI File handling library
 * \file ezini.c
 * \author Michael Dipperstein (mdipperstein@gmail.com)
 * \date November 22, 2015
 *
 * This file implements a set of library functions that maybe be used
 * to create, update, and/or parse INI files.
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
 * \defgroup library Library Code
 * \brief This module contains the code for the ezini INI file handling
 * library
 * @{
 */

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
*                            TYPE DEFINITIONS
***************************************************************************/

/**
 * \struct ini_key_list_t
 * \brief A structure used for creating linked lists of key/value pairs
 * for a each section.
 */

/**
 * \typedef struct ini_key_list_t
 * \brief A shortcut for struct ini_key_list_t
 */

typedef struct ini_key_list_t
{
    char *key;                  /*!< pointer to a a NULL terminated string
                                    containing key name for this entry */
    char *value;                /*!< pointer to a NULL terminated string
                                    containing key value for this entry Use
                                    ASCII strings to represent numbers */
    struct ini_key_list_t *next;/*!< pointer to the next key/value pair in
                                    in this section */

} ini_key_list_t;


/**
 * \struct ini_section_list_t
 * \brief A structure used for creating linked lists of sections, each
 *  maintaining its own list of key/value pairs.
 */

/**
 * \typedef struct ini_section_list_t
 * \brief A shortcut for struct ini_section_list_t
 */

typedef struct ini_section_list_t
{
    char *section;                      /*!< pointer to a NULL terminated string
                                            containing the section name */
    ini_key_list_t *members;            /*!< pointer to the list of all key/value
                                            pairs in this section */
    struct ini_section_list_t *next;    /*!< pointer to the next section in
                                            the list of entries */

} ini_section_list_t;


/***************************************************************************
*                               PROTOTYPES
***************************************************************************/

/* allocate */
static ini_key_list_t *NewKeyList(const char *key, const char *value);
static ini_section_list_t *NewSectionList(const char *section, const char *key,
    const char *value);

/* free */
static void FreeKeyList(ini_key_list_t *list);
static void FreeEntry(ini_entry_t *entry);

/* utilities */
static char *SkipWS(const char *str);
static char *DupStr(const char *src);
static char *GetLine(FILE *fp);

/***************************************************************************
*                                FUNCTIONS
***************************************************************************/

/**
 * \fn int AddEntryToList(ini_entry_list_t *list, const char *section,
 * const char *key, const char *value)
 *
 * \brief This function adds a (section, key, value) entry to an entry list.
 *
 * \param list A pointer to an ini_entry_list_t that points to the
 * head of entry list being modified.  Pass a pointer to an ini_entry_list_t
 * pointing to NULL if the list needs to be created.
 *
 * \param section A NULL terminated string containing the name of the
 * section for the entry being added.
 *
 * \param key A NULL terminated string containing the name of the key for
 * the entry being added.
 *
 * \param value A NULL terminated string containing the value of the key for
 * the entry being added.  All values must be represented as strings.  They may
 * be converted to/from strings by the calling program.
 *
 * \effects
 * Information used to generate an entry structure containing copies of
 * the (section, key, value) entry is added to the list passed as a parameter.
 * Memory will be dynamically allocated as needed.
 *
 * \returns 0 for success, Non-zero on error.  Error type is contained in
 * errno.
 *
 * This function adds information used to create a (section, key, value) entry
 * to an entry list.
 *
 * If an entry containing the same section name and key already exists,
 * the new value will overwrite the old value.
 *
 * If the entry is for an existing section, it will be added to the end of the
 * list for that section.
 *
 * If the entry is for a new section, a new section will be added to the list of
 * sections, and the key/value pair will be the first entry of the section.
 */
int AddEntryToList(ini_entry_list_t *list, const char *section,
    const char *key, const char *value)
{
    ini_section_list_t *next;
    int result;

    /* handle empty list */
    if (NULL == *list)
    {
        /* add the first entry to the list */
        *list = NewSectionList(section, key, value);

        if (NULL == *list)
        {
            return -1;
        }

        return 0;
    }

    next = *list;
    result = 1;

    while (1)
    {
        result = strcmp(section, next->section);

        if (0 == result)
        {
            break;      /* match, insert here */
        }

        if (NULL == next->next)
        {
            break;      /* no match, create new section here */
        }

        next = next->next;
    }

    if (0 == result)
    {
        ini_key_list_t *member;

        member = next->members;

        while (1)
        {
            result = strcmp(key, member->key);

            if (0 == result)
            {
                break;      /* match, insert here */
            }

            if (NULL == member->next)
            {
                break;      /* no match, create new section here */
            }

            member = member->next;
        }

        if (0 == result)
        {
            /* key exists, change value */
            free(member->value);
            member->value = DupStr(value);

            if (NULL == member->value)
            {
                return -1;
            }
        }
        else
        {
            /* new key, add to list */
            member->next = NewKeyList(key, value);

            if (NULL == member->next)
            {
                return -1;
            }
        }
    }
    else
    {
        /* add the section to the list with this key and value */
        next->next = NewSectionList(section, key, value);
    }

    return 0;
}


/**
 * \fn void FreeList(ini_entry_list_t list)
 *
 * \brief This function frees all of the members of an entry list.
 *
 * \param list A pointer to the head of an ini_entry_list_t
 *
 * \effects
 * All of the memory allocated for all of the entries in an entry
 * list will be freed.
 *
 * \returns Nothing
 *
 * This function uses recursion to step to the tail of the list and deletes
 * section entries on the way back up.
 */
void FreeList(ini_entry_list_t list)
{
    /* recurse to the end of the list and free everything on the way back */
    if (list->next != NULL)
    {
        FreeList(list->next);
    }

    if (list->section != NULL)
    {
        /* free the section name */
        free(list->section);
    }

    if (list->members != NULL)
    {
        FreeKeyList(list->members);
    }

    free(list);
}


/**
 * \fn int MakeINIFile(const char *iniFile, const ini_entry_list_t list)
 *
 * \brief This function creates the specified INI file from the list of
 * entries passed as an argument.
 *
 * \param iniFile The name of the INI file to be created.  stdout will be
 * used if iniFile is NULL.
 *
 * \param list A pointer to a list of that will be used to construct
 * (section, key, value) entries.
 *
 * \effects
 * The specified file is created and the (section, key, value)
 * triples generated from the entry list are written to the file.  If the
 * specified file already exists, it will be overwritten.
 *
 * \returns 0 for success, Non-zero on error.  Error type is contained in
 * errno.
 *
 * This function creates the specified INI file from the list of entries
 * passed as an argument.  Any existing INI file with the same name in the
 * same path will be overwritten.
 */
int MakeINIFile(const char *iniFile, const ini_entry_list_t list)
{
    ini_entry_list_t section;
    ini_key_list_t *members;
    FILE *fp;

    if (NULL == list)
    {
        errno = EINVAL;
        return -1;
    }

    if (NULL == iniFile)
    {
        fp = stdout;
    }
    else
    {

        fp = fopen(iniFile, "w");

        if (NULL == fp)
        {
            return -1;
        }
    }

    section = list;

    while (section != NULL)
    {
        fprintf(fp, "[%s]\n", section->section);

        members = section->members;

        while (members != NULL)
        {
            fprintf(fp, "%s = %s\n", members->key, members->value);
            members = members->next;
        }

        fprintf(fp, "\n");
        section = section->next;
    }

    if (fp != stdout)
    {
        fclose(fp);
    }

    return 0;
}


/**
 * \fn int AddEntryToFile(const char *iniFile, const ini_entry_list_t list)
 *
 * \brief This function adds (section, key, value) entries in an entry list
 * to an INI file.
 *
 * \param iniFile The name of the INI file to be modified.
 *
 * \param list A pointer to a list of entries to be added to the INI file.
 *
 * \effects
 * The INI file will be re-written containing the results of adding
 * the entries in the entry list to the entries already contained in the INI
 * file.
 *
 * \returns 0 for success, Non-zero on error.  Error type is contained in
 * errno.
 *
 * This function adds (section, key, value) entries in an entry list to an
 * INI file.  Section order will be maintained with new sections added to the
 * end of the INI file.  If an entry containing the same section name and key
 * already exists, the new value will overwrite the old value.
 */
int AddEntryToFile(const char *iniFile, const ini_entry_list_t list)
{
    ini_entry_t entry;
    ini_entry_list_t merged;
    ini_entry_list_t here;
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
        FreeList(merged);
        return -1;
    }

    /* add entries passed into this function to entries from INI file */
    here = list;

    while (here != NULL)
    {
        ini_key_list_t *members;

        members = here->members;

        while (members != NULL)
        {
            AddEntryToList(&merged, here->section, members->key,
                members->value);
            members = members->next;
        }

        here = here->next;
    }

    /* re-write INI file from merged entry list */
    result = MakeINIFile(iniFile, merged);
    FreeList(merged);

    return result;
}

/**
 * \fn int DeleteEntryFromFile(const char *iniFile, const char *section,
 * const char *key)
 *
 * \brief This function deletes all entries from an INI file that match the
 * section and key passed as an argument.
 *
 * \param iniFile The name of the INI file containing the entry to be
 * deleted.
 *
 * \param section A pointer to a NULL terminated string containing the name
 * of the section of the entry to be deleted.
 *
 * \param key A pointer to a NULL terminated string containing the name of
 * the key of the entry to be deleted.
 *
 * \effects
 * The INI file will be re-written without any entries that match
 * the section and key to be deleted.  Empty sections will not be deleted.
 *
 * \returns 0 for success, Non-zero on error.  Error type is contained in
 * errno.
 *
 * This function deletes all entries from an INI file that match the section
 * and key passed as an argument.  Empty sections will not be deleted.
 *
 * \note There will never be more than one matching entry in INI files
 * created by this library.
 */
int DeleteEntryFromFile(const char *iniFile, const char *section,
    const char *key)
{
    ini_entry_t entry;
    ini_entry_list_t list;
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
        FreeList(list);
        return -1;
    }

    result = MakeINIFile(iniFile, list);
    FreeList(list);

    return result;
}

/**
 * \fn int GetEntryFromFile(FILE *iniFile, ini_entry_t *entry)
 *
 * \brief This function parses an INI file stream passed as an input,
 * searching for the next (section, key, value) triple.
 *
 * \param iniFile A pointer to the INI file to be parsed.  It must be
 * opened for reading.
 *
 * \param entry A pointer to the entry structure used to store the discovered
 * (section, key, value) triple.
 *
 * \effects
 * The specified file is read until it discovers an entry.
 *
 * \returns 1 when an entry is found\n
 *          0 when no more entries can be found\n
 *         -1 for an error.  Error type is contained in errno.
 *
 * This function parses an INI file stream passed as an input, searching for
 * the next (section, key, value) triple.  The resulting triple will be used
 * to populate the entry structure passed as a parameter.
 */
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
    free(entry->key);       /* free old key */
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
    free(entry->value);     /* free old value */
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


/**
 * \fn ini_key_list_t *NewKeyList(const char *key, const char *value)
 *
 * \brief This function allocates memory for a new ini_key_list_t type
 * variable and populates it with the data passed as parameters
 *
 * \param key A pointer to a NULL terminated string containing the key name
 *
 * \param key A pointer to a NULL terminated string containing the value of
 * this key.  Use ASCII strings to represent numbers.
 *
 * \effects
 * Memory will be allocated for a new ini_key_list_t and copies
 * of the key and value strings passed as a parameter.  key and value are
 * copied into the appropriate fields and the next pointer is set to NULL.
 *
 * \returns A pointer to the ini_key_list_t item that was allocated.  The
 * pointer will be NULL if an error occurs.
 *
 * This function allocates memory for a new ini_key_list_t and copies
 * of the key and value strings passed as a parameter.  The next pointer
 * will be set to NULL.
 */
static ini_key_list_t *NewKeyList(const char *key, const char *value)
{
    ini_key_list_t *item;

    item = (ini_key_list_t *)malloc(sizeof(ini_key_list_t));

    if (NULL == item)
    {
        return NULL;
    }

    /* allocation succeeded copy key and value */
    item->next = NULL;

    item->key = DupStr(key);

    if (NULL == item->key)
    {
        free(item);
        return NULL;
    }

    item->value = DupStr(value);

    if (NULL == item->value)
    {
        free(item->key);
        free(item);
        return NULL;
    }

    return item;
}


/**
 * \fn ini_section_list_t *NewSectionList(const char *section, const char *key,
 *      const char *value)
 *
 * \brief This function allocates memory for a new ini_section_list_t type
 * variable and populates it with the data passed as parameters
 *
 * \param section A pointer to a NULL terminated string containing the section
 * name

 * \param key A pointer to a NULL terminated string containing the key name
 *
 * \param key A pointer to a NULL terminated string containing the value of
 * this key.  Use ASCII strings to represent numbers.
 *
 * \effects
 * Memory will be allocated for a new ini_section_list_t and copies
 * of the section, key, value strings passed as a parameter.  section is
 * copied to the appropriate field and a new key list items is created fo
 * the key and value strings.  The next pointer will be set to NULL.
 *
 * \returns A pointer to the ini_section_list_t item that was allocated.  The
 * pointer will be NULL if an error occurs.
 *
 * This function allocates memory for a new ini_section_list_t and copies
 * of the section, key, value strings passed as a parameter.  A ini_key_list_t
 * is allocated for the key and value strings.  The next pointer is set to NULL.
 */
static ini_section_list_t *NewSectionList(const char *section, const char *key,
    const char *value)
{
    ini_section_list_t *item;

    item = (ini_section_list_t *)malloc(sizeof(ini_section_list_t));

    if (NULL == item)
    {
        return NULL;
    }

    /* now populate item */
    item->next = NULL;
    item->section = DupStr(section);

    if (NULL == item->section)
    {
        free(item);
        return NULL;
    }

    /* start a member list with the current key and value */
    item->members = NewKeyList(key, value);

    if (NULL == item->members)
    {
        free(item->section);
        free(item);
        return NULL;
    }

    return item;
}


/**
 * \fn void FreeKeyList(ini_key_list_t *list)
 *
 * \brief This function frees all of the members of a key list.
 *
 * \param list A pointer to the head of an ini_key_list_t
 *
 * \effects All of the memory allocated for all of the key/value pairs in
 * a key list will be freed.
 *
 * \returns Nothing
 *
 * This function uses recursion to step to the tail of the list and deletes
 * key/value entries on the way back up.
 */
static void FreeKeyList(ini_key_list_t *list)
{
    /* recurse to the end of the list and free everything on the way back */
    if (list->next != NULL)
    {
        FreeKeyList(list->next);
    }

    free(list->key);
    free(list->value);
    free(list);
}


/**
 * \fn static void FreeEntry(ini_entry_t *entry)
 *
 * \brief This function frees the allocated memory that is pointed to by the
 * members of an ini_entry_t structure.
 *
 * \param entry A pointer to the entry structure containing the members
 * pointing to the memory to be freed.
 *
 * \effects Dynamically allocated memory is freed and entry data is set
 * to NULL.
 *
 * \returns Nothing
 */
static void FreeEntry(ini_entry_t *entry)
{
    free(entry->section);
    free(entry->key);
    free(entry->value);

    entry->section = NULL;
    entry->key = NULL;
    entry->value = NULL;
}

/**
 * \fn static char *SkipWS(const char *str)
 *
 * \brief This function returns a pointer to the first non-space in the
 * string passed as a parameter.
 *
 * \param str A pointer to the string being searched.
 *
 * \effects None
 *
 * \returns A pointer to the first non-space in str.
 */
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

/**
 * \fn static char *DupStr(const char *src)
 *
 * \brief This function returns a copy of the string passed as a parameter.
 *
 * \param str A pointer to the string being being copied.
 *
 * \effects Memory is dynamically allocated to hold a duplicate of the
 * input string.
 *
 * \returns A copy of str in malloced memory is returned on success.  NULL
 * is returned on failure.
 *
 * This function returns a copy of the string passed as a parameter.  The
 * memory for the copy is allocated by malloc() and must be freed by the caller.
 */
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

/**
 * \fn static char *GetLine(FILE *fp)
 *
 * \brief This function returns a NULL terminated array of char containing the
 * next line in the file passed as an argument.
 *
 * \param fp A pointer to the file being read.
 *
 * \effects One line is read from fp and copied into a dynamically allocated
 * string.
 *
 * \returns A NULL terminated array of char containing the next line in fp
 * is retured.  The array must be free by the caller. NULL is returned at end
 * of file.
 *
 * This function returns a NULL terminated array of char containing the next
 * line in the file passed as an argument.  The memory for the string returned
 * is allocated by malloc() and must be freed by the caller.
 */
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

/**@}*/
