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
    int n, flag = 0;
    char key[256];
    char value[256];
    char line[256];
    char put_buffer[256];
    char get_buffer[256];
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

    while (1)
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
            printf("%s\n", "Child created for dealing with client requests");
            close(serverSocket); // child closes listening(server) socket
            FILE *fp;
            fp = fopen("database.txt", "ab");
            if (fp == NULL)
            {
                printf("Error opening file");
                return 1;
            }
            while ((n = recv(clientSocket, msg, BUFFERSIZE, 0)) > 0)
            {
                // Receiving Message
                if (strcmp(msg, "Bye") == 0)
                {
                    send(clientSocket, "GoodBye", n, 0);
                    printf("Disconnected from %s:%d\n", inet_ntoa(clientAddress.sin_addr),
                           ntohs(clientAddress.sin_port));
                    break;
                }
                char operation[3]; // GET, PUT, DEL
                for (int i = 0; i < 3; i++)
                {
                    operation[i] = msg[i];
                }
                // space is at i=3
                // key is from i=3 to space
                int i;
                for (i = 4;; i++)
                {
                    if (msg[i] == ' ' || msg[i] == '\0')
                        break;
                    key[i - 4] = msg[i];
                }
                FILE *fp;

                switch (operation[0])
                {
                case 'G':
                    // GET Operation
                    printf("The operation is :%s\n", operation);
                    fp = fopen("database.txt", "r");
                    if (fp == NULL)
                    {
                        printf("Error opening file");
                        return 1;
                    }
                    flag = 0;
                    while (fgets(line, sizeof(line), fp))
                    {
                        // Check if the line contains the key
                        if (strstr(line, key) != NULL)
                        {
                            // match found
                            flag = 1;
                            printf("Match has been found for the given key\n");
                            send(clientSocket, line, strlen(line), 0);
                            break;
                        }
                    }
                    if (!flag)
                    {
                        printf("Match not found for the given key\n");
                        send(clientSocket, "Match not found for the given key\n", 35, 0);
                    }
                    fclose(fp);
                    break;
                case 'D':
                    // DEL Operation if key is found in file
                    printf("The operation is :%s\n", operation);
                    fp = fopen("database.txt", "r+");
                    if (fp == NULL)
                    {
                        printf("Error opening file");
                        return 1;
                    }
                    char tempFile[] = "temp.txt";
                    FILE *temp = fopen(tempFile, "w");
                    if (temp == NULL)
                    {
                        printf("Error creating temp file");
                        return 1;
                    }
                    while (fgets(line, sizeof(line), fp))
                    {
                        // Check if the line contains the key
                        if (strstr(line, key) == NULL)
                        {
                            // Write lines without the key to temp file
                            fprintf(temp, "%s", line);
                        }
                    }
                    fclose(fp);
                    fclose(temp);
                    // Replace the original file with the temp file
                    remove("database.txt");
                    rename(tempFile, "database.txt");
                    send(clientSocket, "OK", strlen("OK"), 0);
                    break;

                case 'P':
                    fseek(fp,0,SEEK_END);
                    printf("The operation is :%s\n", operation);
                    char *start = strchr(msg, ' ') + 1;
                    start = strchr(start, ' ') + 1;    
                    sscanf(start, "%s ", value);
                    // PUT Operation
                    fp = fopen("database.txt", "ab");
                    if (fp == NULL)
                    {
                        printf("Error opening file");
                        return 1;
                    }
                    char line[256];
                    flag = 0;
                    while (fgets(line, sizeof(line), fp))
                    {
                        // Check if the line contains the key
                        if (strstr(line, key) != NULL)
                        {
                            // Match found
                            flag = 1;
                            printf("Error: Match has been found for the given key\n");
                            send(clientSocket, "Put request on an existing key", 31, 0);
                            break;
                        }
                    }
                    if (flag)
                        break;
                    fprintf(fp, "Key: %d, Value: %s\n", atoi(key), value);
                    bzero(put_buffer, 256);
                    sprintf(put_buffer, "OK");
                    send(clientSocket, put_buffer, strlen(put_buffer), 0);
                    fclose(fp);
                    break;
                default:
                    send(clientSocket, "Wrong command entered", sizeof(char) * 22, 0);
                    break;
                }
            }
            if (n < 0)
                printf("%s\n", "Read error");
            exit(0); // child terminates
        }
        close(clientSocket); // parent closes connected socket
    }
    // close(serverSocket);
}