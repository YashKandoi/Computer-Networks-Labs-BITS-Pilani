#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define MAXPENDING 5
#define BUFFERSIZE 32

int main()
{
    // Create a TCP Socket
    int n;
    int serverSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket < 0)
    {
        printf("Error while server socket creation");
        exit(0);
    }
    printf("Server Socket Created\n");

    // Construct Local Address Structure
    struct sockaddr_in serverAddress, clientAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(12345);
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    printf("Server address assigned\n");

    // Binding
    int temp = bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
    if (temp < 0)
    {
        printf("Error while binding\n");
        exit(0);
    }
    printf("Binding successful\n");

    // Listening
    int temp1 = listen(serverSocket, MAXPENDING);
    if (temp1 < 0)
    {
        printf("Error in listen");
        exit(0);
    }
    printf("Now Listening\n");

    while(1)
    {
        // Accept Stage
        int clientLength = sizeof(clientAddress);
        int clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddress, &clientLength); // Blocking call

        if (clientLength < 0)
        {
            printf("Error in client socket");
            exit(0);
        }
        printf("Handling Client %s:%d\n", inet_ntoa(clientAddress.sin_addr), ntohs(clientAddress.sin_port));
        int childpid;
        char msg[BUFFERSIZE];
        if ((childpid = fork()) == 0)
        {
            printf ("%s\n","Child created for dealing with client requests");
            close(serverSocket);    // child closes listening(server) socket
            while ((n = recv(clientSocket, msg, BUFFERSIZE,0)) > 0)
            {
                // Receiving Message
                if (strcmp(msg, ":exit") == 0)
                {
                    printf("Disconnected from %s:%d\n", inet_ntoa(clientAddress.sin_addr),
                        ntohs(clientAddress.sin_port));
                    break;
                }
                else
                {
                    // Sending Message
                    puts(msg);
                    send(clientSocket, msg, n, 0);
                    bzero(msg, sizeof(msg));
                }
            }
            if (n < 0)
                printf("%s\n", "Read error");
            exit(0);    // child terminates
        }
        close(clientSocket); // parent closes connected socket
    }
    // close(serverSocket);
}