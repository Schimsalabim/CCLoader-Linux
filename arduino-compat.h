#define HIGH 1
#define LOW  0

#define OUTPUT 0
#define INPUT  1

#define GPIO_CHIP "gpiochip0"

#include <gpiod.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/ioctl.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

struct gpiod_chip *chip;

#define TCPIP_PORT 3434

int sockfd;
int client;


void tcpip_accept() {

    struct sockaddr_in cclient;
    int clientsize = sizeof(cclient);

    client = accept(sockfd, (struct sockaddr *) &cclient, &clientsize);
    if (client == -1) {
        printf("accept() - failed: %s", strerror(errno));
        exit(-1);
    }

    printf("accept() - connected.");
}


void 
Serial_begin(int baudrate) {

	struct sockaddr_in serv_addr;
    int val = 1;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);// | SOCK_NONBLOCK, 0);
    if (sockfd < 0)
        printf("socket() - failed.");

    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
    //fcntl(sockfd, F_SETFL, O_NONBLOCK);

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(TCPIP_PORT);

    if (bind(sockfd, (struct sockaddr *) &serv_addr,
                                    sizeof(serv_addr)) < 0) {
        printf("bind() - failed.");
    	exit(-1);
    }

    printf("Start listen.");

    if (listen(sockfd, 1) < 0)   {//MAGIC NUMBER ??
        printf("listen() - failed.");
        exit(-1);
    }

    printf("Socket established");

    tcpip_accept();
}

void
Serial_write(char kar) {
	write(client, &kar, 1);
}

char
Serial_read() {
	char kar;
	read(client, &kar, 1);

	return kar;
}

int
Serial_available() {

	int nread;
	ioctl(client, FIONREAD, &nread);
	return nread;
}

void 
gpiod_init() {
	chip = gpiod_chip_open_by_name(GPIO_CHIP);
	if (chip == NULL) {
		fprintf(stderr, "Failed to acquire %s", GPIO_CHIP);
		exit(-1);
	}
}

struct gpiod_line *
gpiod_init_line(int pin) {
	struct gpiod_line *line = 
					 gpiod_chip_get_line(chip, pin);
	if (line == NULL)
		fprintf(stderr, "Failed to acquire line");

	if (gpiod_line_request_input(line, "arduino-compat") == -1)
		fprintf(stderr, "Failed to request line %s", strerror(errno));

	return line;
}

void delay(int millis) {
	usleep(millis * 1000);
}

int digitalRead(struct gpiod_line *line) {
	if (gpiod_line_set_direction_input(line) == -1)
		fprintf(stderr, "Failed to set input direction.");

	return gpiod_line_get_value(line);
}

void pinMode(struct gpiod_line *line, int mode) {
	if (mode == INPUT) 
		gpiod_line_set_direction_input(line);
}

void digitalWrite(struct gpiod_line *line, int value) {

	if (gpiod_line_set_direction_output(line, value) == -1)
		fprintf(stderr, "Failed to set output direction.");

}