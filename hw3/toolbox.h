#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>

#define RED   "\x1B[31m"
#define GRN   "\x1B[32m"
#define YEL   "\x1B[33m"
#define BLU   "\x1B[34m"
#define MAG   "\x1B[35m"
#define CYN   "\x1B[36m"
#define WHT   "\x1B[37m"
#define RESET "\x1B[0m"

#define DEBUG true

void close_socket(int s){
	close(s);
}

void print_cwnd(int cwnd){
	printf(GRN "CWND = %d\n" RESET, cwnd);
}

void print_duplicate(){
	printf(GRN "3 duplicate ack\n" RESET);
}

void print_timeout(){
	printf(GRN "Time out\n" RESET);
}

// #######################################################################################

typedef enum {
	_SLOW_START,
	_C_AVOID,
	_RECOVER,
} FSM_STATE;

struct TCP_FSM {
	int seq, last_acked;
	int cwnd, cwnd_seq;
  int dup_ack, dup_count;
  int ack;
  int ssthresh;
  FSM_STATE state;
	int sw_head;
	int slide_window[256];
};

struct TCP_PK {
	int ack;
	int seq;
	char data[100];
  int data_size;
};

typedef enum {
	_ACKED,
	_DUP,
	_OUT_ORDER,
  _UNKOWN,
} ACK_STATE;

void debug_printf(const char *fmt, ...) {
	if(DEBUG){
		va_list args;
		va_start(args, fmt);
		vprintf(fmt, args);
		va_end(args);
	}
}

int window_id(struct TCP_FSM* fsm, int seq) {
	int temp = 0;
	temp = ((seq - fsm->cwnd_seq) + fsm->sw_head) % fsm->cwnd;
	if(temp < 0){
		temp = (fsm->sw_head) % fsm->cwnd;
	}
	return temp;
	// return ((fsm->cwnd_seq % fsm->cwnd) + seq) % fsm->cwnd;
}

ACK_STATE ACK_check(struct TCP_PK *pk, struct TCP_FSM *fsm) {
	int cwnd_index = 0;
	cwnd_index = window_id(fsm, pk->ack - 1);

	if (pk->ack >= 0 && (fsm->slide_window[cwnd_index] >= 1 || fsm->last_acked == pk->ack))
		return _DUP;
  else if (pk->ack >= 0 && fsm->slide_window[cwnd_index] == 0)
    return _ACKED;
  else if (pk->ack >= 0 && fsm->slide_window[cwnd_index] < 0)
    return _OUT_ORDER;
  else
    return _UNKOWN;
}

void SW_rerange(struct TCP_FSM *fsm) {
	/*
	reragne value in slide_window to let head located at 0(array index);
	Do this before resize slideing_window.
	*/
	int new_sw[256] = { 0 };
	for (size_t i = 0; i < 256; i++)
		new_sw[i] = -1;

	for (int i = fsm->sw_head; i < fsm->sw_head + fsm->cwnd; i++) {
		int val = fsm->slide_window[i % fsm->cwnd];
		new_sw[i - fsm->sw_head] = val;
	}

	memcpy(&fsm->slide_window, &new_sw, sizeof(new_sw));
	fsm->sw_head = 0;
}

void SW_reset(struct TCP_FSM* fsm) {
	for (size_t i = 0; i < 256; i++)
		fsm->slide_window[i] = -1;
}
