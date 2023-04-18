// Roux Institute | Spring 2023 | CS5700 Computer Networking
// HW4 Communicating States Across the Network
// file: tic-tac-toe.c
// Apr 15, 2023 | by Chun Sheung Ng

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
// #include <pthread.h>
#include "tic-tac-toe.h"

// #define DEFAULT_PORT 5131
// #define BUFFER_SIZE 1024

// initialize the game board and status variables
char board[BOARD_SIZE][BOARD_SIZE];
char buffer[BUFFER_SIZE];
int row, col;
int is_server = 0, is_client = 0, game_over = 0, move = 0;

// thread_arg_t global_thread_arg;
// pthread_mutex_t input_mutex = PTHREAD_MUTEX_INITIALIZER;
// pthread_cond_t input_cond = PTHREAD_COND_INITIALIZER;
// bool input_ready = true;

/* main function */
int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s -s|-c [hostname] [port]\n", argv[0]);
        return 1;
    }

    // Register the signal handler
    signal(SIGINT, signal_handler);

    int port = DEFAULT_PORT;
    if (argc >= 3) {
        port = atoi(argv[2]);
    }

    if (strcmp(argv[1], "-s") == 0) {
        server_mode(port);
    } else if (strcmp(argv[1], "-c") == 0) {
        const char *hostname = "127.0.0.1";
        if (argc >= 3) {
            hostname = argv[1];
        }
        client_mode(hostname, port, board);
    } else {
        printf("Invalid option. Use -s for server mode or -c for client mode.\n");
        return 1;
    }

    return 0;
}

