/* miniunz.c
   Version 1.2.0, September 16th, 2017
   sample part of the MiniZip project

   Copyright (C) 2012-2017 Nathan Moinvaziri
     https://github.com/nmoinvaz/minizip
   Copyright (C) 2009-2010 Mathias Svensson
     Modifications for Zip64 support
     http://result42.com
   Copyright (C) 2007-2008 Even Rouault
     Modifications of Unzip for Zip64
   Copyright (C) 1998-2010 Gilles Vollant
     http://www.winimage.com/zLibDll/minizip.html

   This program is distributed under the terms of the same license as zlib.
   See the accompanying LICENSE file for the full text of the license.
*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "zlib.h"
#include "ioapi.h"

#ifdef _WIN32
#  include <direct.h>
#  include <io.h>
#else
#  include <utime.h>
#endif

#include "unzip.h"

#ifdef _WIN32
#  define USEWIN32IOAPI
#  include "iowin32.h"
#endif

#include "miniunz.h"

#ifdef _WIN32
#  define MKDIR(d) _mkdir(d)
#  define CHDIR(d) _chdir(d)
#else
#  define MKDIR(d) mkdir(d, 0775)
#  define CHDIR(d) chdir(d)
#endif


#define DEBUG_PRINTF			0
#define debug_printf(...)		\
    do { if (DEBUG_PRINTF) fprintf(stderr, __VA_ARGS__); } while (0)


static char **zip_entries = NULL;
static int num_zip_entries = 0;
static int max_zip_entries = 0;
static const int max_alloc_zip_entries = 256;

static void add_zip_entry(const char *s)
{
    if (zip_entries == NULL)
    {
        max_zip_entries = max_alloc_zip_entries;

	zip_entries = malloc(max_zip_entries * sizeof(char *));

	if (zip_entries == NULL)
	    exit(1);

	num_zip_entries = 0;
    }

    if (num_zip_entries >= max_zip_entries - 1)
    {
        max_zip_entries += max_alloc_zip_entries;

	zip_entries = realloc(zip_entries, max_zip_entries * sizeof(char *));

	if (zip_entries == NULL)
	    exit(1);
    }

    zip_entries[num_zip_entries++] = strdup(s);
    zip_entries[num_zip_entries] = NULL;	// ensure NULL terminated list
}

static void free_zip_entries(void)
{
    if (zip_entries == NULL)
        return;

    int i;

    for (i = 0; i < num_zip_entries; i++)
        free(zip_entries[i]);

    num_zip_entries = 0;
    max_zip_entries = 0;

    free(zip_entries);

    zip_entries = NULL;
}

static int invalid_date(const struct tm *ptm)
{
#define datevalue_in_range(min, max, value) ((min) <= (value) && (value) <= (max))
    return (!datevalue_in_range(0, 207, ptm->tm_year) ||
            !datevalue_in_range(0, 11, ptm->tm_mon) ||
            !datevalue_in_range(1, 31, ptm->tm_mday) ||
            !datevalue_in_range(0, 23, ptm->tm_hour) ||
            !datevalue_in_range(0, 59, ptm->tm_min) ||
            !datevalue_in_range(0, 59, ptm->tm_sec));
#undef datevalue_in_range
}

// Conversion without validation
static void dosdate_to_raw_tm(uint64_t dos_date, struct tm *ptm)
{
    uint64_t date = (uint64_t)(dos_date >> 16);

    ptm->tm_mday = (uint16_t)(date & 0x1f);
    ptm->tm_mon = (uint16_t)(((date & 0x1E0) / 0x20) - 1);
    ptm->tm_year = (uint16_t)(((date & 0x0FE00) / 0x0200) + 80);
    ptm->tm_hour = (uint16_t)((dos_date & 0xF800) / 0x800);
    ptm->tm_min = (uint16_t)((dos_date & 0x7E0) / 0x20);
    ptm->tm_sec = (uint16_t)(2 * (dos_date & 0x1f));
    ptm->tm_isdst = -1;
}

static int dosdate_to_tm(uint64_t dos_date, struct tm *ptm)
{
    dosdate_to_raw_tm(dos_date, ptm);

    if (invalid_date(ptm))
    {
        // Invalid date stored, so don't return it.
        memset(ptm, 0, sizeof(struct tm));
        return -1;
    }
    return 0;
}

#ifndef _WIN32
static time_t dosdate_to_time_t(uint64_t dos_date)
{
    struct tm ptm;
    dosdate_to_raw_tm(dos_date, &ptm);
    return mktime(&ptm);
}
#endif

static void change_file_date(const char *path, uint32_t dos_date)
{
#ifdef _WIN32
    HANDLE handle = NULL;
    FILETIME ftm, ftm_local, ftm_create, ftm_access, ftm_modified;

    handle = CreateFileA(path, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (handle != INVALID_HANDLE_VALUE)
    {
        GetFileTime(handle, &ftm_create, &ftm_access, &ftm_modified);
        DosDateTimeToFileTime((WORD)(dos_date >> 16), (WORD)dos_date, &ftm_local);
        LocalFileTimeToFileTime(&ftm_local, &ftm);
        SetFileTime(handle, &ftm, &ftm_access, &ftm);
        CloseHandle(handle);
    }
#else
    struct utimbuf ut;
    ut.actime = ut.modtime = dosdate_to_time_t(dos_date);
    utime(path, &ut);
#endif
}

static int makedir(const char *newdir)
{
    char *buffer = NULL;
    char *p = NULL;
    int len = (int)strlen(newdir);

    if (len <= 0)
        return 0;

    buffer = (char*)malloc(len + 1);
    if (buffer == NULL)
    {
        debug_printf("Error allocating memory\n");
        return -1;
    }

    strcpy(buffer, newdir);

    if (buffer[len - 1] == '/')
        buffer[len - 1] = 0;

    if (MKDIR(buffer) == 0)
    {
        free(buffer);
        return 1;
    }

    p = buffer + 1;
    while (1)
    {
        char hold;
        while (*p && *p != '\\' && *p != '/')
            p++;
        hold = *p;
        *p = 0;

        if ((MKDIR(buffer) == -1) && (errno == ENOENT))
        {
            debug_printf("couldn't create directory %s (%d)\n", buffer, errno);
            free(buffer);
            return 0;
        }

        if (hold == 0)
            break;

        *p++ = hold;
    }

    free(buffer);
    return 1;
}

static FILE *get_file_handle(const char *path)
{
    FILE *handle = NULL;
#if defined(WIN32)
    wchar_t *pathWide = NULL;
    int pathLength = 0;

    pathLength = MultiByteToWideChar(CP_UTF8, 0, path, -1, NULL, 0) + 1;
    pathWide = (wchar_t*)calloc(pathLength, sizeof(wchar_t));
    MultiByteToWideChar(CP_UTF8, 0, path, -1, pathWide, pathLength);
    handle = _wfopen((const wchar_t*)pathWide, L"rb");
    free(pathWide);
#else
    handle = fopen64(path, "rb");
#endif

    return handle;
}

static int check_file_exists(const char *path)
{
    FILE *handle = get_file_handle(path);
    if (handle == NULL)
        return 0;
    fclose(handle);
    return 1;
}

static void display_zpos64(uint64_t n, int size_char)
{
    /* To avoid compatibility problem we do here the conversion */
    char number[21] = { 0 };
    int offset = 19;
    int pos_string = 19;
    int size_display_string = 19;

    while (1)
    {
        number[offset] = (char)((n % 10) + '0');
        if (number[offset] != '0')
            pos_string = offset;
        n /= 10;
        if (offset == 0)
            break;
        offset--;
    }

    size_display_string -= pos_string;
    while (size_char-- > size_display_string)
        debug_printf(" ");
    debug_printf("%s", &number[pos_string]);
}

