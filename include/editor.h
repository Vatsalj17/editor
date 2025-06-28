#ifndef EDITOR_H
#define EDITOR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#define VERSION "0.0.1"
#define TAB_STOP 4
#define QUIT_TIMES 2

#define CTRL_KEY(v) ((v) & 0x1f)  // Because last 5 digits of a-z, A-Z and ctrl:a-z are same
/* a = 97 && z = 122
 * A = 64 && Z = 90
 * Ctrl+A = 1 && Ctrl+Z = 26 */

enum editorKey {
	BACKSPACE = 127,
	ARROW_LEFT = 1000,
	ARROW_RIGHT,
	ARROW_UP,
	ARROW_DOWN,
	DELETE_KEY,
	HOME_KEY,
	END_KEY,
	PAGE_UP,
	PAGE_DOWN
};

enum editorHighlight {
	HL_NORMAL = 0,
	HL_COMMENT,
	HL_MLCOMMENT,
	HL_KEYWORD1,
	HL_KEYWORD2,
	HL_STRING,
	HL_NUMBER,
	HL_MATCH
};

#define HL_HIGHLIGHT_NUMBERS (1 << 0)
#define HL_HIGHLIGHT_STRINGS (1 << 1)

struct editorSyntax {
	char *filetype;	 // name of filetype displayed to the user
	char **filematch;  // it is an array of strings, where each string contains a pattern to match a filename against
	char **keywords;
	char *singleline_comment_start;
	char *multiline_comment_start;
	char *multiline_comment_end;
	int flags;	// it is a bit field that will contain flags for whether to highlight numbers and whether to highlight strings for that filetype
};

typedef struct erow {
	int idx;  // for erow to know it's own index within the file
	int size;  // Size of the row
	int rsize;	// Size of render
	char *chars;  // String that is present in file
	char *render;  // Render what's going to printed on screen
	unsigned char *hl;	// this array will correspond to characters in render
	int hl_open_comment;  // boolean value to keep track of multiline comments
} erow;	 // Editor row

struct editorConfig {
	int cx, cy;	 // coordinates of cursor
	int rx;	 // like cx is for chars, rx is for render
	int rowoff;	 // row offset to track the top row
	int coloff;	 // column offset to track the first column
	int screenrows;	 // No. of rows on the screen
	int screencols;	 // No. of columns on the screen
	int numrows;  // No. of rows to be printed
	erow *row;	// Array of rows to pe printed on screen
	char *filename;	 // File name passed in argument
	int dirty;	// to keep track of whether the text loaded in our editor differs from the file
	char statusmsg[80];
	time_t statusmsg_time;	// It will contain the timestamp of when we print the status
	struct editorSyntax *syntax;
	struct termios orig_termios;
};

extern struct editorConfig E;

void initEditor(void);
void editorInsertChar(int c);
void editorInsertNewline(void);
void editorDelChar(void);

#endif
