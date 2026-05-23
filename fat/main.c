#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

#include "disk_access.h"
#include "fat.h"
#include "mbr.h"
#include "fat_print.h"
#include "fat_format.h"

#ifndef PATH_MAX
#define PATH_MAX 255
#endif

static struct FATVolume vol;
static struct MBR mbr;
static struct disk_geometry geom;
static void *ctx = 0;

static void do_format(void)
{
    int index;
    struct FATPartition part;
    struct MBR_partition_entry entry;
    char answer;
    int sectors = 0;
    char buffer[17];

    printf(" * FORMAT ROUTINE\n\n");

    do
    {
        MBR_list(&mbr);
        printf("* SELECT PARTITION TO FORMAT: \n");
        scanf("%d", &index);
        putchar('\n');
    } while ((index < 0) || (index > 3));

    if (MBR_get_partition_entry(&mbr, index, &entry))
    {
        printf("* ERROR: CANNOT ACCESS SELECTED PARTITION\n");
        return;
    }

    part.startsector = entry.LBA_first_Sector;
    part.endsector = part.startsector + entry.num_of_sectors - 1;
    part.secpercluster = 0;
    memset(part.label, ' ', sizeof(part.label));
    memset(buffer, 0, sizeof(buffer));
    printf("* ENTER VOLUME NAME [16 characters]: ");
    scanf("%16s", buffer);
    putchar('\n');
    if (!strlen(buffer))
        memset(part.label, ' ', sizeof(part.label));
    else
    {
        if (strlen(buffer) < sizeof(part.label))
            memset(buffer + strlen(buffer), ' ', sizeof(buffer) - strlen(buffer));
        strncpy(part.label, buffer, sizeof(part.label));
    }

    printf("* FORMAT PARTITION [%d] SECTORS [%d] LABEL [%16s]\n", index, entry.num_of_sectors, part.label);
    do
    {
        printf(" Yes or No? ");
        answer = (char)getchar();
        putchar('\n');
    } while ((toupper(answer) != 'Y') && (toupper(answer) != 'N'));

    if (toupper(answer) == 'N')
    {
        return;
    }

    printf("* SECTORS PER CLUSTER ? [0] ");
    scanf("%d", &sectors);
    part.secpercluster = sectors & UCHAR_MAX;

    printf("* FORMAT ROUTINE: FORMATTING ... ");
    if (FAT_format(ctx, &part))
    {
        printf("FAILED.\n");
    }
    else
    {
        printf("SUCCESS.\n");
    }
}

static void do_mount(void)
{
    int index;
    struct MBR_partition_entry entry;

    printf(" * MOUNT ROUTINE\n\n");

    do
    {
        MBR_list(&mbr);
        printf("* SELECT PARTITION TO MOUNT: ");
        scanf("%d", &index);
        putchar('\n');
    } while ((index < 0) || (index > 3));

    if (MBR_get_partition_entry(&mbr, index, &entry))
    {
        printf("* ERROR: CANNOT ACCESS SELECTED PARTITION\n");
        return;
    }

    if (FAT_mount(ctx, entry.LBA_first_Sector, &vol))
    {
        printf("* ERROR: CANNOT MOUNT SELECTED PARTITION\n");
        return;
    }

    FAT_print_Volume(&vol);
    putchar('\n');
    putchar('\n');
    FAT_print_PB(&vol.PB);
}

static void do_unmount()
{
    printf("* UNMOUNT ROUTINE\n\n");

    if (FAT_unmount(&vol))
        printf("* ERROR: CANNOT UNMOUNT\n");
}

static void do_create()
{
    char path[PATH_MAX];
    char name[45];

    printf("* CREATE ROUTINE\n\n");

    memset(path, 0, sizeof(path));
    memset(name, 0, sizeof(name));
    printf("* ENTER PATH TO FILE : ");
    scanf("%s", path);
    putchar('\n');
    printf("* ENTER FILE NAME : ");
    scanf("%44s", name);
    if (strlen(path) > 1)
        strncat(path, "\\", 1);
    strncat(path, name, sizeof(path) - sizeof(name));
    printf("* CREATING %s ...\n", path);
    if (FAT_create_entry(&vol, path, 0))
        printf("* CREATION ERROR\n");
}

