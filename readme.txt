// WICIL.neutrino
// 2019 Winter
// a simple CSR-visiting driver, running on linux system

File list
	acc.c: 	
		The linux kernel module, which runs in kernel and needs to be 
		installed from acc.ko. Nothing need to be changed in this file.
	acc_driver.c/acc_driver.h: 
		APIs that I provide you with to directly visit the register file.
	acc_demo.c:
		A demo that I have tested on Nick's HBM visit module.

APIs (acc_driver.h)
	int fd: 
		global file descriptor of the /dev/acc
	int acc_reg_swrite(int reg_index, uint64_t reg_value, int which):
		single register write operation, reg_index are indexed by 64bit.
		which can be LITE0 or LITE1, selecting your axi_lite control bus.
	uint64_t acc_reg_sread(int reg_index, int which):
		single register read operation, similar to swrite.

