#define BOARD_WIDTH 10
#define BOARD_HEIGHT 24

void tetris_send_message (char * message);

void tetris_connect(char * host, int port);

void tetris_disconnect();

void tetris_translate(int x, int y);

void tetris_rotate(int theta);

void tetris_drop();

void tetris_listen(int board[BOARD_HEIGHT][BOARD_WIDTH]);

void tetris_register(char * username);
