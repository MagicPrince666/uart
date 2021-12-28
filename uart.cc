#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <string.h>

#include "uart.h"

#include <iostream>

#define BAUDRATE     115200

#define DEBUG_UART 0

#define DATA_LEN    1024

Uart::Uart(Xepoll *epoll, std::string device)
: epoll_(epoll)
{
    std::cout << "uart init " << device << std::endl;

    uart_fd_ = OpenSerial(device.c_str(), BAUDRATE, 8, 'N', 1);
    if(uart_fd_ < 0) {
        std::cout <<"can\'t open " << device <<" !" << std::endl;
    } else tcflush(uart_fd_, TCIOFLUSH);//清空串口输入输出缓存

    // std::string cmd = "WLACC\r\n";
    // sendData(cmd.c_str(), cmd.size());

    // 绑定回调函数
    if (uart_fd_ > 0) {
        std::cout << "Bind epoll" << std::endl;
        epoll_->add(uart_fd_, std::bind(&Uart::UartRead, this));
    }
}

Uart::~Uart(void)
{
    std::cout << "uart deinit" << std::endl;

    if(uart_fd_ > 0)
        close(uart_fd_);
}

int Uart::OpenSerial(const char *cSerialName,
                int Bitrate,
                const uint8_t data_bit, 
                const char parity_bit,
                const uint8_t stop_bit)
{
    int iFd;

    struct termios option = {0};   // 串口属性结构体
    speed_t serial_speed = 0;   // 串口波特率

    iFd = open(cSerialName, O_RDWR | O_NOCTTY | O_NDELAY | O_NONBLOCK);
    if(iFd < 0) {
        perror(cSerialName);
        return -1;
    }

    tcgetattr(iFd, &option);

    // 设置波特率
    switch (Bitrate)
    {
    case 460800:
    {
        serial_speed = B460800;
        break;
    }

    case 115200:
    {
        serial_speed = B115200;
        break;
    }
    case 57600:
    {
        serial_speed = B57600;
        break;
    }

    case 38400:
    {
        serial_speed = B38400;
        break;
    }

    case 19200:
    {
        serial_speed = B19200;
        break;
    }

    case 9600:
    {
        serial_speed = B9600;
        break;
    }

    case 4800:
    {
        serial_speed = B4800;
        break;
    }

    case 2400:
    {
        serial_speed = B2400;
        break;
    }

    // 默认波特率为115200
    default:
    {
        serial_speed = B115200;
        break;
    }
    }
    // 设置输入波特率
    cfsetspeed(&option, serial_speed);
    // 设置输出波特率
    cfsetospeed(&option, serial_speed);

    // 设置数据位
    switch (data_bit)
    {
    // 5位数据位
    case 5:
    {
        option.c_cflag &= ~CSIZE;
        option.c_cflag |= CS5;

        break;
    }

    // 6位数据位
    case 6:
    {
        option.c_cflag &= ~CSIZE;
        option.c_cflag |= CS6;

        break;
    }

    // 7位数据位
    case 7:
    {
        option.c_cflag &= ~CSIZE;
        option.c_cflag |= CS7;

        break;
    }

    // 8位数据位
    case 8:
    {
        option.c_cflag &= ~CSIZE;
        option.c_cflag |= CS8;

        break;
    }

    // 默认为8位数据位
    default:
    {
        option.c_cflag &= ~CSIZE;
        option.c_cflag |= CS8;

        break;
    }
    }

    // 设置奇偶检验位
    switch (parity_bit)
    {
    // 无校验
    case 'n':
    case 'N':
    {
        option.c_cflag &= ~PARENB;
        option.c_iflag &= ~INPCK;

        break;
    }

    // 奇校验
    case 'o':
    case 'O':
    {
        option.c_cflag |= (PARODD | PARENB);
        option.c_iflag |= INPCK;

        break;
    }

    // 偶校验
    case 'e':
    case 'E':
    {
        option.c_cflag |= PARENB;
        option.c_cflag &= ~PARODD;
        option.c_iflag |= INPCK;

        break;
    }

    // 默认为无校验
    default:
    {
        option.c_cflag &= ~PARENB;
        option.c_iflag &= ~INPCK;

        break;
    }
    }

    // 设置停止位
    switch (stop_bit)
    {
    // 1位停止位
    case 1:
    {
        option.c_cflag &= ~CSTOPB;
        break;
    }

    // 2位停止位
    case 2:
    {
        option.c_cflag |= CSTOPB;
        break;
    }

    // 默认为1位停止位
    default:
    {
        option.c_cflag &= ~CSTOPB;
        break;
    }
    }

    // 一般必设置的标志
    option.c_cflag |= (CLOCAL | CREAD);
    option.c_oflag &= ~(OPOST);
    option.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    option.c_iflag &= ~(ICRNL | INLCR | IGNCR | IXON | IXOFF | IXANY);

    // 清空输入输出缓冲区
    // tcflush(iFd, TCIOFLUSH);

    // 设置最小接收字符数和超时时间
    // 当MIN=0, TIME=0时, 如果有数据可用, 则read最多返回所要求的字节数,
    // 如果无数据可用, 则read立即返回0
    option.c_cc[VMIN] = 0;
    option.c_cc[VTIME] = 0;

    if (tcsetattr(iFd,   TCSANOW,   &option)<0) {
        return   -1;
    }

    return iFd;
}

int Uart::sendData(const char* bufout, int size)
{
    int len = write(uart_fd_, bufout, size);
    if(len != size) {
        std::cout << "Send Error" << std::endl;
    }
    return len;
}

int Uart::RecvData(char* bufin)
{
    int len = read(uart_fd_, bufin, DATA_LEN);
    return len;
}

bool Uart::UartLoop()
{
    char* buf = new char[1024];
    while(true) {
        int len = RecvData(buf);
        if(len > 0) {
            buf[len] = 0;
            std::cout << buf << std::endl;
        }
        usleep(100000);
    }
    delete[] buf;
    return true;
}

bool Uart::UartRead()
{
    char* buf = new char[DATA_LEN];
    int len = read(uart_fd_, buf, DATA_LEN);
    buf[len] = 0;
    std::cout << buf << std::endl;
    delete[] buf;
    return true;
}

int Uart::Transfer(std::string comand)
{
    std::cout << BLUE << comand << "\n";
    int ret = sendData(comand.c_str(), comand.size());
    return ret;
}
