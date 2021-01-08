


struct VS_INPUT
{
	float3 pos : POSITION;
	float2 uv : TEXCOORD;
	float3 norm : NORMAL;
	float3 tang : TANGENT;
	float3 bitang : BITANGENT;
    //unsure of the semantic 
	int4 boneIndex : BONEINDEX;
	float4 boneWeight : BONEWEIGHT;
	
	float3 vec3deformFactorsTang : DFT;
	float3 vec3deformFactorsBitang : DFB;
	float isTextureFlipped : FLIPPED;
};

cbuffer ConstantBufferTest : register(b0)
{
	float4x4 wvpMat;
	float4x4 normalMatrix;
	float2x4 boneDualQuaternion[32];
};

struct VS_OUTPUT
{
	float4 pos : SV_POSITION;
	float2 texCoord : TEXCOORD;
	float3 normal : NORMAL;
	float3 color : COLOR;
	float3 lightDir : LIGHTDIR;
	float v_norm_err : ERR;
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
	VS_OUTPUT output;
	
	//// test--------------------
	//float2x4 dq_blend = 0.0f;
	//int pivot = input.boneIndex[0];
	//if (pivot != -1)
	//{
	//	float4 q0 = boneDualQuaternion[pivot][0];
	
	//	for (unsigned int i = 0; i < 4; i++)
	//	{
	//		int k = input.boneIndex[i];
	//		float w = input.boneWeight[i];

	//		float2x4 dq = (k == -1) ? float2x4(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f) : boneDualQuaternion[k];
		
	//		if (dot(dq[0], q0) < 0.0f)
	//			w *= -1.0f;
			
	//		dq_blend = dq_blend + dq * w;
	//	}
	//}
	
	//// Compute animated position

	//float4 v0 = normalize(dq_blend[0]);
	//float4 ve = normalize(dq_blend[1]);
	//v0.z = -v0.z;
	//float3 trans = (ve.xyz * v0.w - v0.xyz * ve.w + cross(v0.xyz, ve.xyz)) * 2.0f;
	//output.pos = float4(input.pos + cross(v0.xyz * 2.0f, cross(v0.xyz, input.pos) + input.pos * v0.x) + trans, 1.0f);
	
	////float3 trans = (ve.xyz * v0.w - v0.xyz * ve.w + cross(ve.xyz, v0.xyz)) * 2.0f;
	////output.pos = float4(input.pos + cross(cross(v0.xyz, input.pos) + input.pos * v0.x, v0.xyz * 2.0f) + trans, 1.0f);
	
	//output.pos = mul(output.pos, wvpMat);
	
	
	// Compute animated normal
	
	// test--------------------
	
	
	
	
	
	
	
	
	//// --------------------- SKINNING ---------------------- 
	
	float2x4 dq0 = boneDualQuaternion[input.boneIndex[0]];
	float2x4 dq1 = boneDualQuaternion[input.boneIndex[1]];
	float2x4 dq2 = boneDualQuaternion[input.boneIndex[2]];
	float2x4 dq3 = boneDualQuaternion[input.boneIndex[3]];

    // find "shortest route" before quaternion interplation!
	dq1 = (dot(dq0[0], dq1[0]) < 0) ? -dq1 : dq1;
	dq2 = (dot(dq0[0], dq2[0]) < 0) ? -dq2 : dq2;
	dq3 = (dot(dq0[0], dq3[0]) < 0) ? -dq3 : dq3;

	float2x4 iDualQuat =	dq0 * input.boneWeight[0] +
							dq1 * input.boneWeight[1] +
							dq2 * input.boneWeight[2] +
							dq3 * input.boneWeight[3];

	iDualQuat = normalizedQuaternion(iDualQuat);

	float3 _pos = input.pos + applyDualQuatToPos(iDualQuat, input.pos);
	float3 _tang = input.tang + applyQuatToVec(iDualQuat[0], input.tang);
	float3 _bitang = input.bitang + applyQuatToVec(iDualQuat[0], input.bitang);
	float3 _norm = input.norm + applyQuatToVec(iDualQuat[0], input.norm);

	// --------------------- SKINNING ---------------------- 


	
	
	//output.pos = gl_ModelViewProjectionMatrix * float4(_pos, 1);
	output.pos = mul(float4(_pos, 1), wvpMat);

	float3 lightDir = float3(1.0, 0.0, -1.0); // in VIEW space

 //   // take lightDir to OBJECT space
	////gl_NormalMatrix = transpose(inverse(modelview));
	//lightDir = mul(float4(lightDir.xyz, 0.0f), normalMatrix).xyz;

 //   // take lightDir to TANGENT space
	//lightDir = float3(
 //       dot(lightDir, _tang),
 //       dot(lightDir, _bitang),
 //       dot(lightDir, _norm)
 //   );
	
	output.lightDir = normalize(lightDir);

	output.texCoord = input.uv; /* pass down texture coordintaes */
    
	//output.pos = mul(float4(input.pos, 1.0f), wvpMat);
	output.normal = mul(float4(input.norm, 0.0f), wvpMat).xyz;
	
	
	
	
	
		return output;
	}