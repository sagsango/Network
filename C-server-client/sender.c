#include "util.h"

static int last_sent_seq = 0; // last sent seq_num
static int server_seq_num = 0; // expected server sequence number
static double current_timer = 5.0; // timeout timer
static struct timeval t1, t2; // record start and end time

// Round trip time estimation variables.
double est_rtt = 5.0;
double dev_rtt = 0.0;

double calculate_new_timeout(struct timeval *t1, struct timeval *t2) {
	double elapsed_time = get_elapsed_time(t1, t2);
	est_rtt = get_estimated_rtt(est_rtt, elapsed_time);
	dev_rtt = get_dev_rtt(dev_rtt, est_rtt, elapsed_time);
	double new_timeout = get_timeout_interval(est_rtt, dev_rtt);
	printf("Monitor %f %f %f\n", est_rtt, dev_rtt, new_timeout);
	current_timer = est_rtt;
	return new_timeout;
}

void init_tcp(int socket, struct sockaddr* addr) {
	int retval;
	char buf[BUF_SIZE];
	memset(&buf, 0, sizeof buf);
	
	// start syn
	struct packet *syn = malloc(sizeof *syn);
	syn->hdr.seq_num = last_sent_seq;
	syn->hdr.ack_num = last_sent_seq;
	syn->hdr.type = SYN;
	syn->hdr.pkt_length = 0;
	
	// re-try connection 3 times
	int i; // loop index
	double retry_time = current_timer; // timeout increase
	gettimeofday(&t1, NULL);
	for (i = 0; i < 3; ++i) {
		retval = sendto(socket, syn, sizeof *syn, 0, addr, sizeof (*addr));
		if(retval < 0) { perror("Send failed"); exit(1); }
		
		// Receive syn-ack
		struct sockaddr_in sender_addr;
		int len = sizeof(sender_addr);
		retval = recvfrom(socket, buf, sizeof (struct packet), 0, 
											(struct sockaddr*)&sender_addr, (socklen_t*)&len);
		if (i == 0) { // measure time once only 
			gettimeofday(&t2, NULL);
		}
		if (retval >= 0) break; // received response.
		else if (errno == EAGAIN || errno == EWOULDBLOCK) { //retval < 0
			perror("Receive failed");	
			fprintf(stderr, "Attempt %d: Send Timeout.\n", i+1);
			set_timer(socket, retry_time);
			retry_time = retry_time * 2.0;
		}
	}
	if (i == 3) exit(1); // all attempts failed
	set_timer(socket, calculate_new_timeout(&t1, &t2));
	
	// Read syn-ack
	printf("Recv %d bytes \n", retval);
	struct packet *syn_ack = (struct packet*) buf;
	if (syn_ack->hdr.type != SYN) {
		printf("Not a syn response\n");
		exit(1);
	}
	if (syn_ack->hdr.ack_num != last_sent_seq+1) {
		printf("Invalid ack number\n");
		exit(1);
	}
	last_sent_seq++;
	server_seq_num = syn_ack->hdr.seq_num+1;
	
	// Send ack back.
	struct packet *ack = malloc(sizeof *ack);
	ack->hdr.seq_num = last_sent_seq;
	ack->hdr.ack_num = server_seq_num;
	ack->hdr.type = ACK;
	ack->hdr.pkt_length = 0;
	retval = sendto(socket, ack, sizeof *ack, 0, addr, sizeof (*addr));
	if(retval < 0) { perror("Send failed"); exit(1); }
	last_sent_seq++;
	
	free(ack);
	free(syn);
}

int tcp_send(int socket, struct sockaddr* addr, char* send_buf, int send_size) {
	int retval;
	char buf[BUF_SIZE];
	memset(&buf, 0, sizeof buf);
	
	// send data
	struct packet *data = malloc(send_size + sizeof *data);
	data->hdr.seq_num = last_sent_seq;
	data->hdr.ack_num = server_seq_num+1;
	data->hdr.type = DATA;
	data->hdr.pkt_length = send_size;
	memcpy(data->payload, send_buf, send_size);
	printf("Sending %d bytes with seq %d ack %d\n", send_size, data->hdr.seq_num, data->hdr.ack_num);
	
	// re-try connection 3 times
	int i; // loop index
	double retry_time = current_timer; // timeout increase
	gettimeofday(&t1, NULL);
	for (i = 0; i < 3; ++i) {
		retval = sendto(socket, (char *) data, send_size + sizeof *data, 0, addr, sizeof (*addr));
		if(retval < 0) { perror("Send failed"); exit(1); }
		
		// wait for ack.
		struct sockaddr_in sender_addr;
		int len = sizeof(sender_addr);
		retval = recvfrom(socket, buf, sizeof (struct packet), 0, 
											(struct sockaddr*)&sender_addr, (socklen_t*)&len);
		if (i == 0) { // measure time once only 
			gettimeofday(&t2, NULL);
			printf("Record elapsed time %f\n", get_elapsed_time(&t1, &t2));
		}
		if (retval >= 0) break; // received response.
		else if (errno == EAGAIN || errno == EWOULDBLOCK) { //retval < 0
			perror("Receive failed");	
			fprintf(stderr, "Attempt %d: Send Timeout.\n", i+1);
			set_timer(socket, retry_time);
			retry_time = retry_time * 2.0;
		}
	}
	if (i == 3) exit(1); // all attempts failed
	set_timer(socket, calculate_new_timeout(&t1, &t2));
	
	// read received packet.
	printf("Recv %d bytes \n", retval);
	struct packet *ack = (struct packet*) buf;
	if (ack->hdr.type != ACK) {
		printf("Not an ack response\n");
	}
	if (ack->hdr.ack_num != last_sent_seq+1) {
		printf("Invalid ack number %d %d\n", ack->hdr.ack_num, last_sent_seq);
		exit(1);
	}
	printf("Recv ACK with seq_num %d ack_num %d\n", ack->hdr.seq_num, ack->hdr.ack_num);
	last_sent_seq++;
	server_seq_num = ack->hdr.seq_num;
	
	return 0;
}

