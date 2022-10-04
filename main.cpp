#pragma region �ǂݍ��ރw�b�_�[



#pragma comment(lib,"d3dcompiler.lib")
#include<Windows.h>
#include <tchar.h>
#include<iostream>

#include<dxgi1_6.h>
#include<cassert>

#pragma comment(lib,"dxgi.lib")

#include <vector>
#include <wrl.h>

#include <math.h>
#include "Line.h"
#include "Mesh.h"

#include "Render_basic.h"
#include "Input.h"

using namespace Microsoft::WRL;
using namespace std;


#pragma endregion
const float PI = 3.141592f;

#pragma region ���܂��Ȃ�
// @brief �R���\�[����ʂɃt�H�[�}�b�g�t��������̕\��
// @param format �t�H�[�}�b�g(%d�Ƃ�%f�Ƃ���)
// @param �ϒ�����
// @remarks ���̊֐��̓f�o�b�N�p�ł��B�f�o�b�O���ɂ������삵�܂���
void DebugOutputFormatString(const char* format, ...) {
#ifdef _DEBUG
	va_list valist;
	va_start(valist, format);
	vprintf(format, valist);
	va_end(valist);
#endif
}

LRESULT WindowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	//�E�B���h�E���j�󂳂ꂽ��Ă΂��
	if (msg == WM_DESTROY) {
		PostQuitMessage(0);//OS�ɑ΂��āu���̃A�v���͂����I���v�Ɠ`����
		return 0;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);
}
#pragma endregion

int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR,  _In_ int) {

#pragma region WindowsAPI����������

	// �E�B���h�E����
	const int window_width = 1280;
	// �E�B���h�E�c��
	const int window_height = 720;


	WNDCLASSEX w = {};
	w.cbSize = sizeof(WNDCLASSEX);
	w.lpfnWndProc = (WNDPROC)WindowProcedure;	//�E�B���h�E�v���V�[�W����ݒ�
	w.lpszClassName = _T("DX12Sample");			//�E�B���h�E�N���X��
	w.hInstance = GetModuleHandle(nullptr);		//�E�B���h�E�n���h��
	w.hCursor = LoadCursor(NULL, IDC_ARROW);	//�J�[�\���w��

	//�E�B���h�E�N���X��OS�ɓo�^����
	RegisterClassEx(&w);
	//�E�B���h�E�T�C�Y{X���W�@Y���W�@�����@�c��}
	RECT wrc = { 0,0,window_width,window_height };
	//�֐����g���ăE�B���h�E�̃T�C�Y�������ŕ␳����
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

	//�E�B���h�E�I�u�W�F�N�g�̐���
	HWND hwnd = CreateWindow(w.lpszClassName,//�N���X���w��
		_T("LE2B_18_�q�O�`_���E��_CG2"),     //�^�C�g���o�[�̕���
		WS_OVERLAPPEDWINDOW,			     //�^�C�g���o�[�Ƌ��E��������E�B���h�E
		CW_USEDEFAULT,					     //�\��x���W��OS�ɂ��C��
		CW_USEDEFAULT,					     //�\��y���W��OS�ɂ��C��
		wrc.right - wrc.left,			     //�E�B���h�E��
		wrc.bottom - wrc.top,			     //�E�B���h�E��
		nullptr,						     //�e�E�B���h�E�n���h��
		nullptr,						     //���j���[�n���h��
		w.hInstance,					     //�Ăяo���A�v���P�[�V�����n���h��
		nullptr);						     //�ǉ��p�����[�^�[(�I�v�V����)

	//�E�B���h�E�\��
	ShowWindow(hwnd, SW_SHOW);

	MSG msg = {};

#pragma endregion

#pragma region DirectX����������

	////////////////////////////////////////////////////
	//-------------DirectX12������������������-----------//
	//////////////////////////////////////////////////

#ifdef _DEBUG
//�f�o�b�O���C���[���I����
	ComPtr<ID3D12Debug> debugController;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
		debugController->EnableDebugLayer();
	}
