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
		kNone,     //!< �u�����h�Ȃ�
		kNormal,   //!< �ʏ탿�u�����h�B�f�t�H���g�B Src * SrcA + Dest * (1 - SrcA)
		kAdd,      //!< ���Z�BSrc * SrcA + Dest * 1
		kSubtract, //!< ���Z�BDest * 1 - Src * SrcA
		kMultily,  //!< ��Z�BSrc * 0 + Dest * Src
		kScreen,   //!< �X�N���[���BSrc * (1 - Dest) + Dest * 1

		kCountOfBlendMode, //!< �u�����h���[�h���B�w��͂��Ȃ�
	};
public:// �T�u�N���X
	// ���_�f�[�^�\����
	struct VertexPosUv
	{
		DirectX::XMFLOAT3 pos; // xyz���W
		DirectX::XMFLOAT2 uv;  // uv���W
	};
public: // �ÓI�����o�֐�
	// �ÓI������
	// "device"        �f�o�C�X
	// "window_width"  ��ʂ̉��̕�
	// "window_height" ��ʂ̏c�̕�
	static void StaticInitalize(ID3D12Device* device, int widow_width, int window_height, const std::wstring& directoryPath = L"Resources/");

private: // �ÓI�����o�ϐ�
	// ���[�g�V�O�l�`��
	static Microsoft::WRL::ComPtr<ID3D12RootSignature> sRootSignature_;
	// �p�C�v���C���X�e�[�g�I�u�W�F�N�g
	static std::array<
		Microsoft::WRL::ComPtr<ID3D12PipelineState>, size_t(BlendMode::kCountOfBlendMode)>
		sPipelineStates_;

};

