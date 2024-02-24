#include "Console2D.hpp"

int main()
{
	c2d::Console2D program;
	program.setMode(80, 30);
	program.setCaption(L"Pong");

	bool bCloseApplication = false;

	POINT position = { 0, 0 };
	RECT rect = { 0, 0, 5, 5 };
	int nSpeed = 1;

	while (!bCloseApplication)
	{
		if (program.isError())
			break;

		// =================== START ===================

		for (const auto& input : program.getEvents())
		{
			switch (input.EventType)
			{
				case KEY_EVENT:
					if (input.Event.KeyEvent.bKeyDown)
					{
						if (input.Event.KeyEvent.wVirtualKeyCode == VK_RIGHT)
							position.x += nSpeed;

						if (input.Event.KeyEvent.wVirtualKeyCode == VK_LEFT)
							position.x -= nSpeed;

						if (input.Event.KeyEvent.wVirtualKeyCode == VK_DOWN)
							position.y += nSpeed;

						if (input.Event.KeyEvent.wVirtualKeyCode == VK_UP)
							position.y -= nSpeed;
					}
					break;

				case MOUSE_EVENT:
					break;

				default:
					break;
			}
		}

		program.drawRect(position.x, position.y, rect, true, c2d::FULL_BLOCK, c2d::FG_RED);

		// ==================== END ====================

		//std::this_thread::sleep_for(std::chrono::milliseconds(200));
		program.update();
	}

	return 0;
}