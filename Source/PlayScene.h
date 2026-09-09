#pragma once

#include "../Library/SceneBase.h"
#include "../Library/GuiButton.h"
#include "../Library/VRMModel.h"

#include <vector>
#include <string>


//==================================================
//PlayScene
//==================================================
class PlayScene : public SceneBase

{
public:

	PlayScene();

	~PlayScene();

	void Update() override;

	void Draw() override;


private:

	//==================================================
	//背景画像
	//==================================================
	int bgImage;


	//==================================================
	//ボタン
	//==================================================
	std::vector<GuiButton*> buttons;


	//==================================================
	//現在選択されているボタン
	//==================================================
	int currentSelect;


	//==================================================
	//VRMプレイヤーモデル
	//==================================================
	VRMModel playerModel;


	//==================================================
	//VRMカメラ
	//==================================================
	VECTOR cameraPosition;

	VECTOR cameraTarget;

	VECTOR cameraUp;


	//==================================================
	//VRMモデル位置
	//==================================================
	VECTOR playerPosition;

	float playerScale;

	float playerRotationY;
};