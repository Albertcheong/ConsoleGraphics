#ifndef UNICODE
#define UNICODE
#endif

#define WIN32_LEAN_AND_MEAN
#include "Windows.h"
#include "Utils.hpp"

#include <iostream>
#include <vector>
#include <thread>
#include <algorithm>

class Console2D
{
	public:
	Console2D() 
		: m_nScreenWidth(NULL), m_nScreenHeight(NULL), m_backgroundColor(NULL), m_chBufferFront{}, m_chBufferBack{}, m_rect{}
	{
		m_sAppTitle  = L"Default";
		m_hConsole   = GetStdHandle(STD_OUTPUT_HANDLE);
		m_hConsoleIn = GetStdHandle(STD_INPUT_HANDLE);
		m_mousePos_x = NULL;
		m_mousePos_y = NULL;

		m_bEnableSound = false;
		m_bError       = false;
	}

	~Console2D() {}

	int setMode(int nWidth, int nHeight, int nFontWidth = 8, int nFontHeight = 8)
	{
		if (m_hConsole == INVALID_HANDLE_VALUE || m_hConsoleIn == INVALID_HANDLE_VALUE)
			return error(L"INVALID HANDLE");

		m_nScreenWidth  = nWidth;
		m_nScreenHeight = nHeight;

		m_rect = { 0, 0, 1, 1 };
		SetConsoleWindowInfo(m_hConsole, TRUE, &m_rect);

		COORD const bufferSize = { static_cast<short>(m_nScreenWidth), static_cast<short>(m_nScreenHeight) };
		if (!SetConsoleScreenBufferSize(m_hConsole, bufferSize))
			return error(L"FAILED TO SET CONSOLE SIZE");

		if (!SetConsoleActiveScreenBuffer(m_hConsole))
			return error(L"FAILED TO SET CONSOLE");

		CONSOLE_FONT_INFOEX cfiex{};
		cfiex.cbSize       = sizeof(CONSOLE_FONT_INFOEX);
		cfiex.nFont        = NULL;
		cfiex.FontFamily   = FF_DONTCARE;
		cfiex.FontWeight   = FW_NORMAL;
		cfiex.dwFontSize.X = nFontWidth;
		cfiex.dwFontSize.Y = nFontHeight;

		wcscpy_s(cfiex.FaceName, L"Consolas");
		if (!SetCurrentConsoleFontEx(m_hConsole, FALSE, &cfiex))
			return error(L"FAILED TO SET FONT");

		CONSOLE_SCREEN_BUFFER_INFO csbi;
		if (!GetConsoleScreenBufferInfo(m_hConsole, &csbi))
			return error(L"FAILED TO RETRIVE SCREEN BUFFER INFO");

		if (m_nScreenWidth > csbi.dwMaximumWindowSize.X)
			return error(L"SPECIFIED SCREEN WIDTH IS TOO LARGE");

		if (m_nScreenHeight > csbi.dwMaximumWindowSize.Y)
			return error(L"SPECIFIED SCREEN HEIGHT IS TOO LARGE");

		m_rect = { 0, 0, static_cast<short>(m_nScreenWidth - 1), static_cast<short>(m_nScreenHeight - 1) };
		if (!SetConsoleWindowInfo(m_hConsole, TRUE, &m_rect))
			return error(L"FAILED TO SET SCREEN SIZE");

		m_chBufferFront.resize(m_nScreenWidth * m_nScreenHeight);
		m_chBufferBack.resize(m_nScreenWidth * m_nScreenHeight);

		return 1;
	}

	int setCaption(const std::wstring& sAppTitle)
	{
		m_sAppTitle = sAppTitle;
		if (!SetConsoleTitle(m_sAppTitle.c_str()))
			return error(L"FAILED TO SET CONSOLE TITLE");

		return 1;
	}
	
	int drawLine(int x0, int y0, int x1, int y1, wchar_t wch = 0x2588, short color = 0x000F)
	{
		int dx = abs(x1 - x0);
		int dy = abs(y1 - y0);
		int dirx = (x0 < x1) ? 1 : -1;
		int diry = (y0 < y1) ? 1 : -1;
		
		// error term
		int err = dx - dy;
		int e2;

		while (true)
		{
			drawPixel(x0, y0, wch, color);

			if (x0 == x1 && y0 == y1)
				break;

			e2 = 2 * err;
			if (e2 > -dy)
			{
				err -= dy;
				x0 += dirx;
			}
			if (e2 < dx)
			{
				err += dx;
				y0 += diry;
			}
		}

		return 1;
	}

