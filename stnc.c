#include "stnc.h"


char *ip = NULL; // ip address
char *port = NULL; // port number
char *test_type = NULL; // test type
char *test_param = NULL; // test param
char *file_name = NULL; // file name

bool is_performance = false; // performance test
bool is_client = false; // client mode
bool is_server = false; // server mode
bool is_ipv4 = false;
bool is_ipv6 = false;
bool is_uds = false;
bool is_mmap = false;
bool is_pipe = false;
bool is_tcp = false;
bool is_udp = false;
bool is_dgram = false;
bool quiet_mode = false;
bool is_stream = false;


int main(int argc, char *argv[]) {
    if (argc < 3) {
        print_help();
        return 1;
    }
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-c") == 0) {
            is_client = true;
            ip = argv[i + 1];
            port = argv[i + 2];
        } else if (strcmp(argv[i], "-s") == 0) {
            is_server = true;
            port = argv[i + 1];
        } else if (strcmp(argv[i], "-p") == 0) {
            is_performance = true;
            test_type = argv[i + 1];
            test_param = argv[i + 2];
        } else if (strcmp(argv[i], "-q") == 0) {
            quiet_mode = true;
        }
    }

    if (is_performance && is_client) {
        if (!test_type || !test_param) { // if no test type or param 
            print_help();
            return 1;
        }
        if (strcmp(test_type, "ipv6") == 0) {
            is_ipv6 = true;
        } else if (strcmp(test_type, "ipv4") == 0) {
            is_ipv4 = true;
        } else if (strcmp(test_type, "uds") == 0) {
            is_uds = true;
        } else if (strcmp(test_type, "mmap") == 0) {
            is_mmap = true;
        } else if (strcmp(test_type, "pipe") == 0) {
            is_pipe = true;
        } else {
            printf("No such type\n");

        }

        if (strcmp(test_param, "udp") == 0) {
            is_udp = true;
        } else if (strcmp(test_param, "tcp") == 0) {
            is_tcp = true;
        } else if (strcmp(test_param, "dgram") == 0) {
            is_dgram = true;
        } else if (strcmp(test_param, "stream") == 0) {
            is_stream = true;
        } else {
            file_name = test_param;
        }
    }

    if (is_client) {
        handle_client(ip, port);
    } else if (is_server) {
        handle_server(port);
    } else {
        print_help();
    }
    return 0;
}

void print_help() {
    printf("The usage for client is: stnc -c <IP PORT> (optional:-p <type> <param>)\n");
    printf("The usage for server is: stnc -s <PORT> (optional:-p -q)\n");
}


