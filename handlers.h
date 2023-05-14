#ifndef PARTB_H
#define PARTB_H

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
#include <sys/un.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdbool.h>
#include "util.h"


#define BUFFER_SIZE 32768 // 32KB for the buffer udp 

/**
 * @brief send and recv data using udp tcp or unix socket stream or dgram
 * 
 */
void send_data(char *, char* , char* , int , int, int , bool);
int recive_data(char*,int , int, int ,int , bool);

/**
 * 
 * @brief send and recv using mmap
 * 
 */

void send_mmap(char* , char* ,bool);
void recv_mmap(char* , char* ,int ,bool);
/**
 * @brief send and recv using pipe
 * 
 */

void pipe_server(char *, char* ,bool );
void pipe_client(char *, char* ,bool );

#endif