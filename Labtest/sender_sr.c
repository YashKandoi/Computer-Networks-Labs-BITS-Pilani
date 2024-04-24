/* Write your name and ID below.
   NAME:
   BITS ID:

 * No part of this code should be changed. You can only add your code*
*/

#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <time.h>

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
}ACK_PKT;

/* ***********************************Required for Packet Loss Case *****************************************/

#define RTO_SECS 10           /* Seconds between retransmits- This value can be changed. Your program should work for any value. */
void CatchAlarm(int ignored); /* Handler for SIGALRM */
/*******************************************************************************************************/

void DieWithError(char *errorMessage); /* Error handling function */
void read_file(char *buffer);

/*  Reading data from in.txt file into the buffer[]   */
void read_file(char buffer[])
{
  int count = 0;
  FILE *fp;
  char ch;
  fp = fopen("file.txt", "r");

  if (fp == NULL)
  {
    perror("Error while opening the file.\n");
    exit(0);
  }
  while ((ch = fgetc(fp)) != EOF)
  {
    buffer[count++] = ch;
  }
  fclose(fp);
}

int main(int argc, char *argv[])
{
  int sock;
  struct sockaddr_in receiverAddr; /* Receiver (server) address */
  struct sockaddr_in senderAddr;   /* sender (client) address  */
  unsigned short receiverPort=8002;     /* Receiver (server) port */
  char *receiverIP;                /* IP address of the reciever (server) */
  char buffer[16384];              /* buffer - to be filled from the data file. Assuming file size is within this limit. */
  unsigned int cliAddrLen=sizeof(receiverAddr);
  int pktSize = 500; /* Packet size in bytes. The value can be changed to check your program */
  int windowSize;    /* SR window size used by the sender- to be read as user input */
  int IsLoss = 0;
  if (argc != 2) /* Test for correct number of parameters */
  {
    fprintf(stderr, "Usage:  %s <WITH LOSS = 1, WITHOUT LOSS = 0>\n", argv[0]);
    exit(1);
  }

  IsLoss = atoi(argv[1]); /* With Loss or Without Loss */

  /* For setting signal handler, required for retransmission in case of packet loss */

  struct sigaction myAction;
  /* Set signal handler for alarm signal */
  myAction.sa_handler = CatchAlarm;
  if (sigfillset(&myAction.sa_mask) < 0) /* block everything in handler */
    DieWithError("sigfillset() failed");
  myAction.sa_flags = 0;

  if (sigaction(SIGALRM, &myAction, 0) < 0)
    DieWithError("sigaction() failed for SIGALRM");

  /*  ****************** */

  /* Create datagram socket using UDP */
  if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    DieWithError("socket() failed");

  /* Construct the receiver (server) address structure */
  memset(&receiverAddr, 0, sizeof(receiverAddr)); /* Zero out structure */
  receiverAddr.sin_family = AF_INET;
  receiverAddr.sin_addr.s_addr = inet_addr("127.0.0.1"); /* receiver IP address */
  receiverAddr.sin_port = htons(receiverPort);                  /* receiver port. You may consider to change it if required.*/

  read_file(buffer); // Read file function is called here.

  /* Q.1a) Take user input for window size (W) and send this value to the server (receiver) over the socket. */
  printf("Please enter the size of Window:");
  scanf("%d", &windowSize);
  if (sendto(sock, &windowSize, sizeof(windowSize), 0, (struct sockaddr *)&receiverAddr, cliAddrLen) == -1)
  {
    DieWithError("sendto()");
  }

  /* Q.1b) Write code here to calculate total number of packets to be sent */
  int offset = strlen(buffer);
  int num_of_packets = (offset % pktSize) == 0 ? (offset / pktSize) : (offset / pktSize) + 1;
  int base = 0, newBase = 0;
  printf("Number of packets is %d\n",num_of_packets);
  // loading data into packets from buffer
  DATA_PKT packets[num_of_packets];
  ACK_PKT ack_packets[num_of_packets];
  for (int i = 0; i < num_of_packets; i++)
  {
    int j;
    for (j = 0; j < pktSize - 1; j++)
    {
      if (buffer[i] == '\0')
        break;
      packets[i].data[j] = buffer[i * pktSize + j];
    }
    packets[i].isLast=0;
    if(i==num_of_packets-1)
      packets[i].isLast=1;
    packets[i].data[pktSize] = '\0';
    packets[i].pkt_no = i;
    packets[i].pkt_length = j;
  }

  while (1) /* Change the termination condition appropriately */
  {
    /* Q.1c) Write code here to implement the protocol for "without packet loss" scenario */
    if (IsLoss == 0)
    {
      for (int i = base; (i < base + windowSize) && (base + windowSize < num_of_packets); i++)
      {
        // send packets within the window
        if (sendto(sock, &packets[i], sizeof(packets[i]), 0, (struct sockaddr *)&receiverAddr, cliAddrLen) == -1)
        {
          DieWithError("sendto()");
        }
        printf("SEND PACKET %d: BASE %d \n\n",packets[i].pkt_no,base);
        // printf("Packet content sent is: %s\n",packets[i].data);
      }
      newBase=base;
      // waiting for ack
      for (int i = base; i < base + windowSize; i++)
      {
        // receive ack packets within the window
        if (recvfrom(sock, &ack_packets[i], sizeof(ack_packets[i]), 0, (struct sockaddr *)&receiverAddr, &cliAddrLen) == -1)
        {
          DieWithError("recvfrom()");
        }
        if(ack_packets[i].ack_no==i){
          newBase=ack_packets[i].ack_no+1;
        }
        printf("RECEIVE ACK %d: BASE %d \n",ack_packets[i].ack_no,newBase);
        if(ack_packets[i].isLast==1){
          printf("Received the last ACK\n");
          return 1;
        }
      }
      base=newBase;
    }
    /* Q.1d) Write code here to implement the protocol "with packet loss" scenario */
    else
    {

    }
  }
  close(sock); /* close socket */
  return 0;
}

void CatchAlarm(int ignored) /* Handler for SIGALRM */
{
  /* Q.1e) Write code here required to handle retransmission timeout */
}

void DieWithError(char *errorMessage)
{
  perror(errorMessage);
  exit(1);
}
