#include "handlers.h"

void send_data(char *ip, char *port, char *name, int dm, int type, int protocol, bool q) {

    FILE *file = fopen(name, "rb");
    if (file == NULL) {
        printf("ERROR opening file\n");
        exit(EXIT_FAILURE);
    }
    int size = getSize(name);

    int sock = socket(dm, type, protocol);
    if (sock < 0) {
        printf("ERROR opening socket\n");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_storage addr;
    socklen_t addr_len;

    if (dm == AF_INET6) // ipv6
    {
        int opt = 1;
        setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY, &opt, sizeof(opt)); // set socket to ipv6 only 
        struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *) &addr;
        memset((char *) addr6, 0, sizeof(*addr6)); // clear the struct
        addr6->sin6_family = dm; // set the domain
        addr6->sin6_port = htons(atoi(port)); // set the port
        inet_pton(AF_INET6, ip, &addr6->sin6_addr); // set the address

        addr_len = sizeof(*addr6); // set the length of the address
        addr = *(struct sockaddr_storage *) addr6; // set the address
    } else if (dm == AF_INET) // ipv4
    {
        struct sockaddr_in *addr4 = (struct sockaddr_in *) &addr;
        memset((char *) addr4, 0, sizeof(*addr4));
        addr4->sin_family = dm; // set the domain
        addr4->sin_port = htons(atoi(port)); // set the port
        inet_pton(AF_INET, ip, &addr4->sin_addr);
        addr_len = sizeof(*addr4);
        addr = *(struct sockaddr_storage *) addr4; // set the address
    } else if (dm == AF_UNIX) // unix domain socket
    {
        struct sockaddr_un *addru = (struct sockaddr_un *) &addr;
        memset((char *) addru, 0, sizeof(*addru));
        addru->sun_family = dm; // set the domain
        strcpy(addru->sun_path, port); // set the path to the socket 
        addr_len = sizeof(*addru);
        addr = *(struct sockaddr_storage *) addru;
    }

    if (type == SOCK_STREAM || dm == AF_UNIX)  // TCP or unix domain socket
    {
        if (connect(sock, (struct sockaddr *) &addr, addr_len) < 0) {
            printf("ERROR connecting\n");
            exit(EXIT_FAILURE);
        }
       
    }
    char buf[BUFFER_SIZE] = {0}; // buffer to read from the file
    int sent = 0; // number of bytes sent
    int bytes_read = 0; // number of bytes read from the file
    while (sent < size)  // send the file
    {
        bytes_read = getMin(BUFFER_SIZE, size -
                                         sent); // Read at most BUFFER_SIZE bytes because we don't want to read more than the file size
        fread(buf, 1, bytes_read, file);
        int sent_bytes; // number of bytes sent
        if (type == SOCK_STREAM || dm == AF_UNIX) // TCP or unix domain socket
        {
            sent_bytes = send(sock, buf, bytes_read, 0); // send the data
        } else if (type == SOCK_DGRAM) // UDP
        {
            sleep(0.02); // delay to make sure packets are sent in order
            sent_bytes = sendto(sock, buf, bytes_read, 0, (struct sockaddr *) &addr, sizeof(addr));
        }
        if (sent_bytes < 0) // check for errors
        {
            printf("ERROR send() failed (FILE)\n");
            exit(EXIT_FAILURE);
        }
        sent += sent_bytes;
        bzero(buf, BUFFER_SIZE);
    }
    fclose(file);
    if (!q)
        printf("File sent successfully\n");

    close(sock);
}

