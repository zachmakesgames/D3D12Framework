struct VertexIn{
    float3 Pos      : POSITION;
    float3 Norm     : NORMAL;
    float2 Tex      : TEXCOORD;
};

struct VertexOut{
    float4 PosH      : SV_POSITION;
    float3 PosL     : POSITION0;
    float3 Norm     : NORMAL;
    float2 Tex      : TEXCOORD;
};

// cbuffer cbWorld : register(b0)
// {
//     float4x4 viewMat;
//     float4x4 projMat;
// };

// cbuffer cbObject : register(b1)
// {
//     float4x4 worldTransform;
// };

struct PixelOut
{
    float4 Color : SV_TARGET0;
    float4 Normal : SV_TARGET1;
    float4 Position : SV_TARGET2;
    float4 ShadowPosition : SV_TARGET3;
    float4 Material1 : SV_TARGET4;
    float4 Material2 : SV_TARGET5;
};

SamplerState gsamPointWrap        : register(s0);
SamplerState gsamPointClamp       : register(s1);
SamplerState gsamLinearWrap       : register(s2);
SamplerState gsamLinearClamp      : register(s3);
SamplerState gsamAnisotropicWrap  : register(s4);
SamplerState gsamAnisotropicClamp : register(s5);
SamplerComparisonState gsamShadow : register(s6);

Texture2D gBuffer[6] : register(t0);

float4 vsMain(VertexIn vIn) :SV_POSITION
{
    return float4(vIn.Pos, 1.f);
}

float4 psMain(float4 pos: SV_POSITION) : SV_TARGET
{
    
    int3 screenPos = int3(pos.xy, 0);
    float4 color = gBuffer[0].Load(screenPos);

    return float4(color.xyz, 1.f);
}