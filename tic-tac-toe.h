// Roux Institute | Spring 2023 | CS5700 Computer Networking
// HW4 Communicating States Across the Network
// file: tic-tac-toe.h
// Apr 15, 2023 | by Chun Sheung Ng

#define DEFAULT_PORT 5131
#define BUFFER_SIZE 1024
#define BOARD_SIZE 3

#define ACK_MSG "ack"
#define REPLAY_MSG "replay"

#define COLOR_RESET "\033[0m"
#define COLOR_RED "\033[31m"
#define COLOR_BLUE "\033[34m"

// typedef struct client_thread_arg {
//     int socket;
//     char recv_buffer[BUFFER_SIZE];
//     char board[BUFFER_SIZE][BOARD_SIZE];
// } thread_arg_t;

int server_mode(int port);

int client_mode(const char *hostname, int port, char board[][BOARD_SIZE]);

void assign_player_symbols(char *player, char *opponent);

void server_start_game(int client_socket, char buffer[BUFFER_SIZE], char *server, char *opponent);

char client_start_game(int socket, char buffer[BUFFER_SIZE], char *player, char *opponent);

bool is_valid_input_format(char *input, int row, int col);

bool is_valid_move(char player, int row, int col);

bool is_board_string(const char *str);

bool is_board_full(char board[BOARD_SIZE][BOARD_SIZE]);

void receive_messages(int socket, char buffer[BUFFER_SIZE], char board[][BOARD_SIZE]);

void handle_client_turn(int server_socket, char board[][BOARD_SIZE], char player, char buffer[BUFFER_SIZE]);

void handle_server_turn(char board[][BOARD_SIZE], char player);

void check_game_state(int client_sock, int socket_desc, char player);

void handle_draw(int client_sock, int socket_desc);

int check_winner(char board[][BOARD_SIZE], char player);

int count_moves(char board[][BOARD_SIZE]);

int server_request_new_game(int client_socket, char buffer[BUFFER_SIZE], char board[][BOARD_SIZE], char *server, char *opponent);

char client_reply_new_game(int client_socket, char buffer[BUFFER_SIZE], char board[][BOARD_SIZE], char *player, char *opponent);

void clear_board(char board[][BOARD_SIZE]);

void print_board(char board[][BOARD_SIZE]);

void signal_handler(int signum);