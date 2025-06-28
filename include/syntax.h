#ifndef SYNTAX_H
#define SYNTAX_H

#include "editor.h"

#define HLDB_ENTRIES (sizeof(HLDB) / sizeof(HLDB[0]))

extern char *C_HL_extensions[];
extern char *C_HL_keywords[];
extern struct editorSyntax HLDB[];

int is_separator(int c);
void editorUpdateSyntax(erow *row);
int editorSyntaxToColor(int hl);
void editorSelectSyntaxHighlight(void);

#endif
