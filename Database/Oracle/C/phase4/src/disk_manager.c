/*
 * disk_manager.c - Disk Manager Implementation
 * 
 * Implements disk I/O operations for reading and writing pages.
 * 
 * Author: Database Learning Project
 * Date: May 2026
 */

#include "disk_manager.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* ============================================================================
 * DISK MANAGER CREATION AND DESTRUCTION
 * ============================================================================ */

/*
 * disk_manager_create - Create a new disk manager
 * 
 * Creates and initializes a disk manager for the given database file.
 */
DiskManager* disk_manager_create(const char *filename) {
    if (!filename) {
        fprintf(stderr, "ERROR: Filename is NULL\n");
        return NULL;
    }
    
    /* Allocate memory for disk manager */
    DiskManager *dm = (DiskManager *)malloc(sizeof(DiskManager));
    if (!dm) {
        fprintf(stderr, "ERROR: Failed to allocate memory for disk manager\n");
        return NULL;
    }
    
    /* Copy filename */
    strncpy(dm->filename, filename, MAX_FILENAME_LENGTH - 1);
    dm->filename[MAX_FILENAME_LENGTH - 1] = '\0';
    
    /* Open or create database file */
    dm->file = fopen(filename, "r+b");
    if (!dm->file) {
        /* File doesn't exist, create it */
        dm->file = fopen(filename, "w+b");
        if (!dm->file) {
            fprintf(stderr, "ERROR: Failed to create database file: %s\n", filename);
            free(dm);
            return NULL;
        }
        dm->page_count = 0;
    } else {
        /* File exists, calculate page count */
        fseek(dm->file, 0, SEEK_END);
        long file_size = ftell(dm->file);
        dm->page_count = file_size / PAGE_SIZE;
        fseek(dm->file, 0, SEEK_SET);
    }
    
    return dm;
}

/*
 * disk_manager_destroy - Free disk manager memory
 * 
 * Closes the database file and frees the disk manager.
 */
void disk_manager_destroy(DiskManager *dm) {
    if (!dm) return;
    
    if (dm->file) {
        fclose(dm->file);
    }
    
    free(dm);
}

/* ============================================================================
 * PAGE I/O OPERATIONS
 * ============================================================================ */

/*
 * disk_manager_read_page - Read page from disk
 * 
 * Reads a page from disk into the provided page structure.
 */
int disk_manager_read_page(DiskManager *dm, uint32_t page_id, Page *page) {
    if (!dm || !dm->file || !page) {
        fprintf(stderr, "ERROR: Invalid parameters\n");
        return -1;
    }
    
    if (page_id >= dm->page_count) {
        fprintf(stderr, "ERROR: Page ID %u out of range (max: %u)\n",
                page_id, dm->page_count - 1);
        return -1;
    }
    
    /* Seek to page location */
    long offset = (long)page_id * PAGE_SIZE;
    if (fseek(dm->file, offset, SEEK_SET) != 0) {
        fprintf(stderr, "ERROR: Failed to seek to page %u\n", page_id);
        return -1;
    }
    
    /* Read page data */
    size_t bytes_read = fread(page->data, 1, PAGE_SIZE, dm->file);
    if (bytes_read != PAGE_SIZE) {
        fprintf(stderr, "ERROR: Failed to read page %u (read %zu bytes)\n",
                page_id, bytes_read);
        return -1;
    }
    
    /* Update page metadata */
    page->page_id = page_id;
    page->is_dirty = 0;
    
    return 0;
}

/*
 * disk_manager_write_page - Write page to disk
 * 
 * Writes a page from memory to disk.
 */
int disk_manager_write_page(DiskManager *dm, Page *page) {
    if (!dm || !dm->file || !page) {
        fprintf(stderr, "ERROR: Invalid parameters\n");
        return -1;
    }
    
    if (page->page_id >= dm->page_count) {
        fprintf(stderr, "ERROR: Page ID %u out of range (max: %u)\n",
                page->page_id, dm->page_count - 1);
        return -1;
    }
    
    /* Seek to page location */
    long offset = (long)page->page_id * PAGE_SIZE;
    if (fseek(dm->file, offset, SEEK_SET) != 0) {
        fprintf(stderr, "ERROR: Failed to seek to page %u\n", page->page_id);
        return -1;
    }
    
    /* Write page data */
    size_t bytes_written = fwrite(page->data, 1, PAGE_SIZE, dm->file);
    if (bytes_written != PAGE_SIZE) {
        fprintf(stderr, "ERROR: Failed to write page %u (wrote %zu bytes)\n",
                page->page_id, bytes_written);
        return -1;
    }
    
    /* Flush to disk */
    if (fflush(dm->file) != 0) {
        fprintf(stderr, "ERROR: Failed to flush page %u\n", page->page_id);
        return -1;
    }
    
    /* Mark page as clean */
    page->is_dirty = 0;
    
    return 0;
}

/* ============================================================================
 * PAGE ALLOCATION
 * ============================================================================ */

/*
 * disk_manager_allocate_page - Allocate a new page
 * 
 * Allocates a new page ID and extends the database file if needed.
 */
uint32_t disk_manager_allocate_page(DiskManager *dm) {
    if (!dm || !dm->file) {
        fprintf(stderr, "ERROR: Invalid disk manager\n");
        return INVALID_PAGE_ID;
    }
    
    /* Get new page ID */
    uint32_t new_page_id = dm->page_count;
    
    /* Extend file with empty page */
    char empty_page[PAGE_SIZE];
    memset(empty_page, 0, PAGE_SIZE);
    
    /* Seek to end of file */
    if (fseek(dm->file, 0, SEEK_END) != 0) {
        fprintf(stderr, "ERROR: Failed to seek to end of file\n");
        return INVALID_PAGE_ID;
    }
    
    /* Write empty page */
    size_t bytes_written = fwrite(empty_page, 1, PAGE_SIZE, dm->file);
    if (bytes_written != PAGE_SIZE) {
        fprintf(stderr, "ERROR: Failed to allocate page (wrote %zu bytes)\n",
                bytes_written);
        return INVALID_PAGE_ID;
    }
    
    /* Flush to disk */
    if (fflush(dm->file) != 0) {
        fprintf(stderr, "ERROR: Failed to flush new page\n");
        return INVALID_PAGE_ID;
    }
    
    /* Update page count */
    dm->page_count++;
    
    return new_page_id;
}

/* ============================================================================
 * QUERY OPERATIONS
 * ============================================================================ */

/*
 * disk_manager_get_page_count - Get total page count
 * 
 * Returns the total number of pages in the database.
 */
uint32_t disk_manager_get_page_count(DiskManager *dm) {
    if (!dm) return 0;
    return dm->page_count;
}

/*
 * disk_manager_get_filename - Get database filename
 * 
 * Returns the filename of the database.
 */
const char* disk_manager_get_filename(DiskManager *dm) {
    if (!dm) return NULL;
    return dm->filename;
}
