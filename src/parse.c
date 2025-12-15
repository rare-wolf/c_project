#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>

#include "common.h"
#include "parse.h"


int create_db_header(int fd, struct dbheader_t **headerOut) {
    struct dbheader_t *header = calloc(1, sizeof(struct dbheader_t));
    if (header == NULL) {
        printf("Malloc failed to create db header\n");
        return STATUS_ERROR;
    }
    header->version = 0x1;
    header->count = 0;
    header->magic = HEADER_MAGIC;
    header->filesize = sizeof(struct dbheader_t);

    *headerOut = header;
    return STATUS_SUCCESS;
}

// to write a header to a new file
int output_file(int fd, struct dbheader_t *dbheader){
    if (fd < 0) {
        printf("Got a bad FD from the user\n");
        return STATUS_ERROR;
    }
    
    // Normalize to network byte order before disk write.
    dbheader->version = htons(dbheader->version);
    dbheader->count = htons(dbheader->count);
    dbheader->magic = htonl(dbheader->magic);
    dbheader->filesize = htonl(dbheader->filesize);    

    // Move file cursor to offset 0 before writing.
    lseek(fd, 0, SEEK_SET);

    //write data to disk 
    write(fd, dbheader, sizeof(struct dbheader_t));
    return STATUS_SUCCESS;
}


int validate_db_header(int fd, struct dbheader_t **headerOut) {
    if (fd < 0) {
        printf("Got a bad FD from the user\n");
        return STATUS_ERROR;
    }

    // allocate memory in the heap for the header of the file we want 
    // to validate
    struct dbheader_t *header = calloc(1, sizeof(struct dbheader_t));
    if (header == NULL){
        printf("Malloc failed to create a db header\n");
        return STATUS_ERROR;
    }

    if (read(fd, header, sizeof(struct dbheader_t)) != sizeof(struct dbheader_t)) {
        perror ("read");
        free(header);
        return STATUS_ERROR;
    }

    // trasform the data read from the disc from (network indian) to 
    // (host indian) to read the data properly in a format that the host
    // system can understand
    header->version = ntohs(header->version);
    header->count = ntohs(header->count);
    header->magic = ntohl(header->magic);
    header->filesize = ntohl(header->filesize);

    // now we start to run the header data validation
    if (header->magic != HEADER_MAGIC){
        printf ("Improper header magic\n");
        free (header);
        return -1;
    }
    
    if (header->version != 1){
        printf ("Improper header version\n");
        free (header);
        return -1;
    }
    
    if (header->magic != HEADER_MAGIC){
        printf ("Improper header magic\n");
        free (header);
        return -1;
    }

    // to validate the filesize we use "fstat"
    struct stat dbstat = {0};
    fstat(fd, &dbstat);
    if (header->filesize != dbstat.st_size) {
        printf("Corrupted database\n");
        free(header);
        return -1;
    }

    *headerOut = header;
}