#endif

	HRESULT result;
	ComPtr<ID3D12Device> device = nullptr;
	ComPtr<IDXGIFactory6> dxgiFactory = nullptr;
	ComPtr<IDXGISwapChain4> swapChain = nullptr;
	ComPtr<ID3D12CommandAllocator> commandAllocator = nullptr;
	ComPtr<ID3D12GraphicsCommandList> commandList = nullptr;
	ComPtr<ID3D12CommandQueue> commandQueue = nullptr;
	ComPtr<ID3D12DescriptorHeap> rtvHeap = nullptr;

	//DXGI�t�@�N�g���[�̐���
	result = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));
	assert(SUCCEEDED(result));

	//�A�_�v�^�[�̗񋓗p
	std::vector<ComPtr<IDXGIAdapter4>>adapters;
	//�����ɓ���̖��O�����A�_�v�^�[�I�u�W�F�N�g������
	IDXGIAdapter4* tmpAdapter = nullptr;

	//�p�t�H�[�}���X�����������珇�ɁA�S�ẴA�_�v�^�[��񋓂���
	for (UINT i = 0; dxgiFactory->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&tmpAdapter)) != DXGI_ERROR_NOT_FOUND; i++) {
		//���I�z��ɒǉ�����
		adapters.push_back(tmpAdapter);
	}
	//�Ó��ȃA�_�v�^��I�ʂ���
	for (size_t i = 0; i < adapters.size(); i++) {
		DXGI_ADAPTER_DESC3 adapterDesc;
		//�A�_�v�^�[�̏����擾����
		adapters[i]->GetDesc3(&adapterDesc);

		//�\�t�g�E�F�A�f�o�C�X�����
		if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)) {
			//�f�o�C�X���̗p���ă��[�v�𔲂���
			tmpAdapter = adapters[i].Get();
			break;
		}
	}

	//�Ή����x���̔z��
	D3D_FEATURE_LEVEL levels[] = {
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
	};

	D3D_FEATURE_LEVEL featureLevel;

	for (size_t i = 0; i < _countof(levels); i++) {
		//�̗p�����A�_�v�^�[�Ńf�o�C�X�𐶐�
		result = D3D12CreateDevice(tmpAdapter, levels[i], IID_PPV_ARGS(&device));
		if (result == S_OK) {
			//�f�o�C�X�𐶐��ł������_�Ń��[�v�𔲂���
			featureLevel = levels[i];
			break;
		}
	}

	//�R�}���h�A���[�P�[�^�𐶐�
	result = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));
	assert(SUCCEEDED(result));

	//�R�}���h���X�g�𐶐�
	result = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList));
	assert(SUCCEEDED(result));

	//�R�}���h�L���[�̐ݒ�
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};

	//�R�}���h�L���[�𐶐�
	result = device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&commandQueue));
	assert(SUCCEEDED(result));

	//�X���b�v�`�F�[���̐ݒ�
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
	swapChainDesc.Width = 1280;
	swapChainDesc.Height = 720;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;			//�F���̏���
	swapChainDesc.SampleDesc.Count = 1;							//�}���`�T���v�����Ȃ�
	swapChainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER;			//�o�b�N�o�b�t�@�p
	swapChainDesc.BufferCount = 2;								//�o�b�t�@����2�ɐݒ�
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;	//�t���b�v��͔j��
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	
	//�X���b�v�`�F�[���̐���
	ComPtr<IDXGISwapChain1> swapChain1;

	result = dxgiFactory->CreateSwapChainForHwnd(
		commandQueue.Get(),
		hwnd, 
		&swapChainDesc, 
		nullptr, 
		nullptr, 
		&swapChain1);

	//������IDXGISwapChain1�̃I�u�W�F�N�g��IDXGISwapChain4�ɕϊ�����
	swapChain1.As(&swapChain);

	assert(SUCCEEDED(result));


	//�f�X�N���v�^�q�[�v�̐ݒ�
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc{};
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;			//�����_�[�^�[�Q�b�g�r���[
	rtvHeapDesc.NumDescriptors = swapChainDesc.BufferCount;		//���\��2��

	//�f�X�N���v�^�q�[�v�̐���
	device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvHeap));

	//�o�b�N�o�b�t�@
	std::vector<ComPtr<ID3D12Resource>>backBuffers;
	backBuffers.resize(swapChainDesc.BufferCount);

	//�X���b�v�`�F�[���̑S�Ẵo�b�t�@�ɂ��ď�������
	for (size_t i = 0; i < backBuffers.size(); i++) {
		//�X���b�v�`�F�[������o�b�t�@���擾
		swapChain->GetBuffer((UINT)i, IID_PPV_ARGS(&backBuffers[i]));
		//�f�X�N���v�^�q�[�v�̃n���h�����擾
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();
		//�����\���ŃA�h���X�������
		rtvHandle.ptr += i * device->GetDescriptorHandleIncrementSize(rtvHeapDesc.Type);
		//�����_�[�^�[�Q�b�g�r���[�̐ݒ�
		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
		//�V�F�[�_�[�̌v�Z���ʂ�SRGB�ɕϊ����ď�������
		rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		//�����_�[�^�[�Q�b�g�r���[�̐���
		device->CreateRenderTargetView(backBuffers[i].Get(), &rtvDesc, rtvHandle);
	}

	//�t�F���X�̐���
	ComPtr<ID3D12Fence> fence = nullptr;
	UINT64 fenceVal = 0;

	result = device->CreateFence(fenceVal, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));

	// �|�C���^
	Input* input = nullptr;

	// ���͂̏�����
	input = new Input();
	input->Initialize(w.hInstance,hwnd);



	/////////////////////////////////////////////////////////
	//--------------DirectX12�����������@�����܂�-------------//
	///////////////////////////////////////////////////////
