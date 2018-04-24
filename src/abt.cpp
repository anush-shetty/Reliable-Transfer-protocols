#include "../include/simulator.h"
#include <queue>
#include <string.h>
#include <stdio.h>

#define A 0
#define B 1
#define MESSAGE_SIZE 20

int A_state = 0;	//0-> A is not busy

int ack_no = 0;
int sequence_no = 0;

int prev_ack_no = 0;		//previous packet used for checking at B side
int count = 0;	//count of no. of messages sent

std::queue<msg> buffer;	//for abt buffer the incoming msg from layer 5 if A is waiting for ACK to arrive

void send_packet();
int checksum(struct pkt);
int flip(int);

struct pkt sent_packet;
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

/* called from layer 5, passed the data to be sent to other side */
void A_output(struct msg message) {
	buffer.push(message);
	if (!A_state) {
		struct pkt packet;
		struct msg temp_message = buffer.front();//temp_message containing first element of buffer

		strcpy(packet.payload, temp_message.data);
		packet.seqnum = sequence_no;
		packet.acknum = ack_no;
		packet.checksum = checksum(packet);
		printf("Buffer value is %s and length of buffer is %d \n",
				buffer.front().data, strlen(buffer.front().data));
		printf("Length of payload is %d\n", strlen(packet.payload));
		sent_packet = packet;
		A_state = flip(A_state);	//make the state busy

		sequence_no = flip(sequence_no);
		printf(
				"A sending message from layer5 to layer3 : %s and checksum %d \n",
				packet.payload, packet.checksum);
		buffer.pop();
		tolayer3(A, packet);
		//count++;
		starttimer(A, 20);
	}

}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(struct pkt packet) {
	printf("Inside packet's acknowledgment %d\n", packet.acknum);
	printf("Checksum val is %d and actual checksum is %d\n", packet.checksum,
			checksum(packet));
	printf("packet.acknum is %d and ack_no is %d\n", packet.acknum, ack_no);
	if (packet.checksum == checksum(packet)) {
		if(packet.acknum == ack_no)
		{printf("Checksum val is %d", packet.checksum);
		stoptimer(A);
		ack_no = flip(ack_no);
		A_state = flip(A_state);//free A as acknowledgment has been received
		count++;
		printf("count value is %d\n", count);
		if (!buffer.empty()) {
			struct pkt packet;
			struct msg temp_message = buffer.front();//temp_message containing first element of buffer

			strcpy(packet.payload, temp_message.data);
			packet.seqnum = sequence_no;
			packet.acknum = flip(ack_no);
			packet.checksum = checksum(packet);
			printf("Buffer value is %s and length of buffer is %d \n",
					buffer.front().data, strlen(buffer.front().data));
			printf("Length of payload is %d\n", strlen(packet.payload));
			sent_packet = packet;
			A_state = flip(A_state);	//make the state busy

			printf(
					"A sending message from layer5 to layer3 : %s and checksum %d \n",
					packet.payload, packet.checksum);
			buffer.pop();
			tolayer3(A, packet);
			sequence_no = flip(sequence_no);


			starttimer(A, 20);

		}
		}

	}

}

/* called when A's timer goes off */
void A_timerinterrupt() {
	printf(
			"\nTime out occured Packet seq_no: %d ack_no: %d msg:%s is sent again\n",
			sent_packet.seqnum, sent_packet.acknum, sent_packet.payload);
	tolayer3(A, sent_packet);
	starttimer(A, 20);
}

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init() {
	A_state = 0;
	sequence_no = 0;
	ack_no = 0;
	printf("A_init started\n");
}

/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(struct pkt packet) {

	printf("\n\n\n\npacket received is%s\n\n\n\n", packet.payload);
	//check for checksum first and if seqnum is same as the expected state(Basically if layer 3 doesnt send acks to A then you should not retransmit acknowledgz)

	if (packet.checksum == checksum(packet)) {
		printf("Expected acknowledment no is %d\n", prev_ack_no);
		if (packet.seqnum == prev_ack_no) {
			printf(
					"B received packet with message %s and length is %d, sequence number %d and checksum: %d\n",
					packet.payload, strlen(packet.payload), packet.seqnum,
					packet.checksum);


			struct pkt ack_packet;
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
			prev_ack_no = flip(packet.seqnum);//changing the value of previous ack no
			printf("If: Acknowledgment now becomes %d\n ", prev_ack_no);
		} else {
			//prev_ack_no = flip(prev_ack_no);
			//prev_ack_no = flip(packet.seqnum);
			printf("Else: Acknowledgment now becomes %d\n ", prev_ack_no);
			struct pkt ack_packet;
			ack_packet.seqnum = 0;
			//ack_packet.acknum = packet.seqnum;
			ack_packet.acknum = packet.seqnum;
			memcpy(ack_packet.payload, "\0", 20);
			ack_packet.checksum = checksum(ack_packet);

			tolayer3(B, ack_packet);
		}

	}

}

/* the following routine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init() {

	prev_ack_no = 0;
}

int checksum(struct pkt packet) {
	int sum = (packet.seqnum + packet.acknum);
	for (int i = 0; i < MESSAGE_SIZE; i++) {
		//printf("value of sum %d \n",sum);
		sum += (int) packet.payload[i];

	}
	return sum;
}

int flip(int bit) {
	return (bit == 0) ? 1 : 0;
}
