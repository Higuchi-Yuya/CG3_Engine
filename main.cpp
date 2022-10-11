#pragma region 読み込むヘッダー



#pragma comment(lib,"d3dcompiler.lib")


#include<iostream>

#include<dxgi1_6.h>
#include<cassert>

#pragma comment(lib,"dxgi.lib")

#include <vector>
#include <wrl.h>

#include <math.h>
#include "Line.h"
#include "Mesh.h"

#include "Render_basic.h"
#include "Input.h"
#include "WinApp.h"
using namespace Microsoft::WRL;
using namespace std;


#pragma endregion
const float PI = 3.141592f;

#pragma region おまじない
// @brief コンソール画面にフォーマット付き文字列の表示
// @param format フォーマット(%dとか%fとかの)
// @param 可変長引数
// @remarks この関数はデバック用です。デバッグ時にしか動作しません
void DebugOutputFormatString(const char* format, ...) {
#ifdef _DEBUG
	va_list valist;
	va_start(valist, format);
	vprintf(format, valist);
	va_end(valist);
#endif
}


#pragma endregion

int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR,  _In_ int) {

#pragma region WindowsAPI初期化処理
	// ポインタ
	WinApp* winApp = nullptr;

	// WindouwsAPIの初期化
	winApp = new WinApp();
	winApp->Initialize();

#pragma endregion

#pragma region DirectX初期化処理

	////////////////////////////////////////////////////
	//-------------DirectX12初期化処理ここから-----------//
	//////////////////////////////////////////////////

#ifdef _DEBUG
//デバッグレイヤーをオンに
	ComPtr<ID3D12Debug> debugController;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
		debugController->EnableDebugLayer();
	}
