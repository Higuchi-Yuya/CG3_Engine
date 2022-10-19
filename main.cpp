#pragma region 読み込むヘッダー



#pragma comment(lib,"d3dcompiler.lib")


#include<iostream>

#include <math.h>
#include "Line.h"
#include "Mesh.h"

#include "Render_basic.h"
#include "Input.h"
#include "WinApp.h"
#include "DirectXCommon.h"
using namespace Microsoft::WRL;
using namespace std;


#pragma endregion
const float PI = 3.141592f;

#pragma region おまじない
// @brief コンソール画面にフォーマット付き文字列の表示
// @param format フォーマット(%dとか%fとかの)
// @param 可変長引数
// @remarks この関数はデバック用です。デバッグ時にしか動作しません
void DebugOutputFormatString(const char* format, ...) {
#ifdef _DEBUG
	va_list valist;
	va_start(valist, format);
	vprintf(format, valist);
	va_end(valist);
#endif
}


#pragma endregion

int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR,  _In_ int) {

#pragma region WindowsAPI初期化処理
	// ポインタ
	WinApp* winApp = nullptr;

	// WindouwsAPIの初期化
	winApp = new WinApp();
	winApp->Initialize();

#pragma endregion

#pragma region DirectX初期化処理

	////////////////////////////////////////////////////
	//-------------DirectX12初期化処理ここから-----------//
	//////////////////////////////////////////////////

	// Commonポインタ
	DirectXCommon* dxCommon = nullptr;

	// DirectXの初期化
	dxCommon = new DirectXCommon();
	dxCommon->Initalize(winApp);

	// ポインタ
	Input* input = nullptr;

	// 入力の初期化
	input = new Input();
	input->Initialize(winApp);

	/////////////////////////////////////////////////////////
	//--------------DirectX12初期化処理　ここまで-------------//
	///////////////////////////////////////////////////////
#pragma endregion

	Render_basic::GetInstance()->Initialization(dxCommon->GetDevice());
	
	Render_basic::GetInstance()->Initialization2(dxCommon->GetDevice());

	Mesh mesh[10];

	Line line[20];

	Mesh::Vertex vertices[] = {
		// x      y      z       u      v
		//前
		{{-5.0f, -5.0f, -5.0f}, {},{0.0f, 1.0f}}, // 左下
		{{-5.0f,  5.0f, -5.0f}, {},{0.0f, 0.0f}}, // 左上
		{{ 5.0f, -5.0f, -5.0f}, {},{1.0f, 1.0f}}, // 右下
		{{ 5.0f,  5.0f, -5.0f}, {},{1.0f, 0.0f}}, // 右上
	
		//後
		{{-5.0f, -5.0f,  5.0f}, {},{0.0f, 1.0f}}, // 左下
		{{-5.0f,  5.0f,  5.0f}, {},{0.0f, 0.0f}}, // 左上
		{{ 5.0f, -5.0f,  5.0f}, {},{1.0f, 1.0f}}, // 右下
		{{ 5.0f,  5.0f,  5.0f}, {}, {1.0f, 0.0f}}, // 右上

		//左
		{{-5.0f, -5.0f, -5.0f}, {},{0.0f, 1.0f}}, // 左下
		{{-5.0f, -5.0f,  5.0f}, {},{0.0f, 0.0f}}, // 左上
		{{-5.0f,  5.0f, -5.0f}, {},{1.0f, 1.0f}}, // 右下
		{{-5.0f,  5.0f,  5.0f}, {},{1.0f, 0.0f}}, // 右上

		//右
		{{ 5.0f, -5.0f, -5.0f}, {},{0.0f, 1.0f}}, // 左下
		{{ 5.0f, -5.0f,  5.0f}, {},{0.0f, 0.0f}}, // 左上
		{{ 5.0f,  5.0f, -5.0f}, {},{1.0f, 1.0f}}, // 右下
		{{ 5.0f,  5.0f,  5.0f}, {},{1.0f, 0.0f}}, // 右上

		//下
		{{-5.0f, -5.0f, -5.0f}, {},{0.0f, 1.0f}}, // 左下
		{{-5.0f, -5.0f,  5.0f}, {},{0.0f, 0.0f}}, // 左上
		{{ 5.0f, -5.0f, -5.0f}, {},{1.0f, 1.0f}}, // 右下
		{{ 5.0f, -5.0f,  5.0f}, {},{1.0f, 0.0f}}, // 右上

		//上
		{{-5.0f,  5.0f, -5.0f}, {},{0.0f, 1.0f}}, // 左下
		{{-5.0f,  5.0f,  5.0f}, {},{0.0f, 0.0f}}, // 左上
		{{ 5.0f,  5.0f, -5.0f}, {},{1.0f, 1.0f}}, // 右下
		{{ 5.0f,  5.0f,  5.0f}, {},{1.0f, 0.0f}}, // 右上
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
		//前
		0, 1, 2, // 三角形1つ目
		2, 1, 3, // 三角形2つ目

		//後
		6,5,4,
		7,5,6,

		////左
		8,9,10,
		10,9,11,

		//右
		12,14,13,
		13,14,15,

		//下
		18,17,16,
		19,17,18,

		//上
		20,21,22,
		22,21,23,


	};

	unsigned short indices2[] = {
		0,1
	};

	for (int i = 0; i < _countof(indices) / 3; i++)
	{// 三角形一つごとに計算していく
		// 三角形のインデックスを取り出して、一時的な変数に入れる
		unsigned short indices_1 = indices[i * 3 + 0];
		unsigned short indices_2 = indices[i * 3 + 1];
		unsigned short indices_3 = indices[i * 3 + 2];
		//三角形を構成する頂点座標をベクトルに代入
		XMVECTOR p0 = XMLoadFloat3(&vertices[indices_1].pos);
		XMVECTOR p1 = XMLoadFloat3(&vertices[indices_2].pos);
		XMVECTOR p2 = XMLoadFloat3(&vertices[indices_3].pos);
		// ｐ０ー＞ｐ１ベクトル、ｐ０ー＞ｐ２ベクトルを計算　（ベクトルの減算）
		XMVECTOR v1 = XMVectorSubtract(p1, p0);
		XMVECTOR v2 = XMVectorSubtract(p2, p0);

		//外積は両方から垂直なベクトル
		XMVECTOR normal = XMVector3Cross(v1, v2);

		//正規化（長さを１にする）
		normal = XMVector3Normalize(normal);

		//求めた法線を頂点データに代入
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

	mesh[0].Mesh_Initialization(dxCommon->GetDevice(), vertices, indices, _countof(vertices), indices_count);

#pragma region グリット線の設定
	line[0].Line_Initialize(dxCommon->GetDevice(), line_vertices1, indices2, vertices_count2, indices_count2);
	vertices_count2 = _countof(line_vertices2);
	line[1].Line_Initialize(dxCommon->GetDevice(), line_vertices2, indices2, vertices_count2, indices_count2);
	vertices_count2 = _countof(line_vertices3);
	line[2].Line_Initialize(dxCommon->GetDevice(), line_vertices3, indices2, vertices_count2, indices_count2);
	vertices_count2 = _countof(line_vertices4);
	line[3].Line_Initialize(dxCommon->GetDevice(), line_vertices4, indices2, vertices_count2, indices_count2);
	vertices_count2 = _countof(line_vertices5);
	line[4].Line_Initialize(dxCommon->GetDevice(), line_vertices5, indices2, vertices_count2, indices_count2);
	vertices_count2 = _countof(line_vertices6);
	line[5].Line_Initialize(dxCommon->GetDevice(), line_vertices6, indices2, vertices_count2, indices_count2);
	vertices_count2 = _countof(line_vertices7);
	line[6].Line_Initialize(dxCommon->GetDevice(), line_vertices7, indices2, vertices_count2, indices_count2);
	vertices_count2 = _countof(line_vertices8);
	line[7].Line_Initialize(dxCommon->GetDevice(), line_vertices8, indices2, vertices_count2, indices_count2);
	vertices_count2 = _countof(line_vertices9);
	line[8].Line_Initialize(dxCommon->GetDevice(), line_vertices9, indices2, vertices_count2, indices_count2);
	vertices_count2 = _countof(line_vertices10);
	line[9].Line_Initialize(dxCommon->GetDevice(), line_vertices10, indices2, vertices_count2, indices_count2);
	vertices_count2 = _countof(line_vertices11);
	line[10].Line_Initialize(dxCommon->GetDevice(), line_vertices11, indices2, vertices_count2, indices_count2);
	vertices_count2 = _countof(line_vertices12);
	line[11].Line_Initialize(dxCommon->GetDevice(), line_vertices12, indices2, vertices_count2, indices_count2);
	vertices_count2 = _countof(line_vertices13);
	line[12].Line_Initialize(dxCommon->GetDevice(), line_vertices13, indices2, vertices_count2, indices_count2);
	vertices_count2 = _countof(line_vertices14);
	line[13].Line_Initialize(dxCommon->GetDevice(), line_vertices14, indices2, vertices_count2, indices_count2);
	vertices_count2 = _countof(line_vertices15);
	line[14].Line_Initialize(dxCommon->GetDevice(), line_vertices15, indices2, vertices_count2, indices_count2);
	vertices_count2 = _countof(line_vertices16);
	line[15].Line_Initialize(dxCommon->GetDevice(), line_vertices16, indices2, vertices_count2, indices_count2);
	vertices_count2 = _countof(line_vertices17);
	line[16].Line_Initialize(dxCommon->GetDevice(), line_vertices17, indices2, vertices_count2, indices_count2);
	vertices_count2 = _countof(line_vertices18);
	line[17].Line_Initialize(dxCommon->GetDevice(), line_vertices18, indices2, vertices_count2, indices_count2);
	vertices_count2 = _countof(line_vertices19);
	line[18].Line_Initialize(dxCommon->GetDevice(), line_vertices19, indices2, vertices_count2, indices_count2);
	vertices_count2 = _countof(line_vertices20);
	line[19].Line_Initialize(dxCommon->GetDevice(), line_vertices20, indices2, vertices_count2, indices_count2);
#pragma endregion
	// グリット線の初期設定

	
	//ゲームループ
	while (true) {
#pragma region ウィンドウメッセージ処理

		// Windowsのメッセージ処理
		if (winApp->ProcessMessage()) {
			// ゲームループを抜ける
			break;
		}

#pragma endregion

#pragma region DirectX毎フレーム処理
		/////////////////////////////////////////////////////
		//----------DireceX毎フレーム処理　ここから------------//
		///////////////////////////////////////////////////

		// 入力の更新
		input->Update();
		
		// 数字の0キーが押されていたら
		if (input->PushKey(DIK_0))
		{
			OutputDebugStringA("Hit 0\n");  // 出力ウィンドウに「Hit 0」と表示
		}

		

		

		//2.描画先の変更
		//レンダーターゲットビューのハンドルを取得
		
		

		

		

		////トリガー処理
		//bool キーを押した状態か(uint8_t キー番号);
		//bool キーを離した状態か(uint8_t キー番号);
		//bool キーを押した瞬間か(uint8_t キー番号);
		//bool キーを離した瞬間か(uint8_t キー番号);




		mesh->Mesh_Update(input);

		for (int i = 0; i < 20; i++)
		{
			line[i].Line_Updata();
		}
		
		
		/*transformX = 0.0f;
		transformY = 0.0f;
		rotation = 0.0f;
		scale = 1.0f;*/

		// キー入力

		////平行移動
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

		//// 拡大縮小
		//if (key[DIK_Z]) {
		//	scale -= 0.1f;
		//}

		//if (key[DIK_C]) {
		//	scale += 0.1f;
		//}


		//// 回転
		//if (key[DIK_Q]) {
		//	rotation -= PI / 32;
		//}

		//if (key[DIK_E]) {
		//	rotation += PI / 32;
		//}


		//// アフィン行列の生成
		//affin[0][0] = scale * cos(rotation);
		//affin[0][1] = scale * (-sin(rotation));
		//affin[0][2] = transformX;

		//affin[1][0] = scale * sin(rotation);
		//affin[1][1] = scale * cos(rotation);
		//affin[1][2] = transformY;

		//affin[2][0] = 0.0f;
		//affin[2][1] = 0.0f;
		//affin[2][2] = 1.0f;

		//// アフィン変換
		//for (int i = 0; i < _countof(vertices); i++) {
		//	vertices[i].x = vertices[i].x * affin[0][0] +
		//		vertices[i].y * affin[0][1] + 1.0f * affin[0][2];
		//	vertices[i].y = vertices[i].x * affin[1][0] +
		//		vertices[i].y * affin[1][1] + 1.0f * affin[1][2];
		//	vertices[i].z = vertices[i].x * affin[2][0] +
		//		vertices[i].y * affin[2][1] + 1.0f * affin[2][2];
		//}


		//////////////////////////////////////////////
		//-------DireceX毎フレーム処理　ここまで--------//
		////////////////////////////////////////////

#pragma endregion

#pragma region グラフィックスコマンド

		//4.描画コマンドここから
		dxCommon->PreDraw();
		//Meshの描画--------------------------------------------------------------//
		
		mesh[0].Mesh_Draw(dxCommon->GetDevice(), indices_count, dxCommon->GetCommandList());
		
		for (int i = 0; i < 20; i++)
		{
			line[i].Line_Draw(indices_count2, dxCommon->GetCommandList());
		}
		//4.描画コマンドここまで

#pragma endregion

#pragma region 画面入れ替え
		dxCommon->PostDraw();
#pragma endregion
	}
#pragma region  WindowsAPI後始末

	//もうクラスは使わないので登録を解除する

	// 入力解放
	delete input;

	// WindouwsAPIの終了処理
	winApp->Finalize();

	// WindouwsAPI解放
	delete winApp;

	// DirectXcommonの解放
	delete dxCommon;
#pragma endregion

	//Render_basic::DestroyInstance();

	return 0;
}