static void do_create_dir()
{
    char path[PATH_MAX];
    char name[45];

    printf("* DIRECTORY CREATE ROUTINE\n\n");

    memset(path, 0, sizeof(path));
    memset(name, 0, sizeof(name));
    printf("* ENTER PATH TO DIRECTORY : ");
    scanf("%s", path);
    putchar('\n');
    printf("* ENTER DIRECTORY NAME : ");
    scanf("%12s", name);
    if (strlen(path) > 1)
        strncat(path, "\\", 1);
    strncat(path, name, sizeof(path) - sizeof(name));
    printf("* CREATING %s ...\n", path);

    if (FAT_create_entry(&vol, path, ATTR_DIRECTORY))
        printf("* CREATION ERROR\n");
}

static void do_delete()
{
    char path[PATH_MAX];

    printf("* DELETE ROUTINE\n\n");

    memset(path, 0, sizeof(path));
    printf("* ENTER COMPLETE PATH NAME : ");
    scanf("%s", path);
    putchar('\n');

    if (FAT_delete_entry(&vol, path))
        printf("* DELETION ERROR\n");
}

static void do_rename()
{
    char path[PATH_MAX];
    char oldname[45];
    char newname[45];

    printf("* RENAME ROUTINE\n\n");

    memset(path, 0, sizeof(path));
    memset(oldname, 0, sizeof(oldname));
    memset(newname, 0, sizeof(newname));
    printf("* ENTER PATH TO FILE : ");
    scanf("%s", path);
    putchar('\n');
    printf("* ENTER OLD FILE NAME : ");
    scanf("%44s", oldname);
    printf("* ENTER NEW FILE NAME : ");
    scanf("%44s", newname);
    printf("* RENAMING %s TO %s in %s ...\n", oldname, newname, path);
    if (strlen(path) > 1)
        strncat(path, "\\", 1);
    strncat(path, oldname, sizeof(path) - sizeof(oldname));

    if (FAT_rename_entry(&vol, path, newname))
        printf("* RENAMING ERROR\n");
}

static void do_list()
{
    char path[PATH_MAX];
    uint32_t b;
    uint16_t c;

    printf(" * LIST ROUTINE\n\n");

    printf("* SELECT PATH TO LIST: ");
    scanf("%s", path);
    putchar('\n');

    if (FAT_list(&vol, path))
        printf("* ERROR: CANNOT LIST THIS PATH\n");
    if (FAT_available(&vol, &b, &c))
        printf("* ERROR: CANNOT GET AVAILABLE SPACE\n");
    printf("\nAvailable clusters ... : %u"
           "\nAvailable bytes ...... : %u (%u MB)"
           "\n\n",
           c, b, b / (1024 * 1024));
}

static void do_write()
{
    char path[PATH_MAX];
    unsigned len = 0;
    unsigned offset = 0;
    unsigned wrtlen = 0;
    char *buffer = 0;
    unsigned i = 0;

    printf("* WRITE ROUTINE\n\n");

    memset(path, 0, sizeof(path));
    printf("* ENTER COMPLETE PATH NAME : ");
    scanf("%s", path);
    putchar('\n');
    printf("* ENTER OFFSET : ");
    scanf("%u", &offset);
    putchar('\n');
    printf("* ENTER SIZE : ");
    scanf("%u", &len);
    putchar('\n');
    buffer = malloc(len);

    for (i = 0; i < len; i++)
        buffer[i] = ('0' + i % 16);

    if (FAT_write(&vol, path, offset, buffer, len, &wrtlen))
        printf("* WRITE ERROR %d\n", wrtlen);

    free(buffer);
}