#endif

	HRESULT result;
	ComPtr<ID3D12Device> device = nullptr;
	ComPtr<IDXGIFactory6> dxgiFactory = nullptr;
	ComPtr<IDXGISwapChain4> swapChain = nullptr;
	ComPtr<ID3D12CommandAllocator> commandAllocator = nullptr;
	ComPtr<ID3D12GraphicsCommandList> commandList = nullptr;
	ComPtr<ID3D12CommandQueue> commandQueue = nullptr;
	ComPtr<ID3D12DescriptorHeap> rtvHeap = nullptr;

	//DXGIファクトリーの生成
	result = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));
	assert(SUCCEEDED(result));

	//アダプターの列挙用
	std::vector<ComPtr<IDXGIAdapter4>>adapters;
	//ここに特定の名前を持つアダプターオブジェクトが入る
	IDXGIAdapter4* tmpAdapter = nullptr;

	//パフォーマンスが高い物から順に、全てのアダプターを列挙する
	for (UINT i = 0; dxgiFactory->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&tmpAdapter)) != DXGI_ERROR_NOT_FOUND; i++) {
		//動的配列に追加する
		adapters.push_back(tmpAdapter);
	}
	//妥当なアダプタを選別する
	for (size_t i = 0; i < adapters.size(); i++) {
		DXGI_ADAPTER_DESC3 adapterDesc;
		//アダプターの情報を取得する
		adapters[i]->GetDesc3(&adapterDesc);

		//ソフトウェアデバイスを回避
		if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)) {
			//デバイスを採用してループを抜ける
			tmpAdapter = adapters[i].Get();
			break;
		}
	}

	//対応レベルの配列
	D3D_FEATURE_LEVEL levels[] = {
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
	};

	D3D_FEATURE_LEVEL featureLevel;

	for (size_t i = 0; i < _countof(levels); i++) {
		//採用したアダプターでデバイスを生成
		result = D3D12CreateDevice(tmpAdapter, levels[i], IID_PPV_ARGS(&device));
		if (result == S_OK) {
			//デバイスを生成できた時点でループを抜ける
			featureLevel = levels[i];
			break;
		}
	}

	//コマンドアローケータを生成
	result = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));
	assert(SUCCEEDED(result));

	//コマンドリストを生成
	result = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList));
	assert(SUCCEEDED(result));

	//コマンドキューの設定
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};

	//コマンドキューを生成
	result = device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&commandQueue));
	assert(SUCCEEDED(result));

	//スワップチェーンの設定
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
	swapChainDesc.Width = 1280;
	swapChainDesc.Height = 720;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;			//色情報の書式
	swapChainDesc.SampleDesc.Count = 1;							//マルチサンプルしない
	swapChainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER;			//バックバッファ用
	swapChainDesc.BufferCount = 2;								//バッファ数を2つに設定
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;	//フリップ後は破壊
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	
	//スワップチェーンの生成
	ComPtr<IDXGISwapChain1> swapChain1;

	result = dxgiFactory->CreateSwapChainForHwnd(
		commandQueue.Get(),
		winApp->GetHwnd(),
		&swapChainDesc, 
		nullptr, 
		nullptr, 
		&swapChain1);

	//生成下IDXGISwapChain1のオブジェクトをIDXGISwapChain4に変換する
	swapChain1.As(&swapChain);

	assert(SUCCEEDED(result));


	//デスクリプタヒープの設定
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc{};
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;			//レンダーターゲットビュー
	rtvHeapDesc.NumDescriptors = swapChainDesc.BufferCount;		//裏表の2つ

	//デスクリプタヒープの生成
	device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvHeap));

	//バックバッファ
	std::vector<ComPtr<ID3D12Resource>>backBuffers;
	backBuffers.resize(swapChainDesc.BufferCount);

	//スワップチェーンの全てのバッファについて処理する
	for (size_t i = 0; i < backBuffers.size(); i++) {
		//スワップチェーンからバッファを取得
		swapChain->GetBuffer((UINT)i, IID_PPV_ARGS(&backBuffers[i]));
		//デスクリプタヒープのハンドルを取得
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();
		//裏か表かでアドレスがずれる
		rtvHandle.ptr += i * device->GetDescriptorHandleIncrementSize(rtvHeapDesc.Type);
		//レンダーターゲットビューの設定
		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
		//シェーダーの計算結果をSRGBに変換して書き込む
		rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		//レンダーターゲットビューの生成
		device->CreateRenderTargetView(backBuffers[i].Get(), &rtvDesc, rtvHandle);
	}

	//フェンスの生成
	ComPtr<ID3D12Fence> fence = nullptr;
	UINT64 fenceVal = 0;

	result = device->CreateFence(fenceVal, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));

	// ポインタ
	Input* input = nullptr;

	// 入力の初期化
	input = new Input();
	input->Initialize(winApp);



	/////////////////////////////////////////////////////////
	//--------------DirectX12初期化処理　ここまで-------------//
	///////////////////////////////////////////////////////
