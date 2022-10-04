#include "Mesh.h"
#include "Render_basic.h"
#include <dinput.h>
#include "Texture.h"


// ウィンドウ横幅
static const int window_width = 1280;

// ウィンドウ縦幅
static const int window_height = 720;

XMMATRIX Mesh::matProjection = {};
XMMATRIX Mesh::matView = {};


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


	// 頂点、インデックス用カウント宣言


	TexMetadata metadata{};
	ScratchImage scratchImg{};

	TexMetadata metadata2{};
	ScratchImage scratchImg2{};

	// WICテクスチャのロード
	result = LoadFromWICFile(
		L"Resources/risu.jpg",   //「Resources」フォルダの「texture.png」risu.jpg
		WIC_FLAGS_NONE,
		&metadata, scratchImg);
	assert(SUCCEEDED(result));

	result= LoadFromWICFile(
		L"Resources/reimu.png",   //「Resources」フォルダの「texture.png」reimu.png
		WIC_FLAGS_NONE,
		&metadata2, scratchImg2);
	assert(SUCCEEDED(result));

	ScratchImage mipChain;
	ScratchImage mipChain2;

	// ミップマップ生成
	result = GenerateMipMaps(
		scratchImg.GetImages(), scratchImg.GetImageCount(), scratchImg.GetMetadata(),
		TEX_FILTER_DEFAULT, 0, mipChain);
	if (SUCCEEDED(result)) {
		scratchImg = std::move(mipChain);
		metadata = scratchImg.GetMetadata();
	}
	assert(SUCCEEDED(result));

	// ミップマップ生成
	result = GenerateMipMaps(
		scratchImg2.GetImages(), scratchImg2.GetImageCount(), scratchImg2.GetMetadata(),
		TEX_FILTER_DEFAULT, 0, mipChain2);
	if (SUCCEEDED(result)) {
		scratchImg2 = std::move(mipChain2);
		metadata2 = scratchImg2.GetMetadata();
	}
	assert(SUCCEEDED(result));

	// 読み込んだディフューズテクスチャをSRGBとして扱う
	metadata.format = MakeSRGB(metadata.format);

	// 読み込んだディフューズテクスチャをSRGBとして扱う
	metadata2.format = MakeSRGB(metadata2.format);

	// ヒープ設定
	D3D12_HEAP_PROPERTIES textureHeapProp{};
	textureHeapProp.Type = D3D12_HEAP_TYPE_CUSTOM;
	textureHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
	textureHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;

	D3D12_RESOURCE_DESC textureResourceDesc{};
	textureResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	textureResourceDesc.Format = metadata.format;
	textureResourceDesc.Width = metadata.width;
	textureResourceDesc.Height = (UINT)metadata.height;
	textureResourceDesc.DepthOrArraySize = (UINT16)metadata.arraySize;
	textureResourceDesc.MipLevels = (UINT16)metadata.mipLevels;
	textureResourceDesc.SampleDesc.Count = 1;

	D3D12_RESOURCE_DESC textureResourceDesc2{};
	textureResourceDesc2.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	textureResourceDesc2.Format = metadata2.format;
	textureResourceDesc2.Width = metadata2.width;
	textureResourceDesc2.Height = (UINT)metadata2.height;
	textureResourceDesc2.DepthOrArraySize = (UINT16)metadata2.arraySize;
	textureResourceDesc2.MipLevels = (UINT16)metadata2.mipLevels;
	textureResourceDesc2.SampleDesc.Count = 1;

	

	// テクスチャバッファの生成
	result = device->CreateCommittedResource(
		&textureHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&textureResourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&texBuff));
	assert(SUCCEEDED(result));

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

	// テクスチャバッファの生成
	result = device->CreateCommittedResource(
		&textureHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&textureResourceDesc2,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&texBuff2));
	assert(SUCCEEDED(result));

	// 全ミップマップについて
	for (size_t i = 0; i < metadata2.mipLevels; i++) {
		// ミップマップレベルを指定してイメージを取得
		const Image* img = scratchImg2.GetImage(i, 0, 0);
		// テクスチャバッファにデータ転送
		result = texBuff2->WriteToSubresource(
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
	result = device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(srvHeap.GetAddressOf()));
	assert(SUCCEEDED(result));

	
	// 頂点データ全体のサイズ = 頂点データ一つ分のサイズ * 頂点データの要素数
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
	
	result = device->CreateCommittedResource(
		&heapProp, // ヒープ設定
		D3D12_HEAP_FLAG_NONE,
		&resDesc, // リソース設定
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&indexBuff));

	//二枚目の画像のリソースデスク
	D3D12_RESOURCE_DESC resDesc2{};
	resDesc2.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resDesc2.Width = sizeVB; // 頂点データ全体のサイズ
	resDesc2.Height = 1;
	resDesc2.DepthOrArraySize = 1;
	resDesc2.MipLevels = 1;
	resDesc2.SampleDesc.Count = 1;
	resDesc2.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	result = device->CreateCommittedResource(
		&heapProp, // ヒープ設定
		D3D12_HEAP_FLAG_NONE,
		&resDesc2, // リソース設定
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vertBuff));
	assert(SUCCEEDED(result));

	resDesc2.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resDesc2.Width = sizeIB; // インデックス情報が入る分のサイズ
	resDesc2.Height = 1;
	resDesc2.DepthOrArraySize = 1;
	resDesc2.MipLevels = 1;
	resDesc2.SampleDesc.Count = 1;
	resDesc2.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	result = device->CreateCommittedResource(
		&heapProp, // ヒープ設定
		D3D12_HEAP_FLAG_NONE,
		&resDesc2, // リソース設定
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

	//SRVヒープの先頭ハンドルを取得
	srvHandle = srvHeap->GetCPUDescriptorHandleForHeapStart();
	srvGpuHandle = srvHeap->GetGPUDescriptorHandleForHeapStart();

	// シェーダリソースビュー設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = textureResourceDesc.Format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = textureResourceDesc.MipLevels;

	// ハンドルの指す位置にシェーダーリソースビュー作成
	device->CreateShaderResourceView(texBuff.Get(), &srvDesc, srvHandle);

	//二枚目の画像に必要な設定

	// CBV,SRV,UAVの1個分のサイズを取得
	incrementSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// ハンドルを1つ進める（SRVの位置）
	srvHandle.ptr += incrementSize;

	// シェーダリソースビュー設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc2{};
	srvDesc2.Format = textureResourceDesc2.Format;
	srvDesc2.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc2.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc2.Texture2D.MipLevels = textureResourceDesc2.MipLevels;

	// ハンドルの指す位置にシェーダーリソースビュー作成
	device->CreateShaderResourceView(texBuff2.Get(), &srvDesc2, srvHandle);

	srvHandle.ptr += incrementSize;

	// CBV(コンスタントバッファビュー)の設定
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc{};
	//cbvDescの値設定（省略）
	// 定数バッファビュー生成
	device->CreateConstantBufferView(&cbvDesc, srvHandle);



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
	
	result = constBuffMaterial.Get()->Map(0, nullptr, (void**)&constMapMaterial); // マッピング
	assert(SUCCEEDED(result));

	


	//行列定数バッファ
	{
		for (int i = 0; i < _countof(object3ds); i++)
		{
			//初期化
			InitializeObject3d(& object3ds[i], device);

			//ここから↓は親子構造のサンプル
			//先頭以外なら
			if (i > 0) {
				//ひとつ前のオブジェクトを親オブジェクトとする
				object3ds[i].parent = &object3ds[i - 1];
				//親オブジェクトの９割の大きさ
				object3ds[i].scale = { 0.9f,0.9f,0.9f };
				//親オブジェクトに対してz軸周りに３０度回転
				object3ds[i].rotation = { 0.0f,0.0f,XMConvertToRadians(30.0f) };
				//親オブジェクトに対してz方向-８．０ずらす
				object3ds[i].position = { 0.0f,0.0f,-8.0f };
			}
		}

	}
	
	//ワールド変換行列
	matWorld = XMMatrixIdentity();

	//射影変換行列
	matProjection = XMMatrixPerspectiveFovLH(
		XMConvertToRadians(45.0f),
		(float)window_width / window_height,
		0.1f, 1000.0f);

	matView = XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&target), XMLoadFloat3(&up));

