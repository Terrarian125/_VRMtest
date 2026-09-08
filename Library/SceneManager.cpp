#include "SceneManager.h"
#include "SceneBase.h"
#include "../Source/SceneFactory.h"
#include <DxLib.h>
#include <vector> //履歴管理用

namespace
{
	std::string* currentName; // 現在のシーンの名称
	std::string* nextName;    // 次のシーンの名称
	std::vector<std::string>* sceneHistory; //シーン名の履歴スタック
	SceneBase* currentScene; // 今のシーンのインスタンスを保持
	SceneFactory* factory;   // シーン切り替え用のFactoryのポインター
	bool exitRequest;
};

void SceneManager::Init()
{
	currentName = new std::string;
	nextName = new std::string;
	sceneHistory = new std::vector<std::string>();
	*nextName = "";
	*currentName = "";

	factory = new SceneFactory();
	currentScene = factory->CreateFirst();
	exitRequest = false;
}

void SceneManager::Update()
{
	bool isReturnTransition = false; // RETURNによる遷移かどうかを判定するフラグ

	//もし"RETURN"が指定されたら、履歴から1つ前のシーン名を取り出す
	if (*nextName == "RETURN")
	{
		if (!sceneHistory->empty())
		{
			*nextName = sceneHistory->back(); // 履歴の末尾（一番最近のシーン）を取得
			sceneHistory->pop_back();        // 取得したシーンを履歴から削除
			isReturnTransition = true;       // 戻り処理フラグを立てる
		}
		else
		{
			//履歴が空なのにRETURNされた場合は、エラーにならないよう現在のシーンのままにする
			*nextName = *currentName;
		}
	}

	if (*nextName != *currentName)
	{ //シーン切り替えの指定があったので
		if (currentScene != nullptr)
		{ //今までのシーンを解放
			ObjectManager::DeleteAllGameObject();
			delete currentScene;
			currentScene = nullptr;
		}

		//通常のシーン遷移であり、かつ現在のシーンが空でなければ履歴に保存する
		//（RETURNで戻るときは、履歴に新しく追加してはいけない）
		if (!isReturnTransition && *currentName != "")
		{
			sceneHistory->push_back(*currentName);
		}

		currentScene = factory->Create(*nextName); // 次のシーンを作成
		*currentName = *nextName;
	}
	if (currentScene != nullptr)
		currentScene->Update();
}

void SceneManager::Draw()
{
	if (currentScene != nullptr)
		currentScene->Draw();
}

void SceneManager::Release()
{
	if (currentScene != nullptr)
	{
		delete currentScene;
		currentScene = nullptr;
	}
	if (factory != nullptr) {
		delete factory;
		factory = nullptr;
	}
	delete currentName;
	delete nextName;

	//メモリ解放
	delete sceneHistory;
}

void SceneManager::ChangeScene(const std::string& sceneName)
{
	*nextName = sceneName;
}

void SceneManager::Exit()
{
	exitRequest = true;
}

bool SceneManager::IsExit()
{
	return exitRequest;
}