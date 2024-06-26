#include <string.h>

char *strcat(char *dest, const char *src) {
	char *tmp = dest;
	while (*dest) dest++;

	while (*dest++ = *src++) ;
	return tmp;
}