#pragma endregion

	Render_basic::GetInstance()->Initialization(device.Get());
	
	Render_basic::GetInstance()->Initialization2(device.Get());

	//���\�[�X�ݒ�
	D3D12_RESOURCE_DESC depthResoureDesc{};
	depthResoureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthResoureDesc.Width = window_width;
	depthResoureDesc.Height = window_height;
	depthResoureDesc.DepthOrArraySize = 1;
	depthResoureDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthResoureDesc.SampleDesc.Count = 1;
	depthResoureDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	//�[�x�l�p�q�[�v�v���p�e�B
	D3D12_HEAP_PROPERTIES depthHeapProp{};
	depthHeapProp.Type = D3D12_HEAP_TYPE_DEFAULT;
	//�[�x�l�̃N���A�ݒ�
	D3D12_CLEAR_VALUE depthClearValue{};
	depthClearValue.DepthStencil.Depth = 1.0f;
	depthClearValue.Format = DXGI_FORMAT_D32_FLOAT;

	//���\�[�X����
	ComPtr<ID3D12Resource> depthBuff = nullptr;
	result = device->CreateCommittedResource(
		&depthHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&depthResoureDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&depthClearValue,
		IID_PPV_ARGS(&depthBuff));

	//�[�x�r���[�p�f�X�N���v�^�q�[�v�쐬
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc{};
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	ComPtr<ID3D12DescriptorHeap> dsvHeap = nullptr;
	result = device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&dsvHeap));


	//�[�x�r���[�쐬
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	device->CreateDepthStencilView(
		depthBuff.Get(),
		&dsvDesc,
		dsvHeap->GetCPUDescriptorHandleForHeapStart()
	);

	Mesh mesh[10];

	Line line[20];

	Mesh::Vertex vertices[] = {
		// x      y      z       u      v
		//�O
		{{-5.0f, -5.0f, -5.0f}, {},{0.0f, 1.0f}}, // ����
		{{-5.0f,  5.0f, -5.0f}, {},{0.0f, 0.0f}}, // ����
		{{ 5.0f, -5.0f, -5.0f}, {},{1.0f, 1.0f}}, // �E��
		{{ 5.0f,  5.0f, -5.0f}, {},{1.0f, 0.0f}}, // �E��
	
		//��
		{{-5.0f, -5.0f,  5.0f}, {},{0.0f, 1.0f}}, // ����
		{{-5.0f,  5.0f,  5.0f}, {},{0.0f, 0.0f}}, // ����
		{{ 5.0f, -5.0f,  5.0f}, {},{1.0f, 1.0f}}, // �E��
		{{ 5.0f,  5.0f,  5.0f}, {}, {1.0f, 0.0f}}, // �E��

		//��
		{{-5.0f, -5.0f, -5.0f}, {},{0.0f, 1.0f}}, // ����
		{{-5.0f, -5.0f,  5.0f}, {},{0.0f, 0.0f}}, // ����
		{{-5.0f,  5.0f, -5.0f}, {},{1.0f, 1.0f}}, // �E��
		{{-5.0f,  5.0f,  5.0f}, {},{1.0f, 0.0f}}, // �E��

		//�E
		{{ 5.0f, -5.0f, -5.0f}, {},{0.0f, 1.0f}}, // ����
		{{ 5.0f, -5.0f,  5.0f}, {},{0.0f, 0.0f}}, // ����
		{{ 5.0f,  5.0f, -5.0f}, {},{1.0f, 1.0f}}, // �E��
		{{ 5.0f,  5.0f,  5.0f}, {},{1.0f, 0.0f}}, // �E��

		//��
		{{-5.0f, -5.0f, -5.0f}, {},{0.0f, 1.0f}}, // ����
		{{-5.0f, -5.0f,  5.0f}, {},{0.0f, 0.0f}}, // ����
		{{ 5.0f, -5.0f, -5.0f}, {},{1.0f, 1.0f}}, // �E��
		{{ 5.0f, -5.0f,  5.0f}, {},{1.0f, 0.0f}}, // �E��

		//��
		{{-5.0f,  5.0f, -5.0f}, {},{0.0f, 1.0f}}, // ����
		{{-5.0f,  5.0f,  5.0f}, {},{0.0f, 0.0f}}, // ����
		{{ 5.0f,  5.0f, -5.0f}, {},{1.0f, 1.0f}}, // �E��
		{{ 5.0f,  5.0f,  5.0f}, {},{1.0f, 0.0f}}, // �E��
	};

	Mesh::Vertex line_vertices1[] = {
		{{2.0f,0.0f,18.0f}},
		{{2.0f,0.0f,-18.0f}}
	};
	Mesh::Vertex line_vertices2[] = {
		{{6.0f,0.0f,18.0f}},
		{{6.0f,0.0f,-18.0f}}
	};
	Mesh::Vertex line_vertices3[] = {
		{{10.0f,0.0f,18.0f}},
		{{10.0f,0.0f,-18.0f}},
	};
	Mesh::Vertex line_vertices4[] = {
		{{14.0f,0.0f,18.0f}},
		{{14.0f,0.0f,-18.0f}},
	};
	Mesh::Vertex line_vertices5[] = {
		{{18.0f,0.0f,18.0f}},
		{{18.0f,0.0f,-18.0f}},
	};
	Mesh::Vertex line_vertices6[] = {
		{{-2.0f,0.0f,18.0f}},
		{{-2.0f,0.0f,-18.0f}}
	};
	Mesh::Vertex line_vertices7[] = {
		{{-6.0f,0.0f,18.0f}},
		{{-6.0f,0.0f,-18.0f}},
	};
	Mesh::Vertex line_vertices8[] = {
		{{-10.0f,0.0f,18.0f}},
		{{-10.0f,0.0f,-18.0f}},
	};
	Mesh::Vertex line_vertices9[] = {
		{{-14.0f,0.0f,18.0f}},
		{{-14.0f,0.0f,-18.0f}},
	};
	Mesh::Vertex line_vertices10[] = {
	{{-18.0f,0.0f,18.0f}},
	{{-18.0f,0.0f,-18.0f}},
	};
	Mesh::Vertex line_vertices11[] = {
	{{18.0f,0.0f,2.0f}},
	{{-18.0f,0.0f,2.0f}}

	};
	Mesh::Vertex line_vertices12[] = {
		{{18.0f,0.0f,6.0f}},
		{{-18.0f,0.0f,6.0f}}
	};
	Mesh::Vertex line_vertices13[] = {
		{{18.0f,0.0f,10.0f}},
		{{-18.0f,0.0f,10.0f}},
	};
	Mesh::Vertex line_vertices14[] = {
		{{18.0f,0.0f,14.0f}},
		{{-18.0f,0.0f,14.0f}},
	};
	Mesh::Vertex line_vertices15[] = {
		{{18.0f,0.0f,18.0f}},
		{{-18.0f,0.0f,18.0f}},
	};
	Mesh::Vertex line_vertices16[] = {
		{{18.0f,0.0f,-2.0f}},
		{{-18.0f,0.0f,-2.0f}}
	};
	Mesh::Vertex line_vertices17[] = {
		{{18.0f,0.0f,-6.0f}},
		{{-18.0f,0.0f,-6.0f}},
	};
	Mesh::Vertex line_vertices18[] = {
		{{18.0f,0.0f,-10.0f}},
		{{-18.0f,0.0f,-10.0f}},
	};
	Mesh::Vertex line_vertices19[] = {
		{{18.0f,0.0f,-14.0f}},
		{{-18.0f,0.0f,-14.0f}},
	};
	Mesh::Vertex line_vertices20[] = {
	{{18.0f,0.0f,-18.0f}},
	{{-18.0f,0.0f,-18.0f}},
	};

	unsigned short indices[] = {
		//�O
		0, 1, 2, // �O�p�`1��
		2, 1, 3, // �O�p�`2��

		//��
		6,5,4,
		7,5,6,

		////��
		8,9,10,
		10,9,11,

		//�E
		12,14,13,
		13,14,15,

		//��
		18,17,16,
		19,17,18,

		//��
		20,21,22,
		22,21,23,


	};

	unsigned short indices2[] = {
		0,1
	};

	for (int i = 0; i < _countof(indices) / 3; i++)
	{// �O�p�`����ƂɌv�Z���Ă���
		// �O�p�`�̃C���f�b�N�X�����o���āA�ꎞ�I�ȕϐ��ɓ����
		unsigned short indices_1 = indices[i * 3 + 0];
		unsigned short indices_2 = indices[i * 3 + 1];
		unsigned short indices_3 = indices[i * 3 + 2];
		//�O�p�`���\�����钸�_���W���x�N�g���ɑ��
		XMVECTOR p0 = XMLoadFloat3(&vertices[indices_1].pos);
		XMVECTOR p1 = XMLoadFloat3(&vertices[indices_2].pos);
		XMVECTOR p2 = XMLoadFloat3(&vertices[indices_3].pos);
		// ���O�[�����P�x�N�g���A���O�[�����Q�x�N�g�����v�Z�@�i�x�N�g���̌��Z�j
		XMVECTOR v1 = XMVectorSubtract(p1, p0);
		XMVECTOR v2 = XMVectorSubtract(p2, p0);

		//�O�ς͗������琂���ȃx�N�g��
		XMVECTOR normal = XMVector3Cross(v1, v2);

		//���K���i�������P�ɂ���j
		normal = XMVector3Normalize(normal);

		//���߂��@���𒸓_�f�[�^�ɑ��
		XMStoreFloat3(&vertices[indices_1].normal, normal);
		XMStoreFloat3(&vertices[indices_2].normal, normal);
		XMStoreFloat3(&vertices[indices_3].normal, normal);
	}

	int vertices_count;
	int indices_count;

	int vertices_count2;
	int indices_count2;

	vertices_count = _countof(vertices);
	indices_count = _countof(indices);

	vertices_count2 = _countof(line_vertices1);
	indices_count2 = _countof(indices2);

	mesh[0].Mesh_Initialization(device.Get(), vertices, indices, _countof(vertices), indices_count);