static int miniunz_list(unzFile uf)
{
    free_zip_entries();

    int err = unzGoToFirstFile(uf);
    if (err != UNZ_OK)
    {
        debug_printf("error %d with zipfile in unzGoToFirstFile\n", err);
        return 1;
    }

    debug_printf("  Length  Method     Size Ratio   Date    Time   CRC-32     Name\n");
    debug_printf("  ------  ------     ---- -----   ----    ----   ------     ----\n");

    do
    {
        char filename_inzip[256] = {0};
        unz_file_info64 file_info = {0};
        uint32_t ratio = 0;
        struct tm tmu_date = { 0 };
        const char *string_method = NULL;
        char char_crypt = ' ';

        err = unzGetCurrentFileInfo64(uf, &file_info, filename_inzip, sizeof(filename_inzip), NULL, 0, NULL, 0);
        if (err != UNZ_OK)
        {
            debug_printf("error %d with zipfile in unzGetCurrentFileInfo\n", err);
            break;
        }

	add_zip_entry(filename_inzip);

        if (file_info.uncompressed_size > 0)
            ratio = (uint32_t)((file_info.compressed_size * 100) / file_info.uncompressed_size);

        /* Display a '*' if the file is encrypted */
        if ((file_info.flag & 1) != 0)
            char_crypt = '*';

        if (file_info.compression_method == 0)
            string_method = "Stored";
        else if (file_info.compression_method == Z_DEFLATED)
        {
            uint16_t level = (uint16_t)((file_info.flag & 0x6) / 2);
            if (level == 0)
                string_method = "Defl:N";
            else if (level == 1)
                string_method = "Defl:X";
            else if ((level == 2) || (level == 3))
                string_method = "Defl:F"; /* 2:fast , 3 : extra fast*/
            else
                string_method = "Unkn. ";
        }
        else if (file_info.compression_method == Z_BZIP2ED)
        {
            string_method = "BZip2 ";
        }
        else
            string_method = "Unkn. ";

        display_zpos64(file_info.uncompressed_size, 7);
        debug_printf("  %6s%c", string_method, char_crypt);
        display_zpos64(file_info.compressed_size, 7);

        dosdate_to_tm(file_info.dos_date, &tmu_date);
        debug_printf(" %3u%%  %2.2u-%2.2u-%2.2u  %2.2u:%2.2u  %8.8x   %s\n", ratio,
            (uint32_t)tmu_date.tm_mon + 1, (uint32_t)tmu_date.tm_mday,
            (uint32_t)tmu_date.tm_year % 100,
            (uint32_t)tmu_date.tm_hour, (uint32_t)tmu_date.tm_min,
            file_info.crc, filename_inzip);

        err = unzGoToNextFile(uf);
    }
    while (err == UNZ_OK);

    if (err != UNZ_END_OF_LIST_OF_FILE && err != UNZ_OK)
    {
        debug_printf("error %d with zipfile in unzGoToNextFile\n", err);
        return err;
    }

    return 0;
}

