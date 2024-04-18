#include <stdio.h>
#include <sys/socket.h> //for socket(), connect(), send(), recv() functions
#include <arpa/inet.h>  // different address structures are declared here
#include <stdlib.h>     // atoi() which convert string to integer
#include <string.h>
#include <unistd.h> // close() function
#define BUFSIZE 32

int main()
{
    // Create a TCP Socket
    int sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock < 0)
    {
        printf("Error in opening a socket");
        exit(0);
    }
    printf("Client Socket Created\n");

    // Construct Server Address Structure
    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(12345);                    // port number can be changed here
    serverAddr.sin_addr.s_addr = inet_addr("172.17.23.66"); // specify server IP addr here
    printf("Address assigned\n");

    // Establish Connection
    int c = connect(sock, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
    printf("%d\n", c);
    if (c < 0)
    {
        printf("Error while establishing connection");
        exit(0);
    }
    printf("Connection Established\n");

    // send data
    printf("ENTER MESSAGE FOR SERVER (GET/PUT/DEL) \"key\" {value}\n");
    char msg[BUFSIZE];
    gets(msg);
    
    int bytesSent = send(sock, msg, strlen(msg), 0);
    if (bytesSent != strlen(msg))
    {
        printf("Error while sending the message");
        exit(0);
    }
    printf("Data Sent\n");

    // receive data
    char recvBuffer[BUFSIZE];
    int bytesRecvd = recv (sock, recvBuffer, BUFSIZE-1, 0);
    if (bytesRecvd < 0)
    { 
        printf ("Error while receiving data from server");
        exit (0);
    }
    recvBuffer[bytesRecvd] = '\0';
    printf ("%s\n",recvBuffer);

    close(sock);
}