void tcp_close(int socket, struct sockaddr* addr) {
	int retval;
	char buf[BUF_SIZE];
	memset(&buf, 0, sizeof buf);

	// send data
	struct packet *fin = malloc(sizeof *fin);
	fin->hdr.seq_num = last_sent_seq;
	fin->hdr.ack_num = server_seq_num+1;
	fin->hdr.type = FIN;
	fin->hdr.pkt_length = 0;
	
	// re-try connection 3 times
	int i; // loop index
	double retry_time = current_timer; // timeout increase
	for (i = 0; i < 3; ++i) {
		retval = sendto(socket, (char *) fin, sizeof *fin, 0, addr, sizeof (*addr));
		if(retval < 0) { perror("Send failed"); exit(1); }
		
		// wait for ack.
		struct sockaddr_in sender_addr;
		int len = sizeof(sender_addr);
		retval = recvfrom(socket, buf, sizeof (struct packet), 0, 
											(struct sockaddr*)&sender_addr, (socklen_t*)&len);
		if (retval >= 0) break; // received response.
		else if (errno == EAGAIN || errno == EWOULDBLOCK) { //retval < 0
			perror("Receive failed");	
			fprintf(stderr, "Attempt %d: Send Timeout.\n", i+1);
			retry_time = retry_time * 2.0;
			set_timer(socket, retry_time);
		}
	}
	if (i == 3) exit(1); // all attempts failed
	
	// read received packet.
	printf("Recv %d bytes \n", retval);
	struct packet *ack = (struct packet*) buf;
	if (ack->hdr.type != ACK) {
		printf("Not an ack response\n");
	}
	if (ack->hdr.ack_num != last_sent_seq+1) {
		printf("Invalid ack number\n");
	}
	printf("Recv ACK with seq_num %d ack_num %d\n", ack->hdr.seq_num, ack->hdr.ack_num);
	last_sent_seq++;
	server_seq_num = ack->hdr.seq_num;
}

int main(int argc, char **argv) {	
	if(argc<4) {
		printf("Usage: ./sender <ip> <port> <filename>\n");
	}
	int sock = socket(AF_INET, SOCK_DGRAM, 0);
	if(sock < 0) {
		perror("Creating socket failed: ");
		exit(1);
	}
	
	struct sockaddr_in addr; 	// internet socket address data structure
	addr.sin_family = AF_INET;
	addr.sin_port = htons(atoi(argv[2])); // byte order is significant
	inet_pton(AF_INET,argv[1],&addr.sin_addr.s_addr);
	
	char buf[1000];
	memset(&buf,0,sizeof(buf));
	FILE *f = fopen(argv[3],"r");
	if(!f) {
		perror("problem opening file");
	}
	
	// Set initial timeout.
	current_timer = set_timer(sock, current_timer);
	
	init_tcp(sock, (struct sockaddr*)&addr);
	
	int retval;
	while((retval = fread(buf,1,1000,f)) > 0) {
		printf("reading file %d \n", retval);
		if (retval == 0) {
			if (ferror(f)) {
				printf("encountered error reading file \n");
				break;
			}
			else if (feof(f)) {
				printf("hit EOF\n");
			}
		}
		
		int rv = tcp_send(sock, (struct sockaddr*)&addr, buf, retval);
		// int send_count = sendto(sock, buf, retval, 0,
														// (struct sockaddr*)&addr,sizeof(addr));
		// if(send_count<0) { perror("Send failed");	exit(1); }
	}
	tcp_close(sock, (struct sockaddr*)&addr);
	
	shutdown(sock,SHUT_RDWR);
	close(sock);
}


