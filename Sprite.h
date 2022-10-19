#pragma once
#include <DirectXMath.h>
#include <Windows.h>
#include <d3d12.h>
#include <string>
#include <wrl.h>

class Sprite
{
public:
	enum class BlendMode {
		kNone,     //!< ブレンドなし
		kNormal,   //!< 通常αブレンド。デフォルト。 Src * SrcA + Dest * (1 - SrcA)
		kAdd,      //!< 加算。Src * SrcA + Dest * 1
		kSubtract, //!< 減算。Dest * 1 - Src * SrcA
		kMultily,  //!< 乗算。Src * 0 + Dest * Src
		kScreen,   //!< スクリーン。Src * (1 - Dest) + Dest * 1

		kCountOfBlendMode, //!< ブレンドモード数。指定はしない
	};
public:// サブクラス
	// 頂点データ構造体
	struct VertexPosUv
	{
		DirectX::XMFLOAT3 pos; // xyz座標
		DirectX::XMFLOAT2 uv;  // uv座標
	};
public: // 静的メンバ関数
	// 静的初期化
	// "device"        デバイス
	// "window_width"  画面の横の幅
	// "window_height" 画面の縦の幅
	static void StaticInitalize(ID3D12Device* device, int widow_width, int window_height, const std::wstring& directoryPath = L"Resources/");

private: // 静的メンバ変数
	// ルートシグネチャ
	static Microsoft::WRL::ComPtr<ID3D12RootSignature> sRootSignature_;
	// パイプラインステートオブジェクト
	static std::array<
		Microsoft::WRL::ComPtr<ID3D12PipelineState>, size_t(BlendMode::kCountOfBlendMode)>
		sPipelineStates_;

};

