/*
	文档名称 : LCD.h
	文档作用 : 文档为LCD.c(Window模拟)配置函数
*/

#ifndef __LCD_H
#define __LCD_H

// 包含文档
#include "LCD_Config.h"
#include <stdint.h>
#include <Windows.h>
#include <stdio.h>
#include <math.h>

// 包含宏
#define SWAP(a, b) { int temp = a; a = b; b = temp; }
#define PIXEL_COUNT (LCD_WIDTH * LCD_HEIGHT)

// 包含变量
COLORREF FrameBuffer[PIXEL_COUNT];

// 包含结构体
typedef struct vector2 {
	uint16_t x;
	uint16_t y;
}vector2;

//包含枚举变量
typedef enum STATUS{
	SET, ENSET
}STATUS;

// 包含函数
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
int LCD_Init();
void LCD_Clear(COLORREF color);
void LCD_Update(HDC hdc);
STATUS LCD_Change(uint16_t x, uint16_t y, COLORREF color);
STATUS LCD_DrawLine(vector2 p1, vector2 p2, COLORREF color);
STATUS LCD_DrawCircle(vector2 O, uint16_t R, COLOR16 color);
STATUS LCD_DrawFilledCircle(vector2 O, uint16_t R, COLORREF color);
STATUS LCD_DrawTriangle(vector2 a, vector2 b, vector2 c, COLORREF color);
STATUS LCD_DrawFullTriangle(vector2 a, vector2 b, vector2 c, COLORREF color);
#endif