#include <stdio.h>
#include <iostream>
#include <string>
#ifdef _WIN32
#include <Windows.h>
#elif __APPLE__
#include <Cocoa/Cocoa.h>
#else
#include <cstdlib>
#endif
#include "lib/libjarunner.h"

// 不显示黑窗口
#pragma comment(linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"")

GoString createGoString(const char *str)
{
    ptrdiff_t len = static_cast<ptrdiff_t>(strlen(str));
    GoString string = {str, len};
    return string;
}

void showMessage(const std::string& message)
{
#ifdef _WIN32
    MessageBox(NULL, message.c_str(), "Message", MB_OK);
#elif __APPLE__
    NSAlert *alert = [[NSAlert alloc] init];
    [alert setMessageText:[NSString stringWithUTF8String:message.c_str()]];
    [alert runModal];
#else
    std::string cmd = "zenity --info --text='" + message + "'";
    system(cmd.c_str());
#endif
}

int main(int argc, char* argv[])
{
    GoString string = createGoString("E://app.jar");
    int version = GetJarMajorVersion(string);
    std::string message = "Jar major version is: " + std::to_string(version);
    showMessage(message);
    return 0;
}