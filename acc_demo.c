#include "acc_driver.h"

//regmap
#define START   0 // read and write // set to one automatically 
#define DONE    1 // read and write
#define MODE    2 // read and write
#define ADDR    3 // read and write
#define WD0     4 // read and write 
#define WD1     5 
#define WD2     6
#define WD3     7
#define RD0     8 //read only
#define RD1     9
#define RD2     10
#define RD3     11

int main(int argc, char **argv)
{
    int timeout, done_flag;
    
    // open /dev/acc
    acc_file_open();

    // ask for a write into 9000000
    acc_reg_swrite(MODE, 2, LITE0);
    acc_reg_swrite(ADDR, 0x9000000, LITE0);
    acc_reg_swrite(WD0, 31415, LITE0);
    acc_reg_swrite(WD1, 92653, LITE0);
    acc_reg_swrite(WD2, 58979, LITE0);
    acc_reg_swrite(WD3, 32384, LITE0);
    acc_reg_swrite(START, 1, LITE0);

    timeout = 1000;
    done_flag = 0;
    while (timeout-- && !done_flag)
        done_flag = acc_reg_sread(DONE, LITE0);

    // ask for a read
    acc_reg_swrite(MODE, 3, LITE0);
    acc_reg_swrite(ADDR, 0x9000000, LITE0);
    acc_reg_swrite(START, 1, LITE0);

    timeout = 1000;
    done_flag = 0;
    while (timeout-- && !done_flag)
        done_flag = acc_reg_sread(DONE, LITE0);

    printf("we got %d %d %d %d \n", acc_reg_sread(RD0, LITE0), acc_reg_sread(RD1, LITE0), acc_reg_sread(RD2, LITE0), acc_reg_sread(RD3, LITE0));

    //close /dev/acc
    acc_file_close();
    return 0;
}