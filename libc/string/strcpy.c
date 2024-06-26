#include <string.h>

char *strcpy(char *dest, const char *src) {
	return memcpy(dest, src, sizeof(src) + 1);
}