int recive_data(char *port, int d, int type, int protocol, int fsize, bool q) {

    int sock = socket(d, type, protocol);
    if (sock < 0) {
        printf("ERROR opening socket\n");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_storage server_add, client_add;
    socklen_t addr_len;

    if (d == AF_INET6) // ipv6
    {
        int opt = 1;
        setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY, &opt, sizeof(opt));
        struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *) &server_add;
        memset((char *) addr6, 0, sizeof(*addr6));
        addr6->sin6_family = d; // set the domain
        addr6->sin6_port = htons(atoi(port));
        addr6->sin6_addr = in6addr_any; // set the address
        addr_len = sizeof(*addr6); // set the length of the address
        server_add = *(struct sockaddr_storage *) addr6;
    } else if (d == AF_INET) // ipv4
    {
        struct sockaddr_in *addr4 = (struct sockaddr_in *) &server_add;
        memset((char *) addr4, 0, sizeof(*addr4));
        addr4->sin_family = d;
        addr4->sin_port = htons(atoi(port));
        addr4->sin_addr.s_addr = INADDR_ANY;
        addr_len = sizeof(*addr4);
        server_add = *(struct sockaddr_storage *) addr4;
    } else if (d == AF_UNIX) // unix domain socket
    {
        struct sockaddr_un *addru = (struct sockaddr_un *) &server_add;
        memset((char *) addru, 0, sizeof(*addru));
        addru->sun_family = d;
        strncpy(addru->sun_path, port, sizeof(addru->sun_path) - 1);

        addr_len = sizeof(*addru);
        server_add = *(struct sockaddr_storage *) addru;
    } else {
        printf("ERROR Invalid domain\n");
        exit(1);
    }

    if (bind(sock, (struct sockaddr *) &server_add, addr_len) < 0) {
        printf("ERROR on binding\n");
        exit(1);
    }

    struct pollfd fds[2];
    fds[0] = (struct pollfd) {.fd = sock, .events = POLLIN};

    int client_sock;
    if (type == SOCK_STREAM) // TCP
    {

        listen(sock, 1);
        if (!q)
            printf("Waiting for connection...\n");

        client_sock = accept(sock, (struct sockaddr *) &server_add, &addr_len);
        if (client_sock < 0) {
            printf("ERROR on accept\n");
            exit(EXIT_FAILURE);
        }
        if (!q)
            printf("Accepted connection \n");

        fds[1] = (struct pollfd) {.fd = client_sock, .events = POLLIN};
    }

    char buff[BUFFER_SIZE] = {0};
    FILE *file = fopen("recived.txt", "wb");
    int size = 0;
    while (size < fsize) {
        int ret = poll(fds, 2, 2000);
        if (ret == 0) {
            break;
        }
        if (ret < 0) {
            printf("ERROR: poll() failed\n");
            exit(EXIT_FAILURE);
        }

        int rec;
        if (type == SOCK_DGRAM) // udp
        {
            rec = recvfrom(sock, buff, BUFFER_SIZE, 0, (struct sockaddr *) &client_add, &addr_len); // recive data
        } else if (type == SOCK_STREAM) // tcp
        {
            rec = recv(client_sock, buff, BUFFER_SIZE, 0);
        }
        if (rec < 0) {
            printf("ERROR: recv() failed\n");
            exit(EXIT_FAILURE);
        }
        if (rec == 0) // connection closed
        {
            break;
        }
        size += rec; // update size


        fwrite(buff, rec, 1, file); // write to file
        bzero(buff, BUFFER_SIZE); // clear buffer
    }
    fclose(file); // close file
    if (type == SOCK_STREAM) // tcp
        close(client_sock); // close client socket
    close(sock); // close socket
    if (d == AF_UNIX) // unix domain socket
    {
        unlink(port);
    }
    return size; // return size of file
}