#pragma endregion

	Render_basic::GetInstance()->Initialization(device.Get());
	
	Render_basic::GetInstance()->Initialization2(device.Get());

	//リソース設定
	D3D12_RESOURCE_DESC depthResoureDesc{};
	depthResoureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthResoureDesc.Width = WinApp::window_width;
	depthResoureDesc.Height = WinApp::window_height;
	depthResoureDesc.DepthOrArraySize = 1;
	depthResoureDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthResoureDesc.SampleDesc.Count = 1;
	depthResoureDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	//深度値用ヒーププロパティ
	D3D12_HEAP_PROPERTIES depthHeapProp{};
	depthHeapProp.Type = D3D12_HEAP_TYPE_DEFAULT;
	//深度値のクリア設定
	D3D12_CLEAR_VALUE depthClearValue{};
	depthClearValue.DepthStencil.Depth = 1.0f;
	depthClearValue.Format = DXGI_FORMAT_D32_FLOAT;

	//リソース生成
	ComPtr<ID3D12Resource> depthBuff = nullptr;
	result = device->CreateCommittedResource(
		&depthHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&depthResoureDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&depthClearValue,
		IID_PPV_ARGS(&depthBuff));

	//深度ビュー用デスクリプタヒープ作成
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc{};
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	ComPtr<ID3D12DescriptorHeap> dsvHeap = nullptr;
	result = device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&dsvHeap));


	//深度ビュー作成
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	device->CreateDepthStencilView(
		depthBuff.Get(),
		&dsvDesc,
		dsvHeap->GetCPUDescriptorHandleForHeapStart()
	);

	Mesh mesh[10];

	Line line[20];

	Mesh::Vertex vertices[] = {
		// x      y      z       u      v
		//前
		{{-5.0f, -5.0f, -5.0f}, {},{0.0f, 1.0f}}, // 左下
		{{-5.0f,  5.0f, -5.0f}, {},{0.0f, 0.0f}}, // 左上
		{{ 5.0f, -5.0f, -5.0f}, {},{1.0f, 1.0f}}, // 右下
		{{ 5.0f,  5.0f, -5.0f}, {},{1.0f, 0.0f}}, // 右上
	
		//後
		{{-5.0f, -5.0f,  5.0f}, {},{0.0f, 1.0f}}, // 左下
		{{-5.0f,  5.0f,  5.0f}, {},{0.0f, 0.0f}}, // 左上
		{{ 5.0f, -5.0f,  5.0f}, {},{1.0f, 1.0f}}, // 右下
		{{ 5.0f,  5.0f,  5.0f}, {}, {1.0f, 0.0f}}, // 右上

		//左
		{{-5.0f, -5.0f, -5.0f}, {},{0.0f, 1.0f}}, // 左下
		{{-5.0f, -5.0f,  5.0f}, {},{0.0f, 0.0f}}, // 左上
		{{-5.0f,  5.0f, -5.0f}, {},{1.0f, 1.0f}}, // 右下
		{{-5.0f,  5.0f,  5.0f}, {},{1.0f, 0.0f}}, // 右上

		//右
		{{ 5.0f, -5.0f, -5.0f}, {},{0.0f, 1.0f}}, // 左下
		{{ 5.0f, -5.0f,  5.0f}, {},{0.0f, 0.0f}}, // 左上
		{{ 5.0f,  5.0f, -5.0f}, {},{1.0f, 1.0f}}, // 右下
		{{ 5.0f,  5.0f,  5.0f}, {},{1.0f, 0.0f}}, // 右上

		//下
		{{-5.0f, -5.0f, -5.0f}, {},{0.0f, 1.0f}}, // 左下
		{{-5.0f, -5.0f,  5.0f}, {},{0.0f, 0.0f}}, // 左上
		{{ 5.0f, -5.0f, -5.0f}, {},{1.0f, 1.0f}}, // 右下
		{{ 5.0f, -5.0f,  5.0f}, {},{1.0f, 0.0f}}, // 右上

		//上
		{{-5.0f,  5.0f, -5.0f}, {},{0.0f, 1.0f}}, // 左下
		{{-5.0f,  5.0f,  5.0f}, {},{0.0f, 0.0f}}, // 左上
		{{ 5.0f,  5.0f, -5.0f}, {},{1.0f, 1.0f}}, // 右下
		{{ 5.0f,  5.0f,  5.0f}, {},{1.0f, 0.0f}}, // 右上
	};

	Mesh::Vertex line_vertices1[] = {
		{{2.0f,0.0f,18.0f}},
		{{2.0f,0.0f,-18.0f}}
	};
	Mesh::Vertex line_vertices2[] = {
		{{6.0f,0.0f,18.0f}},
		{{6.0f,0.0f,-18.0f}}
	};
	Mesh::Vertex line_vertices3[] = {
		{{10.0f,0.0f,18.0f}},
		{{10.0f,0.0f,-18.0f}},
	};
	Mesh::Vertex line_vertices4[] = {
		{{14.0f,0.0f,18.0f}},
		{{14.0f,0.0f,-18.0f}},
	};
	Mesh::Vertex line_vertices5[] = {
		{{18.0f,0.0f,18.0f}},
		{{18.0f,0.0f,-18.0f}},
	};
	Mesh::Vertex line_vertices6[] = {
		{{-2.0f,0.0f,18.0f}},
		{{-2.0f,0.0f,-18.0f}}
	};
	Mesh::Vertex line_vertices7[] = {
		{{-6.0f,0.0f,18.0f}},
		{{-6.0f,0.0f,-18.0f}},
	};
	Mesh::Vertex line_vertices8[] = {
		{{-10.0f,0.0f,18.0f}},
		{{-10.0f,0.0f,-18.0f}},
	};
	Mesh::Vertex line_vertices9[] = {
		{{-14.0f,0.0f,18.0f}},
		{{-14.0f,0.0f,-18.0f}},
	};
	Mesh::Vertex line_vertices10[] = {
	{{-18.0f,0.0f,18.0f}},
	{{-18.0f,0.0f,-18.0f}},
	};
	Mesh::Vertex line_vertices11[] = {
	{{18.0f,0.0f,2.0f}},
	{{-18.0f,0.0f,2.0f}}

	};
	Mesh::Vertex line_vertices12[] = {
		{{18.0f,0.0f,6.0f}},
		{{-18.0f,0.0f,6.0f}}
	};
	Mesh::Vertex line_vertices13[] = {
		{{18.0f,0.0f,10.0f}},
		{{-18.0f,0.0f,10.0f}},
	};
	Mesh::Vertex line_vertices14[] = {
		{{18.0f,0.0f,14.0f}},
		{{-18.0f,0.0f,14.0f}},
	};
	Mesh::Vertex line_vertices15[] = {
		{{18.0f,0.0f,18.0f}},
		{{-18.0f,0.0f,18.0f}},
	};
	Mesh::Vertex line_vertices16[] = {
		{{18.0f,0.0f,-2.0f}},
		{{-18.0f,0.0f,-2.0f}}
	};
	Mesh::Vertex line_vertices17[] = {
		{{18.0f,0.0f,-6.0f}},
		{{-18.0f,0.0f,-6.0f}},
	};
	Mesh::Vertex line_vertices18[] = {
		{{18.0f,0.0f,-10.0f}},
		{{-18.0f,0.0f,-10.0f}},
	};
	Mesh::Vertex line_vertices19[] = {
		{{18.0f,0.0f,-14.0f}},
		{{-18.0f,0.0f,-14.0f}},
	};
	Mesh::Vertex line_vertices20[] = {
	{{18.0f,0.0f,-18.0f}},
	{{-18.0f,0.0f,-18.0f}},
	};

	unsigned short indices[] = {
		//前
		0, 1, 2, // 三角形1つ目
		2, 1, 3, // 三角形2つ目

		//後
		6,5,4,
		7,5,6,

		////左
		8,9,10,
		10,9,11,

		//右
		12,14,13,
		13,14,15,

		//下
		18,17,16,
		19,17,18,

		//上
		20,21,22,
		22,21,23,


	};

	unsigned short indices2[] = {
		0,1
	};

	for (int i = 0; i < _countof(indices) / 3; i++)
	{// 三角形一つごとに計算していく
		// 三角形のインデックスを取り出して、一時的な変数に入れる
		unsigned short indices_1 = indices[i * 3 + 0];
		unsigned short indices_2 = indices[i * 3 + 1];
		unsigned short indices_3 = indices[i * 3 + 2];
		//三角形を構成する頂点座標をベクトルに代入
		XMVECTOR p0 = XMLoadFloat3(&vertices[indices_1].pos);
		XMVECTOR p1 = XMLoadFloat3(&vertices[indices_2].pos);
		XMVECTOR p2 = XMLoadFloat3(&vertices[indices_3].pos);
		// ｐ０ー＞ｐ１ベクトル、ｐ０ー＞ｐ２ベクトルを計算　（ベクトルの減算）
		XMVECTOR v1 = XMVectorSubtract(p1, p0);
		XMVECTOR v2 = XMVectorSubtract(p2, p0);

		//外積は両方から垂直なベクトル
		XMVECTOR normal = XMVector3Cross(v1, v2);

		//正規化（長さを１にする）
		normal = XMVector3Normalize(normal);

		//求めた法線を頂点データに代入
		XMStoreFloat3(&vertices[indices_1].normal, normal);
		XMStoreFloat3(&vertices[indices_2].normal, normal);
		XMStoreFloat3(&vertices[indices_3].normal, normal);
	}

	int vertices_count;
	int indices_count;

	int vertices_count2;
	int indices_count2;

	vertices_count = _countof(vertices);
	indices_count = _countof(indices);

	vertices_count2 = _countof(line_vertices1);
	indices_count2 = _countof(indices2);

	mesh[0].Mesh_Initialization(device.Get(), vertices, indices, _countof(vertices), indices_count);