static void do_read()
{
    char path[PATH_MAX];
    unsigned len = 0;
    unsigned offset = 0;
    unsigned rdlen = 0;
    char *buffer = 0;
    unsigned i = 0;

    printf("* READ ROUTINE\n\n");

    memset(path, 0, sizeof(path));
    printf("* ENTER COMPLETE PATH NAME : ");
    scanf("%s", path);
    putchar('\n');
    printf("* ENTER OFFSET : ");
    scanf("%u", &offset);
    putchar('\n');
    printf("* ENTER SIZE : ");
    scanf("%u", &len);
    putchar('\n');
    buffer = malloc(len);

    if (FAT_read(&vol, path, offset, buffer, len, &rdlen))
        printf("* READ ERROR %d\n", rdlen);
    else
        for (i = 0; i < rdlen; i++)
        {
            putchar(buffer[i]);
            if ((i + 1) % 60 == 0)
            {
                printf(" -- %u\n", i + 1);
            }
        }
    putchar('\n');
    free(buffer);
}

static void do_truncate()
{
    char path[PATH_MAX];
    unsigned wrtlen = 0;
    char *buffer = 0;

    printf("* TRUNCATE ROUTINE\n\n");

    memset(path, 0, sizeof(path));
    printf("* ENTER COMPLETE PATH NAME : ");
    scanf("%s", path);
    putchar('\n');

    if (FAT_truncate_entry(&vol, path))
        printf("* TRUNCATE ERROR %d\n", wrtlen);

    free(buffer);
}

