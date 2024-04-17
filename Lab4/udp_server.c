// UDP Socket Implementation

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

int main(void)
{

    int listenfd = 0;
    int confd = 0;
    struct sockaddr_in serv_addr, client_addr;
    int slen = sizeof(client_addr);
    char sendBuff[1025];
    int numrv;

    // create a UDP socket
    listenfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (listenfd < 0)
    {
        printf("Error in socket");
        exit(1);
    }
    printf("Socket created successfully");
    memset(&serv_addr, '0', sizeof(serv_addr));
    memset(sendBuff, '0', sizeof(sendBuff));

    // structure initialize
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(5001);

    bind(listenfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

    while (1)
    {
        unsigned char offset_buffer[10] = {'\0'};
        unsigned char command_buffer[2] = {'\0'};
        int offset;
        int command;

        printf("Waiting for client to send the command (Full File (0)  Partial File (1)\n");

        if (recvfrom(listenfd, command_buffer, 2, 0, (struct sockaddr *)&client_addr, &slen) < 0)
        {
            printf("Error receiving data\n");
            exit(1);
        }
        sscanf(command_buffer, "%d", &command);
        printf("Received packet from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        printf("Command received %s\n", command_buffer);

        if (command == 0)
            offset = 0;
        else
        {
            printf("Waiting for client to send the offset\n");
            // while (read(connfd, offset_buffer, 10) == 0);
            if (recvfrom(listenfd, offset_buffer, 10, 0, (struct sockaddr *)&client_addr, &slen) < 0)
            {
                printf("Error receiving data\n");
                exit(1);
            }
            sscanf(offset_buffer, "%d", &offset);
        }
        // opening the file which we wish to transfer
        FILE *fp = fopen("source_file.txt", "rb");
        if (fp == NULL)
        {
            printf("File open error");
            return 1;
        }
        /* Read data from file and send it */
        fseek(fp, offset, SEEK_SET);

        while (1)
        {
            // First reading the file in chunks of 256 bytes
            unsigned char buff[255] = {0};
            int nread = fread(buff, 1, 255, fp);
            printf("Bytes read %d \n", nread);

            /* If read was success, send data. */
            if (nread > 0)
            {
                printf("Sending \n");
                if (sendto(listenfd, buff, nread*sizeof(char), 0, (struct sockaddr *)&client_addr, slen) < 0)
                {
                    printf("Error sending data\n");
                    exit(1);
                }
            }
            /*
             * There is something tricky going on with read ..
             * Either there was error, or we reached end of file.
             */
            if (nread < 255)
            {
                if (feof(fp))
                    printf("End of file\n");
                if (ferror(fp))
                    printf("Error reading\n");
                break;
            }
        }
        sleep(1);
    }
    close(listenfd);
    return 0;
}