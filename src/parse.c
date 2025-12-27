#include <stdio.h>
#include <unistd.h> // for read
#include <fcntl.h> // for open
#include <stdbool.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h> // for fstat
#include <stdlib.h>

#include "common.h"
#include "parse.h"



// function to create the file header
int create_db_header(struct dbheader_t **headerOut) {


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
    if (header == NULL ) {
        printf("Malloc failed to create a db header\n");
        return STATUS_ERROR;
    }

	if (read(fd, header, sizeof(struct dbheader_t)) != sizeof(struct dbheader_t)) {
		perror("read");
		free(header);
		return STATUS_ERROR;
	}

    // Convert from network byte order (big-endian) 
    header->version = ntohs(header->version);
    header->count = ntohs(header->count);
    header->magic = ntohl(header->magic);
    header->filesize = ntohl(header->filesize);

    // after we read teh file in the byte order that our system 
    // can understand, we can run our checks on the file header

    // check magic value of our file
    if (header->magic != HEADER_MAGIC) {
        printf("Improper header magic\n");
        return STATUS_ERROR;
    }

    // check version of our file
    if (header->version != 1) {
        printf("Improper header version\n");
        return STATUS_ERROR;
    }

    // check the size of our file 
    struct stat dbstat = {0};
    // fstat() is a system call used to:
    // 1. Get the file status information for the file identified by the file descriptor 'fd'.
    // 2. Store the retrieved information (like file size, permissions, etc.) into the provided "struct stat" (in this case, &dbstat).
    fstat(fd, &dbstat);

    // Check if the actual file size (dbstat.st_size) matches the expected size (header->filesize).
    // If they don't match, print an error message indicating the file is corrupted.
    if (header->filesize != dbstat.st_size) {
        printf("Corrupted database\n");
        return STATUS_ERROR; // return -1 if file corrupted
    }

    // pass the header out 
    *headerOut = header;

    // if everything goes well, return success : 
    return STATUS_SUCCESS;
}

int read_employees(int fd, struct dbheader_t *header, struct employee_t **employeesOut) {
    
    // if got a bad fd from user, return an error    
    if (fd < 0) {
        printf("Got a bad FD from the user\n");
        return STATUS_ERROR;
    }

    //figure out how many employees 
    int employeesNum = header -> count;

    //allocate memory for struct *employeesOut
    struct employee_t *employees = calloc (employeesNum, sizeof(struct employee_t));
    if (employees == NULL) {
        printf("Failed to allocate memory \n");
        return STATUS_ERROR;
    }

    // read data from the file 
    if (read (fd, &employees , sizeof(struct employee_t) * employeesNum) != sizeof(struct employee_t) * employeesNum) {
        perror("read");
        free(employees);
        return STATUS_ERROR;
    };

    // store the data read in the pointer
    *employeesOut  = employees;

    return STATUS_SUCCESS;
}

// function to write the data back to disc 
int output_file(int fd, struct dbheader_t *dbhdr, struct employee_t *employees){
    if (fd < 0) {
        printf("Got a bad FD from the user\n");
        return;
    }

    // Convert from host byte order (little-endian) 
    // to network byte order (big-endian),
    // so it can be correctly stored back in the disc.
    dbhdr->count = htons(dbhdr->count);
    dbhdr->version = htons(dbhdr->version);
    dbhdr->magic = htonl(dbhdr->magic);
    dbhdr->filesize = htonl(dbhdr->filesize);

    // return the cursor that types data to the file to the begining of the file
    // because when we read the file earlier via read(), the cursor went to the end
    // of the file 
    lseek(fd, 0, SEEK_SET);

    //write data to the file 
    write (fd, dbhdr, sizeof(struct dbheader_t));
    
    return;
}


