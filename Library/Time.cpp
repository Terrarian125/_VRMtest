#include "Time.h"
#include <Windows.h>

namespace
{
	// 高精度タイマー
	LARGE_INTEGER frequency;
	LARGE_INTEGER currentCounter;

	// 前回のカウンタ
	LARGE_INTEGER lastCounter;

	// DeltaTime
	float deltaTime = 0.0f;

	// FPS
	float currentFPS = 0.0f;

	// 目標FPS
	int targetFPS = 60;

	// 1フレームの目標時間（秒）
	double targetFrameTime = 1.0 / 60.0;

	// 初期化済みか
	bool initialized = false;


	// 高精度タイマーから現在時刻を秒で取得
	double GetHighResolutionTime()
	{
		LARGE_INTEGER counter;
		QueryPerformanceCounter(&counter);

		return static_cast<double>(counter.QuadPart)
			/ static_cast<double>(frequency.QuadPart);
	}
}


//==================================================
// 初期化
//==================================================
void Time::Init()
{
	QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&currentCounter);

	lastCounter = currentCounter;

	deltaTime = 0.0f;
	currentFPS = 0.0f;

	initialized = true;
}


//==================================================
// 毎フレーム更新
//==================================================
void Time::Refresh()
{
	if (!initialized)
	{
		return;
	}

	//--------------------------------------------------
	// FPS制限
	//
	// 前回のフレーム計測から、目標フレーム時間に
	// 達するまで待つ
	//--------------------------------------------------
	if (targetFPS > 0)
	{
		double frameStartTime = GetHighResolutionTime();

		// 前回Refreshからの経過時間
		double elapsed =
			frameStartTime -
			(
				static_cast<double>(lastCounter.QuadPart)
				/ static_cast<double>(frequency.QuadPart)
				);

		if (elapsed < targetFrameTime)
		{
			double waitTime =
				targetFrameTime - elapsed;

			// ある程度長い場合はSleep
			if (waitTime > 0.002)
			{
				DWORD sleepTime =
					static_cast<DWORD>(
						(waitTime - 0.001) * 1000.0
						);

				if (sleepTime > 0)
				{
					Sleep(sleepTime);
				}
			}

			// 残りを高精度タイマーで待つ
			do
			{
				frameStartTime = GetHighResolutionTime();

				elapsed =
					frameStartTime -
					(
						static_cast<double>(lastCounter.QuadPart)
						/ static_cast<double>(frequency.QuadPart)
						);

			} while (elapsed < targetFrameTime);
		}
	}


	//--------------------------------------------------
	// 現在のカウンタ取得
	//--------------------------------------------------
	LARGE_INTEGER newCounter;
	QueryPerformanceCounter(&newCounter);


	//--------------------------------------------------
	// DeltaTime計算
	//--------------------------------------------------
	double rawDeltaTime =
		static_cast<double>(
			newCounter.QuadPart -
			lastCounter.QuadPart
			)
		/ static_cast<double>(frequency.QuadPart);


	//--------------------------------------------------
	// DeltaTime異常値対策
	//
	// デバッグ停止やウィンドウ切り替えなどで
	// 大きな時間が入ることを防ぐ
	//--------------------------------------------------
	if (rawDeltaTime < 0.0)
	{
		deltaTime = 0.0f;
	}
	else if (rawDeltaTime > 0.1)
	{
		deltaTime = 0.1f;
	}
	else
	{
		deltaTime =
			static_cast<float>(rawDeltaTime);
	}


	//--------------------------------------------------
	// FPS計算
	//--------------------------------------------------
	if (rawDeltaTime > 0.0)
	{
		currentFPS =
			static_cast<float>(
				1.0 / rawDeltaTime
				);
	}


	//--------------------------------------------------
	// 次のフレーム用
	//--------------------------------------------------
	lastCounter = newCounter;
}


//==================================================
// DeltaTime取得
//==================================================
float Time::DeltaTime()
{
	return deltaTime;
}


//==================================================
// FPS取得
//==================================================
float Time::FPS()
{
	return currentFPS;
}


//==================================================
// 目標FPS設定
//==================================================
void Time::SetTargetFPS(int fps)
{
	targetFPS = fps;

	if (targetFPS > 0)
	{
		targetFrameTime =
			1.0 /
			static_cast<double>(targetFPS);
	}
	else
	{
		targetFrameTime = 0.0;
	}
}


//==================================================
// 目標FPS取得
//==================================================
int Time::GetTargetFPS()
{
	return targetFPS;
}