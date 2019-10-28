obj-m:=acc.o
ARCH=riscv

CROSS_COMPILE=riscv64-unknown-linux-gnu-
CC:=$(CROSS_COMPILE)gcc
LD:=$(CROSS_COMPILE)ld

KERNEL_DIR:=/home/neutrino/Desktop/riscv-linux
CUR_DIR:=$(shell pwd)

all: kernel_modules test_demo

kernel_modules:
	make ARCH=riscv CROSS_COMPILE=riscv64-unknown-linux-gnu- -C $(KERNEL_DIR) M=$(CUR_DIR) modules

test_demo: acc_demo.c acc_driver.h acc_driver.c
	$(CC) -static acc_demo.c acc_driver.h acc_driver.c -o test
	
clean:
	make ARCH=riscv CROSS_COMPILE=riscv64-unknown-linux-gnu- -C $(KERNEL_DIR) M=$(CUR_DIR) clean
	rm test


