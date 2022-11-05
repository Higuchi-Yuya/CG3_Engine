#include "WinApp.h"

#include <tchar.h>
#pragma comment(lib,"winmm.lib")

void WinApp::Initialize()
{
	// ウィンドウクラスの設定
	w.cbSize = sizeof(WNDCLASSEX);
	w.lpfnWndProc = (WNDPROC)WindowProcedure;	//ウィンドウプロシージャを設定
	w.lpszClassName = _T("DX12Sample");			//ウィンドウクラス名
	w.hInstance = GetModuleHandle(nullptr);		//ウィンドウハンドル
	w.hCursor = LoadCursor(NULL, IDC_ARROW);	//カーソル指定

	//ウィンドウクラスをOSに登録する
	RegisterClassEx(&w);
	//ウィンドウサイズ{X座標　Y座標　横幅　縦幅}
	RECT wrc = { 0,0,window_width,window_height };
	//関数を使ってウィンドウのサイズを自動で補正する
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

	//ウィンドウオブジェクトの生成
	hwnd = CreateWindow(w.lpszClassName,//クラス名指定
		_T("LE2B_18_ヒグチ_ユウヤ_CG2"),     //タイトルバーの文字
		WS_OVERLAPPEDWINDOW,			     //タイトルバーと境界線があるウィンドウ
		CW_USEDEFAULT,					     //表示x座標はOSにお任せ
		CW_USEDEFAULT,					     //表示y座標はOSにお任せ
		wrc.right - wrc.left,			     //ウィンドウ幅
		wrc.bottom - wrc.top,			     //ウィンドウ高
		nullptr,						     //親ウィンドウハンドル
		nullptr,						     //メニューハンドル
		w.hInstance,					     //呼び出しアプリケーションハンドル
		nullptr);						     //追加パラメーター(オプション)

	//ウィンドウ表示
	ShowWindow(hwnd, SW_SHOW);

	// システムタイマーの分解能を上げる
	timeBeginPeriod(1);
}

void WinApp::Update()
{
}

void WinApp::Finalize()
{
	// ウィンドウクラスを登録解除
	UnregisterClass(w.lpszClassName, w.hInstance);
}

bool WinApp::ProcessMessage()
{
	MSG msg{};

	if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	if (msg.message == WM_QUIT)
	{
		return true;
	}

	return false;
}

LRESULT WinApp::WindowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	//ウィンドウが破壊されたら呼ばれる
	switch (msg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);//OSに対して「このアプリはもう終わる」と伝える
		return 0;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);
}
	
