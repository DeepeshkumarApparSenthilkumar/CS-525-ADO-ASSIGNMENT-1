
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

#include "storage_mgr.h"
#include "dberror.h"

/* ---------------------------------------------------------------------------
   return codes
   ---------------------------------------------------------------------------*/
#ifndef RC_ERROR
#define RC_ERROR 500   
#endif

#ifndef RC_READ_AT_BEGINNING
#define RC_READ_AT_BEGINNING 501   /* This code runs to to read before first page initially */
#endif

/* ---------------------------------------------------------------------------
    Running this Global state
   ---------------------------------------------------------------------------*/

/* Keeping one active file pointer globally . */
static FILE *g_active_fp = NULL;

/* Initializing the first file variable . Global file pointers for managing the currently open file. // Initialising file pointer i.e. storage manager */
FILE *fdk = NULL;

/* ---------------------------------------------------------------------------
   Internal functionalities
   ---------------------------------------------------------------------------*/

/* Allocate one empty page (all zeros). This Function Allocates a blank memory page with zeroes.  sm_new_page - Allocates a new memory page filled with zeroes.*/
static char *sm_new_page(void) {
    return (char *)calloc(PAGE_SIZE, 1);
}

/* This function helps to Open the file safely initially. It returns Null if the input is not matched with if conditions 
/**
 * sm_fopen - Safely opens a file.
 * @fname: Name of the file.
 * @mode: Opening mode string ("rb", "wb", etc.).
 * @return File pointer, or NULL if inputs are invalid.
 */

static FILE *sm_fopen(const char *fname, const char *mode) {                  // Open in read mode
    if (!fname || !mode) return NULL;     // Write null bytes one at a time
    return fopen(fname, mode);
}

/* Getting file size one by one in bytes . It gets file size using fstat * sm_file_size - Retrieves file size in bytes.
 * @fp: File pointer.
 * @out: Output parameter for file size.
 * @return 1 if successful, 0 otherwise.*/

static int sm_file_size(FILE *fp, long *out) {
    if (!fp || !out) return 0;
    struct stat st;
    if (fstat(fileno(fp), &st) != 0) {
        return 0;
    }
    *out = (long)st.st_size;
    return 1;
}

/* Converting bytes into pages and helps to return the pages and rounds up. 
* sm_pages_from_bytes - Converts bytes to page count, rounding up.
 * @sz: Size in bytes.
 * @return Number of pages required.*/

static int sm_pages_from_bytes(long sz) {
    if (sz <= 0) return 0;
    int pages = (int)(sz / PAGE_SIZE);
    if (sz % PAGE_SIZE != 0) pages++;
    return pages;
}

/* Validating the page to run . It checks a given page index is valid for a particular file.
* sm_valid_page - Validates page index for a file.
 * @idx: Page index.
 * @fh: File handle.
 * @return 1 if valid, 0 if invalid.*/

static int sm_valid_page(int idx, const SM_FileHandle *fh) {
    if (!fh) return 0;
    if (idx < 0) return 0;
    if (idx >= fh->totalNumPages) return 0;
    return 1;
}

/* Seek to page.It allocates and helps to seek file pointer to particular page 
sm_seek_page - Seeks file pointer to the start of a given page.
 * @fp: File pointer.
 * @idx: Page index.*/

static int sm_seek_page(FILE *fp, int idx) {
    if (!fp) return 0;
    long offset = (long)idx * (long)PAGE_SIZE;
    return (fseek(fp, offset, SEEK_SET) == 0);
}

/* Update curPagePos after read.It allows Updates of  file handle after reading the particular page. */
static void sm_update_cur_after_read(SM_FileHandle *fh, int idx) {
    if (!fh) return;
    fh->curPagePos = (idx + 1) * PAGE_SIZE;
}

/* Update curPagePos after write. This function Updates file handle after writing */
static void sm_update_cur_after_write(SM_FileHandle *fh, FILE *fp, int idx) {
    if (!fh) return;
    long pos = ftell(fp);
    fh->curPagePos = (pos < 0) ? (idx + 1) * PAGE_SIZE : (int)pos;
}

/* Write one empty page.It Writes or appends a blank page if any */
static int sm_write_blank(FILE *fp) {
    if (!fp) return 0;
    char *pg = sm_new_page();
    if (!pg) return 0;
    size_t n = fwrite(pg, 1, PAGE_SIZE, fp);
    free(pg);
    return (n == PAGE_SIZE);
}

/* Append empty page. */
static int sm_append_blank(FILE *fp) {
    if (!fp) return 0;
    if (fseek(fp, 0, SEEK_END) != 0) return 0;
    return sm_write_blank(fp);
}

/* Ensure global open.Manages global file pointer lifecycle for running the entire code */
static int sm_ensure_open(const char *fname, const char *mode) {
    if (g_active_fp) return 1;
    g_active_fp = sm_fopen(fname, mode);
    return (g_active_fp != NULL);
}

