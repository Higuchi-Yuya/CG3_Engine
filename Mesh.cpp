#include "Mesh.h"
#include "Render_basic.h"
#include <dinput.h>

// ウィンドウ横幅
static const int window_width = 1280;
// ウィンドウ縦幅
static const int window_height = 720;

Mesh::Mesh()
{
}

Mesh::~Mesh()
{
}

void Mesh::Mesh_Initialization(ID3D12Device* device, Vertex* vertices, unsigned short* indices, int vertices_count, int indices_count)
{

#pragma region オブジェクトの描画初期化処理
	//////////////////////////////////////////////////////
	//---------------描画初期化処理 ここから----------------//
	/////////////////////////////////////////////////////

	//// 横方向ピクセル数
	//const size_t textureWidth = 256;
	//// 縦方向ピクセル数
	//const size_t textureHeight = 256;
	//// 配列の要素数
	//const size_t imageDataCount = textureWidth * textureHeight;
	//// 画像イメージデータ配列
	//XMFLOAT4* imageData = new XMFLOAT4[imageDataCount]; // ※必ず後で解放する

	//// 全ピクセルの色を初期化
	//for (size_t i = 0; i < imageDataCount; i++) {
	//	imageData[i].x = 0.0f;    // R
	//	imageData[i].y = 1.0f;    // G
	//	imageData[i].z = 0.0f;    // B
	//	imageData[i].w = 1.0f;    // A
	//}


	// 頂点、インデックス用カウント宣言

	TexMetadata metadata{};
	ScratchImage scratchImg{};
	// WICテクスチャのロード
	result = LoadFromWICFile(
		L"Resources/risu.jpg",   //「Resources」フォルダの「texture.png」
		WIC_FLAGS_NONE,
		&metadata, scratchImg);
	ScratchImage mipChain{};
	// ミップマップ生成
	result = GenerateMipMaps(
		scratchImg.GetImages(), scratchImg.GetImageCount(), scratchImg.GetMetadata(),
		TEX_FILTER_DEFAULT, 0, mipChain);
	if (SUCCEEDED(result)) {
		scratchImg = std::move(mipChain);
		metadata = scratchImg.GetMetadata();
	}
	// 読み込んだディフューズテクスチャをSRGBとして扱う
	metadata.format = MakeSRGB(metadata.format);

	// ヒープ設定
	D3D12_HEAP_PROPERTIES textureHeapProp{};
	textureHeapProp.Type = D3D12_HEAP_TYPE_CUSTOM;
	textureHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
	textureHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;

	//// リソース設定
	//D3D12_RESOURCE_DESC textureResourceDesc{};
	//textureResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D; textureResourceDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT; textureResourceDesc.Width = textureWidth;  // 幅
	//textureResourceDesc.Height = textureWidth; // 高さ
	//textureResourceDesc.DepthOrArraySize = 1;
	//textureResourceDesc.MipLevels = 1;
	//textureResourceDesc.SampleDesc.Count = 1;

	D3D12_RESOURCE_DESC textureResourceDesc{};
	textureResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	textureResourceDesc.Format = metadata.format;
	textureResourceDesc.Width = metadata.width;
	textureResourceDesc.Height = (UINT)metadata.height;
	textureResourceDesc.DepthOrArraySize = (UINT16)metadata.arraySize;
	textureResourceDesc.MipLevels = (UINT16)metadata.mipLevels;
	textureResourceDesc.SampleDesc.Count = 1;



	// テクスチャバッファの生成
	ID3D12Resource* texBuff = nullptr;
	result = device->CreateCommittedResource(
		&textureHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&textureResourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&texBuff));

	// テクスチャバッファにデータ転送
	//result = texBuff->WriteToSubresource(
	//	0,
	//	nullptr, // 全領域へコピー
	//	imageData,    // 元データアドレス
	//	sizeof(XMFLOAT4) * textureWidth, // 1ラインサイズ
	//	sizeof(XMFLOAT4) * imageDataCount // 全サイズ
	//);
	
	// 全ミップマップについて
	for (size_t i = 0; i < metadata.mipLevels; i++) {
		// ミップマップレベルを指定してイメージを取得
		const Image* img = scratchImg.GetImage(i, 0, 0);
		// テクスチャバッファにデータ転送
		result = texBuff->WriteToSubresource(
			(UINT)i,
			nullptr,              // 全領域へコピー
			img->pixels,          // 元データアドレス
			(UINT)img->rowPitch,  // 1ラインサイズ
			(UINT)img->slicePitch // 1枚サイズ
		);
		assert(SUCCEEDED(result));
	}

	// SRVの最大個数
	const size_t kMaxSRVCount = 2056;

	// デスクリプタヒープの設定
	
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;//シェーダから見えるように
	srvHeapDesc.NumDescriptors = kMaxSRVCount;

	// 設定を元にSRV用デスクリプタヒープを生成
	
	result = device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&srvHeap));
	assert(SUCCEEDED(result));

	//SRVヒープの先頭ハンドルを取得
	D3D12_CPU_DESCRIPTOR_HANDLE srvHandle = srvHeap->GetCPUDescriptorHandleForHeapStart();


	// 頂点データ全体のサイズ = 頂点データ一つ分のサイズ * 頂点データの要素数
	/*UINT sizeVB = static_cast<UINT>(sizeof(XMFLOAT3) * _countof(vertices));*/
	UINT sizeVB = static_cast<UINT>(sizeof(vertices[0]) * vertices_count);
	// 頂点バッファの設定
	D3D12_HEAP_PROPERTIES heapProp{}; // ヒープ設定
	heapProp.Type = D3D12_HEAP_TYPE_UPLOAD; // GPUへの転送用
	// リソース設定
	D3D12_RESOURCE_DESC resDesc{};
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resDesc.Width = sizeVB; // 頂点データ全体のサイズ
	resDesc.Height = 1;
	resDesc.DepthOrArraySize = 1;
	resDesc.MipLevels = 1;
	resDesc.SampleDesc.Count = 1;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	// 頂点バッファの生成
	ID3D12Resource* vertBuff = nullptr;
	result = device->CreateCommittedResource(
		&heapProp, // ヒープ設定
		D3D12_HEAP_FLAG_NONE,
		&resDesc, // リソース設定
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vertBuff));
	assert(SUCCEEDED(result));

	// インデックスデータ全体のサイズ
	UINT sizeIB = static_cast<UINT>(sizeof(uint16_t) * indices_count);

	// リソース設定
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resDesc.Width = sizeIB; // インデックス情報が入る分のサイズ
	resDesc.Height = 1;
	resDesc.DepthOrArraySize = 1;
	resDesc.MipLevels = 1;
	resDesc.SampleDesc.Count = 1;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	// インデックスバッファの生成
	ID3D12Resource* indexBuff = nullptr;
	result = device->CreateCommittedResource(
		&heapProp, // ヒープ設定
		D3D12_HEAP_FLAG_NONE,
		&resDesc, // リソース設定
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&indexBuff));

	// インデックスバッファをマッピング
	uint16_t* indexMap = nullptr;
	result = indexBuff->Map(0, nullptr, (void**)&indexMap);
	// 全インデックスに対して
	for (int i = 0; i < indices_count; i++)
	{
		indexMap[i] = indices[i];   // インデックスをコピー
	}
	// マッピング解除
	indexBuff->Unmap(0, nullptr);

	// インデックスバッファビューの作成
	ibView.BufferLocation = indexBuff->GetGPUVirtualAddress();
	ibView.Format = DXGI_FORMAT_R16_UINT;
	ibView.SizeInBytes = sizeIB;

	// GPU上のバッファに対応した仮想メモリ(メインメモリ上)を取得
	Vertex* vertMap = nullptr;
	result = vertBuff->Map(0, nullptr, (void**)&vertMap);
	assert(SUCCEEDED(result));

	// 全頂点に対して
	for (int i = 0; i < vertices_count; i++) {
		vertMap[i] = vertices[i]; // 座標をコピー
	}
	// 繋がりを解除
	vertBuff->Unmap(0, nullptr);

	
	// GPU仮想アドレス
	vbView.BufferLocation = vertBuff->GetGPUVirtualAddress();
	// 頂点バッファのサイズ
	vbView.SizeInBytes = sizeVB;
	// 頂点1つ分のデータサイズ
	/*vbView.StrideInBytes = sizeof(XMFLOAT3);*/
	vbView.StrideInBytes = sizeof(vertices[0]);


	// シェーダリソースビュー設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = resDesc.Format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = resDesc.MipLevels;



	// ハンドルの指す位置にシェーダーリソースビュー作成
	device->CreateShaderResourceView(texBuff, &srvDesc, srvHandle);

	// CBV,SRV,UAVの1個分のサイズを取得
	UINT descriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// ハンドルを1つ進める（SRVの位置）
	srvHandle.ptr += descriptorSize * 1;

	// CBV(コンスタントバッファビュー)の設定
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc{};
	//cbvDescの値設定（省略）
	// 定数バッファビュー生成
	device->CreateConstantBufferView(&cbvDesc, srvHandle);

	


	// 定数バッファ用データ構造体（マテリアル）
	struct ConstBufferDataMaterial {
		XMFLOAT4 color; // 色 (RGBA)
	};
	// 定数バッファ用データ構造体Pos（マテリアル）
	struct ConstBufferDataMaterialPos {
		XMFLOAT4 Move; // 移動
	};
	

	//Creation<ConstBufferDataMaterial>();
	// ヒープ設定
	D3D12_HEAP_PROPERTIES cbHeapProp{};
	cbHeapProp.Type = D3D12_HEAP_TYPE_UPLOAD;                   // GPUへの転送用
	// リソース設定
	D3D12_RESOURCE_DESC cbResourceDesc{};
	cbResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	cbResourceDesc.Width = (sizeof(ConstBufferDataMaterial) + 0xff) & ~0xff;   // 256バイトアラインメント
	cbResourceDesc.Height = 1;
	cbResourceDesc.DepthOrArraySize = 1;
	cbResourceDesc.MipLevels = 1;
	cbResourceDesc.SampleDesc.Count = 1;
	cbResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	
	
	// 定数バッファの生成
	result = device->CreateCommittedResource(
		&cbHeapProp, // ヒープ設定
		D3D12_HEAP_FLAG_NONE,
		&cbResourceDesc, // リソース設定
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&constBuffMaterial));
	assert(SUCCEEDED(result));

	// 定数バッファのマッピング
	ConstBufferDataMaterial* constMapMaterial = nullptr;
	result = constBuffMaterial->Map(0, nullptr, (void**)&constMapMaterial); // マッピング
	assert(SUCCEEDED(result));

	constMapMaterial->color = XMFLOAT4(1, 1, 1, 1);


	

	//行列定数バッファ
	{
		// ヒープ設定
		D3D12_HEAP_PROPERTIES cbHeapProp{};
		cbHeapProp.Type = D3D12_HEAP_TYPE_UPLOAD;                   // GPUへの転送用
		// リソース設定
		D3D12_RESOURCE_DESC cbResourceDesc{};
		cbResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		cbResourceDesc.Width = (sizeof(ConstBufferDataTransform) + 0xff) & ~0xff;   // 256バイトアラインメント
		cbResourceDesc.Height = 1;
		cbResourceDesc.DepthOrArraySize = 1;
		cbResourceDesc.MipLevels = 1;
		cbResourceDesc.SampleDesc.Count = 1;
		cbResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	
		// 定数バッファの生成
		result = device->CreateCommittedResource(
			&cbHeapProp, // ヒープ設定
			D3D12_HEAP_FLAG_NONE,
			&cbResourceDesc, // リソース設定
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&constBuffTransform));
		assert(SUCCEEDED(result));

		// 定数バッファのマッピング
		result = constBuffTransform->Map(0, nullptr, (void**)&constMapTransform); // マッピング
		assert(SUCCEEDED(result));
	}



	
	//単位行列を代入
	/*constMapTransform->mat = XMMatrixIdentity();*/

	//constMapTransform->mat.r[0].m128_f32[0] = 2.0f / window_width;
	//constMapTransform->mat.r[1].m128_f32[1] = -2.0f / window_height;

	//// 値を書き込むと自動的に転送される
	//              // RGBAで半透明の赤
	//constMapTransform->mat.r[3].m128_f32[0] = -1.0f;
	//constMapTransform->mat.r[3].m128_f32[1] = 1.0f;

	//constMapTransform->mat = XMMatrixOrthographicOffCenterLH(
	//	0, window_width,
	//	window_height, 0,
	//	0.0f, 1.0);
	
	//ワールド変換行列
	matWorld = XMMatrixIdentity();

	//射影変換行列
	matProjection = XMMatrixPerspectiveFovLH(
		XMConvertToRadians(45.0f),
		(float)window_width / window_height,
		0.1f, 1000.0f);

	
	matView = XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&target), XMLoadFloat3(&up));


	matScale = XMMatrixScaling(1.0f, 0.5f, 1.0f);
	matWorld *= matScale;

	matRot = XMMatrixIdentity();
	matRot *= XMMatrixRotationZ(XMConvertToRadians(0.0f));
	matRot *= XMMatrixRotationX(XMConvertToRadians(15.0f));
	matRot *= XMMatrixRotationY(XMConvertToRadians(30.0f));
	matWorld *= matRot;

	matTrans = XMMatrixTranslation(-50.0f, 0, 0);
	matWorld *= matTrans;

	//行列の合成
	constMapTransform->mat = matWorld * matView * matProjection;