void handle_server(char *port) {

    // Create socket
    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock < 0) {
        printf("ERROR opening socket\n");
        exit(EXIT_FAILURE);
    }
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(port));
    server_addr.sin_addr.s_addr = INADDR_ANY;
    char msg_buff[MSG_BUFF];

    if (bind(sock, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        printf("ERROR on binding\n");
        exit(EXIT_FAILURE);
    }
    if (!quiet_mode)
        printf("Server started\n");
    while (true) {
        if (listen(sock, 1) < 0) {
            printf("ERROR on listen\n");
            exit(EXIT_FAILURE);
        }

        // Accept connection
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_sock = accept(sock, (struct sockaddr *) &client_addr, &client_len);
        if (client_sock < 0) {
            printf("ERROR on accept\n");
            exit(EXIT_FAILURE);
        }

        if (!quiet_mode) {
            printf("Client connected to server\n");
        }

        struct pollfd fds[2];
        fds[0].fd = STDIN_FILENO;
        fds[0].events = POLLIN;
        fds[1].fd = client_sock;
        fds[1].events = POLLIN;


        while (true) {
            int ret = poll(fds, 2, -1);
            if (ret < 0) {
                printf("ERROR poll() failed\n");
                exit(EXIT_FAILURE);
            }
            if (fds[0].revents & POLLIN) {

                int bytes_read = read(STDIN_FILENO, msg_buff, MSG_BUFF);
                if (bytes_read < 0) {
                    printf("ERROR read() failed\n");
                    exit(EXIT_FAILURE);
                }
                msg_buff[bytes_read] = '\0';
                if (send(client_sock, msg_buff, bytes_read, 0) < 0) {
                    printf("ERROR send() failed\n");
                    exit(EXIT_FAILURE);
                }
                bzero(msg_buff, MSG_BUFF);
            }
            if (fds[1].revents & POLLIN) {

                int bytes_recv = recv(client_sock, msg_buff, MSG_BUFF - 1,
                                      0); // -1 to leave room for a null terminator
                if (bytes_recv < 0) {
                    printf("ERROR recv() failed\n");
                    exit(EXIT_FAILURE);
                }
                if (bytes_recv == 0) {
                    if (!quiet_mode) {
                        printf("Client disconnected\n");
                    }
                    break;
                }
                msg_buff[bytes_recv] = '\0';
                if (!quiet_mode) {
                    printf("Client: %s", msg_buff);
                }

                if (is_performance) // if performance mode is on 
                {
                    int size = 0; // size of the file
                    int recv_size = 0; // size of the file that has been recieved
                    uint32_t checksum_recv = 0; // checksum of the file that has been recieved
                    struct timeval start, end;
                    size = atoi(msg_buff);
                    if (!quiet_mode) // if quiet mode is off
                        printf(" - This is the size of the file \n");
                    bzero(msg_buff, MSG_BUFF);

                    // recieve checksum
                    bytes_recv = recv(client_sock, msg_buff, 20, 0);
                    if (bytes_recv <= 0) {
                        printf("ERROR recv() failed\n");
                        exit(EXIT_FAILURE);
                    }
                    msg_buff[bytes_recv] = '\0';
                    sscanf(msg_buff, "%u", &checksum_recv);

                    if (!quiet_mode)
                        printf("Checksum will be: 0x%08x\n", checksum_recv);
                    bzero(msg_buff, MSG_BUFF);

                    // recieve timestart
                    bytes_recv = recv(client_sock, msg_buff, 20, 0);
                    if (bytes_recv <= 0) {
                        printf("ERROR recv() failed\n");
                        exit(EXIT_FAILURE);
                    }
                    msg_buff[bytes_recv] = '\0';
                    sscanf(msg_buff, "%ld.%06ld", &start.tv_sec, &start.tv_usec);

                    bzero(msg_buff, MSG_BUFF);

                    // recieve test type
                    bytes_recv = recv(client_sock, msg_buff, 20, 0);
                    if (bytes_recv <= 0) {
                        printf("ERROR recv() failed\n");
                        exit(EXIT_FAILURE);
                    }
                    msg_buff[bytes_recv] = '\0';
                    if (!quiet_mode)
                        printf("Test type and param will be: %s\n", msg_buff);


                    char port_for_data[10]; // port for new connection
                    sprintf(port_for_data, "%d", atoi(port) + 1);

                    if (!strcmp(msg_buff, "ipv4 tcp")) {
                        recv_size = recive_data(port_for_data, AF_INET, SOCK_STREAM, IPPROTO_TCP, size, quiet_mode);
                    } else if (!strcmp(msg_buff, "ipv4 udp")) {
                        recv_size = recive_data(port_for_data, AF_INET, SOCK_DGRAM, 0, size, quiet_mode);
                    } else if (!strcmp(msg_buff, "ipv6 tcp")) {
                        recv_size = recive_data(port_for_data, AF_INET6, SOCK_STREAM, IPPROTO_TCP, size, quiet_mode);
                    } else if (!strcmp(msg_buff, "ipv6 udp")) {
                        recv_size = recive_data(port_for_data, AF_INET6, SOCK_DGRAM, 0, size, quiet_mode);
                    } else if (!strcmp(msg_buff, "uds dgram")) {
                        recv_size = recive_data(port_for_data, AF_UNIX, SOCK_DGRAM, 0, size, quiet_mode);
                    } else if (!strcmp(msg_buff, "uds stream")) {
                        recv_size = recive_data(port_for_data, AF_UNIX, SOCK_STREAM, 0, size, quiet_mode);
                    } else if (!strcmp(msg_buff, "mmap")) {
                        bytes_recv = recv(client_sock, msg_buff, MSG_BUFF - 1, 0); // recive file name
                        if (bytes_recv < 0) {
                            printf("ERROR recv() failed\n");
                            exit(EXIT_FAILURE);
                        }

                        sleep(0.1); // wait for file to be created
                        recv_mmap("recived.txt", msg_buff, size, quiet_mode); // copy file from shared memory
                        recv_size = getSize("recived.txt");
                    } else if (!strcmp(msg_buff, "pipe")) {
                        bytes_recv = recv(client_sock, msg_buff, MSG_BUFF - 1, 0); // recive file name
                        if (bytes_recv < 0) {
                            printf("ERROR recv() failed\n");
                            exit(EXIT_FAILURE);
                        }

                        sleep(0.1); // wait for file to be created
                        pipe_server("recived.txt", msg_buff, quiet_mode); // copy file from fifo
                        recv_size = getSize("recived.txt");
                    }

                    gettimeofday(&end, NULL);

                    u_int32_t recieved_file_checksum = CHECKSUM("recived.txt", quiet_mode);
                    if (recieved_file_checksum == checksum_recv &&
                        !quiet_mode) // if checksums are equal and quiet mode is off
                    {
                        printf("Checksums are equal\n");
                    } else if (!quiet_mode) {
                        printf("Unfortunatelly, checksums are not equal\n");
                        if (recv_size != size) {
                            printf("Unfortunatelly, file size is not equal\n");
                        }
                    }

                    printf("%s,", msg_buff);
                    print_times(&start, &end);

                    remove_after("recived.txt", quiet_mode); // remove file after test is done for next test
                }
                bzero(msg_buff, MSG_BUFF);
            }
        }
        close(client_sock);
    }
    close(sock);
}


