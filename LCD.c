/*
	文档名称 : LCD.c
	文档作用 : 文档为LCD(Window模拟)底层模拟函数
*/

// 库包含
#include "LCD.h"

/*
 * 窗口初始化函数
 */
int LDC_Init() {
	HINSTANCE hInstance = GetModuleHandle(NULL);

	const wchar_t CLASS_NAME[] = L"LDC_Display";
	
	WNDCLASSW wc = { 0 };
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = CLASS_NAME;
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

	RegisterClassW(&wc);

	HWND hwnd = CreateWindowExW(
		0,                    // 扩展样式（0表示无扩展样式）
		CLASS_NAME,           // 窗口类名（必须已注册）
		L"LDC_Display",    // 窗口标题（显示在标题栏）
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,  // 窗口样式（标准窗口：标题栏、边框、系统菜单等）
		CW_USEDEFAULT,        // X坐标（系统自动选择位置）
		CW_USEDEFAULT,        // Y坐标（系统自动选择位置）
		LCD_WIDTH,            // 宽度 = 320
		LCD_HEIGHT,           // 高度 = 240
		NULL,                 // 父窗口句柄（无父窗口）
		NULL,                 // 菜单句柄（无菜单）
		hInstance,            // 程序实例句柄
		NULL                  // 额外参数（无）
	);

	if (hwnd == NULL) { 
		return 0;   
	}
	ShowWindow(hwnd, SW_SHOW);
	return 1;
}

/*
 * 窗口过程函数
 * HWND hwnd：窗口句柄（窗口的唯一标识符）
 * UINT uMsg：消息类型（如WM_DESTROY、WM_PAINT等）
 * WPARAM wParam：消息的附加参数1
 * LPARAM lParam：消息的附加参数2
 */
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
	
		case WM_PAINT: {
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);
			LCD_Update(hdc);
			EndPaint(hwnd, &ps);
			return 0;
		}
	}
	return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

/*
 * LCD清屏函数 
 * color : COLORREF类 清屏完的背景色
 */
void LCD_Clear(COLORREF color) {
	for (int i = 0; i < PIXEL_COUNT; i++) {
		FrameBuffer[i] = color;
	}
}

/*
 * LCD更新函数
 */
void LCD_Update(HDC hdc) {
	// 创建内存DC
	HDC memDC = CreateCompatibleDC(hdc);

	// 设置位图信息
	BITMAPINFO bmi = { 0 };
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = LCD_WIDTH;
	bmi.bmiHeader.biHeight = -LCD_HEIGHT;  // 负值表示自上而下
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;         // 32位颜色
	bmi.bmiHeader.biCompression = BI_RGB;

	// 创建DIBSection
	void* bits = NULL;
	HBITMAP hBitmap = CreateDIBSection(memDC, &bmi, DIB_RGB_COLORS, &bits, NULL, 0);

	if (hBitmap && bits) {
		// 直接复制COLORREF数据（COLORREF就是32位）
		memcpy(bits, FrameBuffer, LCD_WIDTH * LCD_HEIGHT * sizeof(COLORREF));

		// 选择位图到内存DC
		HBITMAP hOldBitmap = (HBITMAP)SelectObject(memDC, hBitmap);

		// 一次性绘制到窗口
		BitBlt(hdc, 0, 0, LCD_WIDTH, LCD_HEIGHT, memDC, 0, 0, SRCCOPY);

		// 清理
		SelectObject(memDC, hOldBitmap);
		DeleteObject(hBitmap);
	}

	DeleteDC(memDC);
}

/*
 * 改变单个像素点
 * x : uint16_t 横坐标 0 -- LCD_WIDTH - 1
 * y : uint16_t 纵坐标 0 -- LCD_HEIGHT - 1
 * color : COLORREF 目标颜色
 * 返回 : STATUS
 */
STATUS LCD_Change(uint16_t x, uint16_t y, COLORREF color) {
	if (x >= LCD_WIDTH && y >= LCD_HEIGHT)return ENSET;
	uint16_t pos = y * LCD_WIDTH + x;
	FrameBuffer[pos] = color;
	return SET;
}

/*
 * 绘画直线
 * p1 : vector2 第一个点
 * p2 : vector2 第二个点
 * color : COLORREF 颜色
 * 返回 : STATUS
 */