#pragma endregion

}

void Mesh::Mesh_Update(BYTE key[])
{
	//いずれかのキーを押していたら
	if (key[DIK_UP] || key[DIK_DOWN] || key[DIK_RIGHT] || key[DIK_LEFT])
	{
		//座標を移動する処理
		if (key[DIK_UP]) { position.z += 1.0f; }
		else if (key[DIK_DOWN]) { position.z -= 1.0f; }
		if (key[DIK_RIGHT]) { position.x += 1.0f; }
		else if (key[DIK_LEFT]) { position.x -= 1.0f; }
	}
	
	matScale = XMMatrixScaling(scale.x, scale.y, scale.z);
	
	matRot = XMMatrixIdentity();
	matRot *= XMMatrixRotationZ(rotation.z);
	matRot *= XMMatrixRotationX(rotation.x);
	matRot *= XMMatrixRotationY(rotation.y);
	matTrans = XMMatrixTranslation(position.x, position.y, position.z);

	matWorld = XMMatrixIdentity();
	matWorld *= matScale;
	matWorld *= matRot;
	matWorld *= matTrans;
	
	if (key[DIK_D] || key[DIK_A])
	{
		if (key[DIK_D]) { angle += XMConvertToRadians(1.0f); }
		else if (key[DIK_A]) { angle -= XMConvertToRadians(1.0f); }

		// angleラジアンだけｙ軸周りに回転。半径はー１００
		eye.x = -100 * sinf(angle);
		eye.z = -100 * cosf(angle);
		
		matView = XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&target), XMLoadFloat3(&up));
	}
	
	if (key[DIK_W] || key[DIK_S])
	{
		if (key[DIK_W]) { angle += XMConvertToRadians(1.0f); }
		else if (key[DIK_S]) { angle -= XMConvertToRadians(1.0f); }

		eye.y = -100 * sinf(angle);
		eye.z = -100 * cosf(angle);

		matView = XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&target), XMLoadFloat3(&up));
	}
	//行列の合成
	constMapTransform->mat = matWorld * matView * matProjection;

}



