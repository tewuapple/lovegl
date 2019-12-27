#include "pch.h"
#include "main_window.h"
#include "allres.h"

float rand_float(float min, float max);
int rand_int(int range);

void MainWindow::Draw()
{
	m_target.Clear(kk::Color(0.0f, 0.0f, 0.0f, 0.0f));

	UpdateSnow();
#if _DEBUG
	UpdateFps();
#endif
	UpdateMerryChristmas();
	UpdateHint();
}

void MainWindow::UpdateHint()
{
	auto opacity = m_hint_var.GetValue();
	if (opacity > 0)
	{
		m_hint_brush.SetOpacity(static_cast<float>(opacity));
		m_target.DrawTextLayout(m_hint_pos, m_hint_layout, m_hint_brush);
	}
}


void MainWindow::CreateDeviceResources()
{
	m_bmp_snows.clear();
	for (auto image : m_image_snows)
	{
		m_bmp_snows.push_back(m_target.CreateBitmapFromWicBitmap1(image));
	}
	m_bmp_mcs.clear();
	for (auto image : m_image_mcs)
	{
		m_bmp_mcs.push_back(m_target.CreateBitmapFromWicBitmap1(image));
	}

	m_fps_redBrush = m_target.CreateSolidColorBrush(kk::Color(0.9f, 0.1f, 0.2f));
	m_hint_brush = m_target.CreateSolidColorBrush(kk::Color(0.1f, 0.8f, 0.2f));
}

void MainWindow::UpdateFps()
{
	++m_fps_frames;
	LARGE_INTEGER current;
	QueryPerformanceCounter(&current);
	auto diff = current.QuadPart - m_fps_old;
	if (diff > m_fps_freq)
	{
		auto t = 1.0f * diff / m_fps_freq;
		m_fps = m_fps_frames / t;
		m_fps_frames = 0;
		m_fps_old = current.QuadPart;

		CString str;
		str.Format(L"%.2f", m_fps);
		m_fps_layout = m_dwrite.CreateTextLayout(str.GetString(), str.GetLength(), m_fps_format, 1000.0f, 1000.0f);

		DWRITE_TEXT_METRICS metrics;
		kk::HR(m_fps_layout->GetMetrics(&metrics));
		auto size = m_target.GetSize();
		m_fps_target = { size.Width - metrics.width, size.Height - metrics.height };
	}

	//m_target.DrawTextLayout(m_fps_target, m_fps_layout, m_fps_redBrush);
}

void MainWindow::UpdateMerryChristmas()
{
	auto mc = m_bmp_mcs[m_mc_id];
	auto size = mc.GetSize();
	auto dest = kk::RectF{ m_mc_pos.X, m_mc_pos.Y, size.Width + m_mc_pos.X, size.Height + m_mc_pos.Y };
	m_target.DrawBitmap(mc, dest);
}

