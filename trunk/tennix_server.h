#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>

#define TENNIX_SERVER_PORT 32000

#define TRUE  1
#define FALSE 0
#define ERROR -1

#define START_GAME  1
#define ERROR      -1


enum key_types {
	UP   = 0,
	DOWN = 1,
	HIT  = 2
};

enum msg_type {
    GAME_INIT = 0,
    NN_RESPONSE = 1,
    END_GAME  = 2
};

struct NN_to_tennix_msg {
	int msg_type; /* Type of message */
	int keys[3];  /* UP, DOWN, HIT */
	int seq_no;
};

struct tennix_to_NN_msg {
	int msg_type;
        float darwin_x; //Position of Darwin
        float darwin_y;
	float opponent_x;
	float opponent_y;
	float ball_x;
	float ball_y;
	int seq_no;
        float fitness;
	int winner;
};

void server_init();
int server_listen_for_connection( int sockfd);
int server_listen_for_response(int sockfd, struct NN_to_tennix_msg *msg) ;
int server_send_evaluation(float fitness, int winner);
int server_get_input_from_NN(float darwin_x, float darwin_y, float opponent_x, float opponent_y, float ball_x, float ball_y, int *key);
int server_process_msg (struct NN_to_tennix_msg *msg);

