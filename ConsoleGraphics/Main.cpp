#ifndef UNICODE
#define UNICODE
#endif

#define WIN32_LEAN_AND_MEAN
#include "Windows.h"
#include "Vector2.hpp"
#include "Utils.hpp"

#include <iostream>
#include <vector>
#include <thread>
#include <stdexcept>

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

		//m_rect = { 0, 0, 1, 1 };
		//SetConsoleWindowInfo(m_hConsole, TRUE, &m_rect);

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
		if (x0 < 0 || x0 >= m_nScreenWidth || y0 < 0 || y0 >= m_nScreenHeight ||
			x1 < 0 || x1 >= m_nScreenWidth || y1 < 0 || y1 >= m_nScreenHeight)
		{
			return error(L"COORD OUT OF BOUND");
		}

		int dx = abs(x1 - x0);
		int dy = abs(y1 - y0);
		int dirx = (x0 < x1) ? 1 : -1;
		int diry = (y0 < y1) ? 1 : -1;
		
		// error term
		int err = dx - dy;
		int e2;

		while (true)
		{
			setPixel(x0, y0, wch, color);

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

	int drawRect()
	{

	}

	int drawTriangle()
	{

	}

	int drawEllipse()
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
	int setPixel(int x, int y, short wch = 0x2588, short color = 0x000F)
	{
		if (x >= 0 && x < m_nScreenWidth && y >= 0 && y < m_nScreenHeight)
		{
			m_chBuffer[y * m_nScreenWidth + x].Char.UnicodeChar = wch;
			m_chBuffer[y * m_nScreenWidth + x].Attributes = color;
			return 1;
		}

		return error(L"OUT OF BOUND");
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

	Vector2<int> position = { 0, 0 };
	Vector2<int> direction = { 1 , 1 };
	while (true)
	{
		if (program.isError())
			break;

		program.refresh();

		program.drawLine(position.x, position.y, position.x, position.y);
		position += direction;

		position.x = clamp(position.x, 0, program.width() - 1);
		position.y = clamp(position.y, 0, program.height() - 1);
	
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
		program.update();
	}

	return 0;
}
