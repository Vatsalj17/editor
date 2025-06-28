#include "../include/row.h"
#include "../include/syntax.h"

int editorRowCxToRx(erow *row, int cx) {
	int rx = 0;
	for (int j = 0; j < cx; j++) {
		if (row->chars[j] == '\t') rx += (TAB_STOP - 1) - (rx % TAB_STOP);
		rx++;
	}
	return rx;
}

int editorRowRxToCx(erow *row, int rx) {
	int cur_rx = 0;	 // current rx
	int cx;
	for (cx = 0; cx < row->size; cx++) {
		if (row->chars[cx] == '\t') cur_rx += (TAB_STOP - 1) - (cur_rx % TAB_STOP);
		cur_rx++;
		if (cur_rx > rx) return cx;
	}
	return cx;
}

void editorUpdateRow(erow *row) {
	int tabs = 0;
	int j;
	// count no. of tabs in the buffer
	for (j = 0; j < row->size; j++)
		if (row->chars[j] == '\t') tabs++;
	free(row->render);											  // freeing any type of buffer to avoid mem leak
	row->render = malloc(row->size + tabs * (TAB_STOP - 1) + 1);  // Allocate memory for the new render buffer

	// -1 because one counted in row->size already and +1 for null terminator
	int idx = 0;
	for (j = 0; j < row->size; j++) {
		if (row->chars[j] == '\t') {
			row->render[idx++] = ' ';							   // insert atleast one space
			while (idx % TAB_STOP != 0) row->render[idx++] = ' ';  // keep inserting till cursor is aligned to next tab stop
		} else {
			row->render[idx++] = row->chars[j];
		}
	}
	row->render[idx] = '\0';
	row->rsize = idx;  // Update rendered size
	editorUpdateSyntax(row);
}

void editorInsertRow(int at, char *s, size_t len) {
	if (at < 0 || at > E.numrows) return;
	E.row = realloc(E.row, sizeof(erow) * (E.numrows + 1));				   // Resize the E.row array to hold one more erow (line of text)
	memmove(&E.row[at + 1], &E.row[at], sizeof(erow) * (E.numrows - at));  // making space for the new line
	for (int j = at + 1; j <= E.numrows; j++) E.row[j].idx++;			   // update the index of each row that was displaced by the insert or delete
	E.row[at].idx = at;
	E.row[at].size = len;  // initializing the new row in next four lines
	E.row[at].chars = malloc(len + 1);
	memcpy(E.row[at].chars, s, len);
	E.row[at].chars[len] = '\0';
	E.row[at].rsize = 0;
	E.row[at].render = NULL;
	E.row[at].hl = NULL;
	E.row[at].hl_open_comment = 0;

	editorUpdateRow(&E.row[at]);

	E.numrows++;  // Increase row count
	E.dirty++;
}

void editorFreeRow(erow *row) {
	free(row->render);
	free(row->chars);
	free(row->hl);
}

void editorDelRow(int at) {
	if (at < 0 || at >= E.numrows) return;
	editorFreeRow(&E.row[at]);
	memmove(&E.row[at], &E.row[at + 1], sizeof(erow) * (E.numrows - at - 1));  // Shifts rows up
	for (int j = at; j <= E.numrows - 1; j++) E.row[j].idx--;				   // update the index of each row that was displaced by the insert or delete
	// last argument tells how many rows to move
	E.numrows--;
	E.dirty++;
}

// it doesn't have to worry about where the cursor is
void editorRowInsertChar(erow *row, int at, int c) {
	if (at < 0 || at > row->size) at = row->size;						// If position of at is invalid then fix it
	row->chars = realloc(row->chars, row->size + 2);					// 1 for extra char and one for null terminator
	memmove(&row->chars[at + 1], &row->chars[at], row->size - at + 1);	// move one character forward at "at" so that new character can be entered at "at"
	// memmove is like memcpy but is safe to use when the source and destination arrays overlap
	row->size++;		   // to keep track of no. of chars in the row
	row->chars[at] = c;	   // add the char to string
	editorUpdateRow(row);  // send to render
	E.dirty++;
}

void editorRowAppendString(erow *row, char *s, size_t len) {
	row->chars = realloc(row->chars, row->size + len + 1);
	memcpy(&row->chars[row->size], s, len);
	row->size += len;
	row->chars[row->size] = '\0';
	editorUpdateRow(row);
	E.dirty++;
}

void editorRowDelChar(erow *row, int at) {
	if (at < 0 || at >= row->size) return;
	memmove(&row->chars[at], &row->chars[at + 1], row->size - at);
	row->size--;
	editorUpdateRow(row);
	E.dirty++;
}