static int miniunz_extract_currentfile(unzFile uf, int opt_extract_without_path, int *popt_overwrite, const char *password)
{
    unz_file_info64 file_info = {0};
    FILE* fout = NULL;
    void* buf = NULL;
    uint16_t size_buf = 8192;
    int err = UNZ_OK;
    int errclose = UNZ_OK;
    int skip = 0;
    char filename_inzip[256] = {0};
    char *filename_withoutpath = NULL;
    const char *write_filename = NULL;
    char *p = NULL;

    err = unzGetCurrentFileInfo64(uf, &file_info, filename_inzip, sizeof(filename_inzip), NULL, 0, NULL, 0);
    if (err != UNZ_OK)
    {
        debug_printf("error %d with zipfile in unzGetCurrentFileInfo\n",err);
        return err;
    }

    add_zip_entry(filename_inzip);

    p = filename_withoutpath = filename_inzip;
    while (*p != 0)
    {
        if ((*p == '/') || (*p == '\\'))
            filename_withoutpath = p+1;
        p++;
    }

    /* If zip entry is a directory then create it on disk */
    if (*filename_withoutpath == 0)
    {
        if (opt_extract_without_path == 0)
        {
            debug_printf("creating directory: %s\n", filename_inzip);
            MKDIR(filename_inzip);
        }
        return err;
    }

    buf = (void*)malloc(size_buf);
    if (buf == NULL)
    {
        debug_printf("Error allocating memory\n");
        return UNZ_INTERNALERROR;
    }

    err = unzOpenCurrentFilePassword(uf, password);
    if (err != UNZ_OK)
        debug_printf("error %d with zipfile in unzOpenCurrentFilePassword\n", err);

    if (opt_extract_without_path)
        write_filename = filename_withoutpath;
    else
        write_filename = filename_inzip;

    /* Determine if the file should be overwritten or not and ask the user if needed */
    if ((err == UNZ_OK) && (*popt_overwrite == 0) && (check_file_exists(write_filename)))
    {
        char rep = 0;
        do
        {
            char answer[128];
            debug_printf("The file %s exists. Overwrite ? [y]es, [n]o, [A]ll: ", write_filename);
            if (scanf("%1s", answer) != 1)
                exit(EXIT_FAILURE);
            rep = answer[0];
            if ((rep >= 'a') && (rep <= 'z'))
                rep -= 0x20;
        }
        while ((rep != 'Y') && (rep != 'N') && (rep != 'A'));

        if (rep == 'N')
            skip = 1;
        if (rep == 'A')
            *popt_overwrite = 1;
    }

    /* Create the file on disk so we can unzip to it */
    if ((skip == 0) && (err == UNZ_OK))
    {
        fout = fopen64(write_filename, "wb");
        /* Some zips don't contain directory alone before file */
        if ((fout == NULL) && (opt_extract_without_path == 0) &&
            (filename_withoutpath != (char*)filename_inzip))
        {
            char c = *(filename_withoutpath-1);
            *(filename_withoutpath-1) = 0;
            makedir(write_filename);
            *(filename_withoutpath-1) = c;
            fout = fopen64(write_filename, "wb");
        }
        if (fout == NULL)
            debug_printf("error opening %s\n", write_filename);
    }

    /* Read from the zip, unzip to buffer, and write to disk */
    if (fout != NULL)
    {
        debug_printf(" extracting: %s\n", write_filename);

        do
        {
            err = unzReadCurrentFile(uf, buf, size_buf);
            if (err < 0)
            {
                debug_printf("error %d with zipfile in unzReadCurrentFile\n", err);
                break;
            }
            if (err == 0)
                break;
            if (fwrite(buf, err, 1, fout) != 1)
            {
                debug_printf("error %d in writing extracted file\n", errno);
                err = UNZ_ERRNO;
                break;
            }
        }
        while (err > 0);

        if (fout)
            fclose(fout);

        /* Set the time of the file that has been unzipped */
        if (err == 0)
            change_file_date(write_filename, file_info.dos_date);
    }

    errclose = unzCloseCurrentFile(uf);
    if (errclose != UNZ_OK)
        debug_printf("error %d with zipfile in unzCloseCurrentFile\n", errclose);

    free(buf);
    return err;
}

