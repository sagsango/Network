#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>

#define BUF_SIZE 1400
#define ALPHA_EWMA 0.125
#define BETA_EWMA_DEV 0.25

enum enum_state {OPEN_CONN=0, WAIT_DATA=1};

/* Packet data structure */
struct packet_hdr {
	int seq_num;
	int ack_num;
	long pkt_length;
	enum {DATA=0, SYN=1, ACK=2, FIN=3} type;
};

struct packet {
	struct packet_hdr hdr;
	char payload[];
};
/* End of packet data structure. */

double set_timer(int sock, double usec) {
	struct timeval tv;
	if (usec < 1.0) {
		usec = 1.0;
	}
	long temp_usec = (long) (usec * 1000000);
	tv.tv_sec = temp_usec / 1000000;
	tv.tv_usec = temp_usec % 1000000;
	fprintf(stderr, "Timeout %ld %ld\n", tv.tv_sec, tv.tv_usec);
	if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (void*) &tv, sizeof tv) < 0) {
		fprintf(stderr, "%s\n", strerror(errno));
		exit(1);
	}
	if (setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (void*) &tv, sizeof tv) < 0) {
		fprintf(stderr, "%s\n", strerror(errno));
		exit(1);
	}
	return usec;
}

double get_elapsed_time(struct timeval *t1, struct timeval *t2) {
	long start = (long) t1->tv_sec * 1000000L + t1->tv_usec;
	long end = (long) t2->tv_sec * 1000000L + t2->tv_usec;
	double duration = (double) (end-start) / 1000000.0;
	printf("Elapsed time %f\n", duration);
	return duration;
}

double get_estimated_rtt(double old_est_rtt, double sample_rtt) {
	return (1.0-ALPHA_EWMA) * old_est_rtt + ALPHA_EWMA * sample_rtt;
}

double get_dev_rtt(double old_dev_rtt, double est_rtt, double sample_rtt) {
	return (1.0-BETA_EWMA_DEV) * old_dev_rtt + BETA_EWMA_DEV * fabs(sample_rtt - est_rtt);
}

double get_timeout_interval(double est_rtt, double dev_rtt) {
	return est_rtt + 4 * dev_rtt;
}
