// TCP Socket Implementation

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <fcntl.h>

int main(void){

    int sockfd = 0;
    int bytesReceived = 0;
    char recvBuff[256];
    unsigned char buff_offset[10]; // buffer to send the File offset value
    unsigned char buff_command[2]; // buffer to send the Complete File (0) or Partial File Command (1).
    int offset; // required to get the user input for offset in case of partial file command
    int command; // required to get the user input for command
    memset(recvBuff, '0', sizeof(recvBuff));
    struct sockaddr_in serv_addr;

    // creating a socket
    sockfd=socket(AF_INET,SOCK_STREAM,0);
    if(sockfd<0){
        printf("\n Error : Could not create socket \n");
        return 1;
    }

    // initialize sockaddr in data structure
    serv_addr.sin_family=AF_INET;
    serv_addr.sin_port= htons(5001);
    serv_addr.sin_addr.s_addr=inet_addr("172.17.23.66");

    // attempt a connection
    if(connect(sockfd,(struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
        printf("\n Error : Connect Failed \n");
        return 1;
    }

    // creating a file where data will be stored
    FILE* fp;
    fp=fopen("destination_file.txt","ab"); //Open for append in binary mode. Data is added to the end of the file. If the file does not exist, it will be created.
    if(fp==NULL){
        printf("Error opening file");
        return 1;
    }
    fseek(fp, 0, SEEK_END);
    offset=ftell(fp);
    fclose(fp);
    fp = fopen("destination_file.txt", "ab");
    if(fp==NULL){
        printf("Error opening file");
        return 1;
    }

    // user input
    printf("Enter (0) to get complete file, (1) to specify offset, (2) calculate the offset value from local file\n");
    scanf("%d", &command);
    sprintf(buff_command, "%d", command);
    write(sockfd, buff_command, 2);

    if(command ==1 || command==2){
        if(command ==1){
            printf("Enter the value of File offset\n");
            scanf("%d", &offset);
        }
        // otherwise offset = size of local partial file, that we have already calculated
        sprintf(buff_offset, "%d", offset);
        /* sending the value of file offset */
        write(sockfd, buff_offset, 10);
    }

    while((bytesReceived = read(sockfd, recvBuff, 256)) > 0)
    {
        printf("Bytes received %d\n",bytesReceived);
        // recvBuff[n] = 0;
        fwrite(recvBuff, 1,bytesReceived,fp);
        // printf("%s \n", recvBuff);
    }
    if(bytesReceived < 0)
    {
        printf("\n Read Error \n");
    }
    close(sockfd);
    return 0;

}