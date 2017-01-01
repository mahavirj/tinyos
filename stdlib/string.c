#include <stdint.h>
#include <string.h>

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
	size_t i;

	for (i = 0; i < n && src[i] != '\0'; i++)
		dest[i] = src[i];

	for (; i < n; i++)
		dest[i] = '\0';
	
	return dest;
}

int strcmp(const char *s1, const char *s2)
{
	int i;

	for (i = 0; s1[i]; i++) {
		if (s1[i] != s2[i])
			return s1[i] - s2[i];
	}
	return 0;
}

int strncmp(const char *s1, const char *s2, size_t n)
{
	size_t i;

	for (i = 0; i < n; i++) {
		if (s1[i] != s2[i])
			return s1[i] - s2[i];
		else if (s1[i] == '\0')
			return 0;
	}

	return 0;
}

void *memcpy(void *dest, const void *src, size_t n)
{
	size_t index = 0;
	uint8_t *t_dest = (uint8_t *) dest;
	uint8_t *t_src = (uint8_t *) src;

	if ((((size_t) dest ^ (size_t) src) & 0x3) == 0) {
		/* We can align src and dest on word boundary */
		int size = (size_t) dest & 0x3;
		while (size-- && index < n) {
			*t_dest++ = *t_src;
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
	}

	while (index < n) {
		*t_dest++ = *t_src++;
		index++;
	}

	return dest;
}

void *memset(void *s, int c, size_t n)
{
	uint8_t *src = (uint8_t *) s;
	size_t i;

	for (i = 0; i < n; i++)
		src[i] = c;
	return s;
}

void *memsetw(uint16_t *s, uint16_t c, size_t n)
{
	size_t i;

	for (i = 0; i < n; i++)
		s[i] = c;
	return s;
}