#pragma region グリット線の設定
	line[0].Line_Initialize(device.Get(), line_vertices1, indices2, vertices_count2, indices_count2);
	vertices_count2 = _countof(line_vertices2);
	line[1].Line_Initialize(device.Get(), line_vertices2, indices2, vertices_count2, indices_count2);
	vertices_count2 = _countof(line_vertices3);
	line[2].Line_Initialize(device.Get(), line_vertices3, indices2, vertices_count2, indices_count2);
	vertices_count2 = _countof(line_vertices4);
	line[3].Line_Initialize(device.Get(), line_vertices4, indices2, vertices_count2, indices_count2);
	vertices_count2 = _countof(line_vertices5);
	line[4].Line_Initialize(device.Get(), line_vertices5, indices2, vertices_count2, indices_count2);
	vertices_count2 = _countof(line_vertices6);
	line[5].Line_Initialize(device.Get(), line_vertices6, indices2, vertices_count2, indices_count2);
	vertices_count2 = _countof(line_vertices7);
	line[6].Line_Initialize(device.Get(), line_vertices7, indices2, vertices_count2, indices_count2);
	vertices_count2 = _countof(line_vertices8);
	line[7].Line_Initialize(device.Get(), line_vertices8, indices2, vertices_count2, indices_count2);
	vertices_count2 = _countof(line_vertices9);
	line[8].Line_Initialize(device.Get(), line_vertices9, indices2, vertices_count2, indices_count2);
	vertices_count2 = _countof(line_vertices10);
	line[9].Line_Initialize(device.Get(), line_vertices10, indices2, vertices_count2, indices_count2);
	vertices_count2 = _countof(line_vertices11);
	line[10].Line_Initialize(device.Get(), line_vertices11, indices2, vertices_count2, indices_count2);
	vertices_count2 = _countof(line_vertices12);
	line[11].Line_Initialize(device.Get(), line_vertices12, indices2, vertices_count2, indices_count2);
	vertices_count2 = _countof(line_vertices13);
	line[12].Line_Initialize(device.Get(), line_vertices13, indices2, vertices_count2, indices_count2);
	vertices_count2 = _countof(line_vertices14);
	line[13].Line_Initialize(device.Get(), line_vertices14, indices2, vertices_count2, indices_count2);
	vertices_count2 = _countof(line_vertices15);
	line[14].Line_Initialize(device.Get(), line_vertices15, indices2, vertices_count2, indices_count2);
	vertices_count2 = _countof(line_vertices16);
	line[15].Line_Initialize(device.Get(), line_vertices16, indices2, vertices_count2, indices_count2);
	vertices_count2 = _countof(line_vertices17);
	line[16].Line_Initialize(device.Get(), line_vertices17, indices2, vertices_count2, indices_count2);
	vertices_count2 = _countof(line_vertices18);
	line[17].Line_Initialize(device.Get(), line_vertices18, indices2, vertices_count2, indices_count2);
	vertices_count2 = _countof(line_vertices19);
	line[18].Line_Initialize(device.Get(), line_vertices19, indices2, vertices_count2, indices_count2);
	vertices_count2 = _countof(line_vertices20);
	line[19].Line_Initialize(device.Get(), line_vertices20, indices2, vertices_count2, indices_count2);
