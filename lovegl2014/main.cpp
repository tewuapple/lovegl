#include "pch.h"
#include "main_window.h"

int __stdcall wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
{
	kk::ComInitialize com{ kk::Apartment::SingleThreaded };
	{
		MainWindow window;
		window.Create();

		MSG msg;
		/*while (true)
		{
			window.Render();

			while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
			{
				DispatchMessage(&msg);
			}

			if (msg.message == WM_QUIT)
			{
				break;
			}
		}*/
		while (GetMessage(&msg, nullptr, 0, 0))
		{
			DispatchMessage(&msg);
		}
	}
}