static int miniunz_extract_all(unzFile uf, int opt_extract_without_path, int opt_overwrite, const char *password)
{
    free_zip_entries();

    int err = unzGoToFirstFile(uf);
    if (err != UNZ_OK)
    {
        debug_printf("error %d with zipfile in unzGoToFirstFile\n", err);
        return 1;
    }

    do
    {
        err = miniunz_extract_currentfile(uf, opt_extract_without_path, &opt_overwrite, password);
        if (err != UNZ_OK)
            break;
        err = unzGoToNextFile(uf);
    }
    while (err == UNZ_OK);

    if (err != UNZ_END_OF_LIST_OF_FILE)
    {
        debug_printf("error %d with zipfile in unzGoToNextFile\n", err);
        return 1;
    }
    return 0;
}

char **zip_list(char *filename)
{
    if (filename == NULL)
    {
        debug_printf("No filename specified!\n");

	return NULL;
    }

#ifdef USEWIN32IOAPI
    zlib_filefunc64_def ffunc;
    fill_win32_filefunc64A(&ffunc);
    unzFile uf = unzOpen2_64(filename, &ffunc);
#else
    unzFile uf = unzOpen64(filename);
#endif

    if (uf == NULL)
    {
        debug_printf("Cannot open file '%s'!\n", filename);

	return NULL;
    }

    debug_printf("File '%s' opened.\n", filename);

    int success = (miniunz_list(uf) == UNZ_OK);

    unzClose(uf);

    if (!success)
        return NULL;

    return zip_entries;
}

char **zip_extract(char *filename, char *directory)
{
    if (filename == NULL)
    {
        debug_printf("No zip filename specified!\n");

	return NULL;
    }

    if (directory == NULL)
    {
        debug_printf("No target directory specified!\n");

	return NULL;
    }

    struct stat file_status;

    if (stat(directory, &file_status) != 0 || !S_ISDIR(file_status.st_mode))
    {
        debug_printf("Directory '%s' does not exist!\n", directory);

	return NULL;
    }

#ifdef USEWIN32IOAPI
    zlib_filefunc64_def ffunc;
    fill_win32_filefunc64A(&ffunc);
    unzFile uf = unzOpen2_64(filename, &ffunc);
#else
    unzFile uf = unzOpen64(filename);
#endif

    if (uf == NULL)
    {
        debug_printf("Cannot open file '%s'!\n", filename);

	return NULL;
    }

    debug_printf("File '%s' opened.\n", filename);

    int max_directory_size = 1024;
    char last_directory[max_directory_size];

    if (getcwd(last_directory, max_directory_size) == NULL)
    {
        debug_printf("Cannot get current directory!\n");

	unzClose(uf);

	return NULL;
    }

    if (CHDIR(directory) != 0)		// change to target directory
    {
        debug_printf("Cannot change to directory '%s'!\n", directory);

        unzClose(uf);

        return NULL;
    }

    int success = (miniunz_extract_all(uf, 0, 1, NULL) == UNZ_OK);

    if (CHDIR(last_directory) != 0)	// change back to previous directory
    {
        debug_printf("Cannot change to directory '%s'!\n", last_directory);

        unzClose(uf);

        return NULL;
    }

    unzClose(uf);

    if (!success)
        return NULL;

    return zip_entries;
}
