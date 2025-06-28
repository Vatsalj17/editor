#ifndef FILE_H
#define FILE_H

#include "editor.h"

char *editorRowsToString(int *buflen);
void editorOpen(char *filename);
void editorSave(void);

#endif
