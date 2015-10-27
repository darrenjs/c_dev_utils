#ifndef COLOR_H
#define COLOR_H

/* This file is based on the color.h found in the git project.
 */


/*  2 + (2 * num_attrs) + 8 + 1 + 8 + 'm' + NUL */
/* "\033[1;2;4;5;7;38;5;2xx;48;5;2xxm\0" */
/*
 * The maximum length of ANSI color sequence we would generate:
 * - leading ESC '['            2
 * - attr + ';'                 3 * 10 (e.g. "1;")
 * - fg color + ';'             17 (e.g. "38;2;255;255;255;")
 * - bg color + ';'             17 (e.g. "48;2;255;255;255;")
 * - terminating 'm' NUL        2
 *
 * The above overcounts attr (we only use 5 not 8) and one semicolon
 * but it is close enough.
 */
#define COLOR_MAXLEN 70

/* Define a default colour palette.
 *
 * IMPORTANT: Due to the way these color codes are emulated on Windows,
 * write them only using printf(), fprintf(), and fputs(). In particular,
 * do not use puts() or write().
 */
#define GIT_COLOR_NORMAL	""
#define GIT_COLOR_RESET		"\033[m"
#define GIT_COLOR_BOLD		"\033[1m"
#define GIT_COLOR_RED		"\033[31m"
#define GIT_COLOR_GREEN		"\033[32m"
#define GIT_COLOR_YELLOW	"\033[33m"
#define GIT_COLOR_BLUE		"\033[34m"
#define GIT_COLOR_MAGENTA	"\033[35m"
#define GIT_COLOR_CYAN		"\033[36m"
#define GIT_COLOR_BOLD_RED	"\033[1;31m"
#define GIT_COLOR_BOLD_GREEN	"\033[1;32m"
#define GIT_COLOR_BOLD_YELLOW	"\033[1;33m"
#define GIT_COLOR_BOLD_BLUE	"\033[1;34m"
#define GIT_COLOR_BOLD_MAGENTA	"\033[1;35m"
#define GIT_COLOR_BOLD_CYAN	"\033[1;36m"
#define GIT_COLOR_BG_RED	"\033[41m"
#define GIT_COLOR_BG_GREEN	"\033[42m"
#define GIT_COLOR_BG_YELLOW	"\033[43m"
#define GIT_COLOR_BG_BLUE	"\033[44m"
#define GIT_COLOR_BG_MAGENTA	"\033[45m"
#define GIT_COLOR_BG_CYAN	"\033[46m"

#endif