#pragma endregion

}

void Mesh::Mesh_Update(Input *input)
{

#pragma region カメラの処理

	if (input->PushKey(DIK_D) || input->PushKey(DIK_A))
	{
		if (input->PushKey(DIK_D)) { angle.x -= XMConvertToRadians(1.0f); }
		else if (input->PushKey(DIK_A)) { angle.x += XMConvertToRadians(1.0f); }
	}
	if (input->PushKey(DIK_W) || input->PushKey(DIK_S))
	{
		if (input->PushKey(DIK_W)) { angle.y += XMConvertToRadians(1.0f); }
		else if (input->PushKey(DIK_S)) { angle.y -= XMConvertToRadians(1.0f); }
	}

	eye.x = dis * cosf(angle.x) * cosf(angle.y);
	eye.y = -dis * sinf(angle.y);
	eye.z = dis * sinf(angle.x) * cosf(angle.y);

	up.y = cosf(angle.y);

	matView = XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&target), XMLoadFloat3(&up));

#pragma endregion

#pragma region 移動系処理

	//いずれかのキーを押していたら親だけ動く
	if (input->PushKey(DIK_UP) || input->PushKey(DIK_DOWN) || input->PushKey(DIK_RIGHT) || input->PushKey(DIK_LEFT))
	{

		//座標を移動する処理
		if (input->PushKey(DIK_UP)) { object3ds[0].position.y += 1.0f; }
		else if (input->PushKey(DIK_DOWN)) { object3ds[0].position.y -= 1.0f; }
		if (input->PushKey(DIK_RIGHT)) { object3ds[0].position.x += 1.0f; }
		else if (input->PushKey(DIK_LEFT)) { object3ds[0].position.x -= 1.0f; }
	}

	if (input->PushKey(DIK_R) || input->PushKey(DIK_T))
	{
		// 回転する処理Y軸
		if (input->PushKey(DIK_R)) { object3ds[0].rotation.y += 0.05f; }
		else if (input->PushKey(DIK_T)) { object3ds[0].rotation.y -= 0.05f; }
	}
	if (input->PushKey(DIK_F) || input->PushKey(DIK_G))
	{
		// 回転する処理
		if (input->PushKey(DIK_F)) { object3ds[0].rotation.z += 0.1f; }
		else if (input->PushKey(DIK_G)) { object3ds[0].rotation.z -= 0.1f; }
	}

	if (input->PushKey(DIK_V) || input->PushKey(DIK_B))
	{
		// 回転する処理
		if (input->PushKey(DIK_V)) { object3ds[0].rotation.x += 0.05f; }
		else if (input->PushKey(DIK_B)) { object3ds[0].rotation.x -= 0.05f; }
	}

	//行列の合成
	for (size_t i = 0; i < _countof(object3ds); i++)
	{
		UpdateObject3d(&object3ds[i], matView, matProjection);
	}