/**
 * server connection to and interactions with client
*/
int server_mode(int port) {
    // Implement server-side logic for the tic-tac-toe game
    printf("Running in server mode on port %d\n", port);

    // 1. Create a socket
    int socket_desc = socket(AF_INET, SOCK_STREAM, 0);

    // 2. Set up server address and port number + Bind the socket to the port
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Bind to the set port and IP:
    if (bind(socket_desc, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        printf("Couldn't bind to the port\n");
        return -1;
    }
    printf("Done with binding\n");

    // 3. Listen for incoming connections
    if (listen(socket_desc, 1) == -1) {
        perror("listen");
        return 1;
    }
    printf("Waiting for a client to connect...\n");

    // 4. Accept a client connection
    struct sockaddr client_addr;
    socklen_t client_size = sizeof(client_addr);
    int client_sock = accept(socket_desc, (struct sockaddr*)&client_addr, &client_size);
    if (client_sock == -1) {
        perror("accept");
        return 1;
    }
    printf("Client connected\n");

    // 5. Communicate with the client (send and receive data)
    
    // Game logic and communication for the server side
    char server = 'O', opponent = 'X';
    clear_board(board);
    server_start_game(client_sock, buffer, &server, &opponent);

    while (!game_over) {
        if (is_client && !is_server) {
            printf("Server waiting for Client's move...\n\n");
            if (recv(client_sock, buffer, BUFFER_SIZE, 0) <= 0) {
                perror("Failed to receive move from client");
                exit(EXIT_FAILURE);
            }
            // printf("sscanf() client input\n");
            sscanf(buffer, "%d %d", &row, &col);
            board[row][col] = opponent;
            move++;

            // print updated board
            print_board(board);

            // update game status
            game_over = check_winner(board, opponent);
            if (game_over == 1) {
                check_game_state(client_sock, socket_desc, opponent);
                if (server_request_new_game(client_sock, buffer, board, &server, &opponent) == -1) {
                    break;
                }
            }

            // check draw game condition
            if (move >= 9) {
                handle_draw(client_sock, socket_desc);
                if (server_request_new_game(client_sock, buffer, board, &server, &opponent) == -1) {
                    break;
                }
            }

            // Change game state
            is_client = 0;
            is_server = 1;
            // Clear the input buffer
            memset(buffer, '\0', BUFFER_SIZE);

        } else if (!is_client && is_server) {
            // server turn: handle server turn -> scanf, check and send -> board updated
            handle_server_turn(board, server);
            move++;

            // print updated board
            print_board(board);
            game_over = check_winner(board, server);
            if (game_over == 1) {
                check_game_state(client_sock, socket_desc, server);
                if (server_request_new_game(client_sock, buffer, board, &server, &opponent) == -1) {
                    break;
                }
            }

            // check draw game condition
            if (move >= 9) {
                handle_draw(client_sock, socket_desc);
                if (server_request_new_game(client_sock, buffer, board, &server, &opponent) == -1) {
                    break;
                }
            }

            // continue to send board if game continues
            send(client_sock, board, BOARD_SIZE*BOARD_SIZE, 0);

            // Change game state
            is_client = 1;
            is_server = 0;
            // Clear the input buffer
            memset(buffer, '\0', BUFFER_SIZE);
            
        } else {
            printf("Invalid Game State. Please restart the game...\n");
            break;
        }
    }
    
    // 6. Close the connection
    printf("Closing connection from server side....\n");
    close(client_sock);
    close(socket_desc);

    return 0;
}

/**
 * client connection to and interactions with server
*/
int client_mode(const char *hostname, int port, char board[BOARD_SIZE][BOARD_SIZE]) {
    // Implement client-side logic for the tic-tac-toe game
    printf("Running in client mode, connecting to %s on port %d\n", hostname, port);

    // 1. Create a socket
    int socket_desc = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(hostname);

    // 2. Connect to the server
    if (connect(socket_desc, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Failed to connect");
        return 1;
    }
    printf("Connected to server\n");

    // 3. Communicate with the server (send and receive data)

    // Game logic and communication for the client side
    char player = 'X', opponent = 'O';
    clear_board(board);
    char answer = client_start_game(socket_desc, buffer, &player, &opponent);

    // global_thread_arg = create_thread_arg(socket_desc, buffer, board);
    // pthread_t receiver_thread; // thread for continuous listening to server
    // pthread_create(&receiver_thread, NULL, receive_messages, (void *)&global_thread_arg);

    while (!game_over) {

        if (answer == 'N') {
            printf("Waiting for the Server's first move...\n\n");
            // waiting for the end of server's turn
            receive_messages(socket_desc, buffer, board);
            answer = '#';
        }
        // parse client input and send it to the server
        handle_client_turn(socket_desc, board, player, buffer);
        // waiting for the end of server's turn
        receive_messages(socket_desc, buffer, board);

        if (sscanf(buffer, "Player %c wins!\n", &player) == 1 || sscanf(buffer, "Player %c wins!\n", &opponent) == 1) {
            send(socket_desc, ACK_MSG, strlen(ACK_MSG), 0);
            answer = client_reply_new_game(socket_desc, buffer, board, &player, &opponent);
            if (answer == ' ') {
                break;
            }
            // Clear the input buffer
            memset(buffer, '\0', BUFFER_SIZE);
        }

        if (is_board_full(board)) {
            printf("DRAW! Try another one...\n");
            send(socket_desc, ACK_MSG, strlen(ACK_MSG), 0);
            answer = client_reply_new_game(socket_desc, buffer, board, &player, &opponent);
            if (answer == ' ') {
                break;
            }
            // Clear the input buffer
            memset(buffer, '\0', BUFFER_SIZE);
        }
    }

    // 4. Close the connection
    printf("Closing connection from client side....\n");
    close(socket_desc);
    return 0;
}

/**
 * negotiating roles for server and client
*/
void assign_player_symbols(char *cross, char *circle) {
    // Assign symbols to player and opponent
    *cross = 'X';
    *circle = 'O';
}

/**
 * starting steps for server 
*/
void server_start_game(int client_socket, char buffer[BUFFER_SIZE], char *server, char *opponent) {
    printf("\nGame Starts!\n");
    print_board(board);

    game_over = 0, move = 0;
    char *start_msg = "player 1 takes 'X' and player 2 takes 'O'.\nDo you want to start first [Y/N]: \t";
    send(client_socket, start_msg, strlen(start_msg), 0);

    while (1) {
        if (recv(client_socket, buffer, BUFFER_SIZE, 0) <= 0) {
            printf("Server disconnected or an error occurred.\n");
            exit(1);
        }
        printf("Answer from Client: %s\n", buffer);

        if (buffer[0] == 'Y') {
            is_client = 1;
            is_server = 0;
            assign_player_symbols(opponent, server);
            // Clear the input buffer
            memset(buffer, '\0', BUFFER_SIZE);
            // send client ACK message
            send(client_socket, ACK_MSG, BUFFER_SIZE, 0);
            break;
        } else if (buffer[0] == 'N') {
            is_client = 0;
            is_server = 1;
            assign_player_symbols(server, opponent);
            // Clear the input buffer
            memset(buffer, '\0', BUFFER_SIZE);
            // send client ACK message
            send(client_socket, ACK_MSG, BUFFER_SIZE, 0);
            break;
        } else {
            printf("Waiting for the re-input from client...\n");
            start_msg = "Type 'Y' if you want to start first, or otherwise, type 'N'.\n";
            send(client_socket, start_msg, strlen(start_msg), 0);
        }
    }
}

/**
 * Starting steps on client side
*/
char client_start_game(int socket, char buffer[BUFFER_SIZE], char *player, char *opponent) {
    char answer;
    game_over = 0, move = 0;
    while (1) {
        // Clear the input buffer
        memset(buffer, '\0', BUFFER_SIZE);

        // Server's question message
        if (recv(socket, buffer, BUFFER_SIZE, 0) <= 0) {
            printf("Disconnected or an error occurred.\n");
            exit(1);
        }

        // Remove the trailing newline character
        buffer[strcspn(buffer, "\t")] = '\0';

        if (strcmp(buffer, ACK_MSG) == 0) {
            // Clear the input buffer
            memset(buffer, '\0', BUFFER_SIZE);
            // print start message
            printf("\nGame Starts!\n");
            print_board(board);
            return answer;
        }

        printf("Hi new challenger, %s\n", buffer);
        // get user input
        fgets(buffer, BUFFER_SIZE, stdin);
        // Remove the trailing newline character
        buffer[strcspn(buffer, "\n")] = '\0';
        
        answer = buffer[0];
        if (answer == 'Y') {
            assign_player_symbols(player, opponent);
        } else if (answer == 'N') {
            assign_player_symbols(opponent, player);
        }
        // send answer to server
        send(socket, buffer, BUFFER_SIZE, 0);
    }
    return answer;
}

/**
 * Check if the string is the board or other messages
*/
bool is_board_string(const char *str) {
    // check if the size is 3*3
    if (strlen(str) != BOARD_SIZE * BOARD_SIZE) {
        return false;
    }
    // check if char was only 'X', 'O' or ' '
    for (int i = 0; i < BOARD_SIZE * BOARD_SIZE; i++) {
        if (str[i] != 'X' && str[i] != 'O' && str[i] != ' ') {
            return false;
        }
    }
    return true;
}

/**
 * Check if the board is full
*/
bool is_board_full(char board[BOARD_SIZE][BOARD_SIZE]) {
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (board[i][j] == ' ') {
                return false;
            }
        }
    }
    return true;
}

