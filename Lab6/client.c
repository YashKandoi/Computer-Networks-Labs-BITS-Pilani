/* Simple udp server with stop and wait functionality with data packet and ACK packet loss */
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUFLEN 512 // Max length of buffer
#define PORT 8882  // The port on which to send data
#define TIMEOUT 5

typedef enum
{
    true,
    false
} bool;

bool toDiscard;

void discardRandom()
{
    if (rand() % 2 == 0)
    {
        toDiscard = true;
    }
    else
    {
        toDiscard = false;
    }
}

typedef struct packet1
{
    int sq_no;
} ACK_PKT;

typedef struct packet2
{
    int sq_no;
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
    int s, i, slen = sizeof(si_other);
    char buf[BUFLEN];
    char message[BUFLEN];
    DATA_PKT send_pkt, rcv_ack;

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

    int state = 0;
    while (1)
    {
        switch (state)
        {

        case 0:
            printf("Enter message 0: "); // wait for sending packet with seq. no. 0
            fgets(send_pkt.data, sizeof(send_pkt), stdin);
            send_pkt.sq_no = 0;
            if (sendto(s, &send_pkt, sizeof(send_pkt), 0, (struct sockaddr *)&si_other, slen) == -1)
            {
                die("sendto()");
            }
            state = 1;
            break;

        case 1:
        {
            // waiting for ACK 0
            discardRandom();
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
            if (rcv_ack.sq_no == 0 && toDiscard == false)
            {
                // ACK 0 arrived, change state
                printf("Received ack seq. no. %d\n", rcv_ack.sq_no);
                state = 2;
            }
            else
                break;
        }
        case 2:
            printf("Enter message 1: "); // wait for sending packet with seq. no. 1
            fgets(send_pkt.data, sizeof(send_pkt), stdin);
            send_pkt.sq_no = 1;
            if (sendto(s, &send_pkt, sizeof(send_pkt), 0, (struct sockaddr *)&si_other, slen) == -1)
            {
                die("sendto()");
            }
            state = 3;
            break;
        case 3:
        {
            // waiting for ACK 1
            discardRandom();
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
            if (rcv_ack.sq_no == 1 && toDiscard == false)
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