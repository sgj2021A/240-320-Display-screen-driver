/*
	文档名称 : main.c
	文档作用 : 配置控制台，设置监测函数
*/

// 头文件
#include <stdio.h>
#include <Windows.h>
#include "LCD.h"

// 创建控制台函数
void CreateConsole() {
	AllocConsole();
	FILE* f;
	freopen_s(&f, "CONOUT$", "w", stdout);
	freopen_s(&f, "CONOUT$", "r", stdin);
	printf("调试控制台启动完毕!\n");
}

int main() {
	// 启动控制台
	CreateConsole();

	// 启动屏幕
	LCD_Clear(RGB(0, 0, 0));
	if (LDC_Init()) {
		printf("窗口创建成功\n");
	}
	else {
		printf("窗口创建失败！\n");
		return 0;
	}

	LCD_DrawCircle((vector2) { 30, 30 }, 10, RGB(255, 0, 255));
	LCD_DrawFilledCircle((vector2) { 100, 100 }, 10, RGB(255, 255, 0));
	LCD_Change(200, 200, RGB(255, 255, 255));
	// 系统更新函数
	MSG msg = { 0 };
	while (GetMessageW(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}
	return 0;
}
