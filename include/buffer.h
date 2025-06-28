#ifndef BUFFER_H
#define BUFFER_H

#include "editor.h"

struct abuf {
	// It basically stores the string {a pointer to initial char and it's length}
	char *b;  // pointer to the buffer
	int len;  // lenght of the buffer
};

#define ABUF_INIT {NULL, 0}	 // Representing empty buffer

void abAppend(struct abuf *ab, const char *s, int len);
void abFree(struct abuf *ab);

#endif
