#pragma once
#include "base_window.h"

struct snow_item
{
	int snow_id;
	kk::Wam::Variable size;
	kk::Wam::Variable angle;
	kk::Wam::Variable pos;
};

struct MainWindow : BaseWindow<MainWindow>
{
	void Draw();

	void CreateDeviceResources();

	void CreateDeviceIndependentResources();

	void UpdateSnow();
	void UpdateHint();
	void UpdateFps();

	void OncePerSecondHandler();

	void BeginDrag();
	void EndDrag();
	void DragMove();

	snow_item GenerateSnowItem();
	void RemoveDeadSnowItem();
	void UpdateMerryChristmas();

	// snows
	vector<kk::Wic::FormatConverter> m_image_snows;
	vector<kk::Direct2D::Bitmap1> m_bmp_snows;
	vector<snow_item> m_snow_items;

	// mc
	vector<kk::Wic::FormatConverter> m_image_mcs;
	vector<kk::Direct2D::Bitmap1> m_bmp_mcs;
	kk::Point2F m_mc_pos;
	bool m_mc_drag = false;
	kk::Point2F m_mc_pos_old;
	int m_mc_id = 0;
	const int WmTimerUpdateMerryChristmas = 3;
	const int WmTimerUpdateMerryChristmasInterval = 500;
	
	// fps
	int64_t m_fps_old = 0;
	int64_t m_fps_freq;
	float m_fps;
	short m_fps_frames = 0;
	kk::DirectWrite::TextFormat m_fps_format;
	kk::Direct2D::SolidColorBrush m_fps_redBrush;
	kk::DirectWrite::TextLayout m_fps_layout;
	kk::Point2F m_fps_target;

	// hint
	kk::DirectWrite::TextLayout m_hint_layout;
	kk::Point2F m_hint_pos;
	kk::Wam::Variable m_hint_var;
	kk::Direct2D::SolidColorBrush m_hint_brush;

	// music
	const static int WmTimerCreateMusic = 4;
};