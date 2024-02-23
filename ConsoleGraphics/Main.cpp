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

class Graphics
{
	public:
	Graphics() 
		: m_nScreenWidth(NULL), m_nScreenHeight(NULL), m_chBuffer(nullptr), m_rect{}
	{
		m_sAppTitle  = L"Default";
		m_hConsole   = GetStdHandle(STD_OUTPUT_HANDLE);
		m_hConsoleIn = GetStdHandle(STD_INPUT_HANDLE);
		m_bError     = false;
	}

	~Graphics() 
	{
		delete[] m_chBuffer;
	}

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

		m_chBuffer = new CHAR_INFO[m_nScreenWidth * m_nScreenHeight];
		memset(m_chBuffer, NULL, sizeof(CHAR_INFO) * m_nScreenWidth * m_nScreenHeight);

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
			if (!draw(x0, y0, wch, color))
				continue;

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

	int drawCirlce()
	{

	}

	int writeString(int x, int y, const std::wstring& text, short color = 0x000F)
	{
		if (x >= 0 && x < m_nScreenWidth && y >= 0 && y < m_nScreenHeight)
		{
			for (std::size_t i = 0; i < text.size(); i++)
			{
				m_chBuffer[y * m_nScreenWidth + x + i].Char.UnicodeChar = text[i];
				m_chBuffer[y * m_nScreenWidth + x + i].Attributes = color;
			}

			return 1;
		}
		return error(L"OUT OF BOUND");
	}

	int refresh()
	{
		for (int i = 0; i < m_nScreenWidth * m_nScreenHeight; i++)
		{
			m_chBuffer[i].Char.UnicodeChar = L' ';
			m_chBuffer[i].Attributes = NULL;
		}

		return 1;
	}

	int update()
	{
		COORD bufferSize  = { static_cast<short>(m_nScreenWidth), static_cast<short>(m_nScreenHeight) };
		COORD bufferCoord = { 0, 0 };
		
		if (!WriteConsoleOutput(m_hConsole, m_chBuffer, bufferSize, bufferCoord, &m_rect))
			return error(L"FAILED TO UPDATE BUFFER");

		return 1;
	}

	int width() const { return m_nScreenWidth; }

	int height() const { return m_nScreenHeight; }

	bool isError() const { return m_bError; }

	private:
	int draw(int x, int y, short wch = 0x2588, short color = 0x000F)
	{
		if (x >= 0 && x < m_nScreenWidth && y >= 0 && y < m_nScreenHeight)
		{
			m_chBuffer[y * m_nScreenWidth + x].Char.UnicodeChar = wch;
			m_chBuffer[y * m_nScreenWidth + x].Attributes = color;
			return 1;
		}
		return 0;
	}

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

	int m_nScreenWidth;
	int m_nScreenHeight;
	std::wstring m_sAppTitle;

	CHAR_INFO* m_chBuffer;
	SMALL_RECT m_rect;

	bool m_bError;
};

int main()
{
	Graphics program;
	program.setMode(80, 30);
	program.setCaption(L"Test");

	POINT position  = { 0, 0 };
	POINT direction = { 0, 1 };
	RECT rect = { 0, 0, 5, 5 };

	POINT vertices[3];
	vertices[0] = { 0, 0 };
	vertices[1] = { 0, 5 };
	vertices[2] = { 5, 5 };

	while (true)
	{
		// implement later event handler
		if (program.isError())
			break;

		program.refresh();
		// ==================== START ====================

		program.drawRect(position.x, position.y, rect, true);
		//position.x += direction.x;
		//position.y += direction.y;

		program.drawTriangle(vertices[0], vertices[1], vertices[2], true);
		vertices[0].y += direction.y;
		vertices[1].y += direction.y;
		vertices[2].y += direction.y;
		
		// ===================== END =====================

		// implement later proper frame managment
		std::this_thread::sleep_for(std::chrono::milliseconds(500)); // for now suffice
		program.update();
	}

	return 0;
}
