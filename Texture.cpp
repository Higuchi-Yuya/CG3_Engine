#include "Texture.h"


//void Texture::texture_Initialize(DirectX::TexMetadata& metadata, DirectX::ScratchImage& scratchImg,D3D12_HEAP_PROPERTIES& textureHeapProp, ID3D12Device* device)
//{
//
//
//	
//
//	
//	//textureResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
//	//textureResourceDesc.Format = metadata.format;
//	//textureResourceDesc.Width = metadata.width;
//	//textureResourceDesc.Height = (UINT)metadata.height;
//	//textureResourceDesc.DepthOrArraySize = (UINT16)metadata.arraySize;
//	//textureResourceDesc.MipLevels = (UINT16)metadata.mipLevels;
//	//textureResourceDesc.SampleDesc.Count = 1;
//
//	//// テクスチャバッファの生成
//	//result = device->CreateCommittedResource(
//	//	&textureHeapProp,
//	//	D3D12_HEAP_FLAG_NONE,
//	//	&textureResourceDesc,
//	//	D3D12_RESOURCE_STATE_GENERIC_READ,
//	//	nullptr,
//	//	IID_PPV_ARGS(&texBuff));
//	//assert(SUCCEEDED(result));
//
//	//// 全ミップマップについて
//	//for (size_t i = 0; i < metadata.mipLevels; i++) {
//	//	// ミップマップレベルを指定してイメージを取得
//	//	const Image* img = scratchImg.GetImage(i, 0, 0);
//	//	// テクスチャバッファにデータ転送
//	//	result = texBuff->WriteToSubresource(
//	//		(UINT)i,
//	//		nullptr,              // 全領域へコピー
//	//		img->pixels,          // 元データアドレス
//	//		(UINT)img->rowPitch,  // 1ラインサイズ
//	//		(UINT)img->slicePitch // 1枚サイズ
//	//	);
//	//	assert(SUCCEEDED(result));
//	//}
//
//
//}
//
//void Texture::texture_srvInit(D3D12_CPU_DESCRIPTOR_HANDLE& srvHandle, ID3D12Device* device)
//{
//
//
//}