static void do_megatest()
{
    unsigned i = 0;
    char write_buffer[1024 * 7 / 3];
    char read_buffer[1024 * 7 / 3];
    unsigned bufsize;
    unsigned read_size;
    uint8_t pattern_byte;
    int test_passed = 0, test_failed = 0;
    int deletion_error = 0;
    struct FAT entry;
    int error;

    static const struct
    {
        const char name[PATH_MAX];
        uint8_t attr;
    } layout[] =
        {
            {"\\read.me", 0},
            {"\\DiegOS.bin", ATTR_HIDDEN | ATTR_SYSTEM | ATTR_READ_ONLY},
            {"\\diegos", ATTR_DIRECTORY},
            {"\\diegos\\i18n", ATTR_DIRECTORY},
            {"\\diegos\\chkdsk.exe", 0},
            {"\\diegos\\attrib.exe", 0},
            {"\\diegos\\format.exe", 0},
            {"\\diegos\\copy.exe", 0},
            {"\\diegos\\xcopy.exe", 0},
            {"\\diegos\\superMegaGigaTonictOOl.com", 0},
            {"\\diegos\\dosshell.exe", 0},
            {"\\diegos\\edit.exe", 0},
            {"\\diegos\\edlin.exe", 0},
            {"\\diegos\\EMM386.exe", 0},
            {"\\diegos\\tree.exe", 0},
            {"\\diegos\\command.com", 0},
            {"\\DOCS", ATTR_DIRECTORY},
            {"\\DOCS\\readme!!!.TXT", 0},
            {"\\diegos\\I18N\\english.inf", ATTR_READ_ONLY},
            {"\\diegos\\I18N\\german.inf", ATTR_READ_ONLY},
            {"\\diegos\\I18N\\turkish.inf", ATTR_READ_ONLY},
            {"\\diegos\\I18N\\suomi.inf", ATTR_READ_ONLY},
            {"\\diegos\\I18N\\french.inf", ATTR_READ_ONLY},
            {"\\diegos\\i18n\\english.inf", ATTR_READ_ONLY},
            {"\\diegos\\i18n\\german.inf", ATTR_READ_ONLY},
            {"\\diegos\\i18n\\turkish.inf", ATTR_READ_ONLY},
            {"\\diegos\\i18n\\suomi.inf", ATTR_READ_ONLY},
            {"\\diegos\\i18n\\french.inf", ATTR_READ_ONLY},
            {"\\diegos\\dir.exe", 0},
            {"\\diegos\\robocopy.exe", 0},
            {"\\diegos\\more.com", 0},
            {"\\diegos\\defrag.exe", 0},
            {"\\diegos\\debug.com", 0},
            {"\\diegos\\keyb.com", 0},
            {"\\IO.SYS", ATTR_HIDDEN | ATTR_SYSTEM | ATTR_READ_ONLY},
            {"", 0}};

    printf("* MEGATEST ROUTINE - COMPREHENSIVE FAT16.1 UNIT TEST\n\n");

    /* TEST 1: File creation and write/read with pattern verification */
    printf("TEST 1: File Creation, Write with Pattern, and Read Verification\n");
    while (layout[i].name[0])
    {
        error = FAT_create_entry(&vol, layout[i].name, layout[i].attr & ATTR_DIRECTORY);
        if (error)
        {
            printf("  FAIL: Creation error %d for %s\n", error, layout[i].name);
            test_failed++;
        }
        else
        {
            /* This is a file! */
            if ((layout[i].attr & ATTR_DIRECTORY) == 0)
            {
                bufsize = rand() % (1024 * 7 / 3) + 1; /* At least 1 byte */
                pattern_byte = (uint8_t)(rand() & 0xFF);
                memset(write_buffer, pattern_byte, bufsize);

                error = FAT_write(&vol, layout[i].name, 0, write_buffer, bufsize, &bufsize);
                if (error)
                {
                    printf("  FAIL: Write error %d for %s\n", error, layout[i].name);
                    test_failed++;
                }
                else
                {
                    /* Read back and verify pattern */
                    error = FAT_read(&vol, layout[i].name, 0, read_buffer, bufsize, &read_size);
                    if (error)
                    {
                        printf("  FAIL: Read error %d for %s\n", error, layout[i].name);
                        test_failed++;
                    }
                    else if ((read_size != bufsize) || (memcmp(write_buffer, read_buffer, bufsize) != 0))
                    {
                        printf("  FAIL: Pattern verification failed for %s (expected %u bytes, got %u)\n",
                               layout[i].name, bufsize, read_size);
                        test_failed++;
                    }
                    else
                    {
                        error = FAT_modify_entry_attr(&vol, layout[i].name, layout[i].attr);
                        if (error)
                        {
                            printf("  FAIL: Failed to modify attributes error %d for %s\n", error, layout[i].name);
                            test_failed++;
                        }
                        else
                        {
                            printf("  PASS: %s (size: %u bytes, pattern: 0x%02X)\n",
                                   layout[i].name, bufsize, pattern_byte);
                            test_passed++;
                        }
                    }
                }
            }
            else
            {
                printf("  PASS: %s\n", layout[i].name);
                test_passed++;
            }
        }
        ++i;
    }

    printf("\nTest 1 Results: Passed=%d, Failed=%d\n\n", test_passed, test_failed);

    /* TEST 2: Directory structure validation */
    printf("TEST 2: Directory Structure Validation\n");
    test_passed = 0;
    test_failed = 0;

    /* Check that directories exist */
    if (FAT_get_entry(&vol, "\\diegos", &entry) == 0 && (entry.DIR_Attr & ATTR_DIRECTORY))
    {
        printf("  PASS: Root directory 'diegos' exists and is a directory\n");
        test_passed++;
    }
    else
    {
        printf("  FAIL: Root directory 'diegos' does not exist or is not a directory\n");
        test_failed++;
    }

    if (FAT_get_entry(&vol, "\\diegos\\i18n", &entry) == 0 && (entry.DIR_Attr & ATTR_DIRECTORY))
    {
        printf("  PASS: Nested directory 'diegos\\i18n' exists and is a directory\n");
        test_passed++;
    }
    else
    {
        printf("  FAIL: Nested directory 'diegos\\i18n' does not exist or is not a directory\n");
        test_failed++;
    }

    if (FAT_get_entry(&vol, "\\DOCS", &entry) == 0 && (entry.DIR_Attr & ATTR_DIRECTORY))
    {
        printf("  PASS: Root directory 'DOCS' exists and is a directory\n");
        test_passed++;
    }
    else
    {
        printf("  FAIL: Root directory 'DOCS' does not exist or is not a directory\n");
        test_failed++;
    }

    printf("\nTest 2 Results: Passed=%d, Failed=%d\n\n", test_passed, test_failed);

    /* TEST 3: File deletion tests */
    printf("TEST 3: File Deletion Tests\n");
    test_passed = 0;
    test_failed = 0;
    deletion_error = 0;

    /* Test 3a: Simple file deletion */
    printf("  Test 3a: Simple file deletion\n");
    if (FAT_delete_entry(&vol, "\\read.me") == 0)
    {
        printf("    PASS: Successfully deleted simple file 'read.me'\n");
        test_passed++;
    }
    else
    {
        printf("    FAIL: Could not delete simple file 'read.me'\n");
        test_failed++;
    }

    /* Test 3b: Try to delete read-only file (should fail) */
    printf("  Test 3b: Deletion of read-only file (should fail)\n");
    if (FAT_get_entry(&vol, "\\diegos\\i18n\\english.inf", &entry) == 0)
    {
        deletion_error = FAT_delete_entry(&vol, "\\diegos\\i18n\\english.inf");
        if (deletion_error != 0)
        {
            printf("    PASS: Read-only file 'english.inf' correctly cannot be deleted (error: %d)\n", deletion_error);
            test_passed++;
        }
        else
        {
            printf("    FAIL: Read-only file 'english.inf' was incorrectly deleted\n");
            test_failed++;
        }
    }

    /* Test 3c: Try to delete hidden file (should fail) */
    printf("  Test 3c: Deletion of hidden file (should fail)\n");
    if (FAT_get_entry(&vol, "\\DiegOS.bin", &entry) == 0 && (entry.DIR_Attr & ATTR_HIDDEN))
    {
        deletion_error = FAT_delete_entry(&vol, "\\DiegOS.bin");
        if (deletion_error != 0)
        {
            printf("    PASS: Hidden file 'DiegOS.bin' correctly cannot be deleted (error: %d)\n", deletion_error);
            test_passed++;
        }
        else
        {
            printf("    FAIL: Hidden file 'DiegOS.bin' was incorrectly deleted\n");
            test_failed++;
        }
    }

    /* Test 3d: Try to delete system file (should fail) */
    printf("  Test 3d: Deletion of system file (should fail)\n");
    if (FAT_get_entry(&vol, "\\IO.SYS", &entry) == 0 && (entry.DIR_Attr & ATTR_SYSTEM))
    {
        deletion_error = FAT_delete_entry(&vol, "\\IO.SYS");
        if (deletion_error != 0)
        {
            printf("    PASS: System file 'IO.SYS' correctly cannot be deleted (error: %d)\n", deletion_error);
            test_passed++;
        }
        else
        {
            printf("    FAIL: System file 'IO.SYS' was incorrectly deleted\n");
            test_failed++;
        }
    }

    /* Test 3e: Delete regular files in directory before deleting directory */
    printf("  Test 3e: Deletion of regular files in directories\n");
    if (FAT_delete_entry(&vol, "\\DoCs\\readme!!!.TXT") == 0)
    {
        printf("    PASS: Successfully deleted file 'readme!!!.TXT' from DOCS directory\n");
        test_passed++;
    }
    else
    {
        printf("    FAIL: Could not delete file 'readme!!!.TXT' from DOCS directory\n");
        test_failed++;
    }

    /* Test 3f: Delete files from nested directory */
    printf("  Test 3f: Deletion of files from nested directories\n");
    i = 0;
    int files_deleted = 0;
    while (layout[i].name[0])
    {
        /* Only delete files in diegos\i18n that are NOT read-only */
        if (strstr(layout[i].name, "\\diegos\\i18n\\") && !(layout[i].attr & ATTR_READ_ONLY))
        {
            if (FAT_delete_entry(&vol, layout[i].name) == 0)
            {
                printf("    PASS: Deleted file '%s'\n", layout[i].name);
                files_deleted++;
            }
        }
        ++i;
    }
    if (files_deleted > 0)
        test_passed += files_deleted;

    printf("\nTest 3 Results: Passed=%d, Failed=%d\n\n", test_passed, test_failed);

    /* TEST 4: Empty directory deletion */
    printf("TEST 4: Empty Directory Deletion\n");
    test_passed = 0;
    test_failed = 0;

    /* First, delete all remaining files in diegos\i18n by removing read-only attribute */
    printf("  Preparing directories for deletion test...\n");
    i = 0;
    while (layout[i].name[0])
    {
        /* Find and clear read-only files in i18n directories */
        if (strstr(layout[i].name, "\\diegos\\i18n\\") && (layout[i].attr & ATTR_READ_ONLY))
        {
            if (FAT_get_entry(&vol, layout[i].name, &entry) == 0)
            {
                FAT_modify_entry_attr(&vol, layout[i].name, 0);
                error = FAT_delete_entry(&vol, layout[i].name);
                if (error) printf("%d\n", error);
            }
        }
        else if (strstr(layout[i].name, "\\diegos\\I18N\\"))
        {
            if (FAT_get_entry(&vol, layout[i].name, &entry) == 0)
            {
                FAT_modify_entry_attr(&vol, layout[i].name, 0);
                error = FAT_delete_entry(&vol, layout[i].name);
                if (error) printf("%d\n", error);
            }
        }
        ++i;
    }

    /* Now try to delete empty directories */
    printf("  Test 4a: Deletion of empty nested directory\n");
    if (FAT_delete_entry(&vol, "\\diegos\\i18n") == 0)
    {
        printf("    PASS: Successfully deleted empty directory 'diegos\\i18n'\n");
        test_passed++;
    }
    else
    {
        printf("    FAIL: Could not delete empty directory 'diegos\\i18n'\n");
        test_failed++;
    }

    printf("  Test 4b: Deletion of empty root subdirectory\n");
    error = FAT_delete_entry(&vol, "\\DOCS");
    if (error == 0)
    {
        printf("    PASS: Successfully deleted empty directory 'DOCS'\n");
        test_passed++;
    }
    else
    {
        printf("    FAIL: Could not delete empty directory 'DOCS' %d\n", error);
        test_failed++;
    }

    printf("\nTest 4 Results: Passed=%d, Failed=%d\n\n", test_passed, test_failed);

    /* TEST 5: Link creation and deletion tests */
    printf("TEST 5: Link Creation and Deletion Tests\n");
    test_passed = 0;
    test_failed = 0;

    /* Test 5a: Create link to a file (use DiegOS.bin which has ATTR_SYSTEM and cannot be deleted) */
    printf("  Test 5a: Create link to a file\n");
    if (FAT_create_link(&vol, "\\link_to_diegos_bin_1", "\\DiegOS.bin") == 0)
    {
        printf("    PASS: Successfully created link 'link_to_diegos_bin_1' pointing to 'DiegOS.bin'\n");
        test_passed++;
    }
    else
    {
        printf("    FAIL: Could not create link 'link_to_diegos_bin_1'\n");
        test_failed++;
    }

    /* Test 5b: Verify link exists */
    printf("  Test 5b: Verify link exists\n");
    if (FAT_get_entry(&vol, "\\link_to_diegos_bin_1", &entry) == 0)
    {
        printf("    PASS: Link 'link_to_diegos_bin_1' found in filesystem\n");
        test_passed++;
    }
    else
    {
        printf("    FAIL: Link 'link_to_diegos_bin_1' not found in filesystem\n");
        test_failed++;
    }

    /* Test 5c: Create multiple links to different files */
    printf("  Test 5c: Create multiple links to different files\n");
    int links_created = 0;
    if (FAT_create_link(&vol, "\\link_to_diegos_bin", "\\DiegOS.bin") == 0)
        links_created++;
    if (FAT_create_link(&vol, "\\link_to_copy.exe", "\\diegos\\copy.exe") == 0)
        links_created++;
    if (FAT_create_link(&vol, "\\link_to_tree.exe", "\\diegos\\tree.exe") == 0)
        links_created++;

    if (links_created == 3)
    {
        printf("    PASS: Successfully created %d additional links\n", links_created);
        test_passed++;
    }
    else
    {
        printf("    FAIL: Could not create all additional links (created %d/3)\n", links_created);
        test_failed++;
    }

    /* Test 5d: Delete a link */
    printf("  Test 5d: Delete a link\n");
    if (FAT_delete_entry(&vol, "\\link_to_diegos_bin_1") == 0)
    {
        printf("    PASS: Successfully deleted link 'link_to_diegos_bin_1'\n");
        test_passed++;
    }
    else
    {
        printf("    FAIL: Could not delete link 'link_to_diegos_bin_1'\n");
        test_failed++;
    }

    /* Test 5e: Verify deleted link is gone */
    printf("  Test 5e: Verify deleted link is gone\n");
    if (FAT_get_entry(&vol, "\\link_to_diegos_bin_1", &entry) != 0)
    {
        printf("    PASS: Deleted link 'link_to_diegos_bin_1' is no longer accessible\n");
        test_passed++;
    }
    else
    {
        printf("    FAIL: Deleted link 'link_to_diegos_bin_1' is still accessible\n");
        test_failed++;
    }

    /* Test 5f: Verify that the linked file still exists after link deletion */
    printf("  Test 5f: Verify linked file still exists after link deletion\n");
    if (FAT_get_entry(&vol, "\\DiegOS.bin", &entry) == 0)
    {
        printf("    PASS: Original file 'DiegOS.bin' still exists after link deletion\n");
        test_passed++;
    }
    else
    {
        printf("    FAIL: Original file 'DiegOS.bin' was incorrectly deleted with the link\n");
        test_failed++;
    }

    /* Test 5g: Delete all remaining links */
    printf("  Test 5g: Delete all remaining links\n");
    int links_deleted = 0;
    if (FAT_delete_entry(&vol, "\\link_to_diegos_bin") == 0)
        links_deleted++;
    if (FAT_delete_entry(&vol, "\\link_to_copy.exe") == 0)
        links_deleted++;
    if (FAT_delete_entry(&vol, "\\link_to_tree.exe") == 0)
        links_deleted++;

    if (links_deleted == 3)
    {
        printf("    PASS: Successfully deleted %d links\n", links_deleted);
        test_passed++;
    }
    else
    {
        printf("    FAIL: Could not delete all links (deleted %d/3)\n", links_deleted);
        test_failed++;
    }

    /* Test 5h: Verify all linked files still exist */
    printf("  Test 5h: Verify all linked files still exist\n");
    int files_intact = 0;
    if (FAT_get_entry(&vol, "\\DiegOS.bin", &entry) == 0)
        files_intact++;
    if (FAT_get_entry(&vol, "\\diegos\\copy.exe", &entry) == 0)
        files_intact++;
    if (FAT_get_entry(&vol, "\\diegos\\tree.exe", &entry) == 0)
        files_intact++;

    if (files_intact == 3)
    {
        printf("    PASS: All original files still exist after their links were deleted\n");
        test_passed++;
    }
    else
    {
        printf("    FAIL: Some original files were incorrectly deleted (%d/3 intact)\n", files_intact);
        test_failed++;
    }

    printf("\nTest 5 Results: Passed=%d, Failed=%d\n\n", test_passed, test_failed);

    printf("* MEGATEST ROUTINE COMPLETED\n");
}

