#ifndef _CLIENT_CONN_H
#define _CLIENT_CONN_H

#include <sys/types.h>

#include "controller.h"
#include "list.h"
#include "player.h"
#include "tetris_game.h"

typedef struct ttetris_netclient NetClient;

struct ttetris_netclient {
	/* the current file descriptor */
	int fd;
	// mark whether the listening thread is running. There is no "undefined"
	// value to store for pthread_t, so we need this.
	char is_listen_thread_started;
	/* thread for listening to the server */
	pthread_t listen_thread;
	/* optional list of online players */
	List *online_players;
	/* optional field to use for callbacks */
	Player *player;
	List *open_requests;
};

typedef struct ttetris_netrequest NetRequest;

struct ttetris_netrequest {
	/* id should uniquely identify a request. ids can be re-used after the
	request is considered to be completed */
	short id;
	/* pointer to response content */
	char *cursor;
	/* flag to indicate that we got a response to our request */
	int ready_flag;
	/* mutex for the ready flag */
	pthread_mutex_t ready_mutex;
	pthread_cond_t ready_cond;
};

/**
 * send a message to the server using the given client connection
 */
NetRequest *ttetris_net_request(NetClient *client, char *bytes,
                                u_int16_t nbytes);

/**
 * set the response for a given net request
 *
 * Note: This is mostly an internal-only function. This should be called by a
 separate thread that is listening to the
 * TCP network socket.
 */
void ttetris_net_request_complete(NetRequest *request);

/**
 * block until a response is received for the given request
 */
void ttetris_net_request_block_for_response(NetRequest *request);

NetClient *net_client_init();

void tetris_send_message(NetClient *net_client, char *body);

void tetris_connect(NetClient *net_client, char *host, int port);

void tetris_disconnect(NetClient *net_client);

StringArray *tetris_list(NetClient *net_client);

void tetris_listen(NetClient *net_client);

void tetris_register(NetClient *net_client, char *username);

void tetris_opponent(NetClient *net_client, char *username);

TetrisControlSet tcp_control_set(void);

#endif
