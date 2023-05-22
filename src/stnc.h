
#ifndef STNC_H
#define STNC_H

#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netinet/tcp.h>
#include <sys/time.h>
#include <poll.h>
#include <arpa/inet.h>
#include <stdbool.h>

#include "handlers.h"

#define MSG_BUFF 1024

void handle_client(char *ip, char *port);

void handle_server(char *port);

void print_help();

#endif