/**
 * receive messages from the server to the client
*/
void receive_messages(int socket, char buffer[BUFFER_SIZE], char board[][BOARD_SIZE]) {

    if (recv(socket, buffer, BUFFER_SIZE, 0) <= 0) {
        printf("Server disconnected or an error occurred.\n");
        exit(1);
    }
    if (is_board_string(buffer)) {
        for (int i = 0; i < BOARD_SIZE; i++) {
            for (int j = 0; j < BOARD_SIZE; j++) {
                board[i][j] = buffer[i * BOARD_SIZE + j];
            }
        }
        printf("Server made its move: \n");
        // print received board state
        print_board(board);
    } else {
        printf("Server message received: %s\n", buffer);
    }

    // close(socket);
    // pthread_exit(NULL);
}

/**
 * Check if the player input format is valid or not
*/
bool is_valid_input_format(char *input, int row, int col) {
    printf("Validating input...\n"); // test input
    // Check if the input is in the format of "%d %d"
    if (sscanf(input, "%d %d", &row, &col) == 2) {
        printf("Valid input: row = %d, col = %d\n", row, col);
        return true;
    } else {
        printf("Invalid input. Enter with the valid format: \"[row] [column]\": ");
        return false;
    }
}

/**
 * Check if the player move is valid or not
*/
bool is_valid_move(char player, int row, int col) {
    if (row >= 0 && row < BOARD_SIZE && col >= 0 && col < BOARD_SIZE)
    {
        if (board[row][col] == ' ')
        {
            return true;
        }
        else
        {
            printf("Player %c, this place was taken. Enter your move again: ", player);
            return false;
        }
    }
    printf("Player %c, enter your move again within the board size (%d * %d): ", player, BOARD_SIZE, BOARD_SIZE);
    return false;
}

