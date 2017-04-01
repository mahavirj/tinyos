#include <stdio.h>
#include <string.h>
#include <cpio_parser.h>
#include <helper.h>
#include <stdint.h>

/*
 * Parse the header of the given CPIO entry.
 *
 * Return -1 if the header is not valid, 1 if it is EOF.
 */
static int cpio_parse_header(struct cpio_header *archive,
		const char **filename, unsigned long *_filesize, void **data,
		struct cpio_header **next)
{
	unsigned long filesize;
	/* Ensure magic header exists. */
	if (strncmp(archive->c_magic, CPIO_HEADER_MAGIC,
				sizeof(archive->c_magic)) != 0)
		return -1;

	/* Get filename and file size. */
	filesize = strtox(archive->c_filesize, sizeof(archive->c_filesize));
	*filename = ((char *) archive) + sizeof(struct cpio_header);

	/* Ensure filename is not the trailer indicating EOF. */
	if (strncmp(*filename, CPIO_FOOTER_MAGIC, sizeof(CPIO_FOOTER_MAGIC)) == 0)
		return 1;

	/* Find offset to data. */
	unsigned long filename_length = strtox(archive->c_namesize,
					 sizeof(archive->c_namesize));
	*data = (void *) ALIGN(((uintptr_t) archive)
			+ sizeof(struct cpio_header) + filename_length,
			 CPIO_ALIGNMENT);
	*next = (struct cpio_header *) ALIGN(((uintptr_t) *data) + filesize,
			 CPIO_ALIGNMENT);

	if (_filesize)
		*_filesize = filesize;

	return 0;
}

/*
 * Find the location and size of the file named "name" in the given 'cpio'
 * archive.
 *
 * Return NULL if the entry doesn't exist.
 *
 * Runs in O(n) time.
 */
void *cpio_get_file(void *archive, const char *name, unsigned long *size)
{
	struct cpio_header *header = archive;

	/* Find n'th entry. */
	while (1) {
		struct cpio_header *next;
		void *result;
		const char *current_filename;

		int error = cpio_parse_header(header, &current_filename,
				size, &result, &next);
		if (error)
			return NULL;
		if (strncmp(current_filename, name, -1) == 0)
			return result;
		header = next;
	}
}
