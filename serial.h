/**
 * @file      : serial.h
 * @brief     : Linux平台串口驱动头文件
 * @author    : huenrong (huenrong1028@gmail.com)
 * @date      : 2020-10-10 15:39:48
 *
 * @copyright : Copyright (c) 2020  胡恩荣
 *
 */

#ifndef __SERIAL_H
#define __SERIAL_H

#include <stdint.h>

// 串口名前缀
#define SERIAL_PREFIX "/dev/ttyUSB"
// 系统最多支持的串口数
#define MAX_SERIAL_NUM 4

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
                const uint8_t stop_bit);

/**
 * @brief  关闭串口
 * @param  port: 输入参数, 串口号
 * @return 成功: 0
 *         失败: -1
 */
int close_serial(const uint8_t port);

/**
 * @brief  串口发送数据(发送前会清空输入输出缓存)
 * @param  port         : 输入参数, 串口号
 * @param  send_data    : 输入参数, 发送数据
 * @param  send_data_len: 输入参数, 发送数据长度
 * @return 成功: 0
 *         失败: -1
 */
int serial_write_data(const uint8_t port, const uint8_t *send_data,
                      const uint32_t send_data_len);

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
                     const uint32_t recv_data_len, uint32_t timeout);

#endif /* __SERIAL_H */
