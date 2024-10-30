struct VertexIn{
    float3 Pos      : POSITION;
    float3 Norm     : NORMAL;
    float2 Tex      : TEXCOORD;
};

struct VertexOut{
    float4 Pos      : SV_POSITION;
    float3 Norm     : NORMAL;
    float2 Tex      : TEXCOORD;
};

cbuffer cbWorld : register(b0)
{
    float4x4 viewMat;
    float4x4 projMat;
}

cbuffer cbObject : register(b1)
{
    float4x4 worldTransform;
}

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

    float4x4 MV = mul(viewMat, worldTransform);

    float4x4 MVP = mul(projMat, MV);

    float4 newVert = mul(MVP, float4(vIn.Pos, 1.f));

    vOut.Pos = newVert;
    vOut.Tex = vIn.Tex;

    return vOut;
}

float4 psMain(VertexOut pIn) : SV_TARGET
{
    float4 textureSample = gTexture.Sample(gsamPointWrap, pIn.Tex);

    return textureSample;
    //return float4(1, 1, 1, 1);
}