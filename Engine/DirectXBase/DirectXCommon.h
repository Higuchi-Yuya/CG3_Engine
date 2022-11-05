#pragma once
#include <d3d12.h>
#include<dxgi1_6.h>
#include <wrl.h>
#include <vector>
#include <chrono>
#include "WinApp.h"

class DirectXCommon
{
public:// メンバ関数
	// 初期化
	void Initalize(WinApp* winApp);

	// 描画前処理
	void PreDraw();

	// 描画後処理
	void PostDraw();

	/// <summary>
	/// 深度バッファのクリア
	/// </summary>
	void ClearDepthBuffer();

	// デバイス取得
	ID3D12Device* GetDevice() const { return device.Get();}

	// コマンドリスト取得
	ID3D12GraphicsCommandList* GetCommandList()const { return commandList.Get(); }
	
private:
	// デバイスの初期化
	void InitalizeDevice();

	// コマンド関連の初期化
	void InitialzeCommand();

	// スワップチェーンの初期化
	void InitializeSwapchain(WinApp* winApp);

	// レンダーターゲットビューの初期化
	void InitializeRenderTargetView();

	// 深度バッファの初期化
	void InitializeDepthBuffer();

	// フェンスの初期化
	void InitializeFence();

	// FPS固定初期化
	void InitializeFixFPS();

	// FPS固定更新
	void UpdateFixFPS();

private:
	// WindowsAPI
	WinApp* winApp = nullptr;
	// DirectX12デバイス
	Microsoft::WRL::ComPtr<ID3D12Device> device;
	// DXGIファクトリ
	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory;

	// コマンド関連
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator = nullptr;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList = nullptr;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue = nullptr;

	// スワップチェーン関連
	Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain = nullptr;
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};

	// レンダーターゲットビュー関連
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvHeap = nullptr;
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc{};

	// バックバッファ
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> backBuffers;

	// 深度バッファ関連
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvHeap = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> depthBuff;

	// リソースバリア
	D3D12_RESOURCE_BARRIER barrierDesc{};

	// フェンス
	Microsoft::WRL::ComPtr<ID3D12Fence> fence = nullptr;
	UINT64 fenceVal = 0;

	// 結果確認
	HRESULT result;

	// 記録時間(FPS固定用)
	std::chrono::steady_clock::time_point reference_;
};

