#include "ExtraMath.h"

XMFLOAT3 SubtractFloat3(XMFLOAT3 a, XMFLOAT3 b)
{
    float x = a.x - b.x;
    float y = a.y - b.y;
    float z = a.z - b.z;
    return XMFLOAT3(x, y, z);
}

XMFLOAT4 SubtractFloat4(XMFLOAT4 a, XMFLOAT4 b)
{
    float x = a.x - b.x;
    float y = a.y - b.y;
    float z = a.z - b.z;
    float w = a.w - b.w;
    return XMFLOAT4(x, y, z, w);
}

float DotFloat3(XMFLOAT3 a, XMFLOAT3 b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

float DotFloat4(XMFLOAT4 a, XMFLOAT4 b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

XMFLOAT3 CrossFloat3(XMFLOAT3 a, XMFLOAT3 b)
{
    float x = a.y*b.z - a.z*b.y;
    float y = a.z*b.x - a.x*b.z;
    float z = a.x*b.y - a.y*b.x;
    return XMFLOAT3(x, y, z);
}

float detFloat2(XMFLOAT2 a, XMFLOAT2 b)
{
    return a.x*b.y - a.y*b.x;
}

float LengthFloat3(XMFLOAT3 a)
{
    return sqrtf(a.x * a.x + a.y * a.y + a.z * a.z);
}

float LengthFloat4(XMFLOAT4 a)
{
    return sqrtf(a.x * a.x + a.y * a.y + a.z * a.z + a.w * a.w);
}

void SetRowMatrix(XMMATRIX& m, int r, XMFLOAT3 v)
{
    m.r[r].m128_f32[0] = v.x;
    m.r[r].m128_f32[1] = v.y;
    m.r[r].m128_f32[2] = v.z;
}

void SetColMatrix(XMMATRIX& m, int c, XMFLOAT3 v)
{
    m.r[0].m128_f32[c] = v.x;
    m.r[1].m128_f32[c] = v.y;
    m.r[2].m128_f32[c] = v.z;
    //m.r[3].m128_f32[c] = 0.0f;
}

void SetColMatrix(XMMATRIX& m, int c, XMFLOAT4 v)
{
    m.r[0].m128_f32[c] = v.x;
    m.r[1].m128_f32[c] = v.y;
    m.r[2].m128_f32[c] = v.z;
    m.r[3].m128_f32[c] = v.w;
}

XMMATRIX inverseOfIsometry(XMMATRIX m)
{
    XMMATRIX res = m;

    //XMFLOAT4 tra = res.GetColumn4(3);
    XMFLOAT4 tra = XMFLOAT4(m.r[0].m128_f32[3], m.r[1].m128_f32[3], 
        m.r[2].m128_f32[3], m.r[3].m128_f32[3]);

    //res.SetColumn(3, XMFLOAT4(0, 0, 0, 1));
    SetColMatrix(res, 3, XMFLOAT4(0, 0, 0, 1));

    //res.transposeInPlace();
    res = XMMatrixTranspose(res);
    //tra = -(res * tra);
    XMStoreFloat4(&tra, -XMVector3Transform(XMLoadFloat4(&tra), res));
    tra.w = 1;
    //res.SetColumn(3, tra);
    SetColMatrix(res, 3, tra);

    return res;
}

float AngelFloat3(XMFLOAT3 a, XMFLOAT3 b)
{
    float la = LengthFloat3(a);
    float lb = LengthFloat3(b);

    return acos(DotFloat3(XMFLOAT3(a.x / la, a.y / la, a.z / la),
        XMFLOAT3(b.x / lb, b.y / lb, b.z / lb)));
}

XMFLOAT3 MultiplyFloat3Float(XMFLOAT3 v, float f)
{
    return XMFLOAT3(v.x * f, v.y * f, v.z * f);
}

XMFLOAT4 MultiplyFloat4Float(XMFLOAT4 v, float f)
{
    return XMFLOAT4(v.x * f, v.y * f, v.z * f, v.w * f);
}

void AddToFloat3(XMFLOAT3& v, XMFLOAT3 v2)
{
    v.x += v2.x;
    v.y += v2.y;
    v.z += v2.z;
}

XMFLOAT3 AdditionFloat3(XMFLOAT3 v, XMFLOAT3 v2)
{
    float x = v.x + v2.x;
    float y = v.y + v2.y;
    float z = v.z + v2.z;
    return XMFLOAT3(x, y, z);
}

XMFLOAT4 AdditionFloat4(XMFLOAT4 v, XMFLOAT4 v2)
{
    float x = v.x + v2.x;
    float y = v.y + v2.y;
    float z = v.z + v2.z;
    float w = v.w + v2.w;
    return XMFLOAT4(x, y, z, w);
}

void divFloat3float(XMFLOAT3& v, float f)
{
    v.x = v.x / f;
    v.y = v.y / f;
    v.z = v.z / f;
}

void divFloat4float(XMFLOAT4& v, float f)
{
    v.x = v.x / f;
    v.y = v.y / f;
    v.z = v.z / f;
    v.w = v.w / f;
}

XMFLOAT2 SubtractFloat2(XMFLOAT2 a, XMFLOAT2 b)
{
    float x = a.x - b.x;
    float y = a.y - b.y;
    return XMFLOAT2(x, y);
}

XMFLOAT3 Normalize(XMFLOAT3 v)
{
    float n = LengthFloat3(v);
    divFloat3float(v, n);
    return v;
}

bool GreaterThanFloat3(XMFLOAT3 a, XMFLOAT3 b)
{
    return (a.z != b.z) ? (a.z > b.z) : 
           (a.y != b.y) ? (a.y > b.y) : 
           (a.x > b.x);
}

bool LessThanFloat2(XMFLOAT2 a, XMFLOAT2 b)
{
    return (a.y != b.y) ? (a.y < b.y) : 
           (a.x < b.x);
}

bool GreaterThanFloat2(XMFLOAT2 a, XMFLOAT2 b)
{
    return (a.y != b.y) ? (a.y > b.y) :
           (a.x > b.x);
}

void FromEulerAngles(XMMATRIX& m, float alpha, float beta, float gamma)
{
    
   /* for (unsigned int i = 0; i < 4; i++)
    {
        m.r[3].m128_f32[i] = 0;
        m.r[i].m128_f32[3] = 0;
    };*/
    //this->SetZero();
    
    float cosalpha = cos(alpha);
    float cosbeta = cos(beta);
    float cosgamma = cos(gamma);
    float sinalpha = sin(alpha);
    float sinbeta = sin(beta);
    float singamma = sin(gamma);
    
    /// <summary>
    // https://gyazo.com/035de901de67dce991bf8a444543ab70

    /*m.r[2].m128_f32[3] = cosbeta * cosgamma;
    m.r[1].m128_f32[3] = -cosalpha * singamma + sinalpha * sinbeta * cosgamma;
    m.r[0].m128_f32[3] = sinalpha * singamma + cosalpha * sinbeta * cosgamma;

    m.r[2].m128_f32[1] = cosbeta * singamma;
    m.r[1].m128_f32[1] = cosalpha * cosgamma + sinalpha * sinbeta * singamma;
    m.r[0].m128_f32[1] = -sinalpha * cosgamma + cosalpha * sinbeta * singamma;

    m.r[2].m128_f32[0] = -sinbeta;
    m.r[1].m128_f32[0] = sinalpha * cosbeta;
    m.r[0].m128_f32[0] = cosalpha * cosbeta;*/

    /// <param name="gamma"></param>

    m.r[0].m128_f32[0] = cosbeta * cosgamma;
    m.r[1].m128_f32[0] = -cosalpha * singamma + sinalpha * sinbeta * cosgamma;
    m.r[2].m128_f32[0] = sinalpha * singamma + cosalpha * sinbeta * cosgamma;
    
    m.r[0].m128_f32[1] = cosbeta * singamma;
    m.r[1].m128_f32[1] = cosalpha * cosgamma + sinalpha * sinbeta * singamma;
    m.r[2].m128_f32[1] = -sinalpha * cosgamma + cosalpha * sinbeta * singamma;
    
    m.r[0].m128_f32[2] = -sinbeta;
    m.r[1].m128_f32[2] = sinalpha * cosbeta;
    m.r[2].m128_f32[2] = cosalpha * cosbeta;
    
    m.r[3].m128_f32[3] = 1;

}

XMFLOAT4 Hamilton(XMFLOAT4 a, XMFLOAT4 b)
{
    float ww = a.x * b.x - a.y * b.y - a.z * b.z - a.w * b.w;
    float xx = a.x * b.y + a.y * b.x + a.z * b.w - a.w * b.z;
    float yy = a.x * b.z - a.y * b.w + a.z * b.x + a.w * b.y;
    
    a.x = ww;
    a.y = xx;
    a.z = yy;
    a.w = a.x * b.w + a.y * b.z - a.z * b.y + a.w * b.x;
    return a;
}

void MatrixToQuaternion(const XMMATRIX& m, XMFLOAT4& q)
{

    if (m.r[0].m128_f32[0] + m.r[1].m128_f32[1] + m.r[2].m128_f32[2] > 0.0f) {
        float t = m.r[0].m128_f32[0] + m.r[1].m128_f32[1] + m.r[2].m128_f32[2] + 1.0f;
        float s = 0.5f / sqrt(t);
        q.x = s * t;
        q.w = (m.r[1].m128_f32[0] - m.r[0].m128_f32[1]) * s;
        q.z = (m.r[0].m128_f32[2] - m.r[2].m128_f32[0]) * s;
        q.y = (m.r[2].m128_f32[1] - m.r[1].m128_f32[2]) * s;
        
    }
    else if (m.r[0].m128_f32[0] > m.r[1].m128_f32[1] && m.r[0].m128_f32[0] > m.r[2].m128_f32[2]) {
        float t = m.r[0].m128_f32[0] - m.r[1].m128_f32[1] - m.r[2].m128_f32[2] + 1.0f;
        float s = 0.5f / sqrt(t);
        q.y = s * t;
        q.z = (m.r[1].m128_f32[0] + m.r[0].m128_f32[1]) * s;
        q.w = (m.r[0].m128_f32[2] + m.r[2].m128_f32[0]) * s;
        q.x = (m.r[2].m128_f32[1] - m.r[1].m128_f32[2]) * s;
        
    }
    else if (m.r[1].m128_f32[1] > m.r[2].m128_f32[2]) {
        float t = -m.r[0].m128_f32[0] + m.r[1].m128_f32[1] - m.r[2].m128_f32[2] + 1.0f;
        float s = 0.5f / sqrt(t);
        q.z = s * t;
        q.y = (m.r[1].m128_f32[0] + m.r[0].m128_f32[1]) * s;
        q.x = (m.r[0].m128_f32[2] - m.r[2].m128_f32[0]) * s;
        q.w = (m.r[2].m128_f32[1] + m.r[1].m128_f32[2]) * s;
        
    }
    else {
        float t = -m.r[0].m128_f32[0] - m.r[1].m128_f32[1] + m.r[2].m128_f32[2] + 1.0f;
        float s = 0.5f / sqrt(t);
        q.w = s * t;
        q.x = (m.r[1].m128_f32[0] - m.r[0].m128_f32[1]) * s;
        q.y = (m.r[0].m128_f32[2] + m.r[2].m128_f32[0]) * s;
        q.z = (m.r[2].m128_f32[1] + m.r[1].m128_f32[2]) * s;
    }

}

bool LessThanFloat3(XMFLOAT3 a, XMFLOAT3 b)
{
    return (a.z != b.z) ? (a.z < b.z) : 
           (a.y != b.y) ? (a.y < b.y) : 
           (a.x < b.x);
}