void handle_client(char *ip, char *port) {
    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock < 0) {
        printf("ERROR opening socket\n");
        exit(EXIT_FAILURE);
    }
    char buffer_msg[MSG_BUFF];
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(port));
    if (inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0) {
        printf("ERROR invalid address\n");
        exit(EXIT_FAILURE);
    }

    if (connect(sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        printf("ERROR connecting\n");
        exit(EXIT_FAILURE);
    }

    if (!quiet_mode)
        printf("Connected to %s:%s\n", ip, port);

    // Create pollfd to monitor stdin and socket
    struct pollfd fds[2];
    fds[0].fd = STDIN_FILENO;
    fds[0].events = POLLIN;
    fds[1].fd = sock;
    fds[1].events = POLLIN;


    while (true) {
        if (is_performance) {
            generate_data("100mb.txt", 100 * 1024 * 1024, quiet_mode);
            char port_for_transfer[10]; // port for new connection
            sprintf(port_for_transfer, "%d", atoi(port) + 1);

            int size = getSize("100mb.txt");
            char str_size[20];
            sprintf(str_size, "%d", size);
            int bytes_sent = send(sock, str_size, strlen(str_size), 0);
            if (bytes_sent < 0) {
                printf("ERROR send() failed\n");
                exit(EXIT_FAILURE);
            }
            sleep(0.2);

            uint32_t checksum_send = CHECKSUM("100mb.txt", quiet_mode);
            char str_checksum[20];
            sprintf(str_checksum, "%d", checksum_send);
            bytes_sent = send(sock, str_checksum, strlen(str_checksum), 0);
            if (bytes_sent < 0) {
                printf("ERROR send() failed\n");
                exit(EXIT_FAILURE);
            }
            sleep(0.1); // wait for server to be ready

            struct timeval start; // start time
            gettimeofday(&start, NULL); // get start time

            // send start time
            char st_time[20];
            sprintf(st_time, "%ld.%06ld", start.tv_sec, start.tv_usec);
            bytes_sent = send(sock, st_time, strlen(st_time), 0);
            if (bytes_sent < 0) {
                printf("ERROR send() failed\n");
                exit(EXIT_FAILURE);
            }
            sleep(0.1);

            if (is_tcp && is_ipv4) {

                bytes_sent = send(sock, "ipv4 tcp", 8, 0);
            } else if (is_tcp && is_ipv6) {
                bytes_sent = send(sock, "ipv6 tcp", 8, 0);
            } else if (is_udp && is_ipv4) {

                bytes_sent = send(sock, "ipv4 udp", 8, 0);
            } else if (is_udp && is_ipv6) {
                bytes_sent = send(sock, "ipv6 udp", 8, 0);
            } else if (is_uds && is_dgram) {
                bytes_sent = send(sock, "uds dgram", 9, 0);
            } else if (is_uds && is_stream) {
                bytes_sent = send(sock, "uds stream", 10, 0);
            } else if (is_mmap) {
                bytes_sent = send(sock, "mmap", 5, 0); // send test command
            } else if (is_pipe) {
                bytes_sent = send(sock, "pipe", 5, 0);  // send the test command to server
            }
            if (bytes_sent < 0) {
                printf("ERROR send() failed\n");
                exit(EXIT_FAILURE);
            }

            sleep(0.2); // wait for server to be ready to receive

            if (is_tcp && is_ipv4) {
                send_data(ip, port_for_transfer, "100mb.txt", AF_INET, SOCK_STREAM, IPPROTO_TCP, quiet_mode);
            } else if (is_udp && is_ipv4) {
                send_data(ip, port_for_transfer, "100mb.txt", AF_INET, SOCK_DGRAM, 0, quiet_mode);
            } else if (is_udp && is_ipv6) {
                send_data(ip, port_for_transfer, "100mb.txt", AF_INET6, SOCK_DGRAM, 0, quiet_mode);
            } else if (is_tcp && is_ipv6) {
                send_data(ip, port_for_transfer, "100mb.txt", AF_INET6, SOCK_STREAM, IPPROTO_TCP, quiet_mode);
            } else if (is_uds && is_dgram) {
                sleep(0.2);
                send_data(0, port_for_transfer, "100mb.txt", AF_UNIX, SOCK_DGRAM, 0, quiet_mode);
            } else if (is_uds && is_stream) {
                sleep(0.2);
                send_data(0, port_for_transfer, "100mb.txt", AF_UNIX, SOCK_STREAM, 0, quiet_mode);
            } else if (is_mmap) {

                send_mmap("100mb.txt", file_name, quiet_mode);
                bytes_sent = send(sock, file_name, strlen(file_name), 0);
                if (bytes_sent < 0) {
                    printf("ERROR send() failed\n");
                    exit(EXIT_FAILURE);
                }
            } else if (is_pipe) {
                bytes_sent = send(sock, file_name, strlen(file_name), 0); \
                if (bytes_sent < 0) {
                    printf("ERROR send() failed\n");
                    exit(EXIT_FAILURE);
                }
                pipe_client("100mb.txt", file_name, quiet_mode); // copy file to named pipe

            }
            remove_after("100mb.txt", quiet_mode);
            exit(EXIT_SUCCESS);
        }


        int ret = poll(fds, 2, -1);
        if (ret < 0) {
            printf("ERROR poll() failed\n");
            exit(EXIT_FAILURE);
        }

        if (fds[0].revents & POLLIN) // check if user input
        {
            // Read user input
            int bytesRead = read(STDIN_FILENO, buffer_msg, MSG_BUFF);
            if (bytesRead < 0) {
                printf("ERROR read() failed\n");
                exit(EXIT_FAILURE);
            }
            buffer_msg[bytesRead] = '\0';

            int bytes_sent = send(sock, buffer_msg, bytesRead, 0); // send message to server 
            if (bytes_sent < 0) {
                printf("ERROR send() failed\n");
                exit(EXIT_FAILURE);
            }
            bzero(buffer_msg, MSG_BUFF); // clear buffer
        }
        if (fds[1].revents & POLLIN) {

            int bytes_recv = recv(sock, buffer_msg, MSG_BUFF - 1, 0);
            if (bytes_recv < 0) {
                printf("ERROR recv() failed\n");
                exit(EXIT_FAILURE);
            }
            if (bytes_recv == 0) {
                printf("Server disconnected\n");
                exit(EXIT_FAILURE);
            }
            buffer_msg[bytes_recv] = '\0';
            printf("Server: %s", buffer_msg);
            bzero(buffer_msg, MSG_BUFF);
        }
    }
    close(sock);

}