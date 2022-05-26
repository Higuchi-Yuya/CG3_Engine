#pragma once
#include<d3dcompiler.h>
#include<d3d12.h>
#include<DirectXMath.h>
#include <DirectXTex.h>
#include <string>
#pragma comment(lib,"d3d12.lib")

using namespace DirectX;

class Mesh
{
public:
	// 頂点データ構造体
	struct Vertex
	{
		XMFLOAT3 pos; // xyz座標
		XMFLOAT2 uv;  // uv座標
	};
public:
	Mesh();

	~Mesh();

	void Mesh_Initialization(ID3D12Device* device,Vertex *vertices_, unsigned short *indices_, int vertices_count, int device_count);

	void Mesh_Draw(ID3D12Device* device, int indices_count, ID3D12GraphicsCommandList* commandList);

private:
	HRESULT result;
	ID3D12Device* device = nullptr;
	//πプランステートの生成
	ID3D12PipelineState* pipelineStage = nullptr;
	// ルートシグネチャ
	ID3D12RootSignature* rootSignature;
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
};