static void do_print_fat()
{
    putchar('\n');
    FAT_print_FAT(&vol);
    putchar('\n');
}

static void do_show_mbr()
{
    putchar('\n');
    MBR_list(&mbr);
    putchar('\n');
}

static void do_write_mbr()
{
    int index;
    uint32_t start_sector, end_sector, num_sectors;
    char answer;

    printf("* WRITE MBR ROUTINE\n\n");

    /* Show current MBR */
    MBR_list(&mbr);
    putchar('\n');

    /* Ask which partition to edit */
    do
    {
        printf("* SELECT PARTITION TO EDIT [0-3]: ");
        scanf("%d", &index);
        putchar('\n');
    } while ((index < 0) || (index > 3));

    /* Ask for starting sector */
    printf("* ENTER STARTING SECTOR (LBA): ");
    scanf("%u", &start_sector);
    putchar('\n');

    /* Ask for ending sector */
    printf("* ENTER ENDING SECTOR (LBA): ");
    scanf("%u", &end_sector);
    putchar('\n');

    /* Validate sector range */
    if (end_sector < start_sector)
    {
        printf("* ERROR: ENDING SECTOR CANNOT BE LESS THAN STARTING SECTOR\n");
        return;
    }
    num_sectors = end_sector - start_sector + 1;

    /* Ask if partition should be active */
    printf("* SHOULD THIS PARTITION BE ACTIVE? (Y/N): ");
    scanf("%c", &answer);
    putchar('\n');

    /* Update the partition entry */
    mbr.partitions[index].LBA_first_Sector = start_sector;
    mbr.partitions[index].num_of_sectors = num_sectors;
    mbr.partitions[index].status = (toupper(answer) == 'Y') ? '\x80' : '\x00';

    /* Write the MBR to disk */
    if (MBR_write(ctx, &mbr))
    {
        printf("* ERROR: CANNOT WRITE MBR\n");
    }
    else
    {
        printf("* MBR WRITTEN SUCCESSFULLY\n");
    }
}