void MainWindow::CreateDeviceIndependentResources()
{
	// load bitmap snow 1.
	{
		auto stream = SHCreateMemStream(res_s1::MakeResourceBuffer(), res_s1::MakeResourceSize());
		auto decoder = m_wic.CreateDecoderFromStream(stream);
		auto frame = decoder.GetFrame();
		auto image = m_wic.CreateFormatConverter();
		image.Initialize(frame);
		m_image_snows.push_back(image);
	}
	// load bitmap snow 2.
	{
		auto stream = SHCreateMemStream(res_s2::MakeResourceBuffer(), res_s2::MakeResourceSize());
		auto decoder = m_wic.CreateDecoderFromStream(stream);
		auto frame = decoder.GetFrame();
		auto image = m_wic.CreateFormatConverter();
		image.Initialize(frame);
		m_image_snows.push_back(image);
	}
	// load merry christmas 1.
	{
		auto stream = SHCreateMemStream(res_mc1::MakeResourceBuffer(), res_mc1::MakeResourceSize());
		auto decoder = m_wic.CreateDecoderFromStream(stream);
		auto frame = decoder.GetFrame();
		auto image = m_wic.CreateFormatConverter();
		image.Initialize(frame);
		m_image_mcs.push_back(image);
	}
	// load merry christmas 2.
	{
		auto stream = SHCreateMemStream(res_mc2::MakeResourceBuffer(), res_mc2::MakeResourceSize());
		auto decoder = m_wic.CreateDecoderFromStream(stream);
		auto frame = decoder.GetFrame();
		auto image = m_wic.CreateFormatConverter();
		image.Initialize(frame);
		m_image_mcs.push_back(image);
	}
	// load merry christmas 3.
	{
		auto stream = SHCreateMemStream(res_mc3::MakeResourceBuffer(), res_mc3::MakeResourceSize());
		auto decoder = m_wic.CreateDecoderFromStream(stream);
		auto frame = decoder.GetFrame();
		auto image = m_wic.CreateFormatConverter();
		image.Initialize(frame);
		m_image_mcs.push_back(image);
	}
	// m_mc_pos (start at bottom-right upon the taskbar.)
	{
		RECT rect;
		GetClientRect(&rect);
		auto size = m_image_mcs[0].GetSize();
		m_mc_pos.X = static_cast<float>(rect.right - size.Width);
		m_mc_pos.Y = static_cast<float>(rect.bottom - size.Height);
	}
	// music
	SetTimer(WmTimerCreateMusic, 50, [](HWND hwnd, UINT, UINT_PTR, DWORD) {
		vector<wchar_t> file;
		// get temp file name.
		{
			file.resize(MAX_PATH);
			WCHAR temp[MAX_PATH];
			VERIFY(GetTempPath(_countof(temp), temp));
			VERIFY(GetTempFileName(temp, L"LGL", 0, &file[0]));
		}

		// open the file and write music data.
		{
			FILE * f;
			_wfopen_s(&f, &file[0], L"wb+");
			fwrite(res_music::MakeResourceBuffer(), 1, res_music::MakeResourceSize(), f);
			fclose(f);
		}

		// play music
		auto open = L"open \""s + &file[0] + L"\" type sequencer alias music"s;
		auto play = L"play music notify"s;
		mciSendString(open.c_str(), nullptr, 0, nullptr);
		mciSendString(play.c_str(), nullptr, 0, hwnd);

		// animation
		{
			auto ptr = ::GetWindowLongPtr(hwnd, HwndUserData_This);
			auto that = reinterpret_cast<MainWindow *>(ptr);
			auto tran = that->m_wam_lib.CreateLinearTransition(3.0, 0.0);
			that->m_wam.ScheduleTransition(that->m_hint_var, tran, that->GetTimeNow());
		}

		::KillTimer(hwnd, WmTimerCreateMusic);
	});
	QueryPerformanceFrequency((PLARGE_INTEGER)&m_fps_freq);
	m_fps_format = m_dwrite.CreateTextFormat(L"Consolas", 18.0f);

	// hint 
	{
		wchar_t text[] = L"右键点击下面↓↓↓可以关闭程序哦！";
		m_hint_layout = m_dwrite.CreateTextLayout(text, _countof(text), m_fps_format, 1000.0, 1000.0);
		m_hint_var = m_wam.CreateAnimationVariable(2.0);
		
		RECT rect;
		GetClientRect(&rect);

		DWRITE_TEXT_METRICS metrics;
		m_hint_layout->GetMetrics(&metrics);
		auto mcsize = m_image_mcs[m_mc_id].GetSize();
		
		m_hint_pos.X = rect.right - metrics.width;
		m_hint_pos.Y = rect.bottom - mcsize.Height - metrics.height;
	}

	SetTimer(WmTimerOncePerSecond, 1000, [](HWND hwnd, UINT, UINT_PTR, DWORD) {
		auto ptr = ::GetWindowLongPtr(hwnd, HwndUserData_This);
		auto that = reinterpret_cast<MainWindow *>(ptr);
		that->OncePerSecondHandler();
	});
	SetTimer(WmTimerUpdateMerryChristmas, WmTimerUpdateMerryChristmasInterval, [](HWND hwnd, UINT, UINT_PTR, DWORD) {
		auto ptr = ::GetWindowLongPtr(hwnd, HwndUserData_This);
		auto that = reinterpret_cast<MainWindow *>(ptr);
		that->m_mc_id = (that->m_mc_id + 1) % that->m_bmp_mcs.size();
	});
}

void MainWindow::UpdateSnow()
{
	POINT p;
	GetCursorPos(&p);
	::ScreenToClient(m_hWnd, &p);

	for (auto item : m_snow_items)
	{
		double pos[2];
		item.pos.GetVectorValue(pos);
		auto& bmp = m_bmp_snows[item.snow_id];
		auto size = bmp.GetSize();
		auto dest = Rect(-size.Width / 2.0f, -size.Height / 2.0f, size.Width / 2.0f, size.Height / 2.0f);
		auto angle = (float)item.angle.GetValue();
		auto itemSize = (float)item.size.GetValue();

		m_target.SetTransform(
			Matrix3x2F::Scale(SizeF(itemSize, itemSize))
			* Matrix3x2F::Rotation(angle)
			* Matrix3x2F::Translation(SizeF((float)pos[0], (float)pos[1])));
		m_target.DrawBitmap(bmp, dest);

		// check distance.
		{
			auto dist2 = sqrt((p.x - pos[0]) * (p.x - pos[0]) + (p.y - pos[1]) * (p.y - pos[1]));
			auto r2 = itemSize * sqrt(size.Width*size.Width + size.Height*size.Height) / 2.0f;
			if (dist2 < r2)
			{
				const float ItemDismissSpeed = 1.0f;
				auto sizeTran = m_wam_lib.CreateLinearTransitionFromSpeed(ItemDismissSpeed, 0.0f);
				m_wam.ScheduleTransition(item.size, sizeTran, GetTimeNow());
			}
		}
	}
	m_target.SetTransform(Matrix3x2F::Identity());
}

