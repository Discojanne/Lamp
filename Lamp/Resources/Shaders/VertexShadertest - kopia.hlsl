

//attribute float3 pos;
//attribute float3 norm;
//attribute float2 uv;
//attribute float3 tang;
//attribute float3 bitang;

struct VS_INPUT
{
	float3 pos : POSITION;
	float3 norm : NORMAL;
	float2 uv : TEXCOORD;
	float3 tang : TANGENT;
	float3 bitang : BITANGENT;
};


attribute float4 boneWeight;
attribute int4 boneIndex;
uniform float2x4 boneDualQuaternion[32];

//varying float3 v_color;
//varying float3 v_norm;
//varying float2 v_uv;
//varying float3 lightDir;
//varying float v_norm_err;
struct VS_OUTPUT
{
	float4 pos : SV_POSITION;
	float2 texCoord : TEXCOORD;
	float3 normal : NORMAL;
	float3 color : COLOR;
	float3 lightDir : TEXCOORD2;
	float v_norm_err;
};


/* rotation of vector by quaternion */
// NB: result must be ADDED to vector being rotated!
float3 applyQuatToVec(float4 quat, float3 vec)
{
    return 2.0 * cross(quat.xyz, cross(quat.xyz, vec) + quat.w * vec);
}


/* roto-translation of position by dual quaternion */
// NB: result must be ADDED to pos being roto-translated!
float3 applyDualQuatToPos(float2x4 dq, float3 pos)
{
    return 2.0 * (
        +cross(dq[0].xyz, cross(dq[0].xyz, pos) + pos * dq[0].w + dq[1].xyz)
        + dq[1].xyz * dq[0].w
        - dq[0].xyz * dq[1].w
        );
}

float2x4 normalizedQuaternion(float2x4 dq)
{
    dq /= length(dq[0]);
    dq[1] -= dot(dq[1], dq[0]) * dq[0];
    return dq;
}


VS_OUTPUT VSmain(VS_INPUT input, uint id : SV_InstanceID)
{
	float2x4 dq0 = boneDualQuaternion[boneIndex[0]];
	float2x4 dq1 = boneDualQuaternion[boneIndex[1]];
	float2x4 dq2 = boneDualQuaternion[boneIndex[2]];
	float2x4 dq3 = boneDualQuaternion[boneIndex[3]];

    // find "shortest route" before quaternion interplation!
    dq1 = (dot(dq0[0], dq1[0]) < 0) ? -dq1 : dq1;
    dq2 = (dot(dq0[0], dq2[0]) < 0) ? -dq2 : dq2;
    dq3 = (dot(dq0[0], dq3[0]) < 0) ? -dq3 : dq3;

	float2x4 iDualQuat = dq0 * boneWeight[0] +
        dq1 * boneWeight[1] +
        dq2 * boneWeight[2] +
        dq3 * boneWeight[3];

    iDualQuat = normalizedQuaternion(iDualQuat);

    float3 _pos = pos + applyDualQuatToPos(iDualQuat, pos);
    float3 _tang = tang + applyQuatToVec(iDualQuat[0], tang);
    float3 _bitang = bitang + applyQuatToVec(iDualQuat[0], bitang);
    float3 _norm = norm + applyQuatToVec(iDualQuat[0], norm);

    gl_Position = gl_ModelViewProjectionMatrix * float4(_pos, 1);


    lightDir = float3(0.0, 0.0, 1.0); // in VIEW space

    // take lightDir to OBJECT space
    lightDir = lightDir * gl_NormalMatrix;

    // take lightDir to TANGENT space
    lightDir = float3(
        dot(lightDir, _tang),
        dot(lightDir, _bitang),
        dot(lightDir, _norm)
    );

    lightDir = normalize(lightDir);

    v_uv = uv; /* pass down texture coordintaes */
}