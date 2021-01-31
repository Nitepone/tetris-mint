#include "os_compat.h"
#ifdef THIS_IS_WINDOWS
#include <windows.h>
#else
#include <sys/ioctl.h>
#endif

#include "terminal_size.h"

TerminalSize get_terminal_size() {
	TerminalSize size;

#ifdef THIS_IS_WINDOWS
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
	size.columns = csbi.srWindow.Right - csbi.srWindow.Left + 1;
	size.rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
#else
	struct winsize ws;
	ioctl(0, TIOCGWINSZ, &ws);
	size.columns = ws.ws_col;
	size.rows = ws.ws_row;
#endif

	return size;
}