#ifndef UNICODE
#define UNICODE
#endif

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <iostream>
#include <vector>
#include <memory>
#include <stdexcept>

class Graphics
{
	public:
	Graphics() : m_chBuffer(nullptr), m_rect{}
	{
		m_nScreenWidth = 80;
		m_nScreenHeight = 30;

		m_hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		m_hConsoleIn = GetStdHandle(STD_INPUT_HANDLE);
		m_sAppTitle = L"Default";
	}

	~Graphics()
	{
		delete[] m_chBuffer;
	}

	int setMode(int nWidth, int nHeight, int nFontWidth = 8, int nFontHeight = 8)
	{
		if (m_hConsole == INVALID_HANDLE_VALUE || m_hConsoleIn == INVALID_HANDLE_VALUE)
			return error(L"INVALID HANDLE");

		m_nScreenWidth = nWidth;
		m_nScreenHeight = nHeight;

		COORD coord = { static_cast<short>(m_nScreenWidth), static_cast<short>(m_nScreenHeight) };
		if (!SetConsoleScreenBufferSize(m_hConsole, coord))
			return error(L"FAILED TO SET CONSOLE SIZE");

		if (!SetConsoleActiveScreenBuffer(m_hConsole))
			return error(L"FAILED TO SET CONSOLE");

		CONSOLE_FONT_INFOEX cfiex{};
		cfiex.cbSize = sizeof(CONSOLE_FONT_INFOEX);
		cfiex.nFont = NULL;
		cfiex.FontFamily = FF_DONTCARE;
		cfiex.FontWeight = FW_NORMAL;
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

		m_rect = { 0, 0, static_cast<short>(m_nScreenWidth), static_cast<short>(m_nScreenHeight) };
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

	int setPixel(int x, int y, short wch = 0x2588, short color = 0x000F)
	{
		if (x >= 0 && x < m_nScreenWidth && y >= 0 && y < m_nScreenHeight)
		{
			m_chBuffer[y * m_nScreenWidth + x].Char.UnicodeChar = wch;
			m_chBuffer[y * m_nScreenWidth + x].Attributes = color;
			return 1;
		}

		return 0;
	}
	
	int drawLine(int x1, int y1, int x2, int y2, wchar_t wch = 0x2588, short color = 0x000F)
	{
		
	}

	int refresh()
	{

	}

	int update()
	{
		if (!WriteConsoleOutput(m_hConsole,
								m_chBuffer,
								{ static_cast<short>(m_nScreenWidth - 1), static_cast<short>(m_nScreenHeight - 1) },
								{ 0, 0 },
								&m_rect))
			return error(L"FAILED TO DRAW");

		return 1;
	}

	HANDLE handle() { return m_hConsole; }

	private:
	int error(const wchar_t* message)
	{
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
};

int main()
{
	Graphics program;
	program.setMode(80, 30);
	program.setCaption(L"Test");

	while (true)
	{
		program.drawLine(0, 0);
	
		program.refresh();
	}

	return 0;
}
