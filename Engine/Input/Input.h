#pragma once
#include <windows.h>
#include <wrl.h>
#define DIRECTINPUT_VERSION 0x0800 //DirectInputのバージョン指定
#include <dinput.h>
#include "WinApp.h"

// 入力
class Input
{
public:
	// namespaceの省略
	template <class T>using ComPtr = Microsoft::WRL::ComPtr<T>;

public: // メンバ関数
	// 初期化
	void Initialize(WinApp* winApp);

	// 更新
	void Update();

	/// <summary>
	/// キーの押したかをチェック(長押し)
	/// </summary>
	/// <param name="keyNumber">キー番号(DIK_0 等)</param>
	/// <returns>押されているか</returns>
	bool PushKey(BYTE keyNumber);

	/// <summary>
	/// キーのトリガーをチェック(押した瞬間)
	/// </summary>
	/// <param name="keyNumber">キー番号(DIK_0 等)</param>
	/// <returns>トリガーか</returns>
	bool TriggerKey(BYTE keyNumber);

	/// <summary>
	/// キーのトリガーをチェック(離した瞬間)
	/// </summary>
	/// <param name="keyNumber">キー番号(DIK_0 等)</param>
	/// <returns>トリガーか</returns>
	bool ReleasedKey(BYTE keyNumber);

private:// 静的メンバ変数

	// DirectInputのインスタンス
	ComPtr<IDirectInput8> directInput = nullptr;

	// キーボードのデバイス
	ComPtr<IDirectInputDevice8> keyboard = nullptr;

	// 全キーの状態
	BYTE key[256] = {};

	// 前回の全キーの状態
	BYTE keyPre[256] = {};

	// WindowsAPI
	WinApp* winApp_ = nullptr;
};