void Mesh::Mesh_Draw(ID3D12Device* device, int indices_count, ID3D12GraphicsCommandList* commandList)
{
	//パイプラインステートとルートシグネチャの設定コマンド
	commandList->SetPipelineState(Render_basic::GetInstance()->GetPipelineState());
	commandList->SetGraphicsRootSignature(Render_basic::GetInstance()->GetRootSignature());

	// プリミティブ形状の設定コマンド
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST); // 三角形リスト

	//頂点バッファの設定コマンド
	commandList->IASetVertexBuffers(0, 1, &vbView);

	// 定数バッファビュー(CBV)の設定コマンド
	commandList->SetGraphicsRootConstantBufferView(0, constBuffMaterial->GetGPUVirtualAddress());
	commandList->SetGraphicsRootConstantBufferView(2, constBuffTransform->GetGPUVirtualAddress());

	// SRVヒープの設定コマンド
	commandList->SetDescriptorHeaps(1, &srvHeap);

	// SRVヒープの先頭ハンドルを取得（SRVを指しているはず）
	D3D12_GPU_DESCRIPTOR_HANDLE srvGpuHandle = srvHeap->GetGPUDescriptorHandleForHeapStart();

	// SRVヒープの先頭にあるSRVをルートパラメータ1番に設定
	commandList->SetGraphicsRootDescriptorTable(1, srvGpuHandle);
	
	// インデックスバッファビューの設定コマンド
	commandList->IASetIndexBuffer(&ibView);

	// 描画コマンド
	commandList->DrawIndexedInstanced(indices_count, 1, 0, 0, 0);
}

