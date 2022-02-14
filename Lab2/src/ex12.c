#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdbool.h>
#include <sys/un.h>
#include <signal.h>
#include <stdarg.h>

// Prototypes for internal functions and utilities
void error(const char *fmt, ...);
int runClient(char *clientName, int numMessages, char **messages);
int runServer();
void serverReady(int signal);

bool serverIsReady = false;

// Prototypes for functions to be implemented by students
void server();
void client(char *clientName, int numMessages, char *messages[]);

void verror(const char *fmt, va_list argp)
{
    fprintf(stderr, "error: ");
    vfprintf(stderr, fmt, argp);
    fprintf(stderr, "\n");
}

void error(const char *fmt, ...)
{
    va_list argp;
    va_start(argp, fmt);
    verror(fmt, argp);
    va_end(argp);
    exit(1);
}

int runServer(int port) {
    int forkPID = fork();
    if (forkPID < 0)
        error("ERROR forking server");
    else if (forkPID == 0) {
        server();
        exit(0);
    } else {
        printf("Main: Server(%d) launched...\n", forkPID);
    }
    return forkPID;
}

int runClient(char *clientName, int numMessages, char **messages) {
    fflush(stdout);
    printf("Launching client %s...\n", clientName);
    int forkPID = fork();
    if (forkPID < 0)
        error("ERROR forking client %s", clientName);
    else if (forkPID == 0) {
        client(clientName, numMessages, messages);
        exit(0);
    }
    return forkPID;
}

void serverReady(int signal) {
    serverIsReady = true;
}

#define NUM_CLIENTS 2
#define MAX_MESSAGES 5
#define MAX_CLIENT_NAME_LENGTH 10

struct client {
    char name[MAX_CLIENT_NAME_LENGTH];
    int num_messages;
    char *messages[MAX_MESSAGES];
    int PID;
};

// Modify these to implement different scenarios
struct client clients[] = {
        {"Uno", 3, {"Mensaje 1-1", "Mensaje 1-2", "Mensaje 1-3"}},
        {"Dos", 3, {"Mensaje 2-1", "Mensaje 2-2", "Mensaje 2-3"}}
};

int main() {
    signal(SIGUSR1, serverReady);
    int serverPID = runServer(getpid());
    while(!serverIsReady) {
        sleep(1);
    }
    printf("Main: Server(%d) signaled ready to receive messages\n", serverPID);

    for (int client = 0 ; client < NUM_CLIENTS ; client++) {
        clients[client].PID = runClient(clients[client].name, clients[client].num_messages,
                                        clients[client].messages);
    }

    for (int client = 0 ; client < NUM_CLIENTS ; client++) {
        int clientStatus;
        printf("Main: Waiting for client %s(%d) to complete...\n", clients[client].name, clients[client].PID);
        waitpid(clients[client].PID, &clientStatus, 0);
        printf("Main: Client %s(%d) terminated with status: %d\n",
               clients[client].name, clients[client].PID, clientStatus);
    }

    printf("Main: Killing server (%d)\n", serverPID);
    kill(serverPID, SIGINT);
    int statusServer;
    pid_t wait_result = waitpid(serverPID, &statusServer, 0);
    printf("Main: Server(%d) terminated with status: %d\n", serverPID, statusServer);
    return 0;
}



//*********************************************************************************
//**************************** EDIT FROM HERE *************************************
//#you can create the global variables and functions that you consider necessary***
//*********************************************************************************

#define PORT_NUMBER 32814
struct sockaddr_in serverMetaDataFactory();

void client(char *clientName, int numMessages, char *messages[])
{
    char buffer[256];
    //Open the socket
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    //Connect to the server
    struct sockaddr_in serverMetaData;
    serverMetaData = serverMetaDataFactory();
    connect(
        serverSocket,
        &serverMetaData,
        sizeof(serverMetaData)
    );
    //For each message, write to the server and read the response
    for(int i = 0; i < numMessages; i++) {
        strcpy(buffer, messages[i]);
        send(
            serverSocket,
            buffer,
            sizeof(buffer),
            0
        );
        recv(
            serverSocket,
            (void *) buffer,
            sizeof(buffer),
            0
        );
        printf("Client %s(%d): Return message: %s\n", clientName, getpid(), buffer);
    }
    //Close socket
    close(serverSocket);
}

// Server Code
void serverChild(int clientSocket);
void shutdownServer(int signal);

bool serverShutdown = false;
int serverSocketToClose;

// Server Child Process List
int size = 0;
int maxCapacity = 30;
pid_t *serverChildProcesses;
bool processesIsInitialized = false;

void server()
{
    //Handle SIGINT so the server stops when the main process kills it
    signal(SIGINT, shutdownServer);
    //Open the socket
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    //Bind the socket
    struct sockaddr_in serverMetaData = serverMetaDataFactory();
    bind(
        serverSocket,
        (const struct stockaddr*) &serverMetaData,
        sizeof(serverMetaData)
    );
    listen(serverSocket, 10);
    //Signal server is ready
    kill(getppid(), SIGUSR1);

    serverSocketToClose = serverSocket;

    while(1) {
        int clientSocket = accept(serverSocket, NULL, NULL);
        int forkResult = fork();
        if(forkResult == 0){
            serverChild(clientSocket);
            close(clientSocket);
            exit(0);
        }else { // Parent must register server child
            if (!processesIsInitialized) {
                serverChildProcesses = (pid_t *) calloc(maxCapacity, sizeof(pid_t));
                processesIsInitialized = true;
            } else if( size == maxCapacity ) {
                serverChildProcesses = (pid_t *) realloc((void *) serverChildProcesses, sizeof(pid_t) * maxCapacity *2);
                maxCapacity *= 2;
            }
            serverChildProcesses[size++] = forkResult;
        }
    }
}


struct sockaddr_in serverMetaDataFactory() {
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT_NUMBER);
    server_address.sin_addr.s_addr = INADDR_ANY;
    return server_address;
}

void serverChild(int clientSocket) {
    char buffer[256];
    int sleepCounter = 0;
    while(1) {
        while (recv(
            clientSocket,
            buffer,
            sizeof(buffer),
            MSG_DONTWAIT /* Nonblocking IO.  */
        ) <= 0){
            fflush(stdout);
            if (sleepCounter == 10)
                return;
            printf("Child server(%d): sleeping %d...\n", getpid(), sleepCounter++);
            sleep(1);
        }
        printf("Server child(%d): got message: %s\n", getpid(), buffer); //expected output
        sleepCounter = 1;
        send(
            clientSocket,
            buffer,
            sizeof(buffer),
            MSG_DONTWAIT
        );
    }
}

void shutdownServer(int signal) {
    serverShutdown = true;
    if (!processesIsInitialized) exit(0);
    for (int i = 0; i < size; i++) {
        int status;
        waitpid(serverChildProcesses[i], &status, 0);
        printf(
            "Server(%d): child server with pid %d terminated with status: %d\n",
            getpid(),
            serverChildProcesses[i],
            status
        );
    }
    free(serverChildProcesses);
    close(serverSocketToClose);
    exit(0);
}