#pragma once

#include "directxmath.h"
#include<vector>

using namespace DirectX;

XMFLOAT4 MulVec4Matrix4x4(XMFLOAT4 f, XMMATRIX m);

// a - b
XMFLOAT3 SubtractFloat3(XMFLOAT3 a, XMFLOAT3 b);

XMFLOAT4 SubtractFloat4(XMFLOAT4 a, XMFLOAT4 b);

float DotFloat3(XMFLOAT3 a, XMFLOAT3 b);

float DotFloat4(XMFLOAT4 a, XMFLOAT4 b);

XMFLOAT3 CrossFloat3(XMFLOAT3 a, XMFLOAT3 b);

float detFloat2(XMFLOAT2 a, XMFLOAT2 b);

float LengthFloat3(XMFLOAT3 a);

float LengthFloat4(XMFLOAT4 a);

void SetRowMatrix(XMMATRIX& m, int r, XMFLOAT3 v);

void SetColMatrix(XMMATRIX& m, int c, XMFLOAT3 v);

void SetColMatrix(XMMATRIX& m, int c, XMFLOAT4 v);

XMMATRIX inverseOfIsometry(XMMATRIX m);

float AngelFloat3(XMFLOAT3 a, XMFLOAT3 b);

float Angle(XMFLOAT3 a, XMFLOAT3 b);

XMFLOAT3 MultiplyFloat3Float(XMFLOAT3 v, float f);

XMFLOAT4 MultiplyFloat4Float(XMFLOAT4 v, float f);

// replaces += operatorn
void AddToFloat3(XMFLOAT3& v, XMFLOAT3 v2);

XMFLOAT3 AdditionFloat3(XMFLOAT3 v, XMFLOAT3 v2);

XMFLOAT4 AdditionFloat4(XMFLOAT4 v, XMFLOAT4 v2);

void divFloat3float(XMFLOAT3& v, float f);

void divFloat4float(XMFLOAT4& v, float f);

XMFLOAT2 SubtractFloat2(XMFLOAT2 a, XMFLOAT2 b);

XMFLOAT3 Normalize(XMFLOAT3 v);

bool LessThanFloat3(XMFLOAT3 a, XMFLOAT3 b);

bool GreaterThanFloat3(XMFLOAT3 a, XMFLOAT3 b);

bool LessThanFloat2(XMFLOAT2 a, XMFLOAT2 b);

bool GreaterThanFloat2(XMFLOAT2 a, XMFLOAT2 b);

void FromEulerAngles(XMMATRIX& m, float alpha, float beta, float gamma);

// Not sure if this is whats happening here
XMFLOAT4 Hamilton(XMFLOAT4 a, XMFLOAT4 b);

void MatrixToQuaternion(const XMMATRIX& m, XMFLOAT4& q);