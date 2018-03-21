#include "Driver\DrvI2C.h"
#include "Driver\DrvSYS.h"

void init_LCD();
void write_LCD(int x, int y, char data[]);
void clear_LCD();