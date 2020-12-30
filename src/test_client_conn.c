#include "client_conn.h"

#define PORT 5555
#define HOST "localhost"

#define MESSAGE "Basic functionality test"

int main(void) {
	NetClient *net_client = net_client_init();
	tetris_connect(net_client, HOST, PORT);

	/* Send data to the server. */
	tetris_send_message(net_client, MESSAGE);
	tetris_disconnect(net_client);

	return 0;
}