STATUS LCD_DrawLine(vector2 p1, vector2 p2, COLORREF color) {
	if ((p1.x >= LCD_WIDTH && p2.x >= LCD_WIDTH) ||
		(p1.y >= LCD_HEIGHT && p2.y >= LCD_HEIGHT) ||
		(p1.x < 0 && p2.x < 0) ||
		(p1.y < 0 && p2.y < 0)) {
		return ENSET;
	}

	int16_t x0 = p1.x;
	int16_t y0 = p1.y;
	int16_t x1 = p2.x;
	int16_t y1 = p2.y;

	// 计算差值
	int16_t dx = abs(x1 - x0);
	int16_t dy = abs(y1 - y0);

	// 步进方向
	int16_t sx = (x0 < x1) ? 1 : -1;
	int16_t sy = (y0 < y1) ? 1 : -1;

	int16_t err = dx - dy;
	int16_t e2;

	// Bresenham 算法
	while (1) {
		// 绘制当前点（只绘制在屏幕内的点）
		if (x0 >= 0 && x0 < LCD_WIDTH && y0 >= 0 && y0 < LCD_HEIGHT) {
			uint16_t pos = y0 * LCD_WIDTH + x0;
			FrameBuffer[pos] = color;
		}

		// 到达终点
		if (x0 == x1 && y0 == y1) break;

		e2 = 2 * err;
		if (e2 > -dy) {
			err -= dy;
			x0 += sx;
		}
		if (e2 < dx) {
			err += dx;
			y0 += sy;
		}
	}

	return SET;
}

/*
 * 绘画圆形
 * O : vector2 圆心
 * R : uint16_t 半径
 * color : COLORREF 颜色
 * 返回 : STATUS
 */
STATUS LCD_DrawCircle(vector2 O, uint16_t R, COLOR16 color) {
	if (O.x < 0 || O.x >= LCD_WIDTH || O.y < 0 || O.y > LCD_HEIGHT)return ENSET;
	int16_t x = 0;
	int16_t y = R;
	int16_t d = 1 - R;  // 决策参数
	int16_t deltaE = 3;
	int16_t deltaSE = -2 * R + 5;

	// 绘制8个对称点
	while (x <= y) {
		// 绘制8个对称点
		// 第一象限
		if (O.x + x >= 0 && O.x + x < LCD_WIDTH && O.y + y >= 0 && O.y + y < LCD_HEIGHT)
			FrameBuffer[(O.y + y) * LCD_WIDTH + (O.x + x)] = color;
		if (O.x + y >= 0 && O.x + y < LCD_WIDTH && O.y + x >= 0 && O.y + x < LCD_HEIGHT)
			FrameBuffer[(O.y + x) * LCD_WIDTH + (O.x + y)] = color;

		// 第二象限
		if (O.x - x >= 0 && O.x - x < LCD_WIDTH && O.y + y >= 0 && O.y + y < LCD_HEIGHT)
			FrameBuffer[(O.y + y) * LCD_WIDTH + (O.x - x)] = color;
		if (O.x - y >= 0 && O.x - y < LCD_WIDTH && O.y + x >= 0 && O.y + x < LCD_HEIGHT)
			FrameBuffer[(O.y + x) * LCD_WIDTH + (O.x - y)] = color;

		// 第三象限
		if (O.x - x >= 0 && O.x - x < LCD_WIDTH && O.y - y >= 0 && O.y - y < LCD_HEIGHT)
			FrameBuffer[(O.y - y) * LCD_WIDTH + (O.x - x)] = color;
		if (O.x - y >= 0 && O.x - y < LCD_WIDTH && O.y - x >= 0 && O.y - x < LCD_HEIGHT)
			FrameBuffer[(O.y - x) * LCD_WIDTH + (O.x - y)] = color;

		// 第四象限
		if (O.x + x >= 0 && O.x + x < LCD_WIDTH && O.y - y >= 0 && O.y - y < LCD_HEIGHT)
			FrameBuffer[(O.y - y) * LCD_WIDTH + (O.x + x)] = color;
		if (O.x + y >= 0 && O.x + y < LCD_WIDTH && O.y - x >= 0 && O.y - x < LCD_HEIGHT)
			FrameBuffer[(O.y - x) * LCD_WIDTH + (O.x + y)] = color;

		x++;
		if (d < 0) {
			d += deltaE;
			deltaE += 2;
			deltaSE += 2;
		}
		else {
			d += deltaSE;
			deltaE += 2;
			deltaSE += 4;
			y--;
		}
	}
	return SET;
}

/*
 * 绘画实心圆
 * O : vector2 圆心
 * R : uint16_t 半径
 * color : COLORREF 颜色
 * 返回 : STATUS
 */
STATUS LCD_DrawFilledCircle(vector2 O, uint16_t R, COLORREF color) {
	// 圆心边界检查
	if (O.x < 0 || O.x >= LCD_WIDTH || O.y < 0 || O.y >= LCD_HEIGHT) {
		return ENSET;
	}

	for (int y = -R; y <= R; y++) {
		int py = O.y + y;
		if (py < 0 || py >= LCD_HEIGHT) continue;

		int dx = (int)sqrt(R * R - y * y);
		int x1 = O.x - dx;
		int x2 = O.x + dx;

		// 限制x范围
		if (x1 < 0) x1 = 0;
		if (x2 >= LCD_WIDTH) x2 = LCD_WIDTH - 1;

		// 绘制水平线
		for (int x = x1; x <= x2; x++) {
			FrameBuffer[py * LCD_WIDTH + x] = color;
		}
	}

	return SET;
}