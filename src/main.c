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

typedef struct Player {
    struct timespec elapsedLatency;
    int currentIteration;
    char name[255];
} Player;

typedef struct ReactionArgs{
    uint32_t gpio_fd;
    int outputPin;
    int gpioInputPort;
    Player *player;
} ReactionArgs;

void *processPinForPlayer(void* ptr);
uint32_t setupGpio(uint8_t gpioInputPort, uint8_t gpioOutputPort);
void startThreadForPlayer(uint32_t gpio_fd, uint8_t outputPin, uint8_t inputPin, Player *player, pthread_t *thread );
void gpioCleanup(uint32_t gpio_fd, uint8_t inputPin, uint8_t outputPin);
void turnOnLightAfterRandomTime(int outputPin, Player *player);
void signal_handler(int sig);


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

	if(argc != 3) {
		printf("Please include the two player's names");
		return 0;
	}

    strcpy(player1.name, argv[1]);
    strcpy(player2.name, argv[2]);
	player1.currentIteration = 0;
	player1.elapsedLatency.tv_nsec = 0;
	player1.elapsedLatency.tv_sec = 0;
	player2.currentIteration = 0;
	player2.elapsedLatency.tv_nsec = 0;
	player2.elapsedLatency.tv_sec = 0;

	// Hook up the signal handler
	signal(SIGINT, signal_handler);

	// Hard coded ports
	gpioInputPortP1 = 47;
	gpioOutputPortP1 = 26;
	gpioInputPortP2 = 27;
	gpioOutputPortP2 = 46;

	printf("Hello players %s and %s \n",player1.name,player2.name);
	printf("\n%s will use the Left Button.",player1.name);
	printf("\n%s will use the Right Button.",player2.name);

    gpio_fd_P1 = setupGpio(gpioInputPortP1, gpioOutputPortP1);
    gpio_fd_P2 = setupGpio(gpioInputPortP2, gpioOutputPortP2);

    pthread_t thread1, thread2;

    startThreadForPlayer(gpio_fd_P1, gpioOutputPortP1, gpioInputPortP1, &player1, &thread1);
    startThreadForPlayer(gpio_fd_P2, gpioOutputPortP2, gpioInputPortP2, &player2, &thread2);

    pthread_join(thread1,NULL);
    pthread_join(thread2, NULL);

    gpioCleanup(gpio_fd_P1, gpioInputPortP1, gpioOutputPortP1);
    gpioCleanup(gpio_fd_P2, gpioInputPortP2, gpioOutputPortP2);

    printf("\nGame Over!");
    printf("\nPlayer1 latency was %d", timespectoms(&player1.elapsedLatency));
    printf("\nPlayer2 latency was %d\n", timespectoms(&player2.elapsedLatency));

	return 0;
}

void signal_handler(int sig)
{
	printf("Ctrl-c pressed. Exiting game...\n");
	keepgoing = 0;
}

uint32_t setupGpio(uint8_t gpioInputPort, uint8_t gpioOutputPort)
{
    uint32_t gpio_fd;
    (void)gpio_export(gpioInputPort);
	(void)gpio_set_dir(gpioInputPort, 0);
	(void)gpio_set_edge(gpioInputPort, GPIO_BOTH_EDGES);
	gpio_fd = gpio_fd_open(gpioInputPort);

    (void)gpio_export(gpioOutputPort);
	(void)gpio_set_dir(gpioOutputPort, 1);
	(void)gpio_fd_open(gpioOutputPort);
	return gpio_fd;
}

void startThreadForPlayer(uint32_t gpio_fd, uint8_t outputPin, uint8_t inputPin, Player *player, pthread_t *thread)
{
    ReactionArgs *player1Args;
    player1Args = (ReactionArgs *) malloc(sizeof(ReactionArgs));
    player1Args->gpio_fd = gpio_fd;
    player1Args->outputPin = outputPin;
    player1Args->gpioInputPort = inputPin;
    player1Args->player = player;
    pthread_create(thread, NULL, processPinForPlayer, (void *) player1Args);
}

void gpioCleanup(uint32_t gpio_fd, uint8_t inputPin, uint8_t outputPin)
{
    gpio_fd_close(gpio_fd);
    gpio_unexport(inputPin);
    gpio_unexport(outputPin);
}

void *processPinForPlayer(void* ptr)
{
    struct ReactionArgs *args;
    args = (ReactionArgs *) ptr;
    uint32_t gpio_fd = args->gpio_fd;
    int outputPin = args->outputPin;
    int gpioInputPort = args->gpioInputPort;
    Player *player = args->player;

	struct pollfd fdset[2];
	int timeout, rc;
	int nfds = 2;
	char buf[MAX_BUF];
	int len;
    struct timespec startTime;

	char set_value[5];
	char prevState;

	prevState = '?';
	timeout = POLL_TIMEOUT;

	while (keepgoing && player->currentIteration < 3) {
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
                    clock_gettime(CLOCK_REALTIME, &startTime);
                }
                else
                {
                    struct timespec endTime;
                    struct timespec elapsedThisRound;
                    clock_gettime(CLOCK_REALTIME, &endTime);
                    timeval_subtract(&elapsedThisRound,&endTime,&startTime);

                    printf("Reaction time for %s on round %d was %d", player->name, player->currentIteration+1, timespectoms(&elapsedThisRound));
                    timeval_add(&player->elapsedLatency,&player->elapsedLatency,&elapsedThisRound);

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
    free(args);
}

void turnOnLightAfterRandomTime(int outputPin, Player *player)
{
    int r = rand() % (MAX_WAIT-MIN_WAIT) + MIN_WAIT;
    usleep(r);
    gpio_set_value(outputPin, 1);
}