	int drawRect(int x, int y, const RECT& rect, bool bFill = false, wchar_t wch = 0x2588, short color = 0x000F)
	{
		POINT topLeft     = { x, y };
		POINT topRight    = { x + rect.right - rect.left, y };
		POINT bottomLeft  = { x, y + rect.bottom - rect.top };
		POINT bottomRight = { x + rect.right - rect.left, y + rect.bottom - rect.top };

		drawLine(topLeft.x, topLeft.y, topRight.x, topRight.y, wch, color);
		drawLine(bottomLeft.x, bottomLeft.y, bottomRight.x, bottomRight.y, wch, color);
		drawLine(topLeft.x, topLeft.y, bottomLeft.x, bottomLeft.y, wch, color);
		drawLine(topRight.x, topRight.y, bottomRight.x, bottomRight.y, wch, color);

		if (bFill)
		{
			for (int row = topLeft.y + 1; row < bottomLeft.y; row++)
			{
				drawLine(topLeft.x, row, topRight.x, row, wch, color);
			}
		}

		return 1;
	}
	
	int drawTriangle(POINT& a, POINT& b, POINT& c, bool bFill = false, wchar_t wch = 0x2588, short color = 0x000F)
	{
		drawLine(a.x, a.y, b.x, b.y, wch, color);
		drawLine(b.x, b.y, c.x, c.y, wch, color);
		drawLine(c.x, c.y, a.x, a.y, wch, color);

		if (bFill)
		{
			std::vector<POINT> vertices = { a, b, c };
			std::sort(vertices.begin(), vertices.end(), compare_y_axis);

			int min_y_axis = vertices[0].y;
			int max_y_axis = vertices[2].y;
			
			for (int y = min_y_axis + 1; y < max_y_axis; y++)
			{
				std::vector<int> intersections;
				for (int i = 0; i < 3; i++)
				{
					POINT p1 = vertices[i];
					POINT p2 = vertices[(i + 1) % 3];

					if ((p1.y <= y && p2.y > y) || (p2.y <= y && p1.y > y))
					{
						int x = p1.x + (y - p1.y) * (p2.x - p1.x) / (p2.y - p1.y);
						intersections.push_back(x);
					}
				}
				if (intersections.size() == 2)
					drawLine(intersections[0], y, intersections[1], y, wch, color);
			}
		}

		return 1;
	}

	int drawCirlce(int center_x, int center_y, int radius, bool bFill = false, wchar_t wch = 0x2588, short color = 0x000F)
	{
		int x = radius - 1;
		int y = 0;
		int dx = 1;
		int dy = 1;
		int err = dx - (radius << 1);

		while (x >= y)
		{
			if (bFill)
			{
				drawLine(center_x - x, center_y + y, center_x + x, center_y + y, wch, color);
				drawLine(center_x - x, center_y - y, center_x + x, center_y - y, wch, color);
				drawLine(center_x - y, center_y + x, center_x + y, center_y + x, wch, color);
				drawLine(center_x - y, center_y - x, center_x + y, center_y - x, wch, color);
			}
			else
			{
				drawPixel(center_x + x, center_y + y, wch, color);
				drawPixel(center_x + y, center_y + x, wch, color);
				drawPixel(center_x - y, center_y + x, wch, color);
				drawPixel(center_x - x, center_y + y, wch, color);
				drawPixel(center_x - x, center_y - y, wch, color);
				drawPixel(center_x - y, center_y - x, wch, color);
				drawPixel(center_x + y, center_y - x, wch, color);
				drawPixel(center_x + x, center_y - y, wch, color);
			}

			if (err <= 0)
			{
				y++;
				err += dy;
				dy += 2;
			}
			if (err > 0)
			{
				x--;
				dx += 2;
				err += dx - (radius << 1);
			}
		}

		return 1;
	}

	int writeString(int x, int y, const std::wstring& text, short color = 0x000F)
	{
		if (x >= 0 && x < m_nScreenWidth && y >= 0 && y < m_nScreenHeight)
		{
			for (int i = 0; i < text.size(); i++)
			{
				m_chBufferBack[y * m_nScreenWidth + x + i].Char.UnicodeChar = text[i];
				m_chBufferBack[y * m_nScreenWidth + x + i].Attributes = color;
			}

			return 1;
		}
		return error(L"OUT OF BOUND");
	}

