#include "../include/terminal.h"

#include <errno.h>
#include <sys/ioctl.h>

void die(const char *s) {
	write(STDOUT_FILENO, "\x1b[2J", 4);	 // Clear screen
	write(STDOUT_FILENO, "\x1b[H", 3);	// Position the cursor at top-left corner
	perror(s);
	exit(1);
}

void disableRawMode() {
	// reset to original state
	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.orig_termios) == -1) die("tcsetattr");
}

void enableRawMode() {
	if (tcgetattr(STDIN_FILENO, &E.orig_termios) == -1) die("tcgetattr");
	atexit(disableRawMode);
	struct termios raw = E.orig_termios;
	// Input flags - clear these bits to disable input processing features
	raw.c_iflag &= ~(BRKINT	 // Disable SIGINT on break condition
					 | ICRNL  // Disable translation of carriage return '\r' to newline '\n'
					 | INPCK  // Disable parity checking
					 | ISTRIP  // Disable stripping off the 8th bit (i.e., preserve all 8 bits of input)
					 | IXON);  // Disable software flow control (Ctrl-S / Ctrl-Q)
	// Output flags - clear post-processing of output
	raw.c_oflag &= ~(OPOST);  // Disable all output processing (e.g., no automatic newline translation)
	// Control flags - set character size to 8 bits per byte
	raw.c_cflag |= (CS8);  // Set 8-bit characters (no parity)
	// Local flags - clear these bits to disable canonical mode and various signals/features
	raw.c_lflag &= ~(ECHO  // Disable echoing typed characters to the terminal
					 | ICANON  // Disable canonical mode (read input byte-by-byte instead of line-by-line)
					 | IEXTEN  // Disable implementation-defined input processing (e.g., Ctrl-V)
					 | ISIG);  // Disable generating signals like SIGINT (Ctrl-C), SIGTSTP (Ctrl-Z)
	// Control characters
	raw.c_cc[VMIN] = 0;	 // VMIN = Minimum number of bytes before read() return
	raw.c_cc[VTIME] = 1;  // VTIME = Maximum amount of time to wait before read()
	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) die("tcsetattr");
}

int editorReadKey() {
	int nread;
	char c;
	while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
		if (nread == -1 && errno != EAGAIN) die("read");
	}
	if (c == '\x1b') {
		char seq[3];
		/* [A - UP
		 * [B - DOWN
		 * [C - RIGHT
		 * [D - LEFT
		 * [5~ - PageUp
		 * [6~ - PageDown
		 * [1~ | [7~ | [H | OH - HOME  // 1~ in my terminal
		 * [4~ | [8~ | [F | OF - END   // 4~ in my terminal
		 */
		if (read(STDIN_FILENO, &seq[0], 1) != 1) return '\x1b';
		if (read(STDIN_FILENO, &seq[1], 1) != 1) return '\x1b';
		if (seq[0] == '[') {
			if (seq[1] >= '0' && seq[1] <= '9') {
				if (read(STDIN_FILENO, &seq[2], 1) != 1) return '\x1b';
				if (seq[2] == '~') {
					switch (seq[1]) {
						case '1':
							return HOME_KEY;
						case '3':
							return DELETE_KEY;
						case '4':
							return END_KEY;
						case '5':
							return PAGE_UP;
						case '6':
							return PAGE_DOWN;
						case '7':
							return HOME_KEY;
						case '8':
							return END_KEY;
					}
				}
			} else {
				switch (seq[1]) {
					case 'A':
						return ARROW_UP;
					case 'B':
						return ARROW_DOWN;
					case 'C':
						return ARROW_RIGHT;
					case 'D':
						return ARROW_LEFT;
					case 'H':
						return HOME_KEY;
					case 'F':
						return END_KEY;
				}
			}
		} else if (seq[0] == 'O') {
			switch (seq[1]) {
				case 'H':
					return HOME_KEY;
				case 'F':
					return END_KEY;
			}
		}
		return '\x1b';
	} else {
		return c;
	}
}

int getCursorPosition(int *rows, int *cols) {
	char buf[32];
	unsigned int i = 0;
	if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4) return -1;	 // asks where is the cursor
	// Replies ESC [ rows ; cols R
	while (i < sizeof(buf) - 1) {
		if (read(STDIN_FILENO, &buf[i], 1) != 1) break;
		if (buf[i] == 'R') break;
		i++;
	}
	buf[i] = '\0';

	if (buf[0] != '\x1b' || buf[1] != '[') return -1;
	if (sscanf(&buf[2], "%d;%d", rows, cols) != 2) return -1;
	return 0;
}

int getWindowSize(int *rows, int *cols) {
	struct winsize ws;
	// TIOCGWINSZ - Terminal Input Output Control Get WINdow SiZe
	if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
		if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12) return -1;
		// Maxing out the position of cursor to get screen size by getting cursor position
		return getCursorPosition(rows, cols);
	} else {
		*cols = ws.ws_col;
		*rows = ws.ws_row;
		return 0;
	}
}
