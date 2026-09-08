#include "PlayScene.h"
#include <DxLib.h>
#include "../Library/Input.h"
#include "../Library/SceneManager.h"
#include "../Library/GuiButton.h"

/// <summary>
/// 
/// </summary>

PlayScene::PlayScene()
    : bgImage(-1)
    , currentSelect(0)
{
    //背景画像のロード
    bgImage = LoadGraph("data/BG_Ti_01.png");

    //ボタンの配置情報設定 
    int bx = 50;  //ボタンのX座標
    int by = 200; //Y座標
    int bw = 400; //ボタンの幅
    int bh = 90;  //ボタンの高さ
    int bi = 100; //ボタン間隔

	//ボタン画像のロード
    int btnImg_bPl00 = LoadGraph("data/UI_Pl_00.png");
    int btnImg_bPl01 = LoadGraph("data/UI_Pl_01.png");
    int btnImg_bPl02 = LoadGraph("data/UI_Pl_02.png");
	int btnImg_bPl03 = LoadGraph("data/UI_Pl_03.png");

	//ボタン0の生成
    auto bPl00 = new GuiButton(0, 0, 100, 100, "Back");
    bPl00->SetImage(btnImg_bPl00);
    bPl00->onClick = []() {SceneManager::ChangeScene("RETURN");};
    buttons.push_back(bPl00);

	//ボタン1の生成
    auto bPl01 = new GuiButton(bx, by + bi, bw, bh, "01");
    bPl01->SetImage(btnImg_bPl01);
    bPl01->onClick = []() {};
	buttons.push_back(bPl01);

	//ボタン2の生成
    auto bPl02 = new GuiButton(bx, by + bi * 2, bw, bh, "02");
    bPl02->SetImage(btnImg_bPl02);
	bPl02->onClick = []() {};

	//ボタン3の生成
    auto bPl03 = new GuiButton(bx, by + bi * 3, bw, bh, "03");
    bPl03->SetImage(btnImg_bPl03);
    bPl03->onClick = []() {};
	buttons.push_back(bPl03);
}

PlayScene::~PlayScene()
{
    //背景画像の削除
    DeleteGraph(bgImage);
}

void PlayScene::Update()
{
    if (Input::IsKeyDown(KEY_INPUT_ESCAPE)) {
        SceneManager::ChangeScene("TITLE");
        return;
    }

    //上下キー選択
    if (Input::IsKeyDown(KEY_INPUT_DOWN)) currentSelect = (currentSelect + 1) % 2;
    if (Input::IsKeyDown(KEY_INPUT_UP))   currentSelect = (currentSelect - 1 + 2) % 2;
}

void PlayScene::Draw()
{
    //背景の描画
    if (bgImage != -1) {
        DrawExtendGraph(0, 0, 1280, 720, bgImage, FALSE);
    }

	//ボタンの描画
    for (auto b : buttons) b->Draw();
}