void send_mmap(char *file, char *name, bool q) {

    int fd = open(file, O_RDONLY); // open file
    if (fd < 0) {
        printf("ERROR opening file\n");
        exit(EXIT_FAILURE);
    }
    int size = getSize(file);

    int new_fd = shm_open(name, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (new_fd < 0) {
        printf("ERROR opening shared memory\n");
        exit(EXIT_FAILURE);
    }
    if (ftruncate(new_fd, size) < 0) {
        printf("ERROR truncating shared memory\n");
        exit(EXIT_FAILURE);
    }

    char *add = mmap(NULL, size, PROT_WRITE, MAP_SHARED, new_fd, 0); // MAP_SHARED
    if (add == MAP_FAILED) {
        printf("ERROR mapping shared memory\n");
        exit(EXIT_FAILURE);
    }
    char *f = mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
    if (f == MAP_FAILED) {
        printf("ERROR mapping file\n");
        exit(EXIT_FAILURE);
    }

    memcpy(add, f, size);


    if (munmap(add, size) < 0 || munmap(f, size) < 0) {
        printf("ERROR unmapping shared memory\n");
        exit(EXIT_FAILURE);
    }
    close(fd); // shm_unlink(name);
    close(new_fd); // shm_unlink(name);
}

void recv_mmap(char *name, char *file, int size, bool q) {

    int mfd = open(name, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR); // open file
    if (mfd < 0) {
        printf("ERROR opening file to\n");
        exit(EXIT_FAILURE);
    }
    if (ftruncate(mfd, size) < 0) {
        printf("ERROR truncating file\n");
        exit(EXIT_FAILURE);
    }
    int fd = shm_open(file, O_RDWR, S_IRUSR | S_IWUSR); // open shared memory
    if (fd < 0) {
        printf("ERROR opening shared memory\n");
        exit(EXIT_FAILURE);
    }

    char *madd = mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0); // MAP_SHARED
    if (madd == MAP_FAILED) {
        printf("ERROR mapping shared memory\n");
        exit(EXIT_FAILURE);
    }
    char *f = mmap(NULL, size, PROT_WRITE, MAP_SHARED, mfd, 0);
    if (f == MAP_FAILED) // MAP_SHARED
    {
        printf("ERROR mapping file memory\n");
        exit(EXIT_FAILURE);
    }

    memcpy(f, madd, size); // copy from shared memory to file

    if (munmap(madd, size) < 0 || munmap(f, size) < 0) {
        printf("ERROR unmapping shared memory\n");
        exit(EXIT_FAILURE);
    }

    if (shm_unlink(file) < 0) {
        printf("ERROR unlinking shared memory\n");
        exit(EXIT_FAILURE);
    }

    close(mfd); // close file
    close(fd); // close shared memory

}

void pipe_client(char *file, char *name, bool q) {

    char buff[BUFFER_SIZE] = {0}; // buffer for reading from file
    int ffd = open(file, O_RDONLY); // open file
    if (ffd < 0) {
        printf("ERROR opening file from\n");
        exit(EXIT_FAILURE);
    }


    if (mkfifo(name, 0666) < 0 && errno != EEXIST) { // create fifo
        printf("ERROR creating fifo\n");
        exit(EXIT_FAILURE);
    }

    int new_fd = open(name, O_WRONLY);
    if (new_fd < 0) // open fifo
    {
        printf("ERROR opening fifo\n");
        exit(EXIT_FAILURE);
    }

    ssize_t read_bytes, write_bytes;
    while ((read_bytes = read(ffd, buff, BUFFER_SIZE)) > 0) {
        write_bytes = write(new_fd, buff, read_bytes);
        if (write_bytes != read_bytes) {
            printf("ERROR writing to fifo\n");
            exit(EXIT_FAILURE);
        }
    }

    close(ffd); // close file
    close(new_fd); // close fifo
}

void pipe_server(char *file, char *name, bool q) {

    char buffer[BUFFER_SIZE] = {0};
    int fd = open(file, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fd < 0) {
        printf("ERROR opening file to\n");
        exit(EXIT_FAILURE);
    }

    int new_fd = open(name, O_RDONLY);
    if (new_fd < 0) {
        printf("ERROR opening fifo\n");
        exit(EXIT_FAILURE);
    }

    ssize_t read_bytes, write_bytes; // read and write bytes
    while ((read_bytes = read(new_fd, buffer, BUFFER_SIZE)) > 0) {
        write_bytes = write(fd, buffer, read_bytes);
        if (write_bytes != read_bytes) // check if all bytes are written
        {
            printf("ERROR writing to file\n");
            exit(EXIT_FAILURE);
        }
    }
    if (unlink(name) < 0) {
        printf("ERROR unlinking fifo\n");
        exit(EXIT_FAILURE);
    }

    close(fd);
    close(new_fd);
}


