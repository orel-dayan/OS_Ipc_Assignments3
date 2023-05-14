#ifndef UTIL_H
#define UTIL_H

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
#include <arpa/inet.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdbool.h>

#define  CHUNK_SIZE 1024 * 1024 // 1MB
/**
 * @brief  generate file data 100MB
 * 
 */
void generate_data(char *, long, int);
/**
 * @brief  generate checksum for file
 * 
 * @return uint32_t  checksum
 */

uint32_t CHECKSUM(char *, int);
/**
 * @brief remove after the file
 * 
 * @return int 
 */

int remove_after(char *, int);
/**
 * @brief  get the minimum of two numbers
 * 
 * @return int 
 */

int getMin(int, int);
/**
 * @brief Get the Size object of file
 * 
 * @return int 
 */

int getSize(char *);

void print_times(struct timeval *, struct timeval *);


#endif // UTIL_H