/**
 * Handle turn for client, update the board and send to another player
*/
void handle_client_turn(int server_socket, char board[][BOARD_SIZE], char player, char buffer[BUFFER_SIZE]) {
    int row, col;
    // prompt client to input
    printf("Hi player %c, please enter your move: ", player);
    fgets(buffer, BUFFER_SIZE, stdin);
    sscanf(buffer, "%d %d", &row, &col);
    // sprintf(buffer, "%d %d", row, col);
    
    while (!is_valid_input_format(buffer, row, col) || !is_valid_move(player, row, col)) {
        // Get user input
        fgets(buffer, BUFFER_SIZE, stdin);
        sscanf(buffer, "%d %d", &row, &col);
    }

    // Remove the trailing newline character
    buffer[strcspn(buffer, "\n")] = '\0';

    // print the text entered and board
    printf("You entered: %s\n", buffer);

    // update the board
    board[row][col] = player;
    print_board(board);

    // reminding message to the player
    printf("Be patient. Waiting for Server's move...\n\n");

    // send updated buffer to server
    send(server_socket, buffer, strlen(buffer), 0);
}

/**
 * Handle turn for server, update the board and send to another player
*/
void handle_server_turn(char board[][BOARD_SIZE], char player) {
    int row, col;
    char buffer[BUFFER_SIZE];

    printf("Hi Server, enter your move (board size: %d * %d): ", BOARD_SIZE, BOARD_SIZE);
    scanf("%d %d", &row, &col);
    sprintf(buffer, "%d %d", row, col);
    
    while (!is_valid_input_format(buffer, row, col) || !is_valid_move(player, row, col)) {
        // Clear the input buffer
        int c;
        while ((c = getchar()) != '\n' && c != EOF);

        // printf("Player %c, enter your move again with the valid format: \"[row] [column]\": ", player);
        scanf("%d %d", &row, &col);
        sprintf(buffer, "%d %d", row, col);
    }

    // modify the board according to player input
    board[row][col] = player;

    // send buffer to another player
    // send(socket, board, BOARD_SIZE*BOARD_SIZE, 0);
}

/**
 * Check the game state to find winners
*/
void check_game_state(int client_sock, int socket_desc, char player) {
    char result[BUFFER_SIZE];
    sprintf(result, "Player %c wins!\n", player);
    printf("%s\n", result);
    send(client_sock, result, BUFFER_SIZE, 0);

    // receive updated board from server
    if (recv(client_sock, buffer, strlen(ACK_MSG), 0) <= 0) {
        perror("Failed to receive ACK from client");
        close(socket_desc);
        exit(EXIT_FAILURE);
    }
    
    if (strcmp(buffer, ACK_MSG) == 0) {
        printf("Client acknowledged.\n\n");
    } else {
        printf("Wrong ACK message from client");
    }
}

/**
 * handle draw situation by printing message and sending it to client
*/
void handle_draw(int client_sock, int socket_desc) {
    printf("Draw game.\n");
    send(client_sock, board, BOARD_SIZE*BOARD_SIZE, 0);

    // receive updated board from server
    if (recv(client_sock, buffer, strlen(ACK_MSG), 0) <= 0) {
        perror("Failed to receive ACK from client");
        close(socket_desc);
        exit(EXIT_FAILURE);
    }

    if (strcmp(buffer, ACK_MSG) == 0) {
        printf("Client ACKnowledged\n");
    } else {
        printf("Wrong ACK message from client");
    }
}

