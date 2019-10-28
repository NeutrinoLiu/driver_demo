#include "acc_driver.h"

// global variable fd - file index
int acc_file_open()
{
	fd = open("/dev/acc", O_RDWR);
    if (fd < 0) {
        printf("fail to open the /dev/acc\n");    
        return -1;
    }
    return fd;
}

void acc_file_close()
{
    close(fd);
}

int acc_reg_swrite(int reg_index, uint64_t reg_value, int which)
{
	struct io_access *access_unit = malloc(sizeof(struct io_access));
	unsigned int cmd;
	unsigned long arg;

	access_unit->index = reg_index;
	access_unit->value = reg_value;
	cmd = (which == LITE0) ? WRITE_LITE0 : WRITE_LITE1;
	arg = (unsigned long) access_unit;

	if (ioctl(fd, cmd, arg)) {
		printf("write fail!\n");
		free(access_unit);
		return -1;
	} else {
		free(access_unit);
		return 0; 
	}
}

uint64_t acc_reg_sread(int reg_index, int which)
{
	struct io_access *access_unit = malloc(sizeof(struct io_access));
	unsigned int cmd;
	unsigned long arg;
	uint64_t res;

	access_unit->index = reg_index;
	cmd = (which == LITE0) ? READ_LITE0 : READ_LITE1;
	arg = (unsigned long) access_unit;

	if (ioctl(fd, cmd, arg)) {
		printf("read fail!\n");
		free(access_unit);
		return -1;
	} else {
		res = access_unit->value;
		free(access_unit);
		return res; 
	}
}