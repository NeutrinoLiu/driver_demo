
// int acc_reg_write(int reg_nums, int *reg_indexes, uint64_t *reg_file_buf, int which);
// int acc_reg_read(int reg_nums, int *reg_indexes, uint64_t *reg_file_buf, int which);
int acc_reg_swrite(int reg_index, uint64_t *reg_value, int which);
uint64_t acc_reg_sread(int reg_index, uint64_t *reg_value, int which);