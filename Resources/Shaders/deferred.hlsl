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

cbuffer cbWorld : register(b0)
{
    float4x4 viewMat;
    float4x4 projMat;
};

cbuffer cbObject : register(b1)
{
    float4x4 worldTransform;
};

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

Texture2D gTexture : register(t0);

VertexOut vsMain(VertexIn vIn)
{
    VertexOut vOut = (VertexOut)0.0f;

    float4x4 MV = mul(worldTransform, viewMat);

    float4x4 MVP = mul(MV, projMat);

    float4 newVert = mul(float4(vIn.Pos, 1.f), MVP);



    float4 posW = mul(float4(vIn.Pos, 1.0f), worldTransform);
    vOut.PosL = posW.xyz;

    vOut.Norm = mul(vIn.Norm, (float3x3) worldTransform);

    vOut.PosH = newVert;

    vOut.Tex = vIn.Tex;

    return vOut;
}

PixelOut psMain(VertexOut pIn)
{
    PixelOut pOut = (PixelOut)0.0f;
    pOut.Color = gTexture.Sample(gsamPointWrap, pIn.Tex);
    pOut.Normal.xyz = normalize(pIn.Norm);
    pOut.Position.xyz = pIn.PosL;

    return pOut;
}