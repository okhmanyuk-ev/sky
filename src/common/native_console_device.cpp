#ifdef _WIN32
#include "native_console_device.h"

#include <iostream>
#include <string>
#include <conio.h>
#include <Windows.h>

using namespace Common;

NativeConsoleDevice::NativeConsoleDevice()
{
	mReadThread = std::thread([this] {
		while (true)
		{
			std::string s;
			std::getline(std::cin, s);
			FRAME->addOne([this, s] {
				EVENT->emit(ReadEvent({ s }));
			});
		}
	});

	mReadThread.detach();

	mConsoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleOutputCP(1251);
}

NativeConsoleDevice::~NativeConsoleDevice()
{
	//mReadThread.~mReadThread();
}

void NativeConsoleDevice::write(const std::string& s, Console::Color color)
{
	SetConsoleTextAttribute(mConsoleHandle, color == Console::Color::Default ? (WORD)Console::Color::Gray : (WORD)color);
	std::cout << s;
	SetConsoleTextAttribute(mConsoleHandle, (WORD)Console::Color::Gray);
}

void NativeConsoleDevice::writeLine(const std::string& s, Console::Color color)
{
	write(s, color);
	std::cout << std::endl;
}

void NativeConsoleDevice::clear()
{
	system("cls");
}

void NativeConsoleDevice::setTitle(const std::string& s)
{
	SetConsoleTitle(std::string(s).c_str());
}
#endif