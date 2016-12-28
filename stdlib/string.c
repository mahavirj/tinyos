#include <stdio.h>
#include <stdint.h>

char *strcpy(char *dest, const char *src)
{
	int i;

	for (i = 0; src[i]; i++)
		dest[i] = src[i];

	dest[i] = '\0';	
	return dest;
}

char *strncpy(char *dest, const char *src, size_t n)
{
	int i;

	for (i = 0; i < n && src[i] != '\0'; i++)
		dest[i] = src[i];

	for (; i < n; i++)
		dest[i] = '\0';
	
	return dest;
}

int strncmp(const char *s1, const char *s2, size_t n)
{
	int i;

	for (i = 0; i < n; i++) {
		if (s1[i] != s2[i])
			return s1[i] - s2[i];
		else if (s1[i] == '\0')
			return 0;
	}

	return 0;
}

#define ALIGN 4
void *memcpy(void *dest, const void *src, size_t n)
{
	int index = 0;
	uint8_t *t_dest = (uint8_t *) dest;
	uint8_t *t_src = (uint8_t *) src;

	/* It is not possible to align both source and destination pointers,
	 * following aligns destination only and then copies data
	 * word-by-word, a little more efficient than byte copy
	 */

	while ((index < n) && ((size_t) t_dest & ~(ALIGN-1))) {
		*t_dest++ = *t_src++;
		index++;
	}	

	uint32_t *w_dest = (uint32_t *) t_dest;
	uint32_t *w_src = (uint32_t *) t_src;

	while (index < (n - sizeof(uint32_t))) {
		*w_dest++ = *w_src++;
		index += sizeof(uint32_t);
	}

	t_dest = (uint8_t *) w_dest;
	t_src = (uint8_t *) w_src;

	while (index < n) {
		*t_dest++ = *t_src++;
		index++;
	}

	return dest;
}
