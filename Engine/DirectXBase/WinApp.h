#pragma once
#include<Windows.h>
// WindouwsAPI
class WinApp
{
public: // 静的メンバ関数
	static LRESULT WindowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
public: // メンバ関数
	// 初期化
	void Initialize();
	// 更新
	void Update();

	// 終了
	void Finalize();

	// getter
	HWND GetHwnd() const { return hwnd; }

	HINSTANCE GetHInstance() const { return w.hInstance; }

	// メッセージの処理
	bool ProcessMessage();

public: // 定数
	// ウィンドウ横幅
	static const int window_width = 1280;
	// ウィンドウ縦幅
	static const int window_height = 720;

private:
	// ウィンドウハンドル
	HWND hwnd = nullptr;

	// ウィンドウクラスの設定
	WNDCLASSEX w{};
};

