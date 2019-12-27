#pragma once

#include "resource.h"

namespace kk = KennyKerr;

struct LayeredWindow
{
	UPDATELAYEREDWINDOWINFO info;
	BLENDFUNCTION blend;
	SIZE size;
	POINT source, destination;

	LayeredWindow(unsigned width, unsigned height) :
		info{},
		blend{},
		size{},
		source{},
		destination{}
	{
		info.cbSize = sizeof(UPDATELAYEREDWINDOWINFO);
		info.pblend = &blend;
		info.psize = &size;
		info.pptSrc = &source;
		info.pptDst = &destination;
		info.dwFlags = ULW_ALPHA;

		blend.SourceConstantAlpha = 255;
		blend.AlphaFormat = AC_SRC_ALPHA;

		size.cx = width;
		size.cy = height;
	}

	auto Update(HWND window, HDC hdc) -> void
	{
		info.hdcSrc = hdc;
		VERIFY(UpdateLayeredWindowIndirect(window, &info));
	}
};

template <typename T>
struct BaseWindow :
	CWindowImpl<T, CWindow, CWinTraits<
#if _DEBUG
		WS_OVERLAPPED | WS_VISIBLE | WS_POPUP, 
		WS_EX_LAYERED | WS_EX_TOOLWINDOW >>
#else
		WS_OVERLAPPED | WS_VISIBLE | WS_POPUP,
		WS_EX_LAYERED | WS_EX_TOOLWINDOW | WS_EX_TOPMOST>>
