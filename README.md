# Mini Project: Client/Server Applications with Sockets

## Objective
The objective of this mini-project is to develop two client/server applications in C using Unix (Linux) and sockets.

## Project Overview
The project consists of two main parts:
1. A connectionless mode (UDP) client/server application
2. A connected mode (TCP) client/server application with multiple services and a C-based GUI

## Part 1: UDP Application

### Components

- `UDP`: UDP directory

#### Client Program (clientUDP.c)
- Takes server address (host name and port number) as arguments
- Sends a random number n (between 1 and NMAX) to the server
- Displays server response

#### Server Program (serveurUDP.c)
- Takes port number as argument
- Receives number n from client
- Sends n random numbers back to client

#### Build Automation
- Shell script for compilation and linking automation

## Part 2: TCP Application

- `TCP-IP`: TCP directory

### Client Features

#### Authentication
- Username/password validation with server

#### Interactive Menu
- View available services
- Select and request services
- View service results
- Exit option

### Server Services
1. Current date and time retrieval
2. Directory file listing
3. File content retrieval
4. Connection duration tracking

### Development Phases
1. `TCP-IP/MonoClient+MonoServer` Single client/single server implementation
2. `TCP-IP/MultiClient+MonoServer` Multi-client/single server expansion
3. `TCP-IP/MultiClient+MultiServer` Multi-client/multi-server architecture
4. `src/`GUI integration

### Build Scripts
- `CompileUDP.sh`: Compilation script for UDP components
- `CompileTCP.sh`: Compilation script for TCP components

### How to run GUI (for MultiClient and MultiServer TCP C/S communication) ? 

* To see the GUI screenshots check the `Documentation+Screenshots.pdf`

1- Run all 5 servers ( under `unixminiproject/TCP-IP/MultiClient+MultiServer/` ) 

- `./server_1 8081`
- `./server_2 8082`
- `./server_3 8083`
- `./server_4 8084`
- `./load 8080 8081 8082 8084`
  
2- ( under `unixminiproject` ) 
- `make clean && make build && make run`
- Try `make clean && make build && make run_saber` in case of an error with the `make run` command

( Possibily of running other clients on the terminal instead of GUI, under `unixminiproject/TCP-IP/MultiClient+MultiServer/` ) : 
- `./client 127.0.0.1 8080`

  
## Project Structure
```
unixminiproject/
├── src
├── TCP-IP
├── UDP
├── Documentation+Screenshots.pdf
└── README.md
```

## Building and Running

Check the `Documentation+Screenshots.pdf` for more details.



## Requirements
- Unix/Linux operating system
- GCC compiler
- Raylib and RayGUI for GUI implementation
- Basic understanding of socket programming
- Knowledge of C programming language
