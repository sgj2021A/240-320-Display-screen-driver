/*
    文档名称 : main.c
    文档作用 : 配置控制台，设置监测函数
*/

// 头文件
#include "LCD.h"
#include <stdio.h>
#include <windows.h>
#include <time.h>

// 全局变量
volatile BOOL g_bMonitorRunning = TRUE;
time_t timeStart;

// 创建控制台函数
void CreateConsole(void) {
    timeStart = time(NULL);
    AllocConsole();
    FILE* f;
    freopen_s(&f, "CONOUT$", "w", stdout);
    freopen_s(&f, "CONIN$", "r", stdin);
}

// 进度条函数
void ShowProgressBar(int percent, int width) {
    int filled = width * percent / 100;

    printf("\r[");  // \r 回到行首

    // 填充部分
    for (int i = 0; i < filled; i++) {
        printf("+");
    }

    // 未填充部分
    for (int i = filled; i < width; i++) {
        printf("-");
    }

    printf("] %3d%%", percent);
    fflush(stdout);
}

// CPU占有率
typedef struct {
    FILETIME idleTime;
    FILETIME kernelTime;
    FILETIME userTime;
} CPU_TIMES;

void GetCPUTimes(CPU_TIMES* times) {
    GetSystemTimes(&times->idleTime, &times->kernelTime, &times->userTime);
}

double CalculateCPUUsage(CPU_TIMES* prev, CPU_TIMES* curr) {
    // 计算总时间差（100纳秒单位）
    ULONGLONG prevIdle = ((ULONGLONG)prev->idleTime.dwHighDateTime << 32) | prev->idleTime.dwLowDateTime;
    ULONGLONG currIdle = ((ULONGLONG)curr->idleTime.dwHighDateTime << 32) | curr->idleTime.dwLowDateTime;

    ULONGLONG prevKernel = ((ULONGLONG)prev->kernelTime.dwHighDateTime << 32) | prev->kernelTime.dwLowDateTime;
    ULONGLONG currKernel = ((ULONGLONG)curr->kernelTime.dwHighDateTime << 32) | curr->kernelTime.dwLowDateTime;

    ULONGLONG prevUser = ((ULONGLONG)prev->userTime.dwHighDateTime << 32) | prev->userTime.dwLowDateTime;
    ULONGLONG currUser = ((ULONGLONG)curr->userTime.dwHighDateTime << 32) | curr->userTime.dwLowDateTime;

    // 计算差值
    ULONGLONG idleDiff = currIdle - prevIdle;
    ULONGLONG kernelDiff = currKernel - prevKernel;
    ULONGLONG userDiff = currUser - prevUser;

    // 总时间 = 内核时间 + 用户时间
    ULONGLONG totalDiff = (kernelDiff + userDiff);

    // CPU使用率 = (总时间 - 空闲时间) / 总时间 * 100
    if (totalDiff > 0) {
        return 100.0 - (100.0 * idleDiff / totalDiff);
    }
    return 0.0;
}

// 监视线程
DWORD WINAPI thread_Surveillance(LPVOID lpParam) {
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    HANDLE hProcess = GetCurrentProcess();
    SIZE_T minSize, maxSize;
    time_t timeNew = time(NULL);
    CPU_TIMES CPU_prev, CPU_curr;

    // CPU 第一次采样
    GetCPUTimes(&CPU_prev);

    while (g_bMonitorRunning) {
        COORD pos = { 0, 0 };
        SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
        if (GlobalMemoryStatusEx(&memInfo) && GetProcessWorkingSetSize(hProcess, &minSize, &maxSize)) {
            //获取值
            timeNew = time(NULL);
            GetCPUTimes(&CPU_curr);
            double CPU_Usage = CalculateCPUUsage(&CPU_prev, &CPU_curr);
            
            //显示
            printf("============================================================================================================\n");
            printf("|                                         Surveillance                                                     |\n");
            printf("============================================================================================================\n");
            printf("System Uptime : %lld s\n", (long long)(timeNew - timeStart));
            printf("Current work set: %.2f MB\n", minSize / (1024.0 * 1024));
            printf("Maximum working set: %.2f MB\n", maxSize / (1024.0 * 1024));
            printf("Physical memory : %.2f MB\n", memInfo.ullTotalPhys / (1024.0 * 1024));
            printf("Available memory : %.2f MB\n", memInfo.ullAvailPhys / (1024.0 * 1024));
            printf("Memory utilization rate : \n");
            int t = (int)((double)memInfo.ullAvailPhys / memInfo.ullTotalPhys * 100);
            ShowProgressBar(t, 100);
            printf("\n");
            printf("CPU usage rate : \n");
            ShowProgressBar((int)CPU_Usage, 100);
            printf("\n");
            printf("============================================================================================================\n");

            CPU_prev = CPU_curr;
        }
        Sleep(1000);
    }
  

    return 0;
}

// 主程序
int main() {
    // 启动控制台
    CreateConsole();
    printf("Process going on....\n");

    // 创建监视线程
    HANDLE hThread = CreateThread(NULL, 0, thread_Surveillance, NULL, 0, NULL);

    if (hThread == NULL) {
        printf("The monitoring window creation failed : %lu\n", GetLastError());
    }
    else {
        printf("The monitoring window creation succussed\n");
    }

    // 启动屏幕
    LCD_Clear(RGB(255, 255, 255));

    if (!LCD_Init()) {
        printf("windows creation failed\n");
        if (hThread) {
            g_bMonitorRunning = FALSE;
            WaitForSingleObject(hThread, 1000);
            CloseHandle(hThread);
        }
        return 1;
    }

    // 绘制测试图形
    LCD_DrawTriangle((vector2) { 10 ,10 }, (vector2) { 150, 10 }, (vector2) { 130, 50 }, RGB(0, 255, 0));

    // 消息循环
    MSG msg = { 0 };
    while (GetMessageW(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    // 清理
    printf("working clearing\n");
    if (hThread) {
        g_bMonitorRunning = FALSE;
        WaitForSingleObject(hThread, 2000);
        CloseHandle(hThread);
    }

    return 0;
}