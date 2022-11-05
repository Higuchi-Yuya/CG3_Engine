#pragma once

class Vector3
{
public:
	float x;
	float y;
	float z;
public:
	//コンストラクタ
	Vector3();
	Vector3(float x, float y, float z);

	//メンバ関数
	float length()const;
	Vector3& normalize();
	static float dot(const Vector3& v, const Vector3& v2);
	Vector3 cross(const Vector3& v)const;
	static Vector3 cross2(const Vector3& v, const Vector3& v2);
	Vector3  distanceFrom(const Vector3& v, const Vector3& v2)const;

	//単項演算子オーバーロード
	Vector3 operator+()const;
	Vector3 operator-()const;

	//代入演算子オーバーロード
	Vector3& operator+=(const Vector3& v);
	Vector3& operator-=(const Vector3& v);
	Vector3& operator*=(float s);
	Vector3& operator/=(float s);
};


//2項演算子オーバーロード
//※いろんな引数のパターンに対応(引数の順序)するため、以下のように準備している
const Vector3 operator+(const Vector3& v1, const Vector3& v2);
const Vector3 operator-(const Vector3& v1, const Vector3& v2);
const Vector3 operator*(const Vector3& v, float s);
const Vector3 operator*(float s, const Vector3& v);
const Vector3 operator/(const Vector3& v, float s);