	std::vector<WORD> getEvent()
	{
		INPUT_RECORD input[128];
		DWORD dwNumEventsRead = 0;
		std::vector<WORD> keyCodes;

		if (PeekConsoleInput(m_hConsoleIn, input, sizeof(input) / sizeof(INPUT_RECORD), &dwNumEventsRead))
		{
			if (dwNumEventsRead > 0)
			{
				if (ReadConsoleInput(m_hConsoleIn, input, sizeof(input) / sizeof(INPUT_RECORD), &dwNumEventsRead))
				{
					for (DWORD i = 0; i < dwNumEventsRead; ++i)
					{
						if (input[i].EventType == KEY_EVENT && input[i].Event.KeyEvent.bKeyDown)
						{
							WORD wkeyCode = input[i].Event.KeyEvent.wVirtualKeyCode;
							keyCodes.push_back(wkeyCode);
						}
					}
				}
			}
		}

		return keyCodes;
	}

	int update()
	{ 
		/*
		compare each cell in the front buffer with the corresponding cell in the back buffer and if there's a difference
		copy the data from the back buffer to the front buffer. This means that every time
		update() is called, the front buffer is effectively updated to match the back buffer
		*/

		COORD bufferSize = { static_cast<short>(m_nScreenWidth), static_cast<short>(m_nScreenHeight) };
		COORD bufferCoord = { 0, 0 };

		for (int i = 0; i < m_nScreenWidth * m_nScreenHeight; i++)
		{
			if (m_chBufferFront[i].Char.UnicodeChar != m_chBufferBack[i].Char.UnicodeChar ||
				m_chBufferFront[i].Attributes != m_chBufferBack[i].Attributes)
			{
				m_chBufferFront[i] = m_chBufferBack[i];
			}
		}

		if (!WriteConsoleOutput(m_hConsole, m_chBufferFront.data(), bufferSize, bufferCoord, &m_rect))
			return error(L"FAILED TO UPDATE BUFFER");

		std::fill(m_chBufferBack.begin(), m_chBufferBack.end(), CHAR_INFO{ L' ', m_backgroundColor });

		return 1;
	}

	int width() const 
	{ 
		return m_nScreenWidth; 
	}

	int height() const 
	{ 
		return m_nScreenHeight;
	}

	void fillBackground(short color)
	{ 
		m_backgroundColor = color;
	}

	bool isError() const
	{
		return m_bError;
	}

	int drawPixel(int x, int y, short wch = 0x2588, short color = 0x000F)
	{
		if (x >= 0 && x < m_nScreenWidth && y >= 0 && y < m_nScreenHeight)
		{
			m_chBufferBack[y * m_nScreenWidth + x].Char.UnicodeChar = wch;
			m_chBufferBack[y * m_nScreenWidth + x].Attributes = color;
		}
		return 1;
	}

	private:
	int error(const wchar_t* message)
	{
		m_bError = true;
		wchar_t wchBuffer[256];
		FormatMessage(
			FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			GetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			wchBuffer,
			sizeof(wchBuffer) / sizeof(wchar_t),
			NULL
		);

		std::wcerr << L"Error Message    : " << message << std::endl;
		std::wcerr << L"Formatted Message: " << wchBuffer << std::endl;
		std::wcerr << L"Press enter key to continue" << std::endl;
		std::cin.ignore();
		return 0;
	}

	private:
	HANDLE m_hConsole;
	HANDLE m_hConsoleIn;
	SMALL_RECT m_rect;

	std::vector<CHAR_INFO> m_chBufferFront;
	std::vector<CHAR_INFO> m_chBufferBack;

	bool m_bError;
	bool m_bEnableSound;

	int m_nScreenWidth;
	int m_nScreenHeight;
	int m_mousePos_x;
	int m_mousePos_y;

	std::wstring m_sAppTitle;
	USHORT m_backgroundColor;
};

int main()
{
	Console2D program;
	program.setMode(80, 30);
	program.setCaption(L"Test");

	RECT rect = { 0, 0, program.width() - 1, program.height() - 1 };
	int nLineHeight = 5;

	while (true)
	{
		// ================ EVENT HANDLER ================
		
		for (const auto& wEvent : program.getEvent())
		{
			switch (wEvent)
			{
				case VK_RIGHT:
					break;

				case VK_LEFT:
					break;

				case VK_UP:
					break;

				case VK_DOWN:
					break;

				case VK_ESCAPE:
					break;
			}
		}

		if (program.isError())
			break;

		// ==================== START ====================

		for (int y = 0; y < program.height() - 1; y += 2)
		{
			int x = program.width() / 2 - 1;
			program.drawPixel(x, y, 0x007C);
		}

		program.drawRect(0, 0, rect);
		
		// ===================== END =====================

		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		program.update();
	}

	return 0;
}
