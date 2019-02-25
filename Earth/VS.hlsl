struct VS_INPUT
{
	float3 pos : POSITION;
    float2 uv  : TEXCOORD;
	float3 normal: NORMAL;
};

struct VS_OUTPUT
{
	float4 pos: SV_POSITION;
    float2 uv  : TEXCOORD;
	float3 normal: NORMAL;
};

cbuffer ConstantBuffer : register(b0)
{
	float4x4 wvpMat;
	float4 ambientColor;
	float ambientIntensity;
	float3 direction;
};

VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output;
	output.pos = mul(float4(input.pos, 1.0f), wvpMat);
    output.uv = input.uv;
	output.normal = input.normal;
	return output;
}