#pragma endregion
	
#pragma region 画像切り替え処理
	if (input->PushKey(DIK_1)|| input->PushKey(DIK_2))
	{
		if(input->PushKey(DIK_1)){ srvGpuHandle = srvHeap->GetGPUDescriptorHandleForHeapStart(); }
		if(input->PushKey(DIK_2)){ srvGpuHandle = srvHeap->GetGPUDescriptorHandleForHeapStart();
						srvGpuHandle.ptr += incrementSize;}
	}
#pragma endregion

#pragma region 色の切り替え処理
	float speed = 0.01;

	if (R >= 1 && B <= 0) { R = 1;  B = 0; G += speed; }
	if (G >= 1 && B <= 0) { G = 1;  B = 0; R -= speed; }
	if (R <= 0 && G >= 1) { R = 0;  G = 1; B += speed; }
	if (R <= 0 && B >= 1) { R = 0;  B = 1; G -= speed; }
	if (G <= 0 && B >= 1) { G = 0;  B = 1; R += speed; }
	if (R >= 1 && G <= 0) { R = 1;  G = 0; B -= speed; }

	if (A >= 1) annihilation_flag = true;
	if (A <= 0) annihilation_flag = false;
	if (annihilation_flag == true) A -= speed;
	if (annihilation_flag == false) A += speed;

	constMapMaterial->color = XMFLOAT4(R, G, B, A);