static void print_main_menu()
{
    printf("+----------------------------------------------+\n");
    printf("| 1  -> FORMAT VOLUME                          |\n");
    printf("| 2  -> MOUNT VOLUME                           |\n");
    printf("| 3  -> UNMOUNT VOLUME                         |\n");
    printf("| 4  -> CREATE FILE                            |\n");
    printf("| 5  -> CREATE DIRECTORY                       |\n");
    printf("| 6  -> DELETE FILE OR DIRECTORY               |\n");
    printf("| 7  -> RENAME FILE OR DIRECTORY               |\n");
    printf("| 8  -> LIST DIRECTORY                         |\n");
    printf("| 9  -> WRITE TO FILE                          |\n");
    printf("| 10 -> READ FROM FILE                         |\n");
    printf("| 11 -> TRUNCATE FILE                          |\n");
    printf("| 21 -> MEGA TEST                              |\n");
    printf("| 22 -> PRINT FAT                              |\n");
    printf("+----------------------------------------------+\n");
    printf("| 90 -> SHOW MBR                               |\n");
    printf("| 91 -> WRITE MBR                              |\n");
    printf("| 99 -> EXIT                                   |\n");
    printf("+----------------------------------------------+\n");
}

int main()
{
    int retcode;

    if (disk_init("D:\\temp\\FAT16_first.img", &ctx))
    // if (disk_init("output.img",&ctx))
    {
        printf("ERROR INITIALIZING DISK\n");
        return 99;
    }

    if (disk_get_geometry(ctx, &geom))
    {
        printf("ERROR READING GEOMETRY\n");
        return 98;
    }

    memset(&vol, 0, sizeof(vol));
    if (MBR_read(ctx, &mbr))
    {
        printf("ERROR READING MBR\n");
        disk_done(ctx);
        return 97;
    }

    while (1)
    {
        print_main_menu();
        scanf("%d", &retcode);
        if (retcode == 1)
            do_format();
        else if (retcode == 2)
            do_mount();
        else if (retcode == 3)
            do_unmount();
        else if (retcode == 4)
            do_create();
        else if (retcode == 5)
            do_create_dir();
        else if (retcode == 6)
            do_delete();
        else if (retcode == 7)
            do_rename();
        else if (retcode == 8)
            do_list();
        else if (retcode == 9)
            do_write();
        else if (retcode == 10)
            do_read();
        else if (retcode == 11)
            do_truncate();
        else if (retcode == 21)
            do_megatest();
        else if (retcode == 22)
            do_print_fat();
        else if (retcode == 90)
            do_show_mbr();
        else if (retcode == 91)
            do_write_mbr();
        else if (retcode == 99)
            break;
    }

    disk_done(ctx);

    return 0;
}
