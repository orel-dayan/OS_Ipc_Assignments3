#include "util.h"


int getMin(int a, int b) // return the minimum of two numbers
{
    return a < b ? a : b;
}

void generate_data(char *name, long size, int q) // generate a file of size 'size' with name 'name'
{
    int n;
    FILE *fp = fopen(name, "w");
    if (fp == NULL) {
        printf("Eror opening file\n");
        return;
    }


    char buffer[CHUNK_SIZE];


    while (n < size) // write in chunks of 1MB until we reach the desired size 
    {
        int bytes = CHUNK_SIZE;
        if (n + bytes > size) // if we are about to write more than the desired size, write only the remaining bytes
        {
            bytes = size - n; // remaining bytes
        }
        fwrite(buffer, bytes, 1, fp);
        n += bytes; // update the number of bytes written
    }

    fclose(fp);

    if (!q) {
        printf("Generate data for file '%s' of size %ld bytes\n", name, size);
    }
}

uint32_t CHECKSUM(char *filename, int quiet) {
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        printf("Error opening file '%s'\n", filename);
        return -1;
    }

    char buffer[CHUNK_SIZE];
    uint32_t calc = 0;

    while (!feof(file)) {
        size_t bytes_read = fread(buffer, 1, CHUNK_SIZE, file);
        for (size_t i = 0; i < bytes_read; i++) {
            calc += (uint32_t) buffer[i];
        }
    }

    fclose(file);
    if (!quiet) {
        printf("Generated checksum for file '%s': 0x%08x\n", filename, calc);
    }
    return calc;

}

int remove_after(char *name, int q) {
    int res = remove(name);

    if (res != 0) {
        printf("Error deleting file\n");
        return -1;
    }

    return 0;
}

void print_times(struct timeval *start, struct timeval *end) {
    long sec = end->tv_sec - start->tv_sec;
    long micro = end->tv_usec - start->tv_usec;
    if (micro < 0) {
        micro += 1000000;
        sec--;
    }
    long milliseconds = sec * 1000 + micro / 1000;
    printf("%ld\n", milliseconds);
}

int getSize(char *filename) {
    struct stat st;
    stat(filename, &st); // get the file size
    return st.st_size;
}