#ifndef __UART_H__
#define __UART_H__

#include <inttypes.h>
#include <string>

class Uart
{
public:
    Uart(std::string device = "/dev/ttyUSB0");
    ~Uart();
    int sendData(const char* bufout, int size);
    int RecvData(char* bufin);
    bool UartLoop();

private:
    int uart_fd_ = -1;
    int OpenSerial(const char *cSerialName, int Bitrate);
};

#endif
 