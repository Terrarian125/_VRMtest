#pragma once
#include "../Library/SceneBase.h"
#include "../Library/GuiButton.h"
#include <vector>

// クラスの前方宣言
class SettingPanel;

/// <summary>
/// テストシーンクラス
/// </summary>
class TitleScene : public SceneBase
{
public:
    TitleScene();
    ~TitleScene();

    void Update() override;
    void Draw() override;

private:
    int bgImage;                     // 背景画像のグラフィックハンドル
    std::vector<GuiButton*> buttons; // メインメニューのボタン配列

    // 設定パネル関連
    SettingPanel* mySettingPanel;    // 設定パネル

    // 終了確認ダイアログ関連
    bool isExitDialogVisible;        // ダイアログの表示フラグ
    int currentSelect;               // キーボード選択用インデックス
    std::vector<GuiButton*> exitButtons; // ダイアログ内のボタン配列（はい／いいえ）
};