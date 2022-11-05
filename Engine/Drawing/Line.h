#pragma once
#include "Mesh.h"

class Line
{
public:
	template <class T>using ComPtr = Microsoft::WRL::ComPtr<T>;
	void Line_Initialize(ID3D12Device* device, Mesh::Vertex* vertices, unsigned short* indices, int vertices_count, int indices_count);
	void Line_Updata();
	void Line_Draw(int indices_count, ID3D12GraphicsCommandList* commandList);
private:
	HRESULT result;

	// テクスチャバッファ
	ComPtr<ID3D12Resource> texBuff = nullptr;

	//設定を元にデスクリプタヒープを生成
	ComPtr<ID3D12DescriptorHeap> srvHeap = nullptr;

	//バッファマテリアル
	ComPtr<ID3D12Resource> constBuffMaterial = nullptr;

	Mesh::ConstBufferDataMaterial* constMapMaterial = nullptr;

	//デスクリプタヒープ
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};

	// 頂点バッファビューの作成
	D3D12_VERTEX_BUFFER_VIEW vbView{};

	// 頂点バッファ
	ComPtr<ID3D12Resource> vertBuff = nullptr;

	// インデックスバッファ
	ComPtr<ID3D12Resource> indexBuff = nullptr;

	//インデックスバッファ
	D3D12_INDEX_BUFFER_VIEW ibView{};

	D3D12_CPU_DESCRIPTOR_HANDLE srvHandle;

	D3D12_GPU_DESCRIPTOR_HANDLE srvGpuHandle;

	UINT incrementSize;

	Mesh::Object3d object3d;

};

