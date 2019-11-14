#include "client_conn.h"

#define PORT 5555
#define HOST "localhost"

#define MESSAGE "Basic functionality test"

int main(void) {
	tetris_connect(HOST, PORT);

	/* Send data to the server. */
	tetris_send_message(MESSAGE);
	tetris_disconnect();

	return 0;
}
