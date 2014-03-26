#include <stdio.h>
#include "gpioInterface.h"
#include "timeutil.h"
#include <poll.h>
#include <signal.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#define POLL_TIMEOUT (1 *1000)
#define MAX_BUF 64
#define MAX_WAIT 6000000
#define MIN_WAIT 2000000
#define ITERATIONS 10

static int keepgoing = 1;	// Set to 0 when ctrl-c is pressed
void *processPinForPlayer(void *ptr);

typedef struct Player {
    long startTimes[ITERATIONS];
    long endTimes[ITERATIONS];
    int currentIteration;
    char name[255];
} Player;

typedef struct ReactionArgs{
    int gpio_fd;
    int outputPin;
    uint32_t gpioInputPort;
    Player *player;
} ReactionArgs;


long getCurrentNs(){
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return ts.tv_nsec;
}

void turnOnLightAfterRandomTime(int outputPin, Player *player){
    int r = rand() % (MAX_WAIT-MIN_WAIT) + MIN_WAIT;
    usleep(r);
    gpio_set_value(outputPin, 1);
    player->startTimes[player->currentIteration] = getCurrentNs();
}

void *processPinForPlayer(void *ptr)
{
    ReactionArgs *args = ptr;
    int gpio_fd = args->gpio_fd;
    int outputPin = args->outputPin;
    uint32_t gpioInputPort = args->gpioInputPort;
    Player *player = args->player;



	struct pollfd fdset[2];
	int timeout, rc;
	int nfds = 2;
	char buf[MAX_BUF];
	int len;

	char set_value[5];
	char prevState;

	prevState = '?';
	timeout = POLL_TIMEOUT;

	while (keepgoing && player->currentIteration < 10) {
			memset((void*)fdset, 0, sizeof(fdset));

			fdset[0].fd = STDIN_FILENO;
			fdset[0].events = POLLIN;

			fdset[1].fd = gpio_fd;
			fdset[1].events = POLLPRI;

			rc = poll(fdset, nfds, timeout);

			if (rc < 0) {
				printf("\npoll() failed!\n");
				return (void*)"";
			}

			if (rc == 0) {
				printf(".");
			}

			if (fdset[1].revents & POLLPRI) {
				lseek(fdset[1].fd, 0, SEEK_SET);  // Read from the start of the file
				len = read(fdset[1].fd, buf, MAX_BUF);
				printf("\npoll() GPIO %d interrupt occurred, value=%c, len=%d\n",
					 gpioInputPort, buf[0], len);

				if (buf[0]!=prevState)
				{
					if (buf[0]=='0')
					{
						turnOnLightAfterRandomTime(outputPin, player);
					}
					else
					{
						player->endTimes[player->currentIteration] = getCurrentNs();
						long reaction = player->endTimes[player->currentIteration] - player->startTimes[player->currentIteration];
						printf("Reaction time for %s on round %d was %ld", player->name, player->currentIteration, reaction);

						player->currentIteration++;

						//Write our value of "1" to the file
						gpio_set_value(outputPin, 0);
					}
					prevState = buf[0];
				}
			}
			if (fdset[0].revents & POLLIN) {
				(void)read(fdset[0].fd, buf, 1);
				printf("\npoll() stdin read 0x%2.2X\n", (unsigned int) buf[0]);
			}
			fflush(stdout);
		}
}

void signal_handler(int sig) {
	printf("Ctrl-c pressed. Exiting game...\n");
	keepgoing = 0;
}

int main(int argc, const char* argv[])
{
    Player player1;
    Player player2;
    uint32_t gpio_fd_P1;
	uint32_t gpio_fd_P2;
	uint8_t gpioInputPortP1;
	uint8_t gpioOutputPortP1;
	uint8_t gpioInputPortP2;
	uint8_t gpioOutputPortP2;
	char ioPort[56];

	if(argc != 3) {
		printf("Please include the two player's names");
		return 0;
	}

    strcpy(player1.name, argv[1]);
    strcpy(player2.name, argv[2]);
	player1.currentIteration = 0;
	player2.currentIteration = 0;

	// Hook up the signal handler
	signal(SIGINT, signal_handler);

	// Hard coded ports
	gpioInputPortP1 = 47;
	gpioOutputPortP1 = 26;
	gpioInputPortP2 = 27;
	gpioOutputPortP2 = 46;

	printf("Hello players %s and %s \n",argv[1],argv[2]);
	printf("\n%s will use the Left Button.",argv[1]);		// Player 1
	printf("\n%s will use the Right Button.",argv[2]);	// Player 2

	// Setup the input ports
	(void)gpio_export(gpioInputPortP1);
	(void)gpio_set_dir(gpioInputPortP1, 0);
	(void)gpio_set_edge(gpioInputPortP1, GPIO_BOTH_EDGES);
	gpio_fd_P1 = gpio_fd_open(gpioInputPortP1);

	(void)gpio_export(gpioInputPortP2);
	(void)gpio_set_dir(gpioInputPortP2, 0);
	(void)gpio_set_edge(gpioInputPortP2, GPIO_BOTH_EDGES);
	gpio_fd_P2 = gpio_fd_open(gpioInputPortP2);

	// Setup the output ports
	(void)gpio_export(gpioOutputPortP1);
	(void)gpio_set_dir(gpioOutputPortP1, 1);
	(void)gpio_fd_open(gpioOutputPortP1);

	(void)gpio_export(gpioOutputPortP2);
	(void)gpio_set_dir(gpioOutputPortP2, 1);
	(void)gpio_fd_open(gpioOutputPortP2);

    pthread_t thread1, thread2;
    int iret1, iret2;

    ReactionArgs *player1Args;
    player1Args->gpio_fd = gpio_fd_P1;
    player1Args->outputPin = gpioOutputPortP1;
    player1Args->gpioInputPort = gpioInputPortP1;
    player1Args->player = &player1;
    iret1 = pthread_create(&thread1, NULL, processPinForPlayer, (void*)player1Args);

    ReactionArgs *player2Args;
    player2Args->gpio_fd = gpio_fd_P2;
    player2Args->outputPin = gpioOutputPortP2;
    player2Args->gpioInputPort = gpioInputPortP2;
    player2Args->player = &player2;
    iret2 = pthread_create(&thread2, NULL, processPinForPlayer, (void*)player2Args);

    pthread_join(thread1,NULL);
    pthread_join(thread2, NULL);

	gpio_fd_close(gpio_fd_P1);
	gpio_fd_close(gpio_fd_P2);

	// Unexport the pins
	gpio_unexport(gpioInputPortP1);
	gpio_unexport(gpioOutputPortP1);
	gpio_unexport(gpioInputPortP2);
	gpio_unexport(gpioOutputPortP2);

	return 0;
}