//void Mesh::ConstantBuffer_creation(ConstBufferData)
//{
//
//}
//template <typename Buffer>
//Buffer Creation() {
//	 ヒープ設定
//	D3D12_HEAP_PROPERTIES cbHeapProp{};
//	cbHeapProp.Type = D3D12_HEAP_TYPE_UPLOAD;                   // GPUへの転送用
//	 リソース設定
//	D3D12_RESOURCE_DESC cbResourceDesc{};
//	cbResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
//	cbResourceDesc.Width = (sizeof(Buffer) + 0xff) & ~0xff;   // 256バイトアラインメント
//	cbResourceDesc.Height = 1;
//	cbResourceDesc.DepthOrArraySize = 1;
//	cbResourceDesc.MipLevels = 1;
//	cbResourceDesc.SampleDesc.Count = 1;
//	cbResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
//
//	 定数バッファの生成
//	result = device->CreateCommittedResource(
//		&cbHeapProp, // ヒープ設定
//		D3D12_HEAP_FLAG_NONE,
//		&cbResourceDesc, // リソース設定
//		D3D12_RESOURCE_STATE_GENERIC_READ,
//		nullptr,
//		IID_PPV_ARGS(&constBuffMaterial));
//	assert(SUCCEEDED(result));
//
//	 定数バッファのマッピング
//	Buffer* constMapMaterial = nullptr;
//	result = constBuffMaterial->Map(0, nullptr, (void**)&constMapMaterial); // マッピング
//	assert(SUCCEEDED(result));
//
//	
//}