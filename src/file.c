#include <errno.h>
#include <fcntl.h>
#include "../include/file.h"
#include "../include/syntax.h"
#include "../include/terminal.h"
#include "../include/row.h"
#include "../include/output.h"
#include "../include/input.h"

char *editorRowsToString(int *buflen) {
	int totlen = 0;
	int j;
	for (j = 0; j < E.numrows; j++) totlen += E.row[j].size + 1;
	*buflen = totlen;

	char *buf = malloc(totlen);
	char *p = buf;	// making p point the same location as buf
	for (j = 0; j < E.numrows; j++) {
		memcpy(p, E.row[j].chars, E.row[j].size);  // copies each line to p
		p += E.row[j].size;						   // move pointer forward
		*p = '\n';								   // add newline after every line
		p++;
	}
	return buf;
}

void editorOpen(char *filename) {
	free(E.filename);
	E.filename = strdup(filename);

	editorSelectSyntaxHighlight();

	FILE *fp = fopen(filename, "r");
	if (!fp) die("fopen");

	char *line = NULL;
	size_t linecap = 0;
	ssize_t linelen;
	// linecap set to 0 so that getline allocates memory accordingly to line, getline returns the number of bytes including \n
	while ((linelen = getline(&line, &linecap, fp)) != -1) {
		// Loop for trimming new line characters
		while (linelen > 0 && (line[linelen - 1] == '\n' || line[linelen - 1] == '\r'))
			linelen--;
		editorInsertRow(E.numrows, line, linelen);
	}
	free(line);
	fclose(fp);
	E.dirty = 0;
}

void editorSave() {
	if (E.filename == NULL) {
		E.filename = editorPrompt("Save as: %s (ESC to cancel)", NULL);
		if (E.filename == NULL) {
			editorSetStatusMessage("Save aborted");
			return;
		}
		editorSelectSyntaxHighlight();
	}
	int len;
	char *buf = editorRowsToString(&len);
	int fd = open(E.filename, O_RDWR | O_CREAT, 0644);
	if (fd != -1) {
		// ftruncate sets the file to specific size
		if (ftruncate(fd, len) != -1) {
			if (write(fd, buf, len) == len) {
				close(fd);
				free(buf);
				E.dirty = 0;
				editorSetStatusMessage("%d bytes written to disk", len);
				return;
			}
		}
		close(fd);
	}
	free(buf);
	editorSetStatusMessage("Can't Save! I/O error: %s", strerror(errno));
}

