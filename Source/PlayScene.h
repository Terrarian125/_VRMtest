#pragma once
#include "../Library/SceneBase.h"
#include "../Library/GuiButton.h"
#include <vector>
#include <string>

class PlayScene : public SceneBase
{
public:
    PlayScene();
    ~PlayScene();

    void Update() override;
    void Draw() override;

private:
    int bgImage;          // 背景画像のグラフィックハンドル
    std::vector<GuiButton*> buttons;
    int currentSelect;    // 現在選択されているボタンのインデックス (0~3)
};