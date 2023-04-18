Roux Institute | Spring 2023 | CS5700 Computer Networking <br>
HW4 Communicating States Across the Network <br>
file: README.md <br>
Apr 15, 2023 | by Chun Sheung Ng <br>
----

# CS5700 Computer Networking (Tic-Tac-Toe in C)

This is a simple command-line based Tic-Tac-Toe game implemented in C, allowing two players to compete in a classic 3x3 grid. The game supports client-server connections, allowing players to play remotely over a network.<br>

## Table of Contents
- Execution Commands
- Video Walkthrough
- Usage
- Game Features
- Contributing
- License

## Execution Commands
For compiling: make<br>
For executing server: ./ttt -s [port]<br>
For executing client: ./speak -c [hostname] [port]<br>
For removing executable: make clean<br>

## Video Walkthrough
Here's a walkthrough of the implemented program:
<!-- ![](ttt_gameplay_showcase.gif) -->
<img src="ttt_gameplay_showcase.gif" width="550" height="500"/>

## Usage
To start the game, one player needs to run the server mode, and the other player should run the client mode.

### Server Mode
To start the server, use the following command:
```c
./ttt -s [port]
```
If no port is specified, the server will use the default port 5131.

### Client Mode
To connect to the server, use the following command:
```c
./ttt -c [hostname] [port]
```
If no hostname is specified, the client will use the hostname of the device on which it is running. If no port is specified, the client will use the default port 5131.

## Game Features (Actions in the Game)
- Classic 3x3 Tic-Tac-Toe gameplay
- Server-client architecture for remote play
- Turn-based game logic
- Symbol and turn negotiations between players
- Board updates and winner determination
- Replay and reset functionality for multiple games
- Graceful termination and error handling

### Getting Started
To start playing the game, first, make sure you have followed the installation instructions above. After setting up the server and client, the game will begin with an empty board.<br>

The server will ask the client whether they want to play as 'X' (player 1) or 'O' (player 2). After the client's response, the server will start the game, and the player with 'X' will make the first move.<br>

Players will take turns entering the row and column numbers (1, 2, or 3) of their desired move location. The game will validate the input and update the board accordingly. If a player attempts an invalid move, the game will prompt for a new move.<br>

### Winning Conditions
A player wins by having three of their symbols (either 'X' or 'O') in a row, column, or diagonal. The game will check for a winner after each move, and if a winning condition is met, the game will announce the winner and end the current session (but not the entire program).<br>

If there are no more available moves on the board and no winner is found, the game will declare a draw.<br>

### Replay and Reset
After a game has ended, the server will prompt the client to decide if they want to play another game. If the players agree, the game will clear the board, reset the game state, and start a new game. If the players decide not to play another game, the game will terminate gracefully.<br>

### Error Handling and Termination
The program handles errors and unexpected disconnections by printing an error message and exiting the application.<br>
Both server and client sides implement error-handling mechanisms to ensure a clean termination.<br>
Additionally, resources such as sockets are properly closed before the application exits, preventing potential resource leaks and allowing the operating system to reclaim resources immediately.<br>
By incorporating these features, the Tic-Tac-Toe program provides a reliable and user-friendly experience. <br>

## Contributing
Pull requests are welcome. For major changes, please open an issue first to discuss what you would like to change.<br>

## License
Copyright [2023] [Chun Sheung Ng]<br>
