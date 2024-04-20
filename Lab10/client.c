/* Simple udp server with stop and wait functionality with data packet and ACK packet loss */
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUFLEN 256 // Max length of buffer
#define PORT 8881  // The port on which to send data
#define TIMEOUT 5

typedef enum
{
    true,
    false
} bool;

typedef struct packet1
{
    int sq_no;
    bool isLastACK;
} ACK_PKT;

typedef struct packet2
{
    int sq_no;
    bool isLastPacket;
    char data[BUFLEN];
} DATA_PKT;

void die(char *s)
{
    perror(s);
    exit(1);
}

int main(void)
{
    struct sockaddr_in si_other;
    int s, i, n, slen = sizeof(si_other);
    char buf[BUFLEN];
    char message[BUFLEN];
    DATA_PKT send_pkt;
    ACK_PKT rcv_ack;
    send_pkt.isLastPacket=false;
    rcv_ack.isLastACK=false;

    // creating socket
    if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        die("socket");
    }

    // initialising structure
    memset((char *)&si_other, 0, sizeof(si_other));
    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(PORT);
    si_other.sin_addr.s_addr = inet_addr("127.0.0.1");

    FILE* fp;
    fp=fopen("input.txt","r");
    if(fp==NULL){
        die("file opening");
    }
    int state = 0;
    while (1)
    {
        switch (state)
        {
        case 0:
            if(rcv_ack.isLastACK==true){
                printf("File has finished transferring\n");
                return 1;
            }
            printf("Enter message 0: "); // wait for sending packet with seq. no. 0
            memset(send_pkt.data,'\0',sizeof(send_pkt.data));
            fgets(send_pkt.data,BUFLEN,fp);
            // fgets(send_pkt.data, sizeof(send_pkt), stdin);
            send_pkt.sq_no = 0;
            if (feof(fp)){
                printf("End of file\n");
                send_pkt.isLastPacket=true;
            }
            else{
                send_pkt.isLastPacket=false;
            }
            if (ferror(fp))
                printf("Error reading\n");

            if (sendto(s, &send_pkt, sizeof(send_pkt), 0, (struct sockaddr *)&si_other, slen) == -1)
            {
                die("sendto()");
            }
            state = 1;
            
            break;

        case 1:
        {
            // waiting for ACK 0
            fd_set rcv_set;

            struct timeval tv;

            // Clear the fd_set
            FD_ZERO(&rcv_set);
            // Add a descriptor to an fd_set
            FD_SET(s, &rcv_set);

            tv.tv_sec = TIMEOUT;
            tv.tv_usec = 0;

            if ((n = select(s + 1, &rcv_set, NULL, NULL, &tv)) < 0)
            {
                die("error on select");
            }

            if (n == 0)
            {
                // timout expired, send packet again
                printf("Timeout occured for ACK 0\n");
                if (sendto(s, &send_pkt, sizeof(send_pkt), 0, (struct sockaddr *)&si_other, slen) == -1)
                {
                    die("sendto()");
                }
                break;
            }

            // ACK arrived
            if (recvfrom(s, &rcv_ack, sizeof(rcv_ack), 0, (struct sockaddr *)&si_other, &slen) == -1)
            {
                die("recvfrom()");
            }
            if (rcv_ack.sq_no == 0)
            {
                // ACK 0 arrived, change state
                printf("Received ack seq. no. %d\n", rcv_ack.sq_no);
                state = 2;
            }
            else
                break;
        }
        case 2:
                if(rcv_ack.isLastACK==true){
                    printf("File has finished transferring\n");
                    return 1;
                }
            printf("Enter message 1: "); // wait for sending packet with seq. no. 1
            // fgets(send_pkt.data, sizeof(send_pkt), stdin);
            memset(send_pkt.data,'\0',sizeof(send_pkt.data));
            fgets(send_pkt.data,BUFLEN,fp);
            send_pkt.sq_no = 1;
            if (sendto(s, &send_pkt, sizeof(send_pkt), 0, (struct sockaddr *)&si_other, slen) == -1)
            {
                die("sendto()");
            }
            state = 3;
            if (feof(fp)){
                printf("End of file\n");
                send_pkt.isLastPacket=true;
            }
            else{
                send_pkt.isLastPacket=false;
            }
            if (ferror(fp))
                printf("Error reading\n");
            break;
        case 3:
        {
            // waiting for ACK 1
            fd_set rcv_set;
            int n;

            struct timeval tv;

            // Clear the fd_set
            FD_ZERO(&rcv_set);
            // Add a descriptor to an fd_set
            FD_SET(s, &rcv_set);

            tv.tv_sec = TIMEOUT;
            tv.tv_usec = 0;

            if ((n = select(s + 1, &rcv_set, NULL, NULL, &tv)) < 0)
            {
                die("error on select");
            }

            if (n == 0)
            {
                // timout expired, send packet again
                printf("Timeout occured for ACK 1\n");
                if (sendto(s, &send_pkt, sizeof(send_pkt), 0, (struct sockaddr *)&si_other, slen) == -1)
                {
                    die("sendto()");
                }
                break;
            }

            // ACK arrived
            if (recvfrom(s, &rcv_ack, sizeof(rcv_ack), 0, (struct sockaddr *)&si_other, &slen) == -1)
            {
                die("recvfrom()");
            }
            if (rcv_ack.sq_no == 1)
            {
                // ACK 1 arrived, change state
                printf("Received ack seq. no. %d\n", rcv_ack.sq_no);
                state = 0;
            }
            else
                break;
        }
        }
    }
    close(s);
    return 0;
}