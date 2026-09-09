#include "PlayScene.h"

#include <DxLib.h>

#include "../Library/Input.h"
#include "../Library/SceneManager.h"
#include "../Library/GuiButton.h"


//==================================================
//PlayScene
//==================================================
PlayScene::PlayScene()

	: bgImage(-1)
	, currentSelect(0)
	, playerModel()
	, cameraPosition(
		VGet(
			0.0f,
			1.0f,
			-4.0f))
	, cameraTarget(
		VGet(
			0.0f,
			1.0f,
			0.0f))
	, cameraUp(
		VGet(
			0.0f,
			1.0f,
			0.0f))
	, playerPosition(
		VGet(
			0.0f,
			0.0f,
			0.0f))
	, playerScale(1.0f)
	, playerRotationY(0.0f)

{
	//==================================================
	//背景画像のロード
	//==================================================
	bgImage =
		LoadGraph(
			"data/BG_Ti_01.png");


	//==================================================
	//ボタンの配置情報設定
	//==================================================
	int bx = 25;

	int by = 400;

	int bw = 150;

	int bh = 50;

	int bi = 60;


	//==================================================
	//ボタン画像のロード
	//==================================================
	int btnImg_bPl00 =
		LoadGraph(
			"data/UI_Pl_00.png");

	int btnImg_bPl01 =
		LoadGraph(
			"data/UI_Pl_01.png");

	int btnImg_bPl02 =
		LoadGraph(
			"data/UI_Pl_02.png");

	int btnImg_bPl03 =
		LoadGraph(
			"data/UI_Pl_03.png");


	//==================================================
	//ボタン0
	//==================================================
	auto bPl00 =
		new GuiButton(
			0,
			0,
			100,
			100,
			"Back");

	bPl00->SetImage(
		btnImg_bPl00);

	bPl00->onClick =
		[]()
		{
			SceneManager::ChangeScene(
				"RETURN");
		};

	buttons.push_back(
		bPl00);


	//==================================================
	//ボタン1
	//==================================================
	auto bPl01 =
		new GuiButton(
			bx,
			by + bi,
			bw,
			bh,
			"01");

	bPl01->SetImage(
		btnImg_bPl01);

	bPl01->onClick =
		[]() {};

	buttons.push_back(
		bPl01);


	//==================================================
	//ボタン2
	//==================================================
	auto bPl02 =
		new GuiButton(
			bx,
			by + bi * 2,
			bw,
			bh,
			"02");

	bPl02->SetImage(
		btnImg_bPl02);

	bPl02->onClick =
		[]() {};

	buttons.push_back(
		bPl02);


	//==================================================
	//ボタン3
	//==================================================
	auto bPl03 =
		new GuiButton(
			bx,
			by + bi * 3,
			bw,
			bh,
			"03");

	bPl03->SetImage(
		btnImg_bPl03);

	bPl03->onClick =
		[]() {};

	buttons.push_back(
		bPl03);


	//==================================================
	//VRM読み込み
	//==================================================
	if (!playerModel.Load(
		"data/VRM/Player.vrm"))
	{
		printfDx(
			"[VRM] Load Failed\n");

		printfDx(
			"[VRM] %s\n",
			playerModel.GetLastError().c_str());
	}
	else
	{
		printfDx(
			"[VRM] Load Success\n");

		printfDx(
			"[VRM] Model : %s\n",
			playerModel.GetModelName().c_str());

		printfDx(
			"[VRM] Author : %s\n",
			playerModel.GetAuthorName().c_str());

		printfDx(
			"[VRM] Nodes : %d\n",
			static_cast<int>(
				playerModel.GetNodeCount()));

		printfDx(
			"[VRM] Meshes : %d\n",
			static_cast<int>(
				playerModel.GetMeshCount()));

		printfDx(
			"[VRM] Humanoid Bones : %d\n",
			static_cast<int>(
				playerModel.GetHumanoidBoneCount()));


		switch (
			playerModel.GetVersion())
		{
		case VRMModel::Version::VRM_0_X:

			printfDx(
				"[VRM] Version : VRM 0.x\n");

			break;


		case VRMModel::Version::VRM_1_0:

			printfDx(
				"[VRM] Version : VRM 1.0\n");

			break;


		default:

			printfDx(
				"[VRM] Version : Unknown\n");

			break;
		}
	}


	//==================================================
	//3Dカメラ設定
	//==================================================
	SetCameraNearFar(
		0.01f,
		100.0f);

	SetupCamera_Perspective(
		60.0f *
		DX_PI_F /
		180.0f);

	SetCameraPositionAndTargetAndUpVec(
		cameraPosition,
		cameraTarget,
		cameraUp);


	//==================================================
	//ライティング
	//
	//現段階ではVRMのPBRマテリアルを
	//まだ実装していないためOFFにする。
	//==================================================
	SetUseLighting(
		FALSE);
}


//==================================================
//デストラクタ
//==================================================
PlayScene::~PlayScene()
{
	//背景画像の削除
	DeleteGraph(
		bgImage);

	//ボタンの解放は
	//GuiButton自身の管理に任せる

	//VRMModelは自身のデストラクタで
	//VRMデータを解放する
}


//==================================================
//Update
//==================================================
void PlayScene::Update()
{
	//==================================================
	//ESCキー
	//==================================================
	if (Input::IsKeyDown(
		KEY_INPUT_ESCAPE))
	{
		SceneManager::ChangeScene(
			"TITLE");

		return;
	}


	//==================================================
	//上下キー選択
	//==================================================
	if (Input::IsKeyDown(
		KEY_INPUT_DOWN))
	{
		currentSelect =
			(currentSelect + 1) % 2;
	}


	if (Input::IsKeyDown(
		KEY_INPUT_UP))
	{
		currentSelect =
			(currentSelect - 1 + 2) % 2;
	}
}


//==================================================
//Draw
//==================================================
void PlayScene::Draw()
{
	//==================================================
	//2D背景
	//==================================================
	if (bgImage != -1)
	{
		DrawExtendGraph(
			0,
			0,
			1280,
			720,
			bgImage,
			FALSE);
	}


	//==================================================
	//3Dカメラ
	//==================================================
	SetCameraPositionAndTargetAndUpVec(
		cameraPosition,
		cameraTarget,
		cameraUp);


	//==================================================
	//VRM描画
	//==================================================
	if (playerModel.IsLoaded())
	{
		playerModel.Draw(
			playerScale,
			playerPosition,
			playerRotationY);
	}


	//==================================================
	//2D UI
	//==================================================
	for (auto b : buttons)
	{
		b->Draw();
	}
}