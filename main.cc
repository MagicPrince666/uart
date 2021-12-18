#include <iostream>
#include <string>
#include "uart.h"

int main(int argc, char* argv[])
{
    Uart serial(argv[1]);
    std::string cmd = "WLACC\r\n";
    serial.sendData(cmd.c_str(), cmd.size());
    serial.UartLoop();
    return 0;
}