/**
 * Check each horizontal, vertical, or diagonal row to see if any player wins
*/
int check_winner(char board[][BOARD_SIZE], char player) {
    // Check rows, columns, and diagonals for a win
    for (int i = 0; i < BOARD_SIZE; i++) {
        if (board[i][0] == player && board[i][1] == player && board[i][2] == player) {
            return 1;
        }
        if (board[0][i] == player && board[1][i] == player && board[2][i] == player) {
            return 1;
        }
        if (board[0][0] == player && board[1][1] == player && board[2][2] == player) {
            return 1;
        }
        if (board[0][2] == player && board[1][1] == player && board[2][0] == player) {
            return 1;
        }
    }
    return 0;
}

/**
 * count the number of moves made by the players
*/
int count_moves(char board[][BOARD_SIZE]) {
    int move = 0;
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (board[i][j] != ' ') {
                move++;
            }
        }
    }
    return move;
}

/**
 * Ask the player to play another game or not
*/
int server_request_new_game(int client_socket, char buffer[BUFFER_SIZE], char board[][BOARD_SIZE], char *server, char *opponent) {
    // Clear the input buffer
    memset(buffer, '\0', BUFFER_SIZE);
    // assign question to string
    char *question = "Do you want to play another game? (Y/N): ";
    send(client_socket, question, strlen(question), 0);

    while (1) {
        if (recv(client_socket, buffer, BUFFER_SIZE, 0) <= 0) {
            printf("Server disconnected or an error occurred.\n");
            exit(1);
        }
        printf("Answer from Client: %s\n", buffer);

        if (buffer[0] == 'Y' || buffer[0] == 'N') {

            if (buffer[0] == 'Y') {
                // send(client_socket, ACK_MSG, strlen(ACK_MSG), 0);
                // Clear the input buffer
                memset(buffer, '\0', BUFFER_SIZE);
                clear_board(board);
                server_start_game(client_socket, buffer, server, opponent);
                return 0;
            } else {
                // snprintf(buffer, BUFFER_SIZE, "Thank you for playing! Exiting...\n");
                buffer = "Thank you for playing! Exiting...\n";
                send(client_socket, buffer, strlen(buffer) + 1, 0);
                return -1;
            }

        } else {
            printf("Invalid input. Please enter Y or N.\n");
        }
    }
}

/**
 * Client reply of playing another game or not
*/
char client_reply_new_game(int server_socket, char buffer[BUFFER_SIZE], char board[][BOARD_SIZE], char *player, char *opponent) {
    char answer = ' ';
    while (1) {
        if (answer == 'Y') {
            // Clear the input buffer
            memset(buffer, '\0', BUFFER_SIZE);
            clear_board(board);
            return client_start_game(server_socket, buffer, player, opponent);
        } else if (answer == 'N'){
            if (recv(server_socket, buffer, BUFFER_SIZE, 0) <= 0) {
                printf("Disconnected or an error occurred.\n");
                exit(1);
            }
            printf("%s\n", buffer);
            return ' ';
        }

        // Clear the input buffer
        memset(buffer, '\0', BUFFER_SIZE);

        // Server's question message
        if (recv(server_socket, buffer, BUFFER_SIZE, 0) <= 0) {
            printf("Disconnected or an error occurred.\n");
            exit(1);
        }
        // print the question asking for a new game
        printf("%s\n", buffer);
        // get user input
        fgets(buffer, BUFFER_SIZE, stdin);
        // Remove the trailing newline character
        buffer[strcspn(buffer, "\n")] = '\0';
        if (strlen(buffer) == 1) {
            answer = buffer[0];
        }
        // send answer to server
        send(server_socket, buffer, BUFFER_SIZE, 0);
    }
}

/**
 * clear board
*/
void clear_board(char board[][BOARD_SIZE]) {
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            board[i][j] = ' ';
        }
    }
}

/**
 * print board for display
*/
void print_board(char board[][BOARD_SIZE]) {
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            printf(COLOR_BLUE "%c " COLOR_RESET, board[i][j]);
            if (j < BOARD_SIZE - 1) {
                printf(COLOR_RED "| " COLOR_RESET);
            }
        }
        printf("\n");
        if (i < BOARD_SIZE - 1) {
            printf(COLOR_RED "---------\n" COLOR_RESET);
        }
    }
}

/**
 * Handle signal to terminate program gracefully
*/
void signal_handler(int signum) {
    printf("\nReceived signal %d. Exiting gracefully...\n", signum);
    exit(EXIT_SUCCESS);
}