#pragma region �O���b�g���̐ݒ�
	line[0].Line_Initialize(device.Get(), line_vertices1, indices2, vertices_count2, indices_count2);
	vertices_count2 = _countof(line_vertices2);
	line[1].Line_Initialize(device.Get(), line_vertices2, indices2, vertices_count2, indices_count2);
	vertices_count2 = _countof(line_vertices3);
	line[2].Line_Initialize(device.Get(), line_vertices3, indices2, vertices_count2, indices_count2);
	vertices_count2 = _countof(line_vertices4);
	line[3].Line_Initialize(device.Get(), line_vertices4, indices2, vertices_count2, indices_count2);
	vertices_count2 = _countof(line_vertices5);
	line[4].Line_Initialize(device.Get(), line_vertices5, indices2, vertices_count2, indices_count2);
	vertices_count2 = _countof(line_vertices6);
	line[5].Line_Initialize(device.Get(), line_vertices6, indices2, vertices_count2, indices_count2);
	vertices_count2 = _countof(line_vertices7);
	line[6].Line_Initialize(device.Get(), line_vertices7, indices2, vertices_count2, indices_count2);
	vertices_count2 = _countof(line_vertices8);
	line[7].Line_Initialize(device.Get(), line_vertices8, indices2, vertices_count2, indices_count2);
	vertices_count2 = _countof(line_vertices9);
	line[8].Line_Initialize(device.Get(), line_vertices9, indices2, vertices_count2, indices_count2);
	vertices_count2 = _countof(line_vertices10);
	line[9].Line_Initialize(device.Get(), line_vertices10, indices2, vertices_count2, indices_count2);
	vertices_count2 = _countof(line_vertices11);
	line[10].Line_Initialize(device.Get(), line_vertices11, indices2, vertices_count2, indices_count2);
	vertices_count2 = _countof(line_vertices12);
	line[11].Line_Initialize(device.Get(), line_vertices12, indices2, vertices_count2, indices_count2);
	vertices_count2 = _countof(line_vertices13);
	line[12].Line_Initialize(device.Get(), line_vertices13, indices2, vertices_count2, indices_count2);
	vertices_count2 = _countof(line_vertices14);
	line[13].Line_Initialize(device.Get(), line_vertices14, indices2, vertices_count2, indices_count2);
	vertices_count2 = _countof(line_vertices15);
	line[14].Line_Initialize(device.Get(), line_vertices15, indices2, vertices_count2, indices_count2);
	vertices_count2 = _countof(line_vertices16);
	line[15].Line_Initialize(device.Get(), line_vertices16, indices2, vertices_count2, indices_count2);
	vertices_count2 = _countof(line_vertices17);
	line[16].Line_Initialize(device.Get(), line_vertices17, indices2, vertices_count2, indices_count2);
	vertices_count2 = _countof(line_vertices18);
	line[17].Line_Initialize(device.Get(), line_vertices18, indices2, vertices_count2, indices_count2);
	vertices_count2 = _countof(line_vertices19);
	line[18].Line_Initialize(device.Get(), line_vertices19, indices2, vertices_count2, indices_count2);
	vertices_count2 = _countof(line_vertices20);
	line[19].Line_Initialize(device.Get(), line_vertices20, indices2, vertices_count2, indices_count2);