/* Close global. Similarly it manages the global pointer 
* sm_ensure_open - Ensures the global file pointer is open.
 * @fname: File name.
 * @mode: Opening mode.
 * @return 1 if open, 0 otherwise. */

static void sm_close_global(void) {
    if (g_active_fp) {
        fclose(g_active_fp);
        g_active_fp = NULL;
    }
}

/* Current page index from position. */
static int sm_cur_page(int bytePos) {
    if (bytePos <= 0) return 0;
    return bytePos / PAGE_SIZE;
}

/* ---------------------------------------------------------------------------
   Public functions
   ---------------------------------------------------------------------------*/

/* Initialize storage manager. This line helps in initialization */
extern void initStorageManager(void) {
    g_active_fp = NULL;
    fdk      = NULL;
}

/* Create file with one blank page. * createPageFile - Creates a new paginated file (one blank page).
 * @fileName: Name of the file to create.
 * @return Error code. */

extern RC createPageFile(char *fileName) {
    if (!fileName) return RC_FILE_NOT_FOUND;
    FILE *fp = sm_fopen(fileName, "w+b");
    if (!fp) return RC_FILE_NOT_FOUND;

    if (!sm_write_blank(fp)) {
        fclose(fp);
        return RC_WRITE_FAILED;
    }
    fflush(fp);
    fclose(fp);
    return RC_OK;
}

/* Open file and populate handle. 
* openPageFile - Opens an existing paginated file, populates SM_FileHandle.
 * @fileName: Name of the file.
 * @fHandle: Pointer to file handle struct.
 * @return Error code.*/

extern RC openPageFile(char *fileName, SM_FileHandle *fHandle) {
    if (!fileName || !fHandle) return RC_FILE_NOT_FOUND;
    FILE *fp = sm_fopen(fileName, "r+b");
    if (!fp) return RC_FILE_NOT_FOUND;

    long nbytes = 0;
    if (!sm_file_size(fp, &nbytes)) {
        fclose(fp);
        return RC_ERROR;
    }

    fHandle->fileName   = fileName;
    fHandle->curPagePos = 0;

    int pages = sm_pages_from_bytes(nbytes);
    if (pages <= 0) {
        if (!sm_write_blank(fp)) {
            fclose(fp);
            return RC_WRITE_FAILED;
        }
        fHandle->totalNumPages = 1;
    } else {
        fHandle->totalNumPages = pages;
    }
    fclose(fp);
    return RC_OK;
}

/* Close file. */ // Original method for closePageFile
extern RC closePageFile(SM_FileHandle *fHandle) {
    (void)fHandle;
    sm_close_global();
    fdk = NULL;
    return RC_OK;
}

/* Destroy file. */ 
extern RC destroyPageFile(char *fileName) {
    if (!fileName) return RC_FILE_NOT_FOUND;
    if (access(fileName, F_OK) != 0) return RC_FILE_NOT_FOUND;
    if (remove(fileName) != 0) return RC_ERROR;
    return RC_OK;
}

/* Internal read helper. */
static RC sm_read_internal(FILE *fp, int idx, SM_FileHandle *fh, SM_PageHandle memPage) {
    if (!fp || !fh || !memPage) return RC_ERROR;
    if (!sm_valid_page(idx, fh)) return RC_READ_NON_EXISTING_PAGE;
    if (!sm_seek_page(fp, idx)) return RC_READ_NON_EXISTING_PAGE;

    size_t got = fread(memPage, 1, PAGE_SIZE, fp);
    if (got != PAGE_SIZE) return RC_READ_NON_EXISTING_PAGE;

    sm_update_cur_after_read(fh, idx);
    return RC_OK;
}

/* Read block. */
extern RC readBlock(int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage) {
    if (!fHandle || !memPage) return RC_ERROR;
    FILE *fp = sm_fopen(fHandle->fileName, "rb");
    if (!fp) return RC_FILE_NOT_FOUND;
    RC rc = sm_read_internal(fp, pageNum, fHandle, memPage);
    fclose(fp);
    return rc;
}

/* Current position function to navigate. */
extern int getBlockPos(SM_FileHandle *fHandle) {
    if (!fHandle) return -1;
    return fHandle->curPagePos;
}

/* Reading first block of the page. */
extern RC readFirstBlock(SM_FileHandle *fHandle, SM_PageHandle memPage) {
    return readBlock(0, fHandle, memPage);
}

/* Read previous block for accesing the previous blocks. */
extern RC readPreviousBlock(SM_FileHandle *fHandle, SM_PageHandle memPage) {
    if (!fHandle || !memPage) return RC_ERROR;
    if (fHandle->curPagePos == 0) return RC_READ_AT_BEGINNING;
    int cur = sm_cur_page(fHandle->curPagePos);
    int prev = (cur <= 0) ? 0 : (cur - 1);
    return readBlock(prev, fHandle, memPage);
}

