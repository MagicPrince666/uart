#include <iostream>
#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <stdexcept>
#include <vector>
#include <chrono>
#include <codecvt>
#include <string>
#include <iomanip>
#include <algorithm>
#include <sstream>

#include "spdlog/spdlog.h"
#include "spdlog/cfg/env.h"  // support for loading levels from the environment variable
#include "spdlog/fmt/ostr.h" // support for user defined types

#include "uart.h"
#include "xepoll.h"
#include "inotify.h"
#include "interface.h"

int main(int argc, char* argv[])
{
    spdlog::info("Welcome to spdlog version {}.{}.{}  !", SPDLOG_VER_MAJOR, SPDLOG_VER_MINOR, SPDLOG_VER_PATCH);

    Xepoll xepoll;//初始化事件模型

    Uart serial(&xepoll, "/dev/ttyUSB0");
    Inotify inotify(&xepoll, &serial, "/tmp/text");//初始化文件监控事件并加入事件列表

    return xepoll.loop();//等待事件触发
}