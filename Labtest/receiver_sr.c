/* Write your name and ID below.
   NAME:
   BITS ID:

 * No part of this code should be changed. You can only add your code
*/
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <memory.h>
#include <signal.h>
#include <time.h>

#define PDR 0.2 /* Packet Drop Ratio 20 %. This value can be changed to test your program */



typedef struct data_packet /* data packet structure to be used to transfer file data*/
{
  int pkt_no;
  int pkt_length; /* number of  file data bytes */
  char data[1024];
  int isLast;
} DATA_PKT;

typedef struct ack_packet /* ACK packet structure */
{
  int ack_no;
  int isLast;
} ACK_PKT;

void DieWithError(char *errorMessage); /* External error handling function */

int main(int argc, char *argv[])
{
  char buffer[16385];          /* buffer with room for null byte */
  buffer[16384] = '\0';        /* null terminate the buffer */
  int sock;                    /* Socket */
  struct sockaddr_in ServAddr; /* Local address */
  struct sockaddr_in ClntAddr; /* Client address */
  unsigned int cliAddrLen=sizeof(ClntAddr);     /* Length of incoming message */
  unsigned short ServPort=8002;     /* Server port */
  int recvMsgSize;      /* Size of received message */
  int windowSize;       /* SR window size used by the sender- value of this will be received from client */
  int packetSize = 500; /* num of bytes per packet- value of this will be received from client*/
  int IsLoss = 0;
  bzero(buffer, 16384); /* zero out the buffer */

  if (argc != 2) /* Test for correct number of parameters */
  {
    fprintf(stderr, "Usage:  %s <WITH LOSS = 1, WITHOUT LOSS = 0>\n", argv[0]);
    exit(1);
  }

  IsLoss = atoi(argv[1]); /* Error or Without Error */

  if (IsLoss)
    srand48(123456789); /* Seed for pseudorandom number generator. DO NOT CHANGE IT*/

  /* Create socket for sending/receiving datagrams */
  if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    DieWithError("socket() failed");

  ServPort = 8002; /* local port - you can change it */

  /* Construct local address structure */
  memset(&ServAddr, 0, sizeof(ServAddr));       /* Zero out structure */
  ServAddr.sin_family = AF_INET;                /* Internet address family */
  ServAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
  ServAddr.sin_port = htons(ServPort);          /* Local port */

  /* Bind to the local address */
  if (bind(sock, (struct sockaddr *)&ServAddr, sizeof(ServAddr)) < 0)
    DieWithError("bind() failed");

  // opening file to write
  FILE *fp;
  fp = fopen("out.txt", "a");
  if (fp == NULL)
  {
    DieWithError("fopen()");
  }

  int base = 0;
  while (1)
  {
    /* Q.1a) Write code here to receive window size value from client (sender). */
    if (recvfrom(sock, &windowSize, sizeof(windowSize), 0, (struct sockaddr *)&ClntAddr, &cliAddrLen) == -1)
    {
      DieWithError("recvfrom()");
    }

    DATA_PKT packets[windowSize];
    ACK_PKT ack_packets[windowSize];
    /* Q.1b) Write code here to receive data packets. */
    for (int i = 0; i < windowSize; i++)
    {
      if (recvfrom(sock, &packets[i], sizeof(packets[i]), 0, (struct sockaddr *)&ClntAddr, &cliAddrLen) == -1)
      {
        DieWithError("recvfrom()");
      }
      printf("ARRIVE PACKET %d: ACCEPTED: BASE %d\n",packets[i].pkt_no,base);
      base+=1;
    }
    
    /* Q.1c)  Write code here for "without packet loss" scenario. Sending ACK and copying data into the 	output.txt */
    if (IsLoss == 0)
    {
      for (int i = 0; i < windowSize; i++)
      {
        // printf("Packet contents are: %s\n",packets[i].data);
        // writing packet content to file
        fseek(fp,0,SEEK_END);
        printf("Writing to file\n");
        fwrite(packets[i].data, 1, strlen(packets[i].data), fp);
        // making ack packets
        ack_packets[i].ack_no = packets[i].pkt_no;
        if(packets[i].isLast==1)
          ack_packets[i].isLast=1;
        else
          ack_packets[i].isLast=0;
        // sending ack packets
        if (sendto(sock, &ack_packets[i], sizeof(ack_packets[i]), 0, (struct sockaddr *)&ClntAddr, cliAddrLen) == -1)
        {
          DieWithError("sendto()");
        }
        printf("SEND ACK %d\n",ack_packets[i].ack_no);
        if(packets[i].isLast==1){
          printf("Finished transfer of data packets\n");
          return 1;
        }
      }
    }

    /* Q.1d) Write code here for "packet loss" scenario. (Emulate losses, sending of ACK, dropping of packets, and copying of accepted data into output.txt*/
    else if (IsLoss == 1)
    {

    }
  }
  fclose(fp);
}

void DieWithError(char *errorMessage)
{
  perror(errorMessage);
  exit(1);
}
