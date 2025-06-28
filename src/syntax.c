#include "../include/syntax.h"

#include <ctype.h>

char *C_HL_extensions[] = {".c", ".h", ".cpp", NULL};
char *C_HL_keywords[] = {
	"switch", "if", "while", "for", "break", "continue", "return", "else",
	"struct", "union", "typedef", "static", "enum", "class", "case",
	"int|", "long|", "double|", "float|", "char|", "unsigned|", "signed|",
	"void|", NULL};

// Highlight Database
struct editorSyntax HLDB[] = {
	{"c",
	 C_HL_extensions,
	 C_HL_keywords,
	 "//", "/*", "*/",
	 HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS},
};

int is_separator(int c) {
	return isspace(c) || c == '\0' || strchr(",.()+-/*=~%<>[];", c) != NULL;
}

void editorUpdateSyntax(erow *row) {
	row->hl = realloc(row->hl, row->rsize);
	memset(row->hl, HL_NORMAL, row->rsize);	 // Set all characters to normal

	if (E.syntax == NULL) return;  // If no syntax rule is selected

	char **keywords = E.syntax->keywords;  // get keywords for this file

	// aliases for comment delimiters
	char *scs = E.syntax->singleline_comment_start;
	char *mcs = E.syntax->multiline_comment_start;
	char *mce = E.syntax->multiline_comment_end;
	int scs_len = scs ? strlen(scs) : 0;
	int mcs_len = mcs ? strlen(mcs) : 0;
	int mce_len = mce ? strlen(mce) : 0;

	int prev_sep = 1;	// Indicates if the previous character was a separator
	int in_string = 0;	// Tracks if we're inside a string (either ' or ")

	int in_comment = (row->idx > 0 && E.row[row->idx - 1].hl_open_comment);	 // If previous line was still inside a multiline comment, continue it

	int i = 0;
	while (i < row->rsize) {
		char c = row->render[i];
		// Previous character's highlight type (default to HL_NORMAL)
		unsigned char prev_hl = (i > 0) ? row->hl[i - 1] : HL_NORMAL;

		// singleline commetn highlighting
		if (scs_len && !in_string && !in_comment) {
			// If current position starts a single-line comment
			if (!strncmp(&row->render[i], scs, scs_len)) {
				// Highlight the rest of the line as a comment
				memset(&row->hl[i], HL_COMMENT, row->rsize - i);
				break;	// No need to continue highlighting this line
			}
		}

		/*** Multiline Comment Highlighting ***/
		if (mcs_len && mce_len && !in_string) {
			if (in_comment) {
				// Inside a multiline comment → highlight this character
				row->hl[i] = HL_MLCOMMENT;

				// If we encounter the end of the comment
				if (!strncmp(&row->render[i], mce, mce_len)) {
					// Highlight the comment end
					memset(&row->hl[i], HL_MLCOMMENT, mce_len);
					i += mce_len;
					in_comment = 0;
					prev_sep = 1;
					continue;
				} else {
					// Still inside the comment block
					i++;
					continue;
				}
			} else if (!strncmp(&row->render[i], mcs, mcs_len)) {
				// Starting a new multiline comment block
				memset(&row->hl[i], HL_MLCOMMENT, mcs_len);
				i += mcs_len;
				in_comment = 1;
				continue;
			}
		}

		/*** String Highlighting (single or double quotes) ***/
		if (E.syntax->flags & HL_HIGHLIGHT_STRINGS) {
			if (in_string) {
				row->hl[i] = HL_STRING;

				// Escape sequences inside string (e.g., "hello \"world\"")
				if (c == '\\' && i + 1 < row->rsize) {
					row->hl[i + 1] = HL_STRING;
					i += 2;
					continue;
				}

				// If string ends
				if (c == in_string) in_string = 0;

				i++;
				prev_sep = 1;
				continue;
			} else {
				// Beginning of a string literal
				if (c == '"' || c == '\'') {
					in_string = c;
					row->hl[i] = HL_STRING;
					i++;
					continue;
				}
			}
		}

		/*** Number Highlighting ***/
		if (E.syntax->flags & HL_HIGHLIGHT_NUMBERS) {
			if ((isdigit(c) && (prev_sep || prev_hl == HL_NUMBER)) ||
				(c == '.' && prev_hl == HL_NUMBER)) {
				row->hl[i] = HL_NUMBER;
				i++;
				prev_sep = 0;  // Still in number
				continue;
			}
		}

		/*** Keyword Highlighting ***/
		if (prev_sep) {
			int j;
			for (j = 0; keywords[j]; j++) {
				int klen = strlen(keywords[j]);
				int kw2 = keywords[j][klen - 1] == '|';	 // Check for keyword type 2 (ends with '|')

				if (kw2) klen--;  // Remove '|' from keyword length

				// Match keyword at current position and ensure it's followed by a separator
				if (!strncmp(&row->render[i], keywords[j], klen) &&
					is_separator(row->render[i + klen])) {
					// Apply the appropriate keyword highlight
					memset(&row->hl[i], kw2 ? HL_KEYWORD2 : HL_KEYWORD1, klen);
					i += klen;
					break;
				}
			}

			// If keyword matched, move on to next char
			if (keywords[j] != NULL) {
				prev_sep = 0;
				continue;
			}
		}

		// Update separator flag for next character
		prev_sep = is_separator(c);
		i++;
	}

	/*** Post-processing: Propagate multiline comment state to next line if needed ***/
	int changed = (row->hl_open_comment != in_comment);
	row->hl_open_comment = in_comment;

	// If multiline comment state changed and there’s a next line, update it too
	if (changed && row->idx + 1 < E.numrows)
		editorUpdateSyntax(&E.row[row->idx + 1]);
}

int editorSyntaxToColor(int hl) {
	switch (hl) {
		case HL_COMMENT:
		case HL_MLCOMMENT:
			return 36;	// 36 -> cyan
		case HL_KEYWORD1:
			return 33;	// 33 -> yellow
		case HL_KEYWORD2:
			return 32;	// 32 -> green
		case HL_STRING:
			return 35;	// 35 -> magenta
		case HL_NUMBER:
			return 31;	// 31 -> red
		case HL_MATCH:
			return 34;	// 34 -> blue
		default:
			return 37;	// 37 -> white
	}
}

void editorSelectSyntaxHighlight() {
	E.syntax = NULL;
	if (E.filename == NULL) return;	 // If there's no filename
	char *ext = strrchr(E.filename, '.');
	for (unsigned int j = 0; j < HLDB_ENTRIES; j++) {
		struct editorSyntax *s = &HLDB[j];	// Grab a syntax rule from the DB
		unsigned int i = 0;
		// Go through all file patterns
		while (s->filematch[i]) {
			int is_ext = (s->filematch[i][0] == '.');
			if ((is_ext && ext && !strcmp(ext, s->filematch[i])) ||
				(!is_ext && strstr(E.filename, s->filematch[i]))) {
				E.syntax = s;

				int filerow;
				for (filerow = 0; filerow < E.numrows; filerow++) {
					editorUpdateSyntax(&E.row[filerow]);
				}
				return;
			}
			i++;
		}
	}
}
