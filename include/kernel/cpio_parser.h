
#ifndef _CPIO_PARSER_H_
#define _CPIO_PARSER_H_

#include <stdint.h>

/* Magic identifiers for the "cpio" file format. */
#define CPIO_HEADER_MAGIC "070701"
#define CPIO_FOOTER_MAGIC "TRAILER!!!"
#define CPIO_ALIGNMENT 4

struct cpio_header {
	char c_magic [6];      /* Magic header '070701'. */
	char c_ino [8];        /* "i-node" number. */
	char c_mode [8];       /* Permisions. */
	char c_uid [8];        /* User ID. */
	char c_gid [8];        /* Group ID. */
	char c_nlink [8];      /* Number of hard links. */
	char c_mtime [8];      /* Modification time. */
	char c_filesize [8];   /* File size. */
	char c_devmajor [8];   /* Major dev number. */
	char c_devminor [8];   /* Minor dev number. */
	char c_rdevmajor [8];
	char c_rdevminor [8];
	char c_namesize [8];   /* Length of filename in bytes. */
	char c_check [8];      /* Checksum. */
};

long int strtox(const char *nptr, int size);

/**
 * Retrieve file information from a provided file name
 * @param[in] archive  The location of the CPIO archive
 * @param[in] name     The name of the file in question.
 * @param[out] size    The retrieved size of the file in question
 * @return             The location of the file in memory; NULL if the file
 *                     does not exist.
 */
void *cpio_get_file(void *cpio_loc, const char *name, unsigned long *size);

#endif /* _CPIO_PARSER_H_ */

