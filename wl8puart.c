#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "serial.h"
#include "wl8puart.h"

#define SERIAL_NUM 0

char fpath[256] = {0};


typedef enum {
    fs0,
    fs1,
    fs2,
    fs3,
    fs4,
    fs5,
    fs0_1,
    fs1_1,
    fs2_1,
    fs3_1,
    fs4_1,
    fs5_1,
}Fsn;

// 串口测试数据包结构体
struct serial_test_pack
{
    uint8_t header;    // 帧头(固定为0xAA)
    uint8_t cmd;       // 命令
    uint8_t data_len;  // 数据长度
    uint8_t data[256]; // 数据内容
    uint8_t check;     // 校验(check前的所有数据)
};

/**
 * @brief  单字节异或检验
 * @param  data: 源数据
 * @param  data_len: 源数据长度
 * @return 校验值
 */
static uint8_t check_xor(uint8_t *data, uint16_t data_len)
{
    uint8_t chk = 0;

    for (uint16_t i = 0; i < data_len; i++)
    {
        chk = (chk ^ data[i]);
    }

    return chk;
}

/**
 * @brief  串口接收数据包
 * @param  read_pack: 接收到的数据包
 * @param  port: 串口号
 * @param  first_timeout: 首字节超时时间(单位: ms)
 * @param  timeout: 字节间超时时间(单位: ms)
 * @return 成功: 0
 *         失败: -1
 */
static int serial_read_pack(struct serial_test_pack *read_pack, const uint8_t port,
                            const uint32_t first_timeout, const uint32_t timeout)
{
    int ret = -1;
    uint8_t data_len = 0;         // 数据长度
    uint8_t recv_data[256] = {0}; // 接收帧数据
    uint32_t recv_data_len = 0;   // 接收帧数据长度
    uint8_t recv_check = 0;       // 接收数据中的校验值
    uint8_t calc_check = 0;       // 本地计算校验值
    uint8_t machstr[3] = {':','-',')'};

    while(1)
    {
        // 接收帧头   :-)
        ret = serial_read_data(&recv_data[recv_data_len], port, 1, first_timeout);
        // 长度或者帧头错误, 直接返回
        if ((1 == ret) && (machstr[recv_data_len] == recv_data[recv_data_len]))
        {
            recv_data_len++;
            if(recv_data_len == 3) return 1;
        }
        else if((1 == ret)&&(recv_data_len < 3))
        {
            recv_data_len = 0;
        }
        else break;
    }
    return -1;
}

char * fsPath(Fsn fsn)
{
    memset(fpath,0,sizeof(fpath));
	if(fsn <= fs5)
    {
        sprintf(fpath,"directory fs%d:\\FactoryData\\System\\Library\\Caches\\com.apple.factorydata\r\n",(int)fsn);
	}
    else
    {
        sprintf(fpath,"cat -h fs%d:\\FactoryData\\System\\Library\\Caches\\com.apple.factorydata\\",(int)(fsn-fs5));
	}
	return fpath;
}

/**
 * @brief  程序入口
 * @param  argc: 参数个数
 * @param  argv: 参数列表
 * @return 成功: 0
 *         失败: -1
 */
