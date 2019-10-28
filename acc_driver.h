#include <sys/ioctl.h>
//#include <sys/types.h>
//#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

// ioctl cmd
#define READ_LITE0	0x01
#define WRITE_LITE0 0x02
#define READ_LITE1  0x03
#define WRITE_LITE1 0x04

// ctrl module id
#define LITE0 		0x00
#define LITE1		0x01

// fd file index
static int fd;

struct io_access {
    int index;
    uint64_t value;
};

int acc_file_open(void);
void acc_file_close(void);
int acc_reg_swrite(int reg_index, uint64_t reg_value, int which);
uint64_t acc_reg_sread(int reg_index, int which);
// int acc_reg_write(int reg_nums, int *reg_indexes, uint64_t *reg_file_buf, int which);
// int acc_reg_read(int reg_nums, int *reg_indexes, uint64_t *reg_file_buf, int which);