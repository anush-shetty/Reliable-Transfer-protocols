#include "../include/simulator.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"

#define MESSAGE_SIZE 20
#define A 0
#define B 1

struct pkt* sent_packet = (pkt*)malloc(sizeof(pkt)*1000);	//maintaining the packet that was last sent
//struct pkt* rcvd_packet = (pkt*)malloc(sizeof(pkt)*1000); //store correctly received packets(used for debugging, remove this after done)
struct msg* msgbuffer = (msg *)malloc(sizeof(msg)*1000);
/* ******************************************************************
 ALTERNATING BIT AND GO-BACK-N NETWORK EMULATOR: VERSION 1.1  J.F.Kurose

   This code should be used for PA2, unidirectional data transfer 
   protocols (from A to B). Network properties:
   - one way network delay averages five time units (longer if there
     are other messages in the channel for GBN), but can be larger
   - packets can be corrupted (either the header or the data portion)
     or lost, according to user-defined probabilities
   - packets will be delivered in the order in which they were sent
     (although some can be lost).
**********************************************************************/

/********* STUDENTS WRITE THE NEXT SEVEN ROUTINES *********/

int initialSeqNum = 0;
int base=0;
int nextseqnum=0;
int window_size;
int msgCount = 0;

int rcv_count = 0; 		//count of received packets(used for debugging, remove this after done)

int prev_ack_no = 0;
int expectedseqnum = 0;




int checksum(struct pkt);
/* called from layer 5, passed the data to be sent to other side */
void A_output(struct msg message)
{
	printf("called A_output");
	*(msgbuffer+msgCount) = message;
	msgCount++;
	printf("Received msg %s from application layer 5 from above to A\n",message.data);

	if(nextseqnum < base + window_size)
	{
		struct pkt packet;
		packet.seqnum = nextseqnum;
		packet.acknum = 0;
		//copy the buffers data into payload
		printf("Message in msgbuffer is %s\n",(msgbuffer+nextseqnum)->data);
		strcpy(packet.payload,(msgbuffer+nextseqnum)->data);
		packet.checksum = checksum(packet);
		*(sent_packet+nextseqnum) = packet;
		tolayer3(A,packet);
		printf("nextseqnum is %d, base is %d\n ",nextseqnum,base);

		if(base == nextseqnum)
		{
			starttimer(A,30);
			printf("Timer started for packet %s\n",packet.payload);
		}
		nextseqnum++;


	}
	printf("msgCount is %d\n",msgCount);
	//used for debugging -> remove this

			struct msg temp_message = *(msgbuffer+base);
			printf("msgbuffer[%d] is %s\n",base,temp_message.data);


}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(struct pkt packet)
{
	printf("Acknowledgment received is %d and base is %d\n",packet.acknum,base);

	//looking for acks within the range[base,window-1]
	if(packet.checksum == checksum(packet) && (packet.acknum < base +window_size) && (packet.acknum >= base))
	{
		int win_shift;
		printf("Checksum val is %d\n",packet.checksum);
		//if(base == nextseqnum)
		stoptimer(A);

		/**/
			win_shift = packet.acknum - base;		//shift the window with the ackno recvd
		base = packet.acknum +1;
		printf("nextseqnum is %d, base is %d\n ",nextseqnum,base);

		for(int i=0; i <= win_shift; i++)
		{
			if(nextseqnum < msgCount)
			{
				struct pkt new_packet;
				new_packet.seqnum = nextseqnum;
				new_packet.acknum = 0;
				struct msg temp_msgbuffer = *(msgbuffer+nextseqnum);
				strcpy(new_packet.payload,temp_msgbuffer.data);
				new_packet.checksum = checksum(new_packet);
				printf("sending packet %s\n",new_packet.payload);
				*(sent_packet+nextseqnum) = new_packet;
				tolayer3(A,new_packet);

				nextseqnum++;
			}
		}
		if(base != nextseqnum)
		{
			starttimer(A,30);
			//printf("Timer started for packet: %s\n",*(sent_packet+(nextseqnum-1))->payload);
		}
	}

}

/* called when A's timer goes off */
void A_timerinterrupt()
{
	struct pkt new_packet;

	for(int baseCount=base; baseCount<nextseqnum ; baseCount++)
	{

		printf("base value is %d next seqnum is %d\n",base,nextseqnum);
		new_packet = *(sent_packet+baseCount);
		printf("\nTime out occured Packet seq_no: %d ack_no: %d msg:%s is sent again\n",new_packet.seqnum, new_packet.acknum, new_packet.payload);
		tolayer3(A, new_packet);


	}
	starttimer(A, 30);

}  

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{
	msgbuffer = (msg *)malloc(sizeof(msg)*1000);
	base = 0;
	nextseqnum = 0;
	initialSeqNum = 0;
	window_size = getwinsize();
	sent_packet = (pkt*)malloc(sizeof(pkt)*1000);	//Maintaining all the sent packets within the window
	msgCount = 0;
	printf("window size is %d",window_size);
}

/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(struct pkt packet)
{
	printf("Received packet %s \n",packet.payload);
	if(packet.checksum == checksum(packet))
	{
		struct pkt ack_packet;
		printf("Last acknowledged is %d and expectedseqnum is %d\n",prev_ack_no,expectedseqnum);
		if(packet.seqnum == expectedseqnum)
		{
			printf(
				"B received packet with message %s and length is %d, sequence number %d and checksum: %d\n",
				packet.payload, strlen(packet.payload), packet.seqnum,
				packet.checksum);




				ack_packet.seqnum = 0;
				ack_packet.acknum = packet.seqnum;
				memcpy(ack_packet.payload, "\0", 20);
				ack_packet.checksum = checksum(ack_packet);

				//prev_packet = packet;
				printf(
				"SEQ num: %d ACK num: %d packet payload: %s packet checksum: %d\n",
				packet.seqnum, packet.acknum, packet.payload,
				packet.checksum);
				tolayer5(B, packet.payload);
				tolayer3(B, ack_packet);
				prev_ack_no = ack_packet.acknum;
				expectedseqnum=packet.seqnum+1;
		}/*
	//used for debugging -> remove this
	for(int i=0;i<nextseqnum;i++)
		{
			struct msg temp_message = *(msgbuffer+i);
			printf("msgbuffer[%d] is %s\n",i,temp_message.data);
		}
*/
		else
		{
			printf("Discarding out of order OR corrupted packet %s with sequence number%d\n",packet.payload,packet.seqnum);
		}
		/*for(int i=0;i<expectedseqnum;i++)
				{
					struct pkt temp_packet = *(rcvd_packet+i);
					printf("packet received is[%d] is %s\n",i,temp_packet.payload);
				}*/
		printf("No of messages received is %d\n",rcv_count);


	}
}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{
	prev_ack_no = -1;
	expectedseqnum = 0;
	rcv_count = 0;
	//rcvd_packet = (pkt*)malloc(sizeof(pkt)*1000);

}
int checksum(struct pkt packet) {
	int sum = (packet.seqnum + packet.acknum);
	for (int i = 0; i < MESSAGE_SIZE; i++) {
		//printf("value of sum %d \n",sum);
		sum += (int) packet.payload[i];

	}
	return sum;
}