#pragma endregion
	// グリット線の初期設定

	
	//ゲームループ
	while (true) {
#pragma region ウィンドウメッセージ処理

		// Windowsのメッセージ処理
		if (winApp->ProcessMessage()) {
			// ゲームループを抜ける
			break;
		}

#pragma endregion

#pragma region DirectX毎フレーム処理
		/////////////////////////////////////////////////////
		//----------DireceX毎フレーム処理　ここから------------//
		///////////////////////////////////////////////////

		// 入力の更新
		input->Update();
		
		// 数字の0キーが押されていたら
		if (input->PushKey(DIK_0))
		{
			OutputDebugStringA("Hit 0\n");  // 出力ウィンドウに「Hit 0」と表示
		}

		//バックバッファの番号を取得(2つなので0番か1番)
		UINT bbIndex = swapChain->GetCurrentBackBufferIndex();

		//1.リソースバリアで書き込み可能に変更
		D3D12_RESOURCE_BARRIER barrierDesc{};
		barrierDesc.Transition.pResource = backBuffers[bbIndex].Get();
		barrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
		barrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
		commandList->ResourceBarrier(1, &barrierDesc);

		//2.描画先の変更
		//レンダーターゲットビューのハンドルを取得
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();
		rtvHandle.ptr += static_cast<unsigned long long>(bbIndex) * device->GetDescriptorHandleIncrementSize(rtvHeapDesc.Type);
		D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dsvHeap->GetCPUDescriptorHandleForHeapStart();
		commandList->OMSetRenderTargets(1, &rtvHandle, false, &dsvHandle);

		//3.画面クリア			R	  G	   B	A
		FLOAT clearColor[] = { 0.1f,0.25f,0.5f,0.0f };//青っぽい色
		commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
		if (input->PushKey(DIK_SPACE))     // スペースキーが押されていたら
		{
			//画面クリアカラーの数値を書き換える
			FLOAT clearColor[] = { 11.1f,0.25f, 0.5f,0.0f }; // ピンクっぽい色
			commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
		}

		////トリガー処理
		//bool キーを押した状態か(uint8_t キー番号);
		//bool キーを離した状態か(uint8_t キー番号);
		//bool キーを押した瞬間か(uint8_t キー番号);
		//bool キーを離した瞬間か(uint8_t キー番号);




		mesh->Mesh_Update(input);

		for (int i = 0; i < 20; i++)
		{
			line[i].Line_Updata();
		}
		
		
		/*transformX = 0.0f;
		transformY = 0.0f;
		rotation = 0.0f;
		scale = 1.0f;*/

		// キー入力

		////平行移動
		//if (key[DIK_W]) {
		//	transformY += 0.05f;
		//}

		//if (key[DIK_S]) {
		//	transformY -= 0.05f;
		//}

		//if (key[DIK_A]) {
		//	transformX -= 0.05f;
		//}

		//if (key[DIK_D]) {
		//	transformX += 0.05f;
		//}

		//// 拡大縮小
		//if (key[DIK_Z]) {
		//	scale -= 0.1f;
		//}

		//if (key[DIK_C]) {
		//	scale += 0.1f;
		//}


		//// 回転
		//if (key[DIK_Q]) {
		//	rotation -= PI / 32;
		//}

		//if (key[DIK_E]) {
		//	rotation += PI / 32;
		//}


		//// アフィン行列の生成
		//affin[0][0] = scale * cos(rotation);
		//affin[0][1] = scale * (-sin(rotation));
		//affin[0][2] = transformX;

		//affin[1][0] = scale * sin(rotation);
		//affin[1][1] = scale * cos(rotation);
		//affin[1][2] = transformY;

		//affin[2][0] = 0.0f;
		//affin[2][1] = 0.0f;
		//affin[2][2] = 1.0f;

		//// アフィン変換
		//for (int i = 0; i < _countof(vertices); i++) {
		//	vertices[i].x = vertices[i].x * affin[0][0] +
		//		vertices[i].y * affin[0][1] + 1.0f * affin[0][2];
		//	vertices[i].y = vertices[i].x * affin[1][0] +
		//		vertices[i].y * affin[1][1] + 1.0f * affin[1][2];
		//	vertices[i].z = vertices[i].x * affin[2][0] +
		//		vertices[i].y * affin[2][1] + 1.0f * affin[2][2];
		//}


		//////////////////////////////////////////////
		//-------DireceX毎フレーム処理　ここまで--------//
		////////////////////////////////////////////

#pragma endregion

#pragma region グラフィックスコマンド

		//4.描画コマンドここから

		// ビューポート設定コマンド
		D3D12_VIEWPORT viewport{};
		viewport.Width = WinApp::window_width;   //よこ 最大1280
		viewport.Height = WinApp::window_height;  //たて 最大720
		viewport.TopLeftX = 0;  //左上X
		viewport.TopLeftY = 0;  //左上Y
		viewport.MinDepth = 0.0f; //最小頻度
		viewport.MaxDepth = 1.0f; //最大頻度

		// ビューポート設定コマンドを、コマンドリストに積む
		commandList->RSSetViewports(1, &viewport);

		//シザー矩形
		D3D12_RECT scissorRect{};
		scissorRect.left = 0; // 切り抜き座標左
		scissorRect.right = scissorRect.left + WinApp::window_width; // 切り抜き座標右
		scissorRect.top = 0; // 切り抜き座標上
		scissorRect.bottom = scissorRect.top + WinApp::window_height; // 切り抜き座標下

		// シザー矩形設定コマンドを、コマンドリストに積む
		commandList->RSSetScissorRects(1, &scissorRect);

		//Meshの描画--------------------------------------------------------------//
		commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

		
		mesh[0].Mesh_Draw(device.Get(), indices_count, commandList.Get());
		
		for (int i = 0; i < 20; i++)
		{
			line[i].Line_Draw(indices_count2, commandList.Get());
		}
		//4.描画コマンドここまで

#pragma endregion

#pragma region 画面入れ替え

		
		//5.リソースバリアを戻す
		barrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;//描画状態から
		barrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;//表示状態へ
		commandList->ResourceBarrier(1, &barrierDesc);

		//命令のクローズ
		result = commandList->Close();
		assert(SUCCEEDED(result));
		//コマンドリストの実行
		ID3D12CommandList* commandLists[] = { commandList.Get()};
		commandQueue->ExecuteCommandLists(1, commandLists);

		//画面に表示するバッファをフリップ(裏表の入れ替え)
		result = swapChain->Present(1, 0);
		assert(SUCCEEDED(result));

		//コマンドの実行完了を待つ
		commandQueue->Signal(fence.Get(), ++fenceVal);
		if (fence->GetCompletedValue() != fenceVal) {
			HANDLE event = CreateEvent(nullptr, false, false, nullptr);
			fence->SetEventOnCompletion(fenceVal, event);
			if (event != 0)
			{
				WaitForSingleObject(event, INFINITE);
				CloseHandle(event);
			}
			
		}

		//キューをクリア
		result = commandAllocator->Reset();
		assert(SUCCEEDED(result));
		//再びコマンドリストを貯める準備
		result = commandList->Reset(commandAllocator.Get(), nullptr);
		assert(SUCCEEDED(result));

#pragma endregion
	}
#pragma region  WindowsAPI後始末

	//もうクラスは使わないので登録を解除する

	// 入力解放
	delete input;

	// WindouwsAPIの終了処理
	winApp->Finalize();

	// WindouwsAPI解放
	delete winApp;

#pragma endregion

	Render_basic::DestroyInstance();

	return 0;
}
