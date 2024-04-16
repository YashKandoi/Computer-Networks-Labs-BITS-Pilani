#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define BUFLEN 512 //Max length of buffer
#define PORT 8888 //The port on which to send data

void die(char* s){
    perror(s);
    exit(1);
}

int main(void){
    // create socket
    int sock=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
    if(sock<0){
        die("socket");
    }
    printf("Client Socket Created\n");


    // construct server address
    struct sockaddr_in si_other;
    int slen=sizeof(si_other);
    memset((char*)&si_other, 0, sizeof(si_other));
    si_other.sin_family=AF_INET;
    si_other.sin_port=htons(PORT);
    si_other.sin_addr.s_addr=inet_addr("172.17.23.66");

    // sending and receiving messages
    char buf[BUFLEN];
    char message[BUFLEN];
    while(1){
        printf("Enter message");
        gets(message);

        // send the message
        int send_message=sendto(sock,message, strlen(message),0,(struct sockaddr *) &si_other, sizeof(si_other));
        if(send_message < 0){
            die("send message");
        }

        // receiving reply, clear the buffer by filling null, it might have previously received data
        memset(buf,'\0',BUFLEN);

        // receiving some data, this is a blocking call
        int recv_message=recvfrom(sock, buf, BUFLEN, 0,(struct sockaddr *) &si_other, &slen);
        if(recv_message<0){
            die("receiving message");
        }

        puts(buf);
    }
    close(sock);
    return 0;
}