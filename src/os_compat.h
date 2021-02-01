#ifndef TERMINALLY_TETRIS_OS_COMPAT_H
#define TERMINALLY_TETRIS_OS_COMPAT_H

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#define THIS_IS_WINDOWS
// A bad hack, since rand_r is thread safe, and rand is not.
#ifndef rand_r
#define rand_r(n) rand()
#endif
#else
#define THIS_IS_NOT_WINDOWS
typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;
#endif

void last_error_message_to_buffer(char *buffer, unsigned int max_length);

#endif // TERMINALLY_TETRIS_OS_COMPAT_H
