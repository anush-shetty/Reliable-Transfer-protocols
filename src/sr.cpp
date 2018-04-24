/*Referred [Varghese 1997]-Hashed and Hierarchical Timing Wheels: Efficient
 Data Structures for Implementing a Timer Facility*/
#include "../include/simulator.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "math.h"

#define MESSAGE_SIZE 20
#define A 0
#define B 1


#define TIMEOUT 20.00

struct packet_list {
	struct pkt packet;
	int state;		//A-side 1-> data sent 0-> currently free 2-> correctly received ack 	|B-side 0->currently free 1->data received
	int seqnum;		//A_side 0-> not yet received ack 1-> received ack | B_side 1->ack sent
	float timeout;

};

struct packet_list* sent_packet = (packet_list*) malloc(sizeof(packet_list)* 1000);	//maintaining the packet that was last sent
struct packet_list* rcvd_packet = (packet_list*) malloc(sizeof(packet_list) * 1000);	//maintaining the packet which was recently received
struct msg* msgbuffer;
struct packet_list* buffer_A;
int buffer_B[1000];		//B_side 0-> not received state 1-> received state 2-> ack sent

int initialSeqNum = 0;
float elapsed_time=0;
int timer_ptr=0;
float current_time = get_sim_time();
int base_A = 0;
int nextseqnum = 0;
int window_size;
int msgCount = 0;
int ptr_A = 0;

//B side variables

int base_B = 0;
int ptr_B = 0;
int ack_count = 0;
int count = 0;		//count the no. of packets sent from A

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
int checksum(struct pkt);
/* called from layer 5, passed the data to be sent to other side */
/*
void A_output(struct msg message)
{
	printf("called A_output");
	*(msgbuffer+msgCount) = message;
	msgCount++;
	printf("Received msg %s from application layer 5 from above to A\n",message.data);

	if(nextseqnum < base_A + window_size)
	{
		struct pkt packet;
		packet.seqnum = nextseqnum;
		packet.acknum = 0;
		//copy the buffers data into payload
		printf("Message in msgbuffer is %s\n",(msgbuffer+nextseqnum)->data);
		strcpy(packet.payload,(msgbuffer+nextseqnum)->data);
		packet.checksum = checksum(packet);
		//*(sent_packet+nextseqnum) = packet;
		tolayer3(A,packet);
		printf("nextseqnum is %d, base is %d\n ",nextseqnum,base_A);

		if(base_A == nextseqnum)
		{
			starttimer(A,20);
			printf("Timer started for packet %s\n",packet.payload);
		}
		nextseqnum++;


	}
	printf("msgCount is %d\n",msgCount);
	//used for debugging -> remove this
	for(int i=0;i<msgCount;i++)
		{
			struct msg temp_message = *(msgbuffer+i);
			printf("msgbuffer[%d] is %s\n",i,temp_message.data);
		}

}
*/

