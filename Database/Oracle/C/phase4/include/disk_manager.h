/*
 * disk_manager.h - Disk Manager Header
 * 
 * Manages disk I/O operations for reading and writing pages.
 * 
 * Key Concepts:
 * - Read Page: Load page from disk into memory
 * - Write Page: Write page from memory to disk
 * - File Management: Manage database file
 * 
 * Author: Database Learning Project
 * Date: May 2026
 */

#ifndef DISK_MANAGER_H
#define DISK_MANAGER_H

#include "page.h"
#include <stdint.h>
#include <stdio.h>

/* ============================================================================
 * CONSTANTS
 * ============================================================================ */

/* Maximum filename length */
#define MAX_FILENAME_LENGTH 256

/* ============================================================================
 * DATA STRUCTURES
 * ============================================================================ */

/*
 * DiskManager - Manages disk I/O operations
 * 
 * Handles reading and writing pages to/from disk.
 * 
 * Fields:
 *   - filename: Path to database file
 *   - file: File handle
 *   - page_count: Total pages in database
 */
typedef struct {
    char filename[MAX_FILENAME_LENGTH];  /* Database filename */
    FILE *file;                         /* File handle */
    uint32_t page_count;                /* Total pages */
} DiskManager;

/* ============================================================================
 * FUNCTION DECLARATIONS
 * ============================================================================ */

/*
 * disk_manager_create - Create a new disk manager
 * 
 * Creates and initializes a disk manager for the given database file.
 * 
 * Parameters:
 *   filename: Path to database file
 * 
 * Returns:
 *   Pointer to new DiskManager, or NULL on error
 * 
 * Example:
 *   DiskManager *dm = disk_manager_create("database.db");
 *   if (!dm) {
 *       fprintf(stderr, "Failed to create disk manager\n");
 *       return 1;
 *   }
 */
DiskManager* disk_manager_create(const char *filename);

/*
 * disk_manager_destroy - Free disk manager memory
 * 
 * Closes the database file and frees the disk manager.
 * 
 * Parameters:
 *   dm: Pointer to DiskManager
 * 
 * Returns:
 *   void
 * 
 * Example:
 *   disk_manager_destroy(dm);
 */
void disk_manager_destroy(DiskManager *dm);

/*
 * disk_manager_read_page - Read page from disk
 * 
 * Reads a page from disk into the provided page structure.
 * 
 * Parameters:
 *   dm: Pointer to DiskManager
 *   page_id: Page ID to read
 *   page: Pointer to Page structure to fill
 * 
 * Returns:
 *   0 on success, -1 on error
 * 
 * Example:
 *   Page *page = page_create(1);
 *   if (disk_manager_read_page(dm, 1, page) != 0) {
 *       fprintf(stderr, "Failed to read page\n");
 *   }
 */
int disk_manager_read_page(DiskManager *dm, uint32_t page_id, Page *page);

/*
 * disk_manager_write_page - Write page to disk
 * 
 * Writes a page from memory to disk.
 * 
 * Parameters:
 *   dm: Pointer to DiskManager
 *   page: Pointer to Page to write
 * 
 * Returns:
 *   0 on success, -1 on error
 * 
 * Example:
 *   if (disk_manager_write_page(dm, page) != 0) {
 *       fprintf(stderr, "Failed to write page\n");
 *   }
 */
int disk_manager_write_page(DiskManager *dm, Page *page);

/*
 * disk_manager_allocate_page - Allocate a new page
 * 
 * Allocates a new page ID and extends the database file if needed.
 * 
 * Parameters:
 *   dm: Pointer to DiskManager
 * 
 * Returns:
 *   New page ID, or INVALID_PAGE_ID on error
 * 
 * Example:
 *   uint32_t page_id = disk_manager_allocate_page(dm);
 *   if (page_id == INVALID_PAGE_ID) {
 *       fprintf(stderr, "Failed to allocate page\n");
 *   }
 */
uint32_t disk_manager_allocate_page(DiskManager *dm);

/*
 * disk_manager_get_page_count - Get total page count
 * 
 * Returns the total number of pages in the database.
 * 
 * Parameters:
 *   dm: Pointer to DiskManager
 * 
 * Returns:
 *   Total page count
 * 
 * Example:
 *   uint32_t count = disk_manager_get_page_count(dm);
 *   printf("Total pages: %u\n", count);
 */
uint32_t disk_manager_get_page_count(DiskManager *dm);

/*
 * disk_manager_get_filename - Get database filename
 * 
 * Returns the filename of the database.
 * 
 * Parameters:
 *   dm: Pointer to DiskManager
 * 
 * Returns:
 *   Filename string
 * 
 * Example:
 *   printf("Database: %s\n", disk_manager_get_filename(dm));
 */
const char* disk_manager_get_filename(DiskManager *dm);

#endif // DISK_MANAGER_H