#pragma endregion

}

void Mesh::Mesh_Draw(ID3D12Device* device, int indices_count, ID3D12GraphicsCommandList* commandList)
{
	//パイプラインステートとルートシグネチャの設定コマンド
	commandList->SetPipelineState(Render_basic::GetInstance()->GetPipelineState());
	commandList->SetGraphicsRootSignature(Render_basic::GetInstance()->GetRootSignature());

	// プリミティブ形状の設定コマンド
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST); // 三角形リスト

	// 定数バッファビュー(CBV)の設定コマンド
	commandList->SetGraphicsRootConstantBufferView(0, constBuffMaterial.Get()->GetGPUVirtualAddress());

	// SRVヒープの設定コマンド
	commandList->SetDescriptorHeaps(1, srvHeap.GetAddressOf());

	// SRVヒープの先頭ハンドルを取得（SRVを指しているはず


	// SRVヒープの先頭にあるSRVをルートパラメータ1番に設定
	commandList->SetGraphicsRootDescriptorTable(1, srvGpuHandle);

	//全オブジェクトについて処理
	for (int i = 0; i < _countof(object3ds); i++)
	{
		DrawObject3d(&object3ds[i], commandList, vbView, ibView, indices_count);
	}
}

void Mesh::InitializeObject3d(Object3d* object, ID3D12Device* device)
{
	// ヒープ設定
	D3D12_HEAP_PROPERTIES heapProp{};
	heapProp.Type = D3D12_HEAP_TYPE_UPLOAD;                   // GPUへの転送用
	// リソース設定
	D3D12_RESOURCE_DESC redesc{};
	redesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	redesc.Width = (sizeof(ConstBufferDataTransform) + 0xff) & ~0xff;   // 256バイトアラインメント
	redesc.Height = 1;
	redesc.DepthOrArraySize = 1;
	redesc.MipLevels = 1;
	redesc.SampleDesc.Count = 1;
	redesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	// 定数バッファの生成 0番
	result = device->CreateCommittedResource(
		&heapProp, // ヒープ設定
		D3D12_HEAP_FLAG_NONE,
		&redesc, // リソース設定
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&object->constBuffTransform));
	assert(SUCCEEDED(result));

	// 定数バッファのマッピング 0番
	result = object->constBuffTransform->Map(0, nullptr, (void**)&object->constMapTransform); // マッピング
	assert(SUCCEEDED(result));

}

void Mesh::UpdateObject3d(Object3d* object, XMMATRIX& matView, XMMATRIX& matProjection)
{
	XMMATRIX matScale, matRot, matTrans;

	// スケール、回転、平行移動行列の計算
	matScale = XMMatrixScaling(object->scale.x, object->scale.y, object->scale.z);
	matRot = XMMatrixIdentity();
	matRot *= XMMatrixRotationZ(object->rotation.z);
	matRot *= XMMatrixRotationX(object->rotation.x);
	matRot *= XMMatrixRotationY(object->rotation.y);
	matTrans = XMMatrixTranslation(object->position.x, object->position.y, object->position.z);

	//ワールド行列の合成
	object->matWorld = XMMatrixIdentity();//変形をリセット
	object->matWorld *= matScale;
	object->matWorld *= matRot;
	object->matWorld *= matTrans;

	//親オブジェクトがあれば
	if (object->parent != nullptr) {
		//親オブジェクトのワールド行列をかける
		object->matWorld *= object->parent->matWorld;
	}

	object->constMapTransform->mat = object->matWorld * matView * matProjection;
}

