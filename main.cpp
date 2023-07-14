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
#include <jni.h>
#include "lib/libjarunner.h"

void showMessage(const std::string &message)
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

void runJar(const std::string &jarOpts, const std::string &jvmPath, const std::string &mainClassName)
{
    typedef jint(JNICALL CreateJavaVM_t)(JavaVM * *pvm, void **env, void *args);
    CreateJavaVM_t *pfnCreateJavaVM;
    JavaVMInitArgs vmInitArgs;
    JavaVM *jvm = NULL;
    JNIEnv *env = NULL;

    JavaVMOption vmOption[1];
    vmOption[0].optionString = const_cast<char *>(jarOpts.c_str());

    vmInitArgs.version = JNI_VERSION_1_8;
    vmInitArgs.options = vmOption;
    vmInitArgs.nOptions = 1;
    vmInitArgs.ignoreUnrecognized = JNI_TRUE;

    // 指定jvm.dll的路径, 兼容linux\mac\windows
    // 加载dll
    HINSTANCE hInstance = LoadLibrary(jvmPath.c_str());
    pfnCreateJavaVM = (CreateJavaVM_t *)GetProcAddress(hInstance, "JNI_CreateJavaVM");
    // 创建jvm
    int ret = pfnCreateJavaVM(&jvm, (void **)&env, &vmInitArgs);
    // 获取io/xbmlz/jeditor/Main
    jclass mainClass = env->FindClass(mainClassName.c_str());
    // 获取main方法
    jmethodID mainMethod = env->GetStaticMethodID(mainClass, "main", "([Ljava/lang/String;)V");
    // 如果需要创建String[]
    jobjectArray args = env->NewObjectArray(1, env->FindClass("java/lang/String"), NULL);
    // 调用main方法
    env->CallStaticVoidMethod(mainClass, mainMethod, args);
    // 释放
    jvm->DestroyJavaVM();
}

int main(int argc, char *argv[])
{

    char *jarPathRet = NULL;
    int ret = GetIniValue(const_cast<char *>("jarunner.ini"), const_cast<char *>(""), const_cast<char *>("jarPath"), &jarPathRet);
    if (ret < 0)
    {
        showMessage("Find jar path failed, please check jarunner.ini file");
        return 0;
    }
    char *mainClass = NULL;
    ret = GetIniValue(const_cast<char *>("jarunner.ini"), const_cast<char *>(""), const_cast<char *>("mainClass"), &mainClass);
    if (ret < 0)
    {
        showMessage("Find main class failed, please check jarunner.ini file");
        return 0;
    }
    int version = GetJarMajorVersion(jarPathRet);
    if (version <= 0)
    {
        showMessage("Find jar version failed, please check your jar file");
        return 0;
    }
    // GetLocalJavaHome(int jarVersion, char** javaHomeRet);
    std::string versionMsg = "Find jar version: " + std::to_string(version);
    char *javaHomeRet = NULL;
    int localJavaVersion = GetLocalJavaHome(version, &javaHomeRet);
    if (localJavaVersion < 0)
    {
        showMessage("Find java home failed");
        return 0;
    }
    // jvm.dll path, if version is 8, then path is javaHome + "/jre/bin/server/jvm.dll", else path is javaHome + "/bin/server/jvm.dll"
    std::string jvmPath = std::string(javaHomeRet) + "/jre/bin/server/jvm.dll";
    if (localJavaVersion > 8)
    {
        jvmPath = std::string(javaHomeRet) + "/bin/server/jvm.dll";
    }
    std::string jvmOpts = "-Djava.class.path=" + std::string(jarPathRet);
    runJar(jvmOpts, jvmPath, mainClass);
    return 0;
}