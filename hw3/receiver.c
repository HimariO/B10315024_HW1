#include "toolbox.h"

#define BUFLEN 512
#define NPACK 20
#define PORT 9930

#define FREEZE false


int main(void){
	printf("[Receiver start]\n");

	struct sockaddr_in si_me, si_other;
	int s, i, slen=sizeof(si_other);

	char buf[BUFLEN];
	bool recv_buf[NPACK + 1];
	int recv_index = 1;
	bool lose = true;

	recv_buf[0] = true;
	for(i = 1; i < NPACK + 1; i++) recv_buf[i] = false;

	if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
		perror("socket");

	memset((char *) &si_me, 0, sizeof(si_me));
	si_me.sin_family = AF_INET;
	si_me.sin_port = htons(PORT);
	si_me.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(s, (const struct sockaddr*) &si_me, sizeof(si_me))==-1)
		perror("bind");

	while (FREEZE) {
		printf("Sleep 1 sec\n");
		sleep(1);
	}

	while(recv_index < NPACK + 1){
    debug_printf("recv_index %d\n", recv_index);
		struct TCP_PK pk = { -1, 0, "", 0 };

		if (recvfrom(s, (void *)&pk, sizeof(TCP_PK), 0, (struct sockaddr*) &si_other, (socklen_t*) &slen)==-1)
			perror("recvfrom()");

		// int ack = pk.seq;

		if(pk.seq == 3 && lose){
			lose = false;
			continue;
		}

		recv_buf[pk.seq] = true;

		while(recv_buf[recv_index] && recv_index < NPACK + 1) recv_index++;

		printf(
			"Received packet from %s:%d\nSeq: %d\n\n",
			inet_ntoa(si_other.sin_addr),
			ntohs(si_other.sin_port),
			pk.seq
		);

		// memset(buf, 0, BUFLEN);
		// sprintf(buf, "%d", recv_index);
		struct TCP_PK ack_pk = { -1, 0, "", 0 };
		ack_pk.ack = recv_index;

		sendto(s, (void *)&ack_pk, sizeof(TCP_PK), 0, (struct sockaddr*) &si_other, (socklen_t) slen);
	}

	close(s);
	return 0;
}
