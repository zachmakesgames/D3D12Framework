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

// code can be found on pages 123 and 124 of Real-Time Rendering Fourth Edition
float3 lit(float3 l, float3 n, float3 v)
{
    float3 r_l = reflect(-l, n);
    float s = clamp(100.0 * dot(r_l, v) - 97, 0.0, 1.0);
    float3 highlightColor = float3(2,2,2);
    float3 warmColor = float3(1, .9, .9);
    float3 mix = warmColor * (1 - s) + highlightColor * s;
    return mix;

}

float4 psMain(float4 pos: SV_POSITION) : SV_TARGET
{
    
    int3 screenPos = int3(pos.xy, 0);
    float4 material = gBuffer[4].Load(screenPos);
    float4 color = gBuffer[0].Load(screenPos);
    float4 normal = gBuffer[1].Load(screenPos);
    float4 position = gBuffer[2].Load(screenPos);

    // Using the x value in the first material texture to mask out
    // the background from actual objects. Otherwise the clear color
    // gets lit because we're doing a full screen render
    if(material.x < 1)
    {
        discard;
    }

    float3 eyePos = float3(0, 0, 0);


    float4 lightPos = float4(30, 50, 0, 1);
    float4 lightColor = float4(1, 1, 1, 1);


    // code can be found on pages 123 and 124 of Real-Time Rendering Fourth Edition
    float3 n = normalize(normal.xyz);
    float3 v = normalize(eyePos - position.xyz);


    float3 l = normalize(lightPos.xyz - position.xyz);
    float ndl = clamp(dot(n, l), 0.0, 1.0);
    float4 out_color = color;

    out_color.xyz = color.xyz * ndl * lightColor.xyz * lit(l,n,v);

    return out_color;
}