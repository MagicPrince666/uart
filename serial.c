/**
 * @file      : serial.c
 * @brief     : Linux平台串口驱动源文件
 * @author    : huenrong (huenrong1028@gmail.com)
 * @date      : 2020-09-22 14:29:25
 *
 * @copyright : Copyright (c) 2020  胡恩荣
 *
 */

#include "serial.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <poll.h>
#include <string.h>
#include <pthread.h>

// 记录所有串口的文件描述符
static int g_serial_fd[MAX_SERIAL_NUM] = {-1};

// 串口互斥锁
static pthread_mutex_t g_serial_mutex[MAX_SERIAL_NUM] =
    {PTHREAD_MUTEX_INITIALIZER};

/**
 * @brief  打开串口
 * @param  port      : 输入参数, 串口号
 * @param  speed     : 输入参数, 波特率, 默认为115200
 * @param  data_bit  : 输入参数, 数据位, 默认为8位数据位
 * @param  parity_bit: 输入参数, 奇偶检验位, 默认为无校验'n'或'N'
 * @param  stop_bit  : 输入参数, 停止位, 默认为1位停止位
 * @return 成功: 0
 *         失败: -1
 */
int open_serial(const uint8_t port, const uint32_t speed,
                const uint8_t data_bit, const char parity_bit,
                const uint8_t stop_bit)
{
    int fd = -1;
    int ret = -1;
    char serial_name[15] = {0};  // 记录串口名
    struct termios option = {0}; // 串口属性结构体
    speed_t serial_speed = 0;    // 串口波特率

    // 串口号不符合要求, 直接返回
    if ((port < 0) || (port >= MAX_SERIAL_NUM))
    {
        return -1;
    }

    // 串口已经被打开, 直接返回
    if (g_serial_fd[port] > 0)
    {
        return 0;
    }

    // 记录串口全名
    sprintf(serial_name, "%s%d", SERIAL_PREFIX, port);
    // 打开串口
    fd = open(serial_name, O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd < 0)
    {
        perror("open serial error");
        close(fd);

        return -1;
    }

    // 获取终端属性
    tcgetattr(fd, &option);

    // 设置波特率
    switch (speed)
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
    tcflush(fd, TCIOFLUSH);

    // 设置最小接收字符数和超时时间
    // 当MIN=0, TIME=0时, 如果有数据可用, 则read最多返回所要求的字节数,
    // 如果无数据可用, 则read立即返回0
    option.c_cc[VMIN] = 0;
    option.c_cc[VTIME] = 0;

    // 设置终端属性
    ret = tcsetattr(fd, TCSANOW, &option);
    if (ret < 0)
    {
        return -1;
    }

    // 在全局数组中记录打开的串口文件描述符
    g_serial_fd[port] = fd;

    // 初始化互斥锁
    pthread_mutex_init(&g_serial_mutex[port], NULL);

    return 0;
}

/**
 * @brief  关闭串口
 * @param  port: 输入参数, 串口号
 * @return 成功: 0
 *         失败: -1
 */
int close_serial(const uint8_t port)
{
    int ret = -1;

    // 如果串口已经关闭, 直接返回
    if (g_serial_fd[port] < 0)
    {
        return 0;
    }

    // 关闭串口
    ret = close(g_serial_fd[port]);
    if (ret < 0)
    {
        return -1;
    }

    // 文件描述符设为-1, 表明串口已关闭
    g_serial_fd[port] = -1;

    // 销毁互斥锁
    pthread_mutex_destroy(&g_serial_mutex[port]);

    return 0;
}

/**
 * @brief  串口发送数据(发送前会清空输入输出缓存)
 * @param  port         : 输入参数, 串口号
 * @param  send_data    : 输入参数, 发送数据
 * @param  send_data_len: 输入参数, 发送数据长度
 * @return 成功: 0
 *         失败: -1
 */
int serial_write_data(const uint8_t port, const uint8_t *send_data,
                      const uint32_t send_data_len)
{
    int ret = -1;

    // 如果串口未打开, 直接返回
    if (g_serial_fd[port] < 0)
    {
        return -1;
    }

    // 加锁
    pthread_mutex_lock(&g_serial_mutex[port]);

    // 发送数据前, 清空输入输出缓存
    tcflush(g_serial_fd[port], TCIOFLUSH);

    ret = write(g_serial_fd[port], send_data, send_data_len);
    if (send_data_len != ret)
    {
        perror("write data to serial fail");

        // 解锁
        pthread_mutex_unlock(&g_serial_mutex[port]);

        return -1;
    }

    // 解锁
    pthread_mutex_unlock(&g_serial_mutex[port]);

    return 0;
}

/**
 * @brief  串口接收数据
 * @param  recv_data    : 输出参数, 接收数据
 * @param  port         : 输入参数, 串口号
 * @param  recv_data_len: 输入参数, 接收数据长度
 * @param  timeout:     : 输入参数, 接收超时(单位：ms)
 * @return 成功: 实际接收数据长度
 *         失败: -1
 */
int serial_read_data(uint8_t *recv_data, const uint8_t port,
                     const uint32_t recv_data_len, uint32_t timeout)
{
    int ret = -1;
    nfds_t nfds = 1;         // 指定fds数组中的项目数
    struct pollfd fds[1];    // 指定要监视的文件描述符集
    int total_data_len = 0;  // 已读取数据长度
    int remain_data_len = 0; // 未读取数据长度

    // 如果串口未打开, 直接返回
    if (g_serial_fd[port] < 0)
    {
        return -1;
    }

    memset(recv_data, 0, recv_data_len);

    remain_data_len = recv_data_len;

    // 加锁
    pthread_mutex_lock(&g_serial_mutex[port]);

    while (1)
    {
        // 设置需要监听的文件描述符
        memset(fds, 0, sizeof(fds));
        fds[0].fd = g_serial_fd[port];
        fds[0].events = POLLIN;

        ret = poll(fds, nfds, timeout);
        // 返回负值, 发生错误
        if (ret < 0)
        {
            perror("poll error");

            // 解锁
            pthread_mutex_unlock(&g_serial_mutex[port]);

            return -1;
        }
        // 返回0, 超时
        else if (0 == ret)
        {
            // 如果超时后, 已读取数据长度大于0, 返回实际接收数据长度
            if (total_data_len > 0)
            {
                // 解锁
                pthread_mutex_unlock(&g_serial_mutex[port]);

                return total_data_len;
            }

            // 解锁
            pthread_mutex_unlock(&g_serial_mutex[port]);

            return -1;
        }
        // 返回值大于0, 成功
        else
        {
            // 判断是否是期望的返回
            if (fds[0].revents & POLLIN)
            {
                // 从文件起始位置开始读数据
                lseek(fds[0].fd, 0, SEEK_SET);
                ret = read(fds[0].fd, &recv_data[total_data_len],
                           remain_data_len);
                if (ret < 0)
                {
                    perror("serial read error");

                    // 解锁
                    pthread_mutex_unlock(&g_serial_mutex[port]);

                    return -1;
                }

                // 计算已读取数据长度
                total_data_len += ret;
                // 计算剩余需要读取长度
                remain_data_len = (recv_data_len - total_data_len);
                // 读取完毕
                if (total_data_len == recv_data_len)
                {
                    break;
                }
            }
        }
    }

    // 解锁
    pthread_mutex_unlock(&g_serial_mutex[port]);

    return total_data_len;
}
