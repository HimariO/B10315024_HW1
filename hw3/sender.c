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

	struct TCP_FSM fsm = {
    0, 0,
    1, 0,
    -1, 0,
    -1, 10, _SLOW_START, 0
  };
  memset(fsm.slide_window, 0, sizeof(fsm.slide_window));
  for(int i=0; i<256; i++)
    fsm.slide_window[i] = -1;

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
    debug_printf("--------------------------------------------------------\n");
		FD_ZERO(&readfds);
		FD_SET(s, &readfds);
		tv.tv_sec = 1;
		tv.tv_usec = 0;

		// int start_seq = fsm.cwnd_seq > fsm.seq ? fsm.cwnd_seq : fsm.seq;
    debug_printf("Check cwnd seq %d\n", window_id(&fsm, fsm.cwnd_seq));
    while(fsm.slide_window[window_id(&fsm, fsm.cwnd_seq)] == 1){
      fsm.slide_window[window_id(&fsm, fsm.cwnd_seq)] = -1; // reset state
      fsm.cwnd_seq++;
      fsm.sw_head++;
      fsm.sw_head %= fsm.cwnd;
      debug_printf("udpate cwnd head %d\n", fsm.cwnd_seq);
    }

    debug_printf("WINDOW [%d, %d]\n", fsm.cwnd_seq, fsm.cwnd_seq + fsm.cwnd);

		for (int j = fsm.cwnd_seq; j < fsm.cwnd_seq + fsm.cwnd; j++) {
      int cwnd_index = 0;
      cwnd_index = window_id(&fsm, j);

      if (fsm.slide_window[cwnd_index] == -1) {  // -1 denote pk[seq] is not been send yet.
        struct TCP_PK pk = { -1, 0, "HI there", sizeof("HI there") };

        fsm.seq =  j > fsm.seq ? j : fsm.seq;
        pk.seq = j;

        sendto(s, (void *)&pk, sizeof(TCP_PK), 0, (struct sockaddr*)&si_other, sizeof(si_other));
        debug_printf("SEnd [%d], win id[%d]\n", j, cwnd_index);
        fsm.slide_window[cwnd_index] = 0; // 0 denote sended
      }
		}

		select(s + 1, &readfds, NULL, NULL, &tv); // wait for respone or timeout.

		if (!FD_ISSET(s, &readfds)) {
			// time out!
			fsm.ssthresh = fsm.cwnd / 2;
			fsm.cwnd = 1;
			fsm.dup_count = 0;
			fsm.state = _SLOW_START;

			debug_printf("[%d] TIME OUT\n", fsm.seq);
			print_timeout();
		}
		else {

			struct TCP_PK pk = { -1, 0, "", 0 };

			recvfrom(s, (void *)&pk, sizeof(TCP_PK), 0, (struct sockaddr*) &si_other, (socklen_t*) &slen);
      debug_printf("ACK %d, fsm_lastack %d\n", pk.ack, fsm.last_acked);

      ACK_STATE ack_s = ACK_check(&pk, &fsm);
      int cwnd_index = 0;

			switch(ack_s) {
				case _ACKED:
          debug_printf("[%d] ACKED %d\n", fsm.seq, pk.ack);
					// Normal behavior.
          cwnd_index = window_id(&fsm, pk.ack - 1);

          fsm.last_acked = pk.ack;
					fsm.dup_count = 0;
					// fsm.cwnd_seq = pk.ack >= fsm.cwnd_seq ? pk.ack : fsm.cwnd_seq;
          fsm.slide_window[cwnd_index] = 1; // 1 denote acked

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

          fsm.cwnd = fsm.cwnd > 256 ? 256 : fsm.cwnd;  // set cwnd upper bound to 256 pk.

				break;

				case _DUP:
        debug_printf("[%d] DUP_ACK %d\n", fsm.seq, pk.ack);
					fsm.dup_count += 1;

					if (fsm.dup_count >= 3) { // triblue-dup
						// fsm.cwnd_seq = pk.ack; // for retransmit
						fsm.dup_count = 0;
						fsm.state = _RECOVER;
						fsm.ssthresh = fsm.cwnd / 2;
						fsm.cwnd = fsm.ssthresh + 3;
            print_duplicate();
						print_cwnd(fsm.cwnd);
					}
					fsm.last_acked = pk.ack;
				break;

				case _OUT_ORDER:
        debug_printf("[%d] _OUT_ORDER %d\n", fsm.seq, pk.ack);
				break;

				case _UNKOWN:
        debug_printf("[%d] _UNKOWN %d\n", fsm.seq, pk.ack);
				break;
			}

		}

	}
	/* Insert your codes above */

	close_socket(s);
	return 0;
}
