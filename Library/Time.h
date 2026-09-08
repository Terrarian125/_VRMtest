#pragma once

namespace Time
{
	// 初期化
	void Init();

	// 毎フレーム呼び出す
	void Refresh();

	// 前フレームからの経過時間（秒）
	float DeltaTime();

	// 実測FPS
	float FPS();

	// 目標FPSを設定
	// 0以下でFPS制限なし
	void SetTargetFPS(int fps);

	// 現在の目標FPSを取得
	int GetTargetFPS();
}