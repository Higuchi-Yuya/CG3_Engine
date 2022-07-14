#pragma once
#include<d3dcompiler.h>
#include<d3d12.h>
#include <string>


class Render_basic
{
public:
	static Render_basic* GetInstance();
	static void DestroyInstance();
	void Initialization(ID3D12Device* device);
	ID3D12PipelineState*  GetPipelineState()const;
	ID3D12RootSignature* GetRootSignature()const;

private:
	Render_basic() = default;
	~Render_basic() = default;

	Render_basic(const Render_basic&) = delete;
	Render_basic& operator=(const Render_basic&) = delete;
	Render_basic(const Render_basic&&) = delete;
	Render_basic& operator=(const Render_basic&&) = delete;

	HRESULT result;
	// パイプランステートの生成
	ID3D12PipelineState* pipelineStage = nullptr;
	// ルートシグネチャ
	ID3D12RootSignature* rootSignature;
};

