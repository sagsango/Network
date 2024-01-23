#include "util.h"


static int last_sent_seq = 0; // last sent seq_num
static int client_seq_num = 0; // expected client sequence number
static double current_timer = 5.0; // timeout timer
static enum enum_state recv_state = OPEN_CONN; // program state.
static struct timespec t1, t2; // record start and end time

void handle_alarm(int sig) {
	fprintf(stderr,"More than a second passed after the last packet. Exiting.\n");
	exit(1);
}

int main(int argc, char** argv) {
	if(argc<2) {		
		fprintf(stderr, "Usage: ./receiver <port>\n");
		exit(1);
	}
	// signal(SIGALRM,handle_alarm);
	
	int sock = socket(AF_INET, SOCK_DGRAM, 0);
	if(sock < 0) {
		perror("Creating socket failed: ");
		exit(1);
	}
	
	struct sockaddr_in addr; 	// internet socket address data structure
	addr.sin_family = AF_INET;
	addr.sin_port = htons(atoi(argv[1])); // byte order is significant
	addr.sin_addr.s_addr = INADDR_ANY;
	
	int res = bind(sock, (struct sockaddr*)&addr, sizeof(addr));
	if(res < 0) {
		perror("Error binding: ");
		exit(1);
	}
	
	// Set initial timeout.
	current_timer = set_timer(sock, current_timer);
	
	// Buffer and return values.
	char buf[BUF_SIZE];
	memset(&buf,0,sizeof(buf));
	int recv_count, retval;
	while(1) {
		struct sockaddr_in sender_addr;
		int len = sizeof(sender_addr);
		int i;
		double retry_time = current_timer;
		for (i = 0; i < 3; ++i) {
			recv_count = recvfrom(sock, buf, BUF_SIZE, 0, 
												(struct sockaddr*)&sender_addr, (socklen_t*)&len);
			if (recv_count >= 0) break; // received response.
			else if (errno == EAGAIN || errno == EWOULDBLOCK) { //retval < 0
				perror("Receive failed");	
				fprintf(stderr, "Attempt %d: Recv Timeout.\n", i+1);
				set_timer(sock, retry_time);
				retry_time = retry_time * 2.0;
			}
		}
		if (i == 3) exit(1); 
		fprintf(stderr, "Received %d bytes\n", recv_count);
		// stop receiving after a second has passed
		// alarm(1);

		// Examine header
		struct packet_hdr *hd = (struct packet_hdr *) buf;
		// TCP incoming connection.
		if (hd->type == SYN) {
			client_seq_num = hd->seq_num+1;
			fprintf(stderr, "Recv SYN packet\n");
			
			if (recv_state == WAIT_DATA) {
				fprintf(stderr, "Error: Receiving SYN packet from a new sender. Exiting...\n");
				break;
			}
			
			// Reply with SYN-ACK packet.
			struct packet *syn_ack = malloc(sizeof *syn_ack);
			syn_ack->hdr.seq_num = last_sent_seq;
			syn_ack->hdr.ack_num = client_seq_num;
			syn_ack->hdr.type = SYN;
			syn_ack->hdr.pkt_length = 0;
			
			//Retry 3 times
			for (i = 0; i < 3; ++i) {
				retval = sendto(sock, syn_ack, sizeof *syn_ack, 0, (struct sockaddr*)&sender_addr, (socklen_t)len);
				if(retval < 0) { perror("Send failed"); exit(1); }
				
				// Receive ack
				recv_count = recvfrom(sock, buf, BUF_SIZE, 0, 
												(struct sockaddr*)&sender_addr, (socklen_t*)&len);
				if (recv_count >= 0) break; // received response.
				else if (errno == EAGAIN || errno == EWOULDBLOCK) { //retval < 0
					perror("Receive failed");	
					fprintf(stderr, "Attempt %d: Recv Timeout.\n", i+1);
					set_timer(sock, retry_time);
					retry_time = retry_time * 2.0;
				}
			}
			if (i == 3) exit(1);
			
			struct packet *ack = (struct packet*) buf;
			if (ack->hdr.type != ACK) {
				fprintf(stderr, "Not an ack response\n");
			}
			if (ack->hdr.ack_num != last_sent_seq+1) {
				fprintf(stderr, "Invalid ack number\n");
			}
			fprintf(stderr, "%d Recv ACK with seq_num %d ack_num %d\n", recv_count, ack->hdr.seq_num, ack->hdr.ack_num);
			
			last_sent_seq++;
			client_seq_num = ack->hdr.seq_num+1;
			recv_state = WAIT_DATA; // Change program state.
		}
		else if (hd->type == ACK) {
			fprintf(stderr, "Recv ACK packet with seq_num %d ack_num%d\n", hd->seq_num, hd->ack_num);
		}
		else if (hd->type == FIN) {
			fprintf(stderr, "Recv FIN packet\n");
			
			// Respond with ack.
			last_sent_seq++;
			client_seq_num = hd->seq_num+1;
			struct packet *ack = malloc(sizeof *ack);
			ack->hdr.seq_num = last_sent_seq;
			ack->hdr.ack_num = client_seq_num;
			ack->hdr.type = ACK;
			ack->hdr.pkt_length = 0;
			retval = sendto(sock, ack, sizeof *ack, 0, (struct sockaddr*)&sender_addr, (socklen_t)len);
			if(retval < 0) { perror("Send failed"); exit(1); }
			break;
		}
		else if (hd->type == DATA) {
			fprintf(stderr, "Recv DATA packet with seq_num %d ack_num %d\n", hd->seq_num, hd->ack_num);
			fprintf(stderr, "pkt_length %ld csn %d lss %d\n", hd->pkt_length, client_seq_num, last_sent_seq);	
			if (hd->ack_num != last_sent_seq+1) {
				fprintf(stderr, "WARNING out of order packet received\n");
			}
			
			// Get data and write to file. Ignore if re-transmission.
			if (client_seq_num == hd->seq_num) {
				fprintf(stderr, "writing\n");
				last_sent_seq++;
				client_seq_num = hd->seq_num+1;
				write(1, buf + sizeof (struct packet_hdr), hd->pkt_length);
			}
			// else {
				// last_sent_seq++;
				// client_seq_num = hd->seq_num+1;
				// write(1, buf + sizeof (struct packet_hdr), hd->pkt_length);
			// }
			
			// Respond with ack.
			// sleep(5);
			struct packet *ack = malloc(sizeof *ack);
			ack->hdr.seq_num = last_sent_seq;
			ack->hdr.ack_num = client_seq_num;
			ack->hdr.type = ACK;
			ack->hdr.pkt_length = 0;
			fprintf(stderr, "Replying with seq %d ack %d\n", ack->hdr.seq_num, ack->hdr.ack_num);
			retval = sendto(sock, ack, sizeof *ack, 0, (struct sockaddr*)&sender_addr, (socklen_t)len);
			if(retval < 0) { perror("Send failed"); exit(1); }
		}
		else {
			fprintf(stderr, "Received invalid format\n");
		}
	}
	
	shutdown(sock,SHUT_RDWR);
	close(sock);
}


