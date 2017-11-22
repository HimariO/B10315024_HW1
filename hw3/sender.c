#define SRV_IP "127.0.0.1"

#include "toolbox.h"

#define BUFLEN 512
#define NPACK 50
#define PORT 9930
#define BUF_SIZE 1000

 /* diep(), #includes and #defines like in the server */

 struct sockaddr_in initial(){
 	struct sockaddr_in si_other;

 	memset((char *) &si_other, 0, sizeof(si_other));
 	si_other.sin_family = AF_INET;
 	si_other.sin_port = htons(PORT);

 	if (inet_aton(SRV_IP, &si_other.sin_addr)==0){
 		fprintf(stderr, "inet_aton() failed\n");
 		return si_other;
 	}
 	return si_other;
 }


 int connect_socket(void){
    	int s;

     	if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
     		perror("socket");
     	return s;
 }


int main(void){
	printf("[Sender start]\n");

	struct TCP_FSM fsm = { 0, -1, 1, 0, -1, 0, -1, 10, _SLOW_START };

	struct sockaddr_in si_other, recv_ip;
	int s, i, slen = sizeof(si_other), rlen = sizeof(recv_ip);
 	char buf[BUFLEN];

	struct timeval tv;
	fd_set readfds;

	si_other = initial();
	s = connect_socket();

	int send_buffer[NPACK + 1];
	double cwnd_double = 1.0;

	int pre_acked;

	for(i = 0; i < NPACK + 1; i++){
		send_buffer[i] = i;
	}

	/* Insert your codes below */

	while(fsm.seq < NPACK){
		// printf("HIHI");
		FD_ZERO(&readfds);
		FD_SET(s, &readfds);
		tv.tv_sec = 1;
		tv.tv_usec = 0;

		// printf("before send %d", i);
		int start_seq = fsm.cwnd_seq ? fsm.cwnd_seq > fsm.seq : fsm.seq;

		for (int j=start_seq; j <= fsm.cwnd_seq + fsm.cwnd; j++) {
			struct TCP_PK pk = { -1, 0, "HI there", sizeof("HI there") };
			fsm.seq = j ? j > fsm.seq : fsm.seq;
			pk.seq = j;

			sendto(s, (void *)&pk, sizeof(TCP_PK), 0, (struct sockaddr*)&si_other, sizeof(si_other));
		}

		select(s + 1, &readfds, NULL, NULL, &tv); // wait for respone or timeout.

		if (!FD_ISSET(s, &readfds)) {
			// time out!
			fsm.ssthresh = fsm.cwnd / 2;
			fsm.cwnd = 1;
			fsm.dup_count = 0;
			fsm.state = _SLOW_START;

			printf("TIME OUT %d\n", fsm.seq);
			print_timeout();
		}
		else {
			printf("NOT TIME OUT %d\n", fsm.seq);
			struct TCP_PK pk = { -1, 0, "", 0 };

			recvfrom(s, (void *)&pk, sizeof(TCP_PK), 0, (struct sockaddr*) &si_other, (socklen_t*) &slen);

			ACK_STATE ack_s = ACK_check(&pk, &fsm);

			switch(fsm.state) {
				case _ACKED:
					// Normal behavior.
					fsm.last_acked = pk.ack;
					fsm.dup_count = 0;
					fsm.cwnd_seq = pk.ack >= fsm.cwnd_seq ? pk.ack + 1 : fsm.cwnd_seq;

					// cwnd control.
					if (fsm.state == _SLOW_START) {
						fsm.cwnd *= 2;

						if (fsm.cwnd > fsm.ssthresh)
							fsm.state = _C_AVOID;
						print_cwnd(fsm.cwnd);
					}
					else if (fsm.state == _RECOVER) {
						fsm.cwnd = fsm.ssthresh;
						fsm.state = _C_AVOID;
						print_cwnd(fsm.cwnd);
					}
					else if (fsm.state == _C_AVOID) {
						fsm.cwnd += 1;
						print_cwnd(fsm.cwnd);
					}

				break;

				case _DUP:
					fsm.dup_count += 1;
					print_duplicate();

					if (fsm.dup_count >= 3) { // triblue-dup
						fsm.cwnd_seq = pk.ack; // for retransmit
						fsm.dup_count = 0;
						fsm.state = _RECOVER;
						fsm.ssthresh = fsm.cwnd / 2;
						fsm.cwnd = fsm.ssthresh + 3;
						print_cwnd(fsm.cwnd);
					}
					fsm.last_acked = pk.ack;
				break;

				case _OUT_ORDER:
				break;

				case _UNKOWN:
				break;
			}

		}

	}
	/* Insert your codes above */

	close_socket(s);
	return 0;
}
