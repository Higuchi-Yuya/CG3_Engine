#pragma once
#include<d3dcompiler.h>
#include<d3d12.h>
#include<DirectXMath.h>
#include <DirectXTex.h>
#include <string>
#pragma comment(lib,"d3d12.lib")
#include "Vector3.h"

#include <wrl.h>
using namespace DirectX;

class Mesh
{
public:
	template <class T>using ComPtr = Microsoft::WRL::ComPtr<T>;

	// 頂点データ構造体
	struct Vertex
	{
		XMFLOAT3 pos;    // xyz座標
		XMFLOAT3 normal; // 法線ベクトル
		XMFLOAT2 uv;     // uv座標
	};

	// 定数バッファ用データ構造体  (3D変換行列)
	struct ConstBufferDataTransform {
		XMMATRIX mat; // 3D変換行列
	};

	struct Object3d
	{
		//定数バッファ (行列用)
		ComPtr<ID3D12Resource> constBuffTransform = nullptr;

		//定数バッファマップ(行列用)
		ConstBufferDataTransform* constMapTransform = nullptr;

		//アフィン変換情報
		XMFLOAT3 scale = { 1.0f,1.0f,1.0f };
		XMFLOAT3 rotation = { 0.0f,0.0f,0.0f };
		XMFLOAT3 position = { 0.0f,0.0f,0.0f };

		//ワールド変換行列
		XMMATRIX matWorld;

		//親オブジェクトへのポインタ
		Object3d* parent = nullptr;
	};

	

public:
	Mesh();

	~Mesh();

	void Mesh_Initialization(ID3D12Device* device,Vertex *vertices_, unsigned short *indices_, int vertices_count, int device_count);

	void Mesh_Update(BYTE key[]);

	void Mesh_Draw(ID3D12Device* device, int indices_count, ID3D12GraphicsCommandList* commandList);

	void InitializeObject3d(Object3d* object, ID3D12Device* device);

	void UpdateObject3d(Object3d* object, XMMATRIX& matView, XMMATRIX& matProjection);

	void DrawObject3d(Object3d* object, ID3D12GraphicsCommandList* commandList, D3D12_VERTEX_BUFFER_VIEW& vdView, D3D12_INDEX_BUFFER_VIEW& ibView, UINT numIndices);

	template<class T>using ComPtr = Microsoft::WRL::ComPtr<T>;
private:
	/*void ConstantBuffer_creation(struct ConstBufferData);*/
	
	
private:
	HRESULT result;

	// 定数バッファ用データ構造体（マテリアル）
	struct ConstBufferDataMaterial {
		XMFLOAT4 color; // 色 (RGBA)
	};

	float R = 1.0f, G = 1.0f, B = 1.0f;


	ConstBufferDataMaterial* constMapMaterial = nullptr;

	// 頂点バッファビューの作成
	D3D12_VERTEX_BUFFER_VIEW vbView{};

	//設定を元にデスクリプタヒープを生成
	ComPtr<ID3D12DescriptorHeap> srvHeap = nullptr;

	//バッファマテリアル
	ComPtr<ID3D12Resource> constBuffMaterial = nullptr;

	//デスクリプタヒープ
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};

	//インデックスバッファ
	D3D12_INDEX_BUFFER_VIEW ibView{};
	
	D3D12_CPU_DESCRIPTOR_HANDLE srvHandle;

	D3D12_GPU_DESCRIPTOR_HANDLE srvGpuHandle;

	// テクスチャバッファ
	ComPtr<ID3D12Resource> texBuff = nullptr;

	ComPtr<ID3D12Resource> texBuff2 = nullptr;

	// 頂点バッファ
	ComPtr<ID3D12Resource> vertBuff = nullptr;

	// インデックスバッファ
	ComPtr<ID3D12Resource> indexBuff = nullptr;

	//ワールド変換行列 0番
	XMMATRIX matWorld;
	XMMATRIX matScale;
	XMMATRIX matRot;
	XMMATRIX matTrans;

	//ワールド変換行列 1番
	XMMATRIX matWorld1;
	XMMATRIX matScale1;
	XMMATRIX matRot1;
	XMMATRIX matTrans1;

	//投影行列
	XMMATRIX matProjection;

	//ビュー変換行列
	XMMATRIX matView;
	XMFLOAT3 eye = { 0, 0, -100 };  //視点座標
	XMFLOAT3 target = { 0, 0, 0 };  //注視点座標
	XMFLOAT3 up = { 0, 1, 0 };      //上方向ベクトル

	//角度
	float angle = 0.0f;

	UINT incrementSize;

	static const size_t kObjectCount = 50;

	Object3d object3ds[kObjectCount];

};



