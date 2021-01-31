#include <stddef.h>
#include <string.h>

#include "os_compat.h"

#ifdef THIS_IS_WINDOWS
#include <winsock.h>
#else
#include <errno.h>
#endif

void last_error_message_to_buffer(char *buffer, unsigned int max_length) {
#ifdef THIS_IS_WINDOWS
	wchar_t wide_char_buffer[256];
	FormatMessageW(
	    FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL,
	    GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
	    wide_char_buffer, (sizeof(wide_char_buffer) / sizeof(wchar_t)),
	    NULL);
	wcstombs(buffer, wide_char_buffer, max_length);
#else
	strncpy(buffer, strerror(errno), max_length);
#endif
	// ensure null termination
	buffer[max_length - 1] = 0;
}