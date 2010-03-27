#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <strings.h>
#include  "tennix_server.h"

/* Sample UDP server */

static int sockfd = 0;
static struct sockaddr_in client_addr;


void server_init()
{
	struct sockaddr_in server_addr;

        if (!sockfd) {

	    sockfd = socket(AF_INET,SOCK_DGRAM,0);

	    bzero(&server_addr,sizeof(server_addr));
	    server_addr.sin_family = AF_INET;
	    server_addr.sin_addr.s_addr=htonl(INADDR_ANY);
	    server_addr.sin_port=htons(TENNIX_SERVER_PORT);
	    bind(sockfd,(struct sockaddr *)&server_addr,sizeof(server_addr));
        }

	server_listen_for_connection(sockfd);

}

int
server_listen_for_connection( int sockfd)
{
	struct NN_to_tennix_msg msg;
        socklen_t len;
	int n, ret;

	for (;;)
	{
                printf(" Waiting for GAME START COMMAND \n");
		len = sizeof(client_addr);
		n = recvfrom(sockfd, &msg, sizeof(msg), 0, (struct sockaddr *)&client_addr, &len);

		if (n != sizeof(msg))
		{
			printf("ERROR: Recieved Incomplete message\n");
		} else {
			ret = server_process_msg(&msg);
			if (ret == ERROR)
			{
				printf("%s: ERROR: processing message\n", __FUNCTION__);
			} else if (ret == START_GAME)
			{
				/* Break out and start game */
				break;
			}
		}
	}
}


int
server_listen_for_response(int sockfd, struct NN_to_tennix_msg *msg)
{
    socklen_t len;
    int n;

	for (;;) /* We might not need this for() loop */
	{
		len = sizeof(client_addr);
		n = recvfrom(sockfd, msg, sizeof(struct NN_to_tennix_msg), 0, (struct sockaddr *) &client_addr, &len);

		if (n != sizeof(struct NN_to_tennix_msg))
		{
			printf("%s: ERROR: Recieved Incomplete message\n", __FUNCTION__);
		} else {
			break;
		}
	}
}

int
server_send_evaluation(float fitness)
{
    struct tennix_to_NN_msg msg;

    msg.msg_type = END_GAME;
    msg.fitness = fitness;
    printf("\n\n##################################################\n\nFITNESS = %f\n\n################################################################\n",
            msg.fitness);
    //sleep(0);

    sendto(sockfd, &msg, sizeof(msg), 0, (struct sockaddr *) &client_addr, sizeof(client_addr));

}

int
server_get_input_from_NN(float opponent_x, float opponent_y, float ball_x, float ball_y, char *key)
{
	struct tennix_to_NN_msg msg;
	struct NN_to_tennix_msg NN_resp;

	msg.opponent_x = opponent_x;
	msg.opponent_y = opponent_y;
	msg.ball_x     = ball_x;
	msg.ball_y     = ball_y;
        msg.seq_no     = 1234;

	sendto(sockfd, &msg, sizeof(msg), 0, (struct sockaddr *)&client_addr, sizeof(client_addr));

	server_listen_for_response(sockfd, &NN_resp);

//        printf (" UP = %d  DOWN = %d  HIT = %d ===========================================\n", NN_resp.keys[UP], NN_resp.keys[DOWN], NN_resp.keys[HIT]);

        memcpy(key, NN_resp.keys, sizeof(NN_resp.keys));
}

int
server_process_msg (struct NN_to_tennix_msg *msg)
{
	switch (msg->msg_type)
	{

	    case GAME_INIT:
	    {
		printf(" GAME_INIT msg recieved\n");
                printf("keys[1] = %d\n", msg->keys[1]);
		return START_GAME;
	    }
	    default:
	    {
		return ERROR;
	    }
	}
}