int loop(void)
{
    int ret = -1;
    uint8_t send_data[] = {'W', 'L', 'A', 'C', 'C'};
    uint32_t send_data_len = 5;
    uint8_t data_len = 0;         // 数据长度
    uint8_t recv_data[256*1024] = {0}; // 接收帧数据
    uint8_t path_data[256*1024] = {0}; // 接收帧数据
    uint32_t recv_data_len = 0;   // 接收帧数据长度
    struct serial_test_pack read_pack = {0};
    char fPath[512] = {0};
    char cmd1[] = "true\r\n";
    uint8_t bbpv_data[256*1024] = {0};   //bbpv file
    uint8_t seal_data[256*1024] = {0};   //seal file

    // 打开串口
    ret = open_serial(SERIAL_NUM, 115200, 8, 'N', 1);
    if (ret < 0)
    {
        perror("open serial fail");

        return -1;
    }

    // 打印发送数据
    printf("serial write data[len = %d]: ", send_data_len);
    for (uint8_t i = 0; i < send_data_len; i++)
    {
        printf("%c", send_data[i]);
    }
    printf("\n");

    // 串口发送数据
    ret = serial_write_data(SERIAL_NUM, send_data, send_data_len);
    if (ret < 0)
    {
        printf("serial send data fail\n");

        return -1;
    }

    // 串口接收数据包
    ret = serial_read_pack(&read_pack, SERIAL_NUM, 2000, 500);
    //设备启动成功
    if (1 == ret)
    {
        printf("Start OK!!!\n");

        ret = serial_write_data(SERIAL_NUM, (const uint8_t *)cmd1, strlen(cmd1));
        if (ret < 0)
        {
            printf("serial send data fail\n");
            return -1;
        }

        ret = serial_read_data(&recv_data[recv_data_len], SERIAL_NUM, 256, 500);
        if(ret > 0)
        {
            ////printf("%s\r\n",recv_data);   //true\r\n 应答结果
        }

        Fsn fsn = fs0;
        for(fsn = fs0; fsn <= fs5; fsn++)
        {
            // 串口发送数据
            ////printf("Send cmd: %s",fsPath(fsn));
            ret = serial_write_data(SERIAL_NUM, (const uint8_t *)fsPath(fsn), strlen(fsPath(fsn)));
            if (ret < 0)
            {
                printf("serial send data fail\n");
                return -1;
            }

            ret = serial_read_data(&recv_data[recv_data_len], SERIAL_NUM, 4096, 500);
            if(ret > 0)
            {
                if(strstr((const char *)recv_data,"FsFileInfo is NULL")||strstr((const char *)recv_data,"Can't open the file"))
                {
                    continue;
                }
                else if(strstr((const char *)recv_data,"---------"))
                {
                    printf("%s\r\n",recv_data);
                    char bbpvPath[256];
                    char sealPath[256];
                    int  index;
                    char *pPath;
                    memcpy(path_data,recv_data,ret);  //备份目录列表
                    pPath = strstr((const char *)path_data,"bbpv-");
                    if(pPath)
                    {
                        index = 0;
                        for(;*pPath != 0x0A;pPath++,index++)
                        {
                            bbpvPath[index] = *pPath;
                        }
                        bbpvPath[index] = '\0';
                        memset(fPath,0,sizeof(fPath));
                        sprintf(fPath,"%s%s\r\n",fsPath(fsn+fs5),bbpvPath);
                        ret = serial_write_data(SERIAL_NUM, (const uint8_t *)fPath, strlen(fPath));
                        if (ret < 0)
                        {
                            printf("serial send data fail\n");
                            return -1;
                        }
                        
                        ret = serial_read_data(recv_data, SERIAL_NUM, 256*1024, 500);
                        if(ret > 0)
                        {
                            FILE *fpRead = NULL;
                            char bbpvSavePath[512] = {0};
                            int rn;
                            sprintf(bbpvSavePath,"/tmp/factorydata/%s",bbpvPath);
                            if(access("/tmp/factorydata",0)==-1)  //access函数是查看文件是不是存在
                            {
                                    if (mkdir("/tmp/factorydata",0777)) //如果不存在就用mkdir函数来创建
                                    {
                                        printf("creat file bag failed!!!\n");
                                    }
                            }
                            fpRead = fopen(bbpvSavePath,"wb"); //打开文件
                            if(fpRead == NULL)
                            {
                                printf("--: %s---%d--00 open error\r\n",__FILE__,__LINE__);
                            }
                            fwrite(recv_data,ret,1,fpRead);
                            printf("Save bbpv File in /tmp/factorydata OK\r\n");
                            fclose(fpRead);
                        }
                    }

                    pPath = strstr((const char *)path_data,"seal-");
                    if(pPath)
                    {
                        index = 0;
                        for(;*pPath != 0x0A;pPath++,index++)
                        {
                            sealPath[index] = *pPath;
                        }
                        sealPath[index] = '\0';
                        memset(fPath,0,sizeof(fPath));
                        sprintf(fPath,"%s%s\r\n",fsPath(fsn+fs5),sealPath);
                        ret = serial_write_data(SERIAL_NUM, (const uint8_t *)fPath, strlen(fPath));
                        if (ret < 0)
                        {
                            printf("serial send data fail\n");
                            return -1;
                        }

                        ret = serial_read_data(recv_data, SERIAL_NUM, 256*1024, 500);
                        if(ret > 0)
                        {
                            FILE *fpSealRead = NULL;
                            char sealSavePath[512];
                            int rn;
                            sprintf(sealSavePath,"/tmp/factorydata/%s",sealPath);

                            if(access("/tmp/factorydata",0)==-1)  //access函数是查看文件是不是存在
                            {
                                    if (mkdir("/tmp/factorydata",0777)) //如果不存在就用mkdir函数来创建
                                    {
                                        printf("creat file dir failed!!!\n");
                                    }
                            }

                            fpSealRead = fopen(sealSavePath,"wb"); //打开文件
                            if(fpSealRead == NULL)
                            {
                                printf("--: %s---%d--00 open error\r\n",__FILE__,__LINE__);
                            }

                            fwrite(recv_data,ret,1,fpSealRead);
                            printf("Save seal File in /tmp/factorydata OK\r\n");
                            fclose(fpSealRead);
                        }
                    }
                    break;
                }
                printf("%s\r\n",recv_data);
            }
        }
    }
    else
    {
        printf("Start fail\n");
    }

    // 关闭串口
    close_serial(SERIAL_NUM);

    return 0;
}