#endif
{
	const static int WmCloseWindow = WM_USER;
	const static UINT_PTR WmTimerUpdateWindowLazy = 1;
	const static UINT_PTR WmTimerOncePerSecond = 2;
	const static int HwndUserData_This = GWLP_USERDATA;
	
	DECLARE_WND_CLASS_EX(nullptr, 0, -1)

	BEGIN_MSG_MAP(c)
		MESSAGE_HANDLER(WM_DESTROY, DestroyHandler)
		MESSAGE_HANDLER(WM_PAINT, PaintHandler)
		MESSAGE_HANDLER(WmCloseWindow, CloseWindow)
		MESSAGE_HANDLER(WM_LBUTTONDOWN, LButtonDownHandler)
		MESSAGE_HANDLER(WM_LBUTTONUP, LButtonUpHandler)
		MESSAGE_HANDLER(WM_MOUSEMOVE, MouseMoveHandler)
		MESSAGE_HANDLER(MM_MCINOTIFY, NotifyHandler)
		MESSAGE_HANDLER(WM_CONTEXTMENU, ContextMenuHandler)
		COMMAND_ID_HANDLER(ID_EXIT, ExitHandler)
	END_MSG_MAP()

	LRESULT ExitHandler(WORD, WORD, HWND, BOOL &)
	{
		PostMessage(WmCloseWindow, 0, 0);
		return 0;
	}

	LRESULT ContextMenuHandler(UINT, WPARAM, LPARAM lparam, BOOL &)
	{
		HMENU menu;
		menu = LoadMenu(nullptr, MAKEINTRESOURCE(IDR_MENU1));
		menu = GetSubMenu(menu, 0);
		TrackPopupMenu(menu, TPM_TOPALIGN | TPM_LEFTALIGN, GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam), 0, m_hWnd, NULL);
		return 0;
	}

	LRESULT NotifyHandler(UINT, WPARAM, LPARAM, BOOL &)
	{
		mciSendString(L"seek music to start", NULL, 0, NULL);
		mciSendString(L"play music notify", NULL, 0, m_hWnd);
		return 0;
	}

	LRESULT LButtonDownHandler(UINT, WPARAM, LPARAM lparam, BOOL &)
	{
		SetCapture();
		m_drag_start.x = GET_X_LPARAM(lparam);
		m_drag_start.y = GET_Y_LPARAM(lparam);
		static_cast<T *>(this)->BeginDrag();
		return 0;
	}

	LRESULT MouseMoveHandler(UINT, WPARAM, LPARAM lparam, BOOL &)
	{
		if (GetCapture() != nullptr)
		{
			auto x = GET_X_LPARAM(lparam);
			auto y = GET_Y_LPARAM(lparam);
			m_drag_offset.x = x - m_drag_start.x;
			m_drag_offset.y = y - m_drag_start.y;
			static_cast<T *>(this)->DragMove();
		}
		return 0;
	}

	LRESULT LButtonUpHandler(UINT, WPARAM, LPARAM, BOOL &)
	{
		ReleaseCapture();
		m_drag_offset.x = m_drag_offset.y = 0;
		static_cast<T *>(this)->EndDrag();
		return 0;
	}

	LRESULT CloseWindow(UINT, WPARAM, LPARAM, BOOL &)
	{
		DestroyWindow();
		return 0;
	}

	LRESULT PaintHandler(UINT, WPARAM, LPARAM, BOOL &)
	{
		Render();
		return 0;
	}

	LRESULT DestroyHandler(UINT, WPARAM, LPARAM, BOOL &)
	{
		PostQuitMessage(0);
		return 0;
	}

	void Render()
	{
		if (!m_target)
		{
			auto d3d = kk::Direct3D::CreateDevice();
			/*kk::Direct3D::Device d3d;
			kk::Direct3D::CreateDevice(d3d, kk::Direct3D::DriverType::Warp);*/
			auto d2d = m_2dfac.CreateDevice(d3d);
			m_target = d2d.CreateDeviceContext();
			
			/*kk::Dxgi::SwapChainDescription1 desc{};
			desc.Flags = kk::Dxgi::SwapChainFlag::GdiCompatible;
			auto dxgi = d3d.GetDxgiFactory();
			auto swapChain = dxgi.CreateSwapChainForHwnd(d3d, m_hWnd, desc);
			auto surface = swapChain.GetBuffer();			
			auto bitmap = m_target.CreateBitmapFromDxgiSurface(surface);
			swapChain->GetContainingOutput(m_output.ReleaseAndGetAddressOf());
			m_target.SetTarget(bitmap);*/

			/*RECT rect;
			GetClientRect(&rect);
			kk::Direct3D::TextureDescription2D desc{};
			desc.Width = rect.right - rect.left;
			desc.Height = rect.bottom - rect.top;
			desc.MiscFlags = kk::Direct3D::ResourceMiscFlag::GdiCompatible;
			desc.BindFlags = kk::Direct3D::BindFlag::RenderTarget;
			desc.MipLevels = 1;
			auto texture = d3d.CreateTexture2D(desc);
			kk::Dxgi::Surface surface;
			kk::HR(texture->QueryInterface(surface.GetAddressOf()));
			auto bitmap = m_target.CreateBitmapFromDxgiSurface(surface);
			m_target.SetTarget(bitmap); */

			/*kk::Direct2D::BitmapProperties1 prop{};
			prop.BitmapOptions =
				kk::Direct2D::BitmapOptions::GdiCompatible | kk::Direct2D::BitmapOptions::Target;
			prop.PixelFormat = kk::PixelFormat(kk::Dxgi::Format::B8G8R8A8_UNORM, kk::AlphaMode::Premultiplied);

			auto image = m_wic.CreateBitmap(kk::SizeU(m_layered->size.cx, m_layered->size.cy));
			kk::Direct2D::Bitmap1 bitmap = m_target.CreateBitmapFromWicBitmap1(image, prop);
			m_target.SetTarget(bitmap);*/

			kk::Direct2D::BitmapProperties1 prop{};
			prop.BitmapOptions =
				kk::Direct2D::BitmapOptions::GdiCompatible | kk::Direct2D::BitmapOptions::Target;
			prop.PixelFormat = kk::PixelFormat(kk::Dxgi::Format::B8G8R8A8_UNORM, kk::AlphaMode::Premultiplied);
			auto bitmap = m_target.CreateBitmap(kk::SizeU(m_layered->size.cx, m_layered->size.cy), prop);
			m_target.SetTarget(bitmap);

			static_cast<T *>(this)->CreateDeviceResources();
			static_cast<T *>(this)->CreateDeviceSizeResources();
		}

		/*m_output->WaitForVBlank();*/
		m_wam.Update(GetTimeNow());
		m_target.BeginDraw();
		static_cast<T *>(this)->Draw();		
		{
			auto interop = m_target.AsGdiInteropRenderTarget();
			auto hdc = interop.GetDC(kk::Direct2D::DcInitializeMode::Copy);
			m_layered->Update(m_hWnd, hdc);
			interop.ReleaseDC();
		}
		m_target.EndDraw();

		ValidateRect(nullptr);
	}

	double GetTimeNow()
	{
		double now = m_wam_timer.GetTime();
		return now;
	}

	void Create()
	{
		RECT rect = { 0, 0, 500, 500 };
		MONITORINFO mi = { sizeof(mi) };
		auto monitor = MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTONEAREST);
		GetMonitorInfo(monitor, &mi);
		rect = mi.rcWork;
		__super::Create(nullptr, rect, L"Main Window");

		// CreateDeviceIndependentResources
		m_2dfac = kk::Direct2D::CreateFactory();
		m_dwrite = kk::DirectWrite::CreateFactory1();
		m_wic = kk::Wic::CreateFactory();
		m_wam = kk::Wam::CreateManager();
		m_wam_lib = kk::Wam::CreateTransitionLibrary();
		m_wam_timer = kk::Wam::CreateTimer();

		static_cast<T *>(this)->CreateDeviceIndependentResources();
		m_layered = make_unique<LayeredWindow>(rect.right - rect.left, rect.bottom - rect.top);

		SetTimer(WmTimerUpdateWindowLazy, 20, [](HWND hwnd, UINT, UINT_PTR, DWORD) {
			::InvalidateRect(hwnd, nullptr, false);
		});
		InvalidateRect(nullptr, false);
		SetWindowLongPtr(HwndUserData_This, (LONG_PTR)this);
	}

	// virtual functions
	void CreateDeviceIndependentResources() {}
	void CreateDeviceResources() {}
	void Draw() {}
	void CreateDeviceSizeResources() {}
	void BeginDrag() {}
	void EndDrag() {}
	void DragMove() {}

	// device independent resources.
	kk::Direct2D::Factory1 m_2dfac;
	kk::DirectWrite::Factory1 m_dwrite;
	kk::Wam::Manager m_wam;
	kk::Wam::TransitionLibrary m_wam_lib;
	kk::Wam::Timer m_wam_timer;

	// device resources.
	kk::Direct2D::DeviceContext m_target;
	Microsoft::WRL::ComPtr<IDXGIOutput> m_output;
	kk::Wic::Factory m_wic;

	// object.
	unique_ptr<LayeredWindow> m_layered;
	POINT m_drag_offset;
	POINT m_drag_start;
};