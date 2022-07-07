#pragma once
#include<d3dcompiler.h>
#include<d3d12.h>
#include<DirectXMath.h>
#include <DirectXTex.h>
#include <string>
#pragma comment(lib,"d3d12.lib")
#include "Vector3.h"
using namespace DirectX;

class Mesh
{
public:
	// 頂点データ構造体
	struct Vertex
	{
		XMFLOAT3 pos;    // xyz座標
		XMFLOAT3 normal; // 法線ベクトル
		XMFLOAT2 uv;     // uv座標
	};
public:
	Mesh();

	~Mesh();

	void Mesh_Initialization(ID3D12Device* device,Vertex *vertices_, unsigned short *indices_, int vertices_count, int device_count);

	void Mesh_Update(BYTE key[]);

	void Mesh_Draw(ID3D12Device* device, int indices_count, ID3D12GraphicsCommandList* commandList);

private:
	/*void ConstantBuffer_creation(struct ConstBufferData);*/
	
	
private:
	HRESULT result;
	ID3D12Device* device = nullptr;

	// 頂点バッファビューの作成
	D3D12_VERTEX_BUFFER_VIEW vbView{};
	//設定を元にデスクリプタヒープを生成
	ID3D12DescriptorHeap* srvHeap = nullptr;
	//バッファマテリアル
	ID3D12Resource* constBuffMaterial = nullptr;
	//デスクリプタヒープ
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	//インデックスバッファ
	D3D12_INDEX_BUFFER_VIEW ibView{};

	ID3D12Resource* constBuffTransform = nullptr;
	
	// 定数バッファ用データ構造体  (3D変換行列)
	struct ConstBufferDataTransform {
		XMMATRIX mat; // 3D変換行列
	};

	ConstBufferDataTransform* constMapTransform = nullptr;

	//ワールド変換行列
	XMMATRIX matWorld;
	XMMATRIX matScale;
	XMMATRIX matRot;
	XMMATRIX matTrans;

	//投影行列
	XMMATRIX matProjection;

	//ビュー変換行列
	XMMATRIX matView;
	XMFLOAT3 eye = { 0, 0, -100 };  //視点座標
	XMFLOAT3 target = { 0, 0, 0 };  //注視点座標
	XMFLOAT3 up = { 0, 1, 0 };      //上方向ベクトル

	//角度
	float angle = 0.0f;
	//座標
	XMFLOAT3 scale = { 1.0f,1.0f,1.0f };
	XMFLOAT3 rotation = { 0.0f,0.0f,0.0f };
	XMFLOAT3 position = { 0.0f,0.0f,0.0f };

};



