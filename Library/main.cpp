///<summary>
///CoGフレームワーク
///WinMain()から始まります
/// 多少いじりましたが、基本的にN.Hanaiさんのコードを参考にしています
/// By_PointP
///</summary>
///<author>N.Hanai</author>
///

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

#ifdef _DEBUG
#define DBG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__);
#else
#define DBG_NEW new
#endif

#define IMGUI 1

#include <DxLib.h>

#include "App.h"

#include "../Source/Screen.h"
#include "../ImGui/imgui_impl_dxlib.hpp"

// Inputをインクルード
#include "../Library/Input.h"
#include "../Library/GameSetting.h"
#include "../Library/Time.h"

#define CoGVersion (4.1)


//==================================================
// プログラムは WinMain から始まります
//==================================================
int WINAPI WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow)
{
	//--------------------------------------------------
	// DxLib基本設定
	//--------------------------------------------------
	SetGraphMode(
		Screen::WIDTH,
		Screen::HEIGHT,
		32);

	SetOutApplicationLogValidFlag(FALSE);

	SetMainWindowText(Screen::WINDOW_NAME);

	SetWindowSizeExtendRate(
		Screen::WINDOW_EXTEND);

	ChangeWindowMode(
		Screen::WINDOW_MODE);

	SetWindowStyleMode(7);

	SetWindowSizeChangeEnableFlag(
		TRUE,
		TRUE);


	//--------------------------------------------------
	// DxLib初期化
	//--------------------------------------------------
	if (DxLib_Init() == -1)
	{
		return -1;
	}

	SetDrawScreen(DX_SCREEN_BACK);

	SetAlwaysRunFlag(TRUE);

	SetUseZBuffer3D(TRUE);
	SetWriteZBuffer3D(TRUE);


	//--------------------------------------------------
	// ウィンドウプロシージャ
	//
	// DxLibとImGuiのウィンドウプロシージャを両立
	//--------------------------------------------------
	SetHookWinProc(
		[](HWND hWnd,
			UINT msg,
			WPARAM wParam,
			LPARAM lParam) -> LRESULT
		{
			SetUseHookWinProcReturnValue(FALSE);

			return ImGui_ImplWin32_WndProcHandler(
				hWnd,
				msg,
				wParam,
				lParam);
		});


	//--------------------------------------------------
	// ImGui初期化
	//--------------------------------------------------
#if IMGUI

	IMGUI_CHECKVERSION();

	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO();

	io.ConfigFlags |=
		ImGuiConfigFlags_DockingEnable;

	io.ConfigFlags |=
		ImGuiConfigFlags_ViewportsEnable;

	io.Fonts->AddFontFromFileTTF(
		u8"c:\\Windows\\Fonts\\meiryo.ttc",
		18.0f,
		nullptr,
		io.Fonts->GetGlyphRangesJapanese());

	ImGui_ImplDXlib_Init();

#endif


	//--------------------------------------------------
	// ゲーム初期化
	//--------------------------------------------------
	Time::Init();

	// 目標FPS
	// 60なら60FPS固定
	// 120なら120FPS固定
	// 0ならFPS制限なし
	Time::SetTargetFPS(120);

	AppInit();

	// 起動時に設定をロード
	GameSetting::Load();


	//--------------------------------------------------
	// メインループ
	//--------------------------------------------------
	while (true)
	{
		//--------------------------------------------------
		// Windowsメッセージ処理
		//--------------------------------------------------
		if (ProcessMessage() == -1 ||
			AppIsExit())
		{
			break;
		}


		//--------------------------------------------------
		// 時間更新
		//--------------------------------------------------
		Time::Refresh();


		//--------------------------------------------------
		// FPSをウィンドウタイトルに表示
		//--------------------------------------------------
		char windowTitle[256];

		sprintf_s(
			windowTitle,
			"%s  [ %.1f FPS ]",
			Screen::WINDOW_NAME,
			Time::FPS());

		SetMainWindowText(windowTitle);


		//--------------------------------------------------
		// 入力状態更新
		//--------------------------------------------------
		Input::KeyStateUpdate();
		Input::MouseStateUpdate();


		//--------------------------------------------------
		// ImGuiフレーム開始
		//--------------------------------------------------
#if IMGUI

		ImGui_ImplDXlib_NewFrame();
		ImGui::NewFrame();

#endif


		//--------------------------------------------------
		// ゲーム更新
		//--------------------------------------------------
		AppUpdate();


		//--------------------------------------------------
		// 画面クリア
		//--------------------------------------------------
		ClearDrawScreen();


		//--------------------------------------------------
		// ゲーム描画
		//--------------------------------------------------
		AppDraw();


		//--------------------------------------------------
		// ImGui描画
		//--------------------------------------------------
#if IMGUI

		ImGui::EndFrame();
		ImGui::Render();

		ImGui_ImplDXlib_RenderDrawData();

#endif


		//--------------------------------------------------
		// Direct3D設定更新
		//--------------------------------------------------
		RefreshDxLibDirect3DSetting();


		//--------------------------------------------------
		// ImGuiの別ViewPort描画
		//--------------------------------------------------
#if IMGUI

		if (ImGui::GetIO().ConfigFlags &
			ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}

#endif


		//--------------------------------------------------
		// 画面更新
		//
		// そのフレームの描画が全部終わってから
		// ScreenFlipする
		//--------------------------------------------------
		ScreenFlip();
	}


	//--------------------------------------------------
	// ゲーム終了処理
	//--------------------------------------------------
	AppRelease();


	//--------------------------------------------------
	// ImGui終了処理
	//--------------------------------------------------
#if IMGUI

	ImGui_ImplDXlib_Shutdown();

	ImGui::DestroyContext();

#endif


	//--------------------------------------------------
	// DxLib終了
	//--------------------------------------------------
	DxLib_End();


	//--------------------------------------------------
	// メモリリークチェック
	//--------------------------------------------------
#ifdef _DEBUG

	_CrtSetReportMode(
		_CRT_WARN,
		_CRTDBG_MODE_DEBUG);

	_CrtDumpMemoryLeaks();

#endif


	return 0;
}