void A_output(struct msg message) {
	printf("called A_output");

	*(msgbuffer + msgCount) = message;
	msgCount++;
	printf("Received msg %s from application layer 5 from above to A\n",
			message.data);

	if (nextseqnum < base_A + window_size) {
		struct pkt new_packet;
		new_packet.seqnum = nextseqnum;
		new_packet.acknum = 0;
		printf("Message in msgbuffer is %s and nextseqnum is %d\n", (msgbuffer + nextseqnum)->data,nextseqnum);
		struct msg temp_msg = *(msgbuffer + nextseqnum);
		strcpy(new_packet.payload, temp_msg.data);
		new_packet.checksum = checksum(new_packet);

		ptr_A = nextseqnum;
		printf("ptr_A is %d base_A is %d\n",ptr_A,base_A);
		(sent_packet + ptr_A)->packet = new_packet;
		(sent_packet + ptr_A)->seqnum = nextseqnum;

		(sent_packet+ptr_A)->timeout = get_sim_time()+TIMEOUT;
		tolayer3(A, new_packet);
		(sent_packet+ptr_A)->state = 1;
		if(base_A == nextseqnum)
		{
			/*elapsed_time = get_sim_time()+TIMEOUT;
			(sent_packet + ptr_A)->timeout = elapsed_time;
			printf("Simulation time is %f timeout value is %f\n",get_sim_time(),(sent_packet+ptr_A)->timeout);
			*/starttimer(A,TIMEOUT);
			printf("timer started\n");
		}
		/*else{
			elapsed_time = get_sim_time()+TIMEOUT-((sent_packet + base_A)->timeout);
			(sent_packet + ptr_A)->timeout = elapsed_time;
		}*/
		printf("Simulation time is %f ...and %s will time out at %f ",get_sim_time(),(msgbuffer+nextseqnum)->data,(sent_packet+ptr_A)->timeout);
		nextseqnum++;


	}

}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(struct pkt packet) {
printf("Acknowledgment received is %d and base_A is %d at time %f\n",packet.acknum,base_A,get_sim_time());

if(packet.checksum == checksum(packet) && (packet.acknum < base_A +window_size) && (packet.acknum >= base_A))
	{
		//(sent_packet + packet.acknum)->state = 2;
		if(base_A == packet.acknum)
		{	/*float delay = 0;
			float current_time = 0;
			float temp_base_time = (sent_packet+base_A)->timeout;
			printf("temp_base_time is %f\n",temp_base_time);
			stoptimer(A);
			current_time = get_sim_time();
			base_A = base_A + 1;
			(sent_packet+base_A)->timeout += temp_base_time;
			printf("Timeout of of %s is %f",((sent_packet+base_A)->packet).payload,(sent_packet+base_A)->timeout);
			for(int i=base_A+1; i<nextseqnum; i++)
			{
				(sent_packet+i)->timeout = fabs(temp_base_time-current_time+(sent_packet+i)->timeout);
				printf("New Timeout Value of %s is %f",((sent_packet+i)->packet).payload,(sent_packet+i)->timeout);
			}
			for(int i =base_A; i<base_A+window_size; i++)
			{
				if((sent_packet + i)->state !=2)
				{	starttimer(A,(sent_packet+i)->timeout);
				printf("Starting timer at time %f\n",get_sim_time());
				break;
				}
			}*/
			stoptimer(A);
			base_A = base_A +1;

			for(int i=base_A; base_A<nextseqnum;i++)
			{
				if(nextseqnum > i + window_size && (sent_packet + i+window_size)->state!=2)
				{
					tolayer3(A,(sent_packet + i+window_size)->packet);
					(sent_packet+i)->state = 1;
					(sent_packet + i+window_size)->timeout = get_sim_time()+TIMEOUT;
					nextseqnum++;
					break;
				}
				if((sent_packet + i)->state!=2)
				{
					printf("next time out will occur at %f\n",(sent_packet + i)->timeout-get_sim_time());
					starttimer(A,TIMEOUT+(sent_packet+i)->timeout - get_sim_time());
					break;
				}
			}

		}
	/*	else{
			printf("Setting the packet %s ack to received\n",((sent_packet+packet.acknum)->packet).payload);
			(sent_packet+packet.acknum)->state = 2;
			//(sent_packet+packet.acknum)->timeout = 0;
		}*/
	}


}

