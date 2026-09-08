#include "TitleScene.h"
#include <DxLib.h>
#include "../Library/Input.h"
#include "../Library/SceneManager.h"
#include "../Library/GuiButton.h"
#include "../Library/GameSetting.h"
#include "SettingPanel.h"

TitleScene::TitleScene()
    : bgImage(-1)
    , mySettingPanel(nullptr)
    , isExitDialogVisible(false)
    , currentSelect(0)
{
    //背景画像のロード
    bgImage = LoadGraph("data/BG_Ti_00.png");

    //設定パネルの生成
    mySettingPanel = new SettingPanel();
    mySettingPanel->SetVisible(false);

    //メインメニューのボタン配置設定
    int bx = 50;  //ボタンのX座標
    int by = 200; //Y座標
    int bw = 400; //ボタンの幅
    int bh = 90;  //ボタンの高さ
    int bi = 100; //ボタン間隔

    //ボタン画像のロード
    int btnImg_bTi00 = LoadGraph("data/UI_Ti_00.png");
    int btnImg_bTi01 = LoadGraph("data/UI_Ti_01.png");
    int btnImg_bTi02 = LoadGraph("data/UI_Ti_02.png");
    int btnImg_bTi03 = LoadGraph("data/UI_Ti_03.png");

    //NewGameボタン
    auto bTi00 = new GuiButton(bx, by, bw, bh, "GoPlayScene");
    bTi00->SetImage(btnImg_bTi00);
    bTi00->onClick = []() {SceneManager::ChangeScene("PLAY");};
    buttons.push_back(bTi00);

    //Continueボタン
    auto bTi01 = new GuiButton(bx, by + bi, bw, bh, "None");
    bTi01->SetImage(btnImg_bTi01);
    bTi01->onClick = []() {};
    buttons.push_back(bTi01);

    //Optionボタン (設定)
    auto bTi02 = new GuiButton(bx, by + bi * 2, bw, bh, "Option");
    bTi02->SetImage(btnImg_bTi02);
    bTi02->onClick = [this]() { this->mySettingPanel->SetVisible(true); };
    buttons.push_back(bTi02);

    //Exitボタン (終了確認ダイアログを開く)
    auto bTi03 = new GuiButton(bx, by + bi * 3, bw, bh, "Exit");
    bTi03->SetImage(btnImg_bTi03);
    bTi03->onClick = [this]() {
        this->isExitDialogVisible = true;
        for (auto b : this->exitButtons) b->SetActive(true);
        this->currentSelect = 1; //初期選択を「いいえ」(1) に設定して誤終了を防ぐ
        };
    buttons.push_back(bTi03);

    //終了確認ダイアログ用のボタン配置
    int cx = 1280 / 2;
    int cy = 720 / 2;

    //終了確認のYesボタン
    auto bYes = new GuiButton(cx - 160, cy + 50, 150, 50, "Yes");
    bYes->onClick = []() { SceneManager::Exit(); };
    bYes->SetActive(false);
    exitButtons.push_back(bYes);

    //終了確認のNoボタン
    auto bNo = new GuiButton(cx + 10, cy + 50, 150, 50, "No");
    bNo->onClick = [this]() {
        this->isExitDialogVisible = false;
        for (auto b : this->exitButtons) b->SetActive(false);
        };
    bNo->SetActive(false);
    exitButtons.push_back(bNo);
}

TitleScene::~TitleScene()
{
    //メモリ解放と画像削除
    DeleteGraph(bgImage);
    delete mySettingPanel;
}

void TitleScene::Update()
{
    //終了確認ダイアログ表示中の処理
    if (isExitDialogVisible) {
        for (auto b : exitButtons) b->Update();

        //左右キーでYES/NO切り替え
        if (Input::IsKeyDown(KEY_INPUT_LEFT))  currentSelect = 0;
        if (Input::IsKeyDown(KEY_INPUT_RIGHT)) currentSelect = 1;

        for (int i = 0; i < (int)exitButtons.size(); i++) {
            exitButtons[i]->SetFocus(i == currentSelect);
            if (exitButtons[i]->IsMouseOver()) currentSelect = i;
        }
        return; //他の処理を遮断
    }

    //設定パネル表示中の処理
    if (mySettingPanel->IsVisible()) {
        mySettingPanel->Update();
        //GameSetting::Apply(Op_Music); //必要に応じてBGMハンドル等を渡してください
        return; //他の処理を遮断
    }

    //通常時の処理
    for (auto b : buttons) {
        b->Update();
    }

    //上下キーでメインメニューの選択切り替え
    if (Input::IsKeyDown(KEY_INPUT_DOWN)) {
        currentSelect = (currentSelect + 1) % buttons.size();
    }
    if (Input::IsKeyDown(KEY_INPUT_UP)) {
        currentSelect = (currentSelect - 1 + (int)buttons.size()) % (int)buttons.size();
    }

    for (int i = 0; i < (int)buttons.size(); i++) {
        buttons[i]->SetFocus(i == currentSelect);
        if (buttons[i]->IsMouseOver()) currentSelect = i;
    }
}

void TitleScene::Draw()
{
    //背景描画
    DrawExtendGraph(0, 0, 1280, 720, bgImage, FALSE);

    //メインメニューボタン描画
    for (auto b : buttons) b->Draw();

    //設定パネル描画
    if (mySettingPanel->IsVisible()) {
        mySettingPanel->Draw();
    }

    //終了確認ダイアログの最前面描画
    if (isExitDialogVisible) {
        //背景を暗くする (アスペクト比 1280x720)
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 180);
        DrawBox(0, 0, 1280, 720, GetColor(0, 0, 0), TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

        //ダイアログ外枠
        int vx = 440, vy = 260, vw = 400, vh = 200;
        DrawBox(vx, vy, vx + vw, vy + vh, GetColor(20, 20, 40), TRUE);
        DrawBox(vx, vy, vx + vw, vy + vh, GetColor(255, 255, 255), FALSE);

        //メッセージ描画
        DrawFormatString(vx + 95, vy + 50, GetColor(255, 255, 255), "really?");

        //終了確認のYes/Noボタン描画
        for (auto b : exitButtons) b->Draw();
    }
}