#pragma endregion
	// �O���b�g���̏����ݒ�

	
	//�Q�[�����[�v
	while (true) {
#pragma region �E�B���h�E���b�Z�[�W����

		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		//�A�v���P�[�V�������I��鎞��message��WM_QUIT�ɂȂ�
		if (msg.message == WM_QUIT) {
			break;
		}


#pragma endregion

#pragma region DirectX���t���[������
		/////////////////////////////////////////////////////
		//----------DireceX���t���[�������@��������------------//
		///////////////////////////////////////////////////

		// ���͂̍X�V
		input->Update();
		
		// ������0�L�[��������Ă�����
		if (input->PushKey(DIK_0))
		{
			OutputDebugStringA("Hit 0\n");  // �o�̓E�B���h�E�ɁuHit 0�v�ƕ\��
		}

		//�o�b�N�o�b�t�@�̔ԍ����擾(2�Ȃ̂�0�Ԃ�1��)
		UINT bbIndex = swapChain->GetCurrentBackBufferIndex();

		//1.���\�[�X�o���A�ŏ������݉\�ɕύX
		D3D12_RESOURCE_BARRIER barrierDesc{};
		barrierDesc.Transition.pResource = backBuffers[bbIndex].Get();
		barrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
		barrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
		commandList->ResourceBarrier(1, &barrierDesc);

		//2.�`���̕ύX
		//�����_�[�^�[�Q�b�g�r���[�̃n���h�����擾
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();
		rtvHandle.ptr += static_cast<unsigned long long>(bbIndex) * device->GetDescriptorHandleIncrementSize(rtvHeapDesc.Type);
		D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dsvHeap->GetCPUDescriptorHandleForHeapStart();
		commandList->OMSetRenderTargets(1, &rtvHandle, false, &dsvHandle);

		//3.��ʃN���A			R	  G	   B	A
		FLOAT clearColor[] = { 0.1f,0.25f,0.5f,0.0f };//���ۂ��F
		commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
		if (input->PushKey(DIK_SPACE))     // �X�y�[�X�L�[��������Ă�����
		{
			//��ʃN���A�J���[�̐��l������������
			FLOAT clearColor[] = { 11.1f,0.25f, 0.5f,0.0f }; // �s���N���ۂ��F
			commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
		}

		////�g���K�[����
		//bool �L�[����������Ԃ�(uint8_t �L�[�ԍ�);
		//bool �L�[�𗣂�����Ԃ�(uint8_t �L�[�ԍ�);
		//bool �L�[���������u�Ԃ�(uint8_t �L�[�ԍ�);
		//bool �L�[�𗣂����u�Ԃ�(uint8_t �L�[�ԍ�);




		mesh->Mesh_Update(input);

		for (int i = 0; i < 20; i++)
		{
			line[i].Line_Updata();
		}
		
		
		/*transformX = 0.0f;
		transformY = 0.0f;
		rotation = 0.0f;
		scale = 1.0f;*/

		// �L�[����

		////���s�ړ�
		//if (key[DIK_W]) {
		//	transformY += 0.05f;
		//}

		//if (key[DIK_S]) {
		//	transformY -= 0.05f;
		//}

		//if (key[DIK_A]) {
		//	transformX -= 0.05f;
		//}

		//if (key[DIK_D]) {
		//	transformX += 0.05f;
		//}

		//// �g��k��
		//if (key[DIK_Z]) {
		//	scale -= 0.1f;
		//}

		//if (key[DIK_C]) {
		//	scale += 0.1f;
		//}


		//// ��]
		//if (key[DIK_Q]) {
		//	rotation -= PI / 32;
		//}

		//if (key[DIK_E]) {
		//	rotation += PI / 32;
		//}


		//// �A�t�B���s��̐���
		//affin[0][0] = scale * cos(rotation);
		//affin[0][1] = scale * (-sin(rotation));
		//affin[0][2] = transformX;

		//affin[1][0] = scale * sin(rotation);
		//affin[1][1] = scale * cos(rotation);
		//affin[1][2] = transformY;

		//affin[2][0] = 0.0f;
		//affin[2][1] = 0.0f;
		//affin[2][2] = 1.0f;

		//// �A�t�B���ϊ�
		//for (int i = 0; i < _countof(vertices); i++) {
		//	vertices[i].x = vertices[i].x * affin[0][0] +
		//		vertices[i].y * affin[0][1] + 1.0f * affin[0][2];
		//	vertices[i].y = vertices[i].x * affin[1][0] +
		//		vertices[i].y * affin[1][1] + 1.0f * affin[1][2];
		//	vertices[i].z = vertices[i].x * affin[2][0] +
		//		vertices[i].y * affin[2][1] + 1.0f * affin[2][2];
		//}


		//////////////////////////////////////////////
		//-------DireceX���t���[�������@�����܂�--------//
		////////////////////////////////////////////

#pragma endregion

#pragma region �O���t�B�b�N�X�R�}���h

		//4.�`��R�}���h��������

		// �r���[�|�[�g�ݒ�R�}���h
		D3D12_VIEWPORT viewport{};
		viewport.Width = window_width;   //�悱 �ő�1280
		viewport.Height = window_height;  //���� �ő�720
		viewport.TopLeftX = 0;  //����X
		viewport.TopLeftY = 0;  //����Y
		viewport.MinDepth = 0.0f; //�ŏ��p�x
		viewport.MaxDepth = 1.0f; //�ő�p�x

		// �r���[�|�[�g�ݒ�R�}���h���A�R�}���h���X�g�ɐς�
		commandList->RSSetViewports(1, &viewport);

		//�V�U�[��`
		D3D12_RECT scissorRect{};
		scissorRect.left = 0; // �؂蔲�����W��
		scissorRect.right = scissorRect.left + window_width; // �؂蔲�����W�E
		scissorRect.top = 0; // �؂蔲�����W��
		scissorRect.bottom = scissorRect.top + window_height; // �؂蔲�����W��

		// �V�U�[��`�ݒ�R�}���h���A�R�}���h���X�g�ɐς�
		commandList->RSSetScissorRects(1, &scissorRect);

		//Mesh�̕`��--------------------------------------------------------------//
		commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

		
		mesh[0].Mesh_Draw(device.Get(), indices_count, commandList.Get());
		
		for (int i = 0; i < 20; i++)
		{
			line[i].Line_Draw(indices_count2, commandList.Get());
		}
		//4.�`��R�}���h�����܂�

#pragma endregion

#pragma region ��ʓ���ւ�

		
		//5.���\�[�X�o���A��߂�
		barrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;//�`���Ԃ���
		barrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;//�\����Ԃ�
		commandList->ResourceBarrier(1, &barrierDesc);

		//���߂̃N���[�Y
		result = commandList->Close();
		assert(SUCCEEDED(result));
		//�R�}���h���X�g�̎��s
		ID3D12CommandList* commandLists[] = { commandList.Get()};
		commandQueue->ExecuteCommandLists(1, commandLists);

		//��ʂɕ\������o�b�t�@���t���b�v(���\�̓���ւ�)
		result = swapChain->Present(1, 0);
		assert(SUCCEEDED(result));

		//�R�}���h�̎��s������҂�
		commandQueue->Signal(fence.Get(), ++fenceVal);
		if (fence->GetCompletedValue() != fenceVal) {
			HANDLE event = CreateEvent(nullptr, false, false, nullptr);
			fence->SetEventOnCompletion(fenceVal, event);
			if (event != 0)
			{
				WaitForSingleObject(event, INFINITE);
				CloseHandle(event);
			}
			
		}

		//�L���[���N���A
		result = commandAllocator->Reset();
		assert(SUCCEEDED(result));
		//�ĂуR�}���h���X�g�𒙂߂鏀��
		result = commandList->Reset(commandAllocator.Get(), nullptr);
		assert(SUCCEEDED(result));

#pragma endregion
	}
#pragma region  WindowsAPI��n��

	//�����N���X�͎g��Ȃ��̂œo�^����������
	UnregisterClass(w.lpszClassName, w.hInstance);
	delete input;
#pragma endregion

	Render_basic::DestroyInstance();

	return 0;
}