/* called when A's timer goes off */
void A_timerinterrupt() {
printf("Timeout occured base is %d at time %f\n",base_A,get_sim_time());
	/*for(int i=base_A;i< base_A+window_size ; i++)
		printf("(sent_packet+%d)->state is %d   (sent_packet+%d)->timeout is %f\n",i,(sent_packet+i)->state,i,(sent_packet+i)->timeout);
	*/for(int i=base_A ; i<base_A+window_size ; i++)
	{

		if(i< nextseqnum)
		{
			if((sent_packet+i)->state!=2 &&(sent_packet+i)->state == 1)
		{
			tolayer3(A,((sent_packet+i)->packet));
			(sent_packet+i)->timeout = get_sim_time()+TIMEOUT;
			break;
			//base_A++;

		}

		//
		}
		}
	if((sent_packet+base_A)->timeout-get_sim_time()>TIMEOUT)
	{
		printf("Set timeout for %s %f\n",((sent_packet+base_A)->packet).payload,- get_sim_time() + (sent_packet+base_A)->timeout);
		starttimer(A, (sent_packet+base_A)->timeout- get_sim_time() );
		//nextseqnum++;
	}
	else
	{
		starttimer(A,TIMEOUT);
	}
	(sent_packet+base_A)->timeout = get_sim_time()+TIMEOUT;
}

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init() {
	msgbuffer = (msg *) malloc(sizeof(msg)* 1000);
	sent_packet = (packet_list*) malloc(sizeof(packet_list)* 1000);	//Maintaining all the sent packets within the window
	//buffer_A = (packet_list*) malloc(sizeof(packet_list) * window_size);
	msgCount = 0;
	ptr_A = 0;
	base_A = 0;
	nextseqnum = 0;
	elapsed_time=0.0;
	window_size = getwinsize();
	current_time = get_sim_time();
	printf("window size is %d and current_time is %f\n", window_size,
			current_time);
	/*for(int i=0; i<window_size; i++)
	{
		(sent_packet + i)->state = 0;	//initializing state of every packet

	}*/
	//starttimer(A, TICK);


}

/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(struct pkt packet) {
	printf("Received packet %s with seq no %d\n", packet.payload,packet.seqnum);
	if(packet.checksum == checksum(packet))
	{
		struct pkt ack_packet;

		if (packet.seqnum < base_B + window_size && packet.seqnum >= base_B) {
			printf("B received packet with message %s and length is %d, sequence number %d and checksum: %d\n",
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
			(rcvd_packet+packet.seqnum)->packet = packet;
			(rcvd_packet+packet.seqnum)->state = 1;	//packet received within the window

			for(int i=base_B ; i<base_B+ window_size-1 ; i++)
				{
					if((rcvd_packet + i)-> state != 1 )//check if the packet is already got in the buffer
						break;

					printf("sending packet %s to layer 5, buffer_B state is %d and ack is %d\n",((rcvd_packet+base_B)->packet).payload,(rcvd_packet+i)->state);

					tolayer5(B, ((rcvd_packet+i)->packet).payload);
					(rcvd_packet+i)->state == 0;		//ack sent

					base_B ++;
					printf("Base_B value is %d\n",base_B);

				}

			tolayer3(B, ack_packet);

		}
		else if((rcvd_packet+packet.seqnum)->state == 1){
			printf("Duplicate packet received \n");
			printf("B received packet with message %s and length is %d, sequence number %d and checksum: %d\n",
								packet.payload, strlen(packet.payload), packet.seqnum,
								packet.checksum);
			ack_packet.seqnum = 0;
			ack_packet.acknum = packet.seqnum;
			memcpy(ack_packet.payload, "\0", 20);
			ack_packet.checksum = checksum(ack_packet);
			tolayer3(B,ack_packet);


		}
	}
}

		/* the following rouytine will be called once (only) before any other */
		/* entity B routines are called. You can use it to do any initialization */
void B_init() {

	rcvd_packet = (packet_list*) malloc(sizeof(packet_list) * 1000);
	base_B = 0;
	ptr_B = 0;
	for(int i=0; i< 1000;i++)
		buffer_B[i] = 0;
}
int checksum(struct pkt packet) {
	int sum = (packet.seqnum + packet.acknum);
	for (int i = 0; i < MESSAGE_SIZE; i++) {
		//printf("value of sum %d \n",sum);
		sum += (int) packet.payload[i];

	}
	return sum;
}