void Mesh::DrawObject3d(Object3d* object, ID3D12GraphicsCommandList* commandList, D3D12_VERTEX_BUFFER_VIEW& vdView, D3D12_INDEX_BUFFER_VIEW& ibView, UINT numIndices)
{
	//頂点バッファの設定
	commandList->IASetVertexBuffers(0, 1, &vbView);
	// インデックスバッファビューの設定コマンド
	commandList->IASetIndexBuffer(&ibView);
	// 定数バッファビュー(CBV)の設定コマンド
	commandList->SetGraphicsRootConstantBufferView(2, object->constBuffTransform->GetGPUVirtualAddress());

	// 描画コマンド
	commandList->DrawIndexedInstanced(numIndices, 1, 0, 0, 0);
}

void Mesh::Mesh_InitializeLine_Line(ID3D12Device* device, Vertex2* vertices_,int vertices_count)
{
	//頂点データ全体のサイズ＝頂点データ一つ分のサイズ＊頂点データの要素数
	UINT sizeVB = static_cast<UINT>(sizeof(XMFLOAT3) * vertices_count);


	//頂点バッファの設定
	D3D12_HEAP_PROPERTIES heapProp{};  //ヒープ設定
	heapProp.Type = D3D12_HEAP_TYPE_UPLOAD;  //GPUへの転送用
	//リソース設定
	D3D12_RESOURCE_DESC resDesc{};
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resDesc.Width = sizeVB; //頂点データ全体のサイズ
	resDesc.Height = 1;
	resDesc.DepthOrArraySize = 1;
	resDesc.MipLevels = 1;
	resDesc.SampleDesc.Count = 1;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	//頂点バッファの生成
	ID3D12Resource* vertBuff = nullptr;

	result = device->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vertBuff));

	assert(SUCCEEDED(result));


	//GPU上のバッファに対応した仮想メモリ（メインメモリ上）を取得....1
	XMFLOAT3* vertMap = nullptr;
	result = vertBuff->Map(0, nullptr, (void**)&vertMap);
	assert(SUCCEEDED(result));

	//全頂点に対して
	for (int i = 0; i < vertices_count; i++) {
		vertMap[i] = vertices_[i].pos;  //座標をコピー
	}


	//繋がりを解除
	vertBuff->Unmap(0, nullptr);

	//頂点バッファビューの作成
	D3D12_VERTEX_BUFFER_VIEW vbView{};
	//GPU仮想アドレス
	vbView.BufferLocation = vertBuff->GetGPUVirtualAddress();
	//頂点バッファのサイズ
	vbView.SizeInBytes = sizeVB;
	//頂点一つ分のデータサイズ
	vbView.StrideInBytes = sizeof(XMFLOAT3);

	ID3DBlob* vsBlob = nullptr; // 頂点シェーダオブジェクト
	ID3DBlob* psBlob = nullptr; // ピクセルシェーダオブジェクト
	ID3DBlob* errorBlob = nullptr; // エラーオブジェクト

	

}

void Mesh::Mesh_Draw_Line(int indices_count, ID3D12GraphicsCommandList* commandList)
{
	//パイプラインステートとルートシグネチャの設定コマンド
	commandList->SetPipelineState(Render_basic::GetInstance()->GetPipelineState());
	commandList->SetGraphicsRootSignature(Render_basic::GetInstance()->GetRootSignature());

	// プリミティブ形状の設定コマンド
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST); // ラインリスト

	// 定数バッファビュー(CBV)の設定コマンド
	commandList->SetGraphicsRootConstantBufferView(0, constBuffMaterial.Get()->GetGPUVirtualAddress());

	// SRVヒープの設定コマンド
	commandList->SetDescriptorHeaps(1, srvHeap.GetAddressOf());

	// SRVヒープの先頭ハンドルを取得（SRVを指しているはず


	// SRVヒープの先頭にあるSRVをルートパラメータ1番に設定
	commandList->SetGraphicsRootDescriptorTable(1, srvGpuHandle);

	//頂点バッファの設定
	commandList->IASetVertexBuffers(0, 1, &vbView);

	// インデックスバッファビューの設定コマンド
	commandList->IASetIndexBuffer(&ibView);


	// 描画コマンド
	commandList->DrawIndexedInstanced(indices_count, 1, 0, 0, 0);
}