/* Read current block after reading the previous blocks. */
extern RC readCurrentBlock(SM_FileHandle *fHandle, SM_PageHandle memPage) {
    if (!fHandle || !memPage) return RC_ERROR;
    int cur = sm_cur_page(fHandle->curPagePos);
    if (!sm_valid_page(cur, fHandle)) return RC_READ_NON_EXISTING_PAGE;
    return readBlock(cur, fHandle, memPage);
}

/* Reads next block. */
extern RC readNextBlock(SM_FileHandle *fHandle, SM_PageHandle memPage) {
    if (!fHandle || !memPage) return RC_ERROR;
    int cur = sm_cur_page(fHandle->curPagePos);
    int next = cur + 1;
    if (!sm_valid_page(next, fHandle)) return RC_READ_NON_EXISTING_PAGE;
    return readBlock(next, fHandle, memPage);
}

/* Read last block. */
extern RC readLastBlock(SM_FileHandle *fHandle, SM_PageHandle memPage) {
    if (!fHandle || !memPage) return RC_ERROR;
    int last = (fHandle->totalNumPages > 0) ? (fHandle->totalNumPages - 1) : 0;
    return readBlock(last, fHandle, memPage);
}

/* Internal write helper. * sm_read_internal - Reads contents of a page into buffer.
 * @fp: File pointer.
 * @idx: Page index.
 * @fh: File handle.
 * @memPage: Buffer to read into.
 * @return Error code */

static RC sm_write_internal(FILE *fp, int idx, SM_FileHandle *fh, SM_PageHandle memPage) {
    if (!fp || !fh || !memPage) return RC_ERROR;
    if (!sm_valid_page(idx, fh)) return RC_WRITE_FAILED;
    if (!sm_seek_page(fp, idx)) return RC_WRITE_FAILED;

    size_t wrote = fwrite(memPage, 1, PAGE_SIZE, fp);
    if (wrote != PAGE_SIZE) return RC_WRITE_FAILED;

    sm_update_cur_after_write(fh, fp, idx);
    return RC_OK;
}

/* Write block. */
extern RC writeBlock(int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage) {
    if (!fHandle || !memPage) return RC_ERROR;
    FILE *fp = sm_fopen(fHandle->fileName, "r+b");
    if (!fp) return RC_FILE_NOT_FOUND;
    RC rc = sm_write_internal(fp, pageNum, fHandle, memPage);

    long sz = 0;
    if (sm_file_size(fp, &sz)) {
        int total = sm_pages_from_bytes(sz);
        if (total > 0) fHandle->totalNumPages = total;
    }
    fclose(fp);
    return rc;
}

/* Write current block. */
extern RC writeCurrentBlock(SM_FileHandle *fHandle, SM_PageHandle memPage) {
    if (!fHandle || !memPage) return RC_ERROR;
    int cur = sm_cur_page(fHandle->curPagePos);
    FILE *fp = sm_fopen(fHandle->fileName, "r+b");
    if (!fp) return RC_FILE_NOT_FOUND;
    RC rc = sm_write_internal(fp, cur, fHandle, memPage);
    if (rc == RC_OK) fHandle->curPagePos = (cur + 1) * PAGE_SIZE;

    long sz = 0;
    if (sm_file_size(fp, &sz)) {
        int total = sm_pages_from_bytes(sz);
        if (total > 0) fHandle->totalNumPages = total;
    }
    fclose(fp);
    return rc;
}

/* Append empty block. */
extern RC appendEmptyBlock(SM_FileHandle *fHandle) {
    if (!fHandle) return RC_ERROR;
    if (!sm_ensure_open(fHandle->fileName, "a+b")) return RC_FILE_NOT_FOUND;
    if (!sm_append_blank(g_active_fp)) return RC_WRITE_FAILED;

    long sz = 0;
    if (sm_file_size(g_active_fp, &sz)) {
        int total = sm_pages_from_bytes(sz);
        fHandle->totalNumPages = (total > 0) ? total : fHandle->totalNumPages + 1;
    }
    return RC_OK;
}

/* Ensure file has at least N pages. */
extern RC ensureCapacity(int numberOfPages, SM_FileHandle *fHandle) {
    if (!fHandle) return RC_ERROR;
    if (!sm_ensure_open(fHandle->fileName, "a+b")) return RC_FILE_NOT_FOUND;

    long sz = 0;
    if (sm_file_size(g_active_fp, &sz)) {
        int total = sm_pages_from_bytes(sz);
        if (total > 0) fHandle->totalNumPages = total;
    }

    int needed = (numberOfPages < 0) ? 0 : numberOfPages;
    while (fHandle->totalNumPages < needed) {
        if (!sm_append_blank(g_active_fp)) return RC_WRITE_FAILED;
        long now = 0;
        if (sm_file_size(g_active_fp, &now)) {
            int total = sm_pages_from_bytes(now);
            fHandle->totalNumPages = (total > 0) ? total : fHandle->totalNumPages + 1;
        } else {
            fHandle->totalNumPages++;
        }
    }
    return RC_OK;
}

