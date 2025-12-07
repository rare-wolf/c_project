#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "common.h"
#include "parse.h"

// the analogy of this program is to read data from (disc) and parse it in (memory)
// then write data back to (disc)

int create_db_header(int fd, struct dbheader_t **headerOut) {

    // create a pointer in the (heap) to the header, to be able to pass it 
    // to other functions in the program
    struct dbheader_t *header = calloc(1, sizeof(struct dbheader_t));
    // make sure calloc() did not fail :
    if (header == -1) {
        printf("Malloc() failed to create db header\n");
        return  STATUS_ERROR;
    }

    // create the values of a new header :
    header->version = 0x1;
    header->count = 0;
    header->magic = HEADER_MAGIC;
    header->filesize = sizeof(struc dbheader_t);

    // store the pointer of the created header to use it later in the program
    *headerOut = header;

    // return if program success:
    return STATUS_SUCCESS;
}
int validate_db_header(int fd, struct dbheader_t **headerOut);