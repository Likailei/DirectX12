struct VS_OUTPUT
{
	float4 pos: SV_POSITION;
    float2 uv  : TEXCOORD;
	float3 normal: NORMAL;
    uint face : FACE;
};

//cbuffer ConstantBuffer : register(b0)
//{
//	float4x4 wvpMat;
//	float4 ambientColor;
//	float ambientIntensity;
//	float3 direction;
//};

cbuffer CB : register(b0) {
    float4x4 wvpMat;
}

Texture2D tArr[] : register(t0);
SamplerState s1 : register(s0);

float4 main(VS_OUTPUT input) : SV_TARGET
{
    //float4 litColor = float4(0.0f, 0.0f, 0.0f, 0.0f);

    //litColor += float4(1.0f, 1.0f, 1.0f, 1.0f) * ambientColor * ambientIntensity;

    //float t = dot(-direction, input.normal);
    //if (t > 0.0f) {
    //	litColor += t * float4(0.3f, 0.3f, 0.3f, 0.3f);
    //}

    return tArr[input.face].Sample(s1, input.uv);
}