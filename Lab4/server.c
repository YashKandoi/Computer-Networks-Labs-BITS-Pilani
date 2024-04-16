#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define BUFLEN 512 //Max length of buffer
#define PORT 8888 //The port on which to listen for incoming data

void die(char* s){
    perror(s);
    exit(1);
}

int main(void){

    // create a UDP Socket
    int sock=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(sock<0){
        die("socket");
    }

    // construct local address structure
    struct sockaddr_in si_me, si_other;
    int slen=sizeof(si_other);
    memset((char *) &si_me, 0, sizeof(si_me)); // zero out the structure
    si_me.sin_family=AF_INET;
    si_me.sin_port = htons(PORT);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);

    // Binding
    int temp=bind(sock, (struct sockaddr*)&si_me,sizeof(si_me));
    if(temp<0){
        die("bind");
    }
    printf("Binding successful\n");

    // Listening, no need to establish connection
    char buf[BUFLEN];
    while(1){
        printf("Waiting for data");
        fflush(stdout);

        memset(buf,'\0',BUFLEN);
        // trying to receive data, this is a blocking call
        int recv_len=recvfrom(sock, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen);
        if(recv_len<0){
            die("receiving");
        }

        //print details of the client/peer and the data received
        printf("Received packet from %s:%d\n", inet_ntoa(si_other.sin_addr),ntohs(si_other.sin_port));
        printf("Data: %s\n" , buf);

        // replying
        int reply_len=sendto(sock,buf, recv_len, 0, (struct sockaddr*) &si_other, sizeof(si_other));
        if(reply_len<0){
            die("sendto()");
        }
    }
    close(sock);
    return 0;
}