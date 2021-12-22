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

#include "common.h"
#include "uart.h"
#include "xepoll.h"
#include "inotify.h"
#include "interface.h"

std::string CurrentTime()
{
	std::stringstream ss;
	std::string str = "%Y-%m-%d %H-%M-%S";
	std::chrono::system_clock::time_point a = std::chrono::system_clock::now();      //时间点可以做差
	time_t t1 = std::chrono::system_clock::to_time_t(a);				  //time_t可以格式化
	ss << std::put_time(localtime(&t1), str.c_str());
	std::string str1 = ss.str();
	return str1;
}

int main(int argc, char* argv[])
{
    spdlog::info("Welcome to spdlog version {}.{}.{}  !", SPDLOG_VER_MAJOR, SPDLOG_VER_MINOR, SPDLOG_VER_PATCH);

	std::string title = "Huang liquan ";
	title += CurrentTime();

	LcdRgb lcd(0);
	lcd.fill_screen_solid(0x0000ff);
	lcd.fb_put_string(30, 0, (char*)title.c_str(), title.size(), 0xffffff, 0, title.size());

    Xepoll xepoll;//初始化事件模型

    Uart serial(&xepoll, "/dev/ttyUSB0");
    Inotify inotify(&xepoll, &serial, "/tmp/text");//初始化文件监控事件并加入事件列表

    return xepoll.loop();//等待事件触发
}