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

// the analogy of this program is to read data from (disc) and parse it in (memory)
// then write data back to (disc)

// function to write the data back to disc 
void output_file(int fd, struct dbheader_t *dbhdr) {
    if (fd < 0) {
        printf("Got a bad FD from the user\n");
        return;
    }

    // Convert from host byte order (little-endian) 
    // to network byte order (big-endian),
    // so it can be correctly stored back in the disc.
    dbhdr->magic = htonl(dbhdr->magic);
    dbhdr->filesize = htonl(dbhdr->filesize);
    dbhdr->count = htonl(dbhdr->count);
    dbhdr->version = htonl(dbhdr->version);

    // return the cursor that types data to the file to the begining of the file
    // because when we read the file earlier via read(), the cursor went to the end
    // of the file 
    lseek(fd, 0, SEEK_SET);
    return;
}

// function to validate a file header 
int validate_db_header (int fd, struct dbheader_t **headerOut) {
    // if the file descriptor is -1 == not a valid fd
    if (fd < 0) {
        printf("Got a bad FD from the user\n");
        return STATUS_ERROR;
    }

    // otherwise, allocate memory space in the heap for the dbheader 
    struct dbheader_t *header = calloc(1,sizeof(struct dbheader_t));
    // when calloc fails, it returns (NULL) not (-1)
    if (header == NULL) {
        printf("Malloc failed to create a db header\n");
        return STATUS_ERROR;
    }

	if (read(fd, header, sizeof(struct dbheader_t)) != sizeof(struct dbheader_t)) {
		perror("read");
		free(header);
		return STATUS_ERROR;
	}

    // Convert from network byte order (big-endian) 
    // to host byte order (little-endian),
    // so it can be correctly processed by the system.
    header->version = ntohs(header->version);
    header->count = ntohs(header->count);
    header->magic = ntohl(header->magic);
    header->filesize = ntohl(header->filesize);

    // after we read teh file in the byte order that our system 
    // can understand, we can run our checks on the file header
    if (header->magic != 1) {
        printf("Improper header magic\n");
        return -1;
    }

        if (header->version != 1) {
        printf("Improper header version\n");
        return -1;
    }

    // ?
    struct stat dbstat = {0};
    // fstat() is a system call used to:
    // 1. Get the file status information for the file identified by the file descriptor 'fd'.
    // 2. Store the retrieved information (like file size, permissions, etc.) into the provided "struct stat" (in this case, &dbstat).
    fstat(fd, &dbstat);

    // Check if the actual file size (dbstat.st_size) matches the expected size (header->filesize).
    // If they don't match, print an error message indicating the file is corrupted.
    if (header->filesize != dbstat.st_size) {
        printf("Corrupted database\n");
        return -1; // return -1 if file corrupted
    }
}

// function to create the file header
int create_db_header(int fd, struct dbheader_t **headerOut) {

    // create a pointer in the (heap) to the header, to be able to pass it 
    // to other functions in the program
    struct dbheader_t *header = calloc(1, sizeof(struct dbheader_t));
    // make sure calloc() did not fail :
    if (header == NULL) {
        printf("Malloc() failed to create db header\n");
        return  STATUS_ERROR;
    }

    // create the values of a new header :
    header->version = 0x1;
    header->count = 0;
    header->magic = HEADER_MAGIC;
    header->filesize = sizeof(struct dbheader_t);

    // store the pointer of the created header to use it later in the program
    *headerOut = header;

    // return if program success:
    return STATUS_SUCCESS;
}
int validate_db_header(int fd, struct dbheader_t **headerOut);