snow_item MainWindow::GenerateSnowItem()
{
	auto size = m_target.GetSize();
	snow_item item;

	item.snow_id = rand_int(m_bmp_snows.size());
	item.size = m_wam.CreateAnimationVariable(rand_float(0.4f, 1.0f));

	float startOffset = 20.0f;
	/*auto startPos = rand_int(3);*/
	auto startPos = 1;
	int endPos = 0;
	kk::Point2F start;
	switch (startPos)
	{
	case 0: // starts from top left
		start.X = -startOffset;
		start.Y = rand_float(0.0f, size.Height / 3.0f);
		endPos = rand_int(2) + 1;
		break;
	case 1: // starts from top 
		start.X = rand_float(0.0f, size.Width);
		start.Y = -startOffset;
		endPos = rand_int(3);
		break;
	case 2: // starts from top right
		start.X = size.Width + startOffset;
		start.Y = rand_float(0.0f, size.Height / 3.0f);
		endPos = rand_int(2);
		break;
	}
	double startVec[] = { start.X, start.Y };

	kk::Point2F dest;
	const float extend_offset = 50.0f;
	switch (endPos)
	{
	case 0: // ends with bottom left
		dest.X = 0.0f - extend_offset;
		dest.Y = rand_float(size.Height / 3.0f * 2.0f, size.Height);
		break;
	case 1: // ends with bottom
		dest.X = rand_float(0.0f, size.Width);
		dest.Y = size.Height + extend_offset;
		break;
	case 2: // ends with bottom right
		dest.X = size.Width + extend_offset;
		dest.Y = rand_float(size.Height / 3.0f * 2.0f, size.Height);
		break;
	}

	item.pos = m_wam.CreateAnimationVectorVariable(startVec);
	auto speed = rand_float(60.0f, 100.0f);
	double destDouble[] = { (double)dest.X, (double)dest.Y };
	auto transition = m_wam_lib.CreateLinearVectorTransitionFromSpeed(speed, destDouble);
	m_wam.ScheduleTransition(item.pos, transition, GetTimeNow());

	item.angle = m_wam.CreateAnimationVariable(rand_float(0.0f, 360.0f));
	auto angleSpeed = rand_float(20.0f, 40.0f);
	auto angleTransition = m_wam_lib.CreateLinearTransitionFromSpeed(angleSpeed, rand_int(2) == 1 ? 50000.0f : -50000.0f);
	m_wam.ScheduleTransition(item.angle, angleTransition, GetTimeNow());	

	return item;
}

void MainWindow::RemoveDeadSnowItem()
{
	auto size = m_target.GetSize();
	auto isout = [&](snow_item const & item) {
		double pos[2];
		item.pos.GetVectorValue(pos);
		double itemSize = item.size.GetValue();
		const float outoffset = 40.0f;

		if (itemSize == 0.0 ||
			pos[0]+outoffset < 0.0f ||
			pos[0]-outoffset > size.Width ||
			pos[1]-outoffset > size.Height)
		{
			return true;
		}
		else
		{
			return false;
		}
	};

	m_snow_items.erase(
		remove_if(begin(m_snow_items), end(m_snow_items), isout),
		end(m_snow_items));
}

void MainWindow::OncePerSecondHandler()
{
	if (!m_target)
	{
		return;
	}
	const int SnowItemCount = 15;
	if (m_snow_items.size() < SnowItemCount)
	{
		auto item = GenerateSnowItem();
		m_snow_items.push_back(item);
	}
	RemoveDeadSnowItem();
	InvalidateRect(nullptr, false);
}

void MainWindow::BeginDrag()
{
	{
		auto size = m_bmp_mcs[m_mc_id].GetSize();
		auto rect = RECT
		{
			static_cast<LONG>(m_mc_pos.X),
			static_cast<LONG>(m_mc_pos.Y),
			static_cast<LONG>(m_mc_pos.X + size.Width),
			static_cast<LONG>(m_mc_pos.Y + size.Height)
		};
		if (PtInRect(&rect, m_drag_start))
		{
			m_mc_pos_old = m_mc_pos;
			m_mc_drag = true;
		}
	}
}

void MainWindow::EndDrag()
{
	m_mc_drag = false;
}

void MainWindow::DragMove()
{
	if (m_mc_drag)
	{
		m_mc_pos.X = m_mc_pos_old.X + m_drag_offset.x;
		m_mc_pos.Y = m_mc_pos_old.Y + m_drag_offset.y;
	}
}


float rand_float(float min, float max)
{
	uniform_real_distribution<float> urd{ min, max };
	random_device rd;
	return urd(rd);
}

int rand_int(int range)
{
	random_device rd;
	return rd() % range;
}