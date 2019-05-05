/* Copyright 2019
 * Ramon Fried <ramon.fried@gmail.com>
 */

#include <ctype.h>
#include <string.h>
#include "bitwise.h"

#define KB (1ULL << 10)
#define MB (1ULL << 20)
#define GB (1ULL << 30)
#define TB (1ULL << 40)
#define PB (1ULL << 50)

int g_has_color = 1;
int g_width = 0;
bool g_input_avail;
int g_input;

static int readline_input_avail(void)
{
	return g_input_avail;
}

static int readline_getc(FILE *dummy)
{
	g_input_avail = false;
	return g_input;
}
static void got_command(char *line)
{
	if (!line) {
		/* Handle CTL-D */
		g_leave_req = true;
		return;
	}

	if (strcmp("q", line) == 0)
	    g_leave_req = true;

        if (*line)
            add_history(line);

	LOG("got command: %s\n", line);
	free(line);
}

void readline_redisplay(void)
{
	size_t prompt_width = strlen(rl_display_prompt);
	size_t cursor_col = prompt_width + strlen(rl_line_buffer);
	werase(cmd_win);
	mvwprintw(cmd_win, 0, 0, "%s%s", rl_display_prompt, rl_line_buffer);
	wmove(cmd_win, 0, cursor_col);
	curs_set(2);
	wrefresh(cmd_win);
//    cmd_win_redisplay(false);
}


void init_terminal(void)
{
	initscr();
	if(has_colors() == FALSE) {
		use_default_colors();
		g_has_color = 0;
	}
	else
		start_color();
	cbreak();
	noecho();
	nonl();
	keypad(stdscr, TRUE);
	intrflush(NULL, FALSE);
	curs_set(0);
}

void init_readline(void)
{
	rl_catch_signals = 0;
	rl_catch_sigwinch = 0;
	rl_change_environment = 0;
	rl_getc_function = readline_getc;
	rl_input_available_hook = readline_input_avail;
	rl_redisplay_function = readline_redisplay;
	rl_callback_handler_install(":", got_command);

	rl_bind_key('\t', rl_insert);
}
void deinit_terminal(void)
{
	endwin();
}

void deinit_readline(void)
{
	rl_callback_handler_remove();
}

void die(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	deinit_terminal();
	vfprintf(stderr, fmt, args);
	va_end(args);
	exit(EXIT_FAILURE);
}

int validate_input(int ch, int base)
{
	switch (base) {
	case 2:
		if (ch == '0' || ch == '1')
			return 0;
		break;
	case 8:
		if (ch >= '0' && ch <= '7')
			return 0;
		break;
	case 16:
		if ((ch >= '0' && ch <= '9') ||
		     (ch >= 'A' && ch <= 'F') ||
		     (ch >= 'a' && ch <= 'f'))
			 return 0;
		 break;
	case 10:
		 if (isdigit(ch))
			 return 0;
		 break;
	default:
		 break;
	}

	return 1;
}

uint64_t base_scanf(const char *buf, int base)
{
	uint64_t value = 0;
	int ret = 0;

	switch (base) {
	case 10:
		ret = sscanf(buf, "%lu", &value);
		break;
	case 16:
		ret = sscanf(buf, "%lx", &value);
		break;
	case 8:
		ret = sscanf(buf, "%lo", &value);
		break;
	default:
		fprintf(stderr, "Unknown base\n");
		break;
	}

	if (ret == EOF || !ret) {
		die("Couldn't parse parameter\n");
	}

	return value;
}

void lltostr(uint64_t val, char *buf, int base)
{
	switch (base) {
	case 10:
		sprintf(buf, "%lu", val);
		return;
	case 16:
		sprintf(buf, "%lx", val);
		return;
	case 8:
		sprintf(buf, "%lo", val);
		return;
	case 2:
		sprintf(buf, "Not implemeted");
	}
}

int sprintf_size(uint64_t val, char *buf)
{
	int ret;
	double f_val = val;

	if (val >= PB)
		ret = sprintf(buf, "%.2lfPB", f_val / PB);
	else if (val >= TB)
		ret = sprintf(buf, "%.2lfTB", f_val / TB);
	else if (val >= GB)
		ret = sprintf(buf, "%.2lfGB", f_val / GB);
	else if (val >= MB)
		ret = sprintf(buf, "%.2lfMB", f_val / MB);
	else if (val >= KB)
		ret = sprintf(buf, "%.2lfKB", f_val / KB);
	else
		ret = sprintf(buf, "%luB", val);

	return ret;
}

void set_width_by_val(uint64_t val)
{
	if (val & 0xFFFFFFFF00000000)
		g_width = 64;
	else if (val & 0xFFFF0000)
		g_width = 32;
	else if (val & 0xFF00)
		g_width = 16;
	else if (val & 0xFF)
		g_width = 8;
	else
		g_width = 32;
}


int set_width(char width)
{
	int size;

	if (tolower(width) == 'b')
		g_width = 8;
	else if (tolower(width) == 'w')
		g_width = 16;
	else if (tolower(width) == 'l')
		g_width = 32;
	else if (tolower(width) == 'd')
		g_width = 64;
	else
		return 1;

	return 0;
}
