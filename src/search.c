#include "../include/search.h"
#include "../include/row.h"
#include "../include/input.h"

void editorFindCallback(char *query, int key) {
	static int last_match = -1;	 // static so that it remembers its value between function calls
	static int direction = 1;

	static int saved_hl_line;
	static char *saved_hl = NULL;
	if (saved_hl) {
		memcpy(E.row[saved_hl_line].hl, saved_hl, E.row[saved_hl_line].rsize);
		free(saved_hl);
		saved_hl = NULL;
	}

	if (key == '\r' || key == '\x1b') {
		last_match = -1;
		direction = 1;
		return;
	} else if (key == ARROW_DOWN || key == ARROW_RIGHT) {
		direction = 1;	// 1 for down or forward
	} else if (key == ARROW_LEFT || key == ARROW_UP) {
		direction = -1;	 // -1 for up or backward
	} else {
		last_match = -1;  // reset last match
		direction = 1;
	}

	if (last_match == -1) direction = 1;  // if no previous match, we force the forward search
	int current = last_match;			  // start searching from where we left

	// Incremental search
	for (int i = 0; i < E.numrows; i++) {
		current += direction;  // decide in which direction to go
		if (current == -1)
			current = E.numrows - 1;  // went above zero when searching backward
		else if (current == E.numrows)
			current = 0;  // went past bottom when searching forward

		erow *row = &E.row[current];			   // get the row to search
		char *match = strstr(row->render, query);  // to find the query
		if (match) {
			last_match = current;  // save last match index
			E.cy = current;		   // jump
			E.cx = editorRowRxToCx(row, match - row->render);
			E.rowoff = E.numrows;  // scroll to cursor

			saved_hl_line = current;
			saved_hl = malloc(row->rsize);
			memcpy(saved_hl, row->hl, row->rsize);
			memset(&row->hl[match - row->render], HL_MATCH, strlen(query));
			break;
		}
	}
}

void editorFind() {
	int saved_cx = E.cx;
	int saved_cy = E.cy;
	int saved_coloff = E.coloff;
	int saved_rowoff = E.rowoff;

	char *query = editorPrompt("Search: %s (Use ESC/ARROW/Enter)", editorFindCallback);
	if (query) {
		free(query);
	} else {
		// restore cursor position if search cancelled
		E.cx = saved_cx;
		E.cy = saved_cy;
		E.coloff = saved_coloff;
		E.rowoff = saved_rowoff;
	}
}

