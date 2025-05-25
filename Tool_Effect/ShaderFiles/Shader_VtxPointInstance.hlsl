
#include "../../../EngineSDK/hlsl/Engine_Shader_Defines.hlsli"

float4x4		g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;
Texture2D		g_Texture;

float4			g_vCamPosition;
float3			g_vStartColor;
float3			g_vEndColor;

float			g_fFinalSize;

int3			g_vNumAtlas; // 가로, 세로, 전체 개수
float2			g_vUVInterval;

Texture2D       g_DepthTexture;


struct VS_IN
{
	float3				vPosition : POSITION;	
	float				fPSize : PSIZE;	

	row_major float4x4	TransformMatrix : WORLD;	
	float2				vLifeTime : TEXCOORD0;
};

struct VS_OUT
{
	float4			vPosition : POSITION;
	float			fPSize : PSIZE;
	float2			vLifeTime : TEXCOORD0;
};

VS_OUT VS_MAIN(VS_IN In)
{	
	VS_OUT			Out = (VS_OUT)0;
	
	/* 로컬스페이스내에서 움직임을 가져간다. */
	vector			vPosition = mul(vector(In.vPosition, 1.f), In.TransformMatrix);

	Out.vPosition = mul(vPosition, g_WorldMatrix);
	Out.fPSize = length(In.TransformMatrix._11_12_13);
	Out.vLifeTime = In.vLifeTime;

	return Out;
}
    
struct GS_IN
{
	float4			vPosition : POSITION;
	float			fPSize : PSIZE;
	float2			vLifeTime : TEXCOORD0;
};

struct GS_OUT
{
	float4			vPosition : SV_POSITION;	
	float2			vTexcoord : TEXCOORD0;
	float2			vLifeTime : TEXCOORD1;
    float			fLifeLerp : PSIZE;
};


/* Geometry Shader == 기하셰이더 */
/* */

[maxvertexcount(6)]
void GS_MAIN_ATLAS(point GS_IN In[1], inout TriangleStream<GS_OUT> DataStream)
{
	GS_OUT		Out[4];

    float		fLifeTimeLerp = In[0].vLifeTime.y / In[0].vLifeTime.x;
    float		fSizeLerp = (1 - fLifeTimeLerp) + (fLifeTimeLerp * g_fFinalSize);
    
	float3		vLook = g_vCamPosition.xyz - In[0].vPosition.xyz;
    float3		vRight = normalize(cross(float3(0.f, 1.f, 0.f), vLook)) * (In[0].fPSize * 0.5f * fSizeLerp);
    float3		vUp = normalize(cross(vLook, vRight)) * (In[0].fPSize * 0.5f * fSizeLerp);
    
	float4x4	matVP = mul(g_ViewMatrix, g_ProjMatrix);	
    int			iIndex = fLifeTimeLerp * g_vNumAtlas.z;
    int         iIndexX = iIndex % g_vNumAtlas.x;
    int         iIndexY = iIndex / g_vNumAtlas.x;
    float2      vAtlasUVLT = float2(iIndexX * g_vUVInterval.x, iIndexY * g_vUVInterval.y);
    float2      vAtlasUVRB = float2((iIndexX + 1) * g_vUVInterval.x, (iIndexY + 1) * g_vUVInterval.y);
	
	Out[0].vPosition = float4(In[0].vPosition.xyz + vRight + vUp, 1.f);
	Out[0].vPosition = mul(Out[0].vPosition, matVP);
    Out[0].vTexcoord = float2(vAtlasUVLT.x, vAtlasUVLT.y);
	Out[0].vLifeTime = In[0].vLifeTime;
    Out[0].fLifeLerp = fLifeTimeLerp;

	Out[1].vPosition = float4(In[0].vPosition.xyz - vRight + vUp, 1.f);
	Out[1].vPosition = mul(Out[1].vPosition, matVP);
    Out[1].vTexcoord = float2(vAtlasUVRB.x, vAtlasUVLT.y);
	Out[1].vLifeTime = In[0].vLifeTime;
    Out[1].fLifeLerp = fLifeTimeLerp;

	Out[2].vPosition = float4(In[0].vPosition.xyz - vRight - vUp, 1.f);
	Out[2].vPosition = mul(Out[2].vPosition, matVP);
    Out[2].vTexcoord = float2(vAtlasUVRB.x, vAtlasUVRB.y);
	Out[2].vLifeTime = In[0].vLifeTime;
    Out[2].fLifeLerp = fLifeTimeLerp;

	Out[3].vPosition = float4(In[0].vPosition.xyz + vRight - vUp, 1.f);
	Out[3].vPosition = mul(Out[3].vPosition, matVP);
    Out[3].vTexcoord = float2(vAtlasUVLT.x, vAtlasUVRB.y);
	Out[3].vLifeTime = In[0].vLifeTime;
    Out[3].fLifeLerp = fLifeTimeLerp;
	
	DataStream.Append(Out[0]);
	DataStream.Append(Out[1]);
	DataStream.Append(Out[2]);
	DataStream.RestartStrip();

	DataStream.Append(Out[0]);
	DataStream.Append(Out[2]);
	DataStream.Append(Out[3]);
	DataStream.RestartStrip();
}

[maxvertexcount(6)]
void GS_MAIN(point GS_IN In[1], inout TriangleStream<GS_OUT> DataStream)
{
    GS_OUT Out[4];

    float fLifeTimeLerp = In[0].vLifeTime.y / In[0].vLifeTime.x;
    float fSizeLerp = (1 - fLifeTimeLerp) + (fLifeTimeLerp * g_fFinalSize);
    float3 vLook = g_vCamPosition.xyz - In[0].vPosition.xyz;
    float3 vRight = normalize(cross(float3(0.f, 1.f, 0.f), vLook)) * (In[0].fPSize * 0.5f * fSizeLerp);
    float3 vUp = normalize(cross(vLook, vRight)) * (In[0].fPSize * 0.5f * fSizeLerp);

    float4x4 matVP = mul(g_ViewMatrix, g_ProjMatrix);
	
    Out[0].vPosition = float4(In[0].vPosition.xyz + vRight + vUp, 1.f);
    Out[0].vPosition = mul(Out[0].vPosition, matVP);
    Out[0].vTexcoord = float2(0.f, 0.f);
    Out[0].vLifeTime = In[0].vLifeTime;
    Out[0].fLifeLerp = fLifeTimeLerp;

    Out[1].vPosition = float4(In[0].vPosition.xyz - vRight + vUp, 1.f);
    Out[1].vPosition = mul(Out[1].vPosition, matVP);
    Out[1].vTexcoord = float2(1.f, 0.f);
    Out[1].vLifeTime = In[0].vLifeTime;
    Out[1].fLifeLerp = fLifeTimeLerp;

    Out[2].vPosition = float4(In[0].vPosition.xyz - vRight - vUp, 1.f);
    Out[2].vPosition = mul(Out[2].vPosition, matVP);
    Out[2].vTexcoord = float2(1.f, 1.f);
    Out[2].vLifeTime = In[0].vLifeTime;
    Out[2].fLifeLerp = fLifeTimeLerp;

    Out[3].vPosition = float4(In[0].vPosition.xyz + vRight - vUp, 1.f);
    Out[3].vPosition = mul(Out[3].vPosition, matVP);
    Out[3].vTexcoord = float2(0.f, 1.f);
    Out[3].vLifeTime = In[0].vLifeTime;
    Out[3].fLifeLerp = fLifeTimeLerp;
	
    DataStream.Append(Out[0]);
    DataStream.Append(Out[1]);
    DataStream.Append(Out[2]);
    DataStream.RestartStrip();

    DataStream.Append(Out[0]);
    DataStream.Append(Out[2]);
    DataStream.Append(Out[3]);
    DataStream.RestartStrip();
}

struct GS_OUT_BLEND
{
    float4  vPosition : SV_POSITION;
    float2  vTexcoord : TEXCOORD0;
    float2  vLifeTime : TEXCOORD1;
    float4  vProjPos : TEXCOORD2;
    float   fLifeLerp : PSIZE;
};


[maxvertexcount(6)]
void GS_MAIN_BLEND(point GS_IN In[1], inout TriangleStream<GS_OUT_BLEND> DataStream)
{
    GS_OUT_BLEND Out[4];

    float fLifeTimeLerp = In[0].vLifeTime.y / In[0].vLifeTime.x;
    float fSizeLerp = (1 - fLifeTimeLerp) + (fLifeTimeLerp * g_fFinalSize);
    float3 vLook = g_vCamPosition.xyz - In[0].vPosition.xyz;
    float3 vRight = normalize(cross(float3(0.f, 1.f, 0.f), vLook)) * (In[0].fPSize * 0.5f * fSizeLerp);
    float3 vUp = normalize(cross(vLook, vRight)) * (In[0].fPSize * 0.5f * fSizeLerp);

    float4x4 matVP = mul(g_ViewMatrix, g_ProjMatrix);
	
    int iIndex = fLifeTimeLerp * g_vNumAtlas.z;
    int iIndexX = iIndex % g_vNumAtlas.x;
    int iIndexY = iIndex / g_vNumAtlas.x;
    float2 vAtlasUVLT = float2(iIndexX * g_vUVInterval.x, iIndexY * g_vUVInterval.y);
    float2 vAtlasUVRB = float2((iIndexX + 1) * g_vUVInterval.x, (iIndexY + 1) * g_vUVInterval.y);
	
    Out[0].vPosition = float4(In[0].vPosition.xyz + vRight + vUp, 1.f);
    Out[0].vPosition = mul(Out[0].vPosition, matVP);
    Out[0].vTexcoord = float2(vAtlasUVLT.x, vAtlasUVLT.y);
    Out[0].vLifeTime = In[0].vLifeTime;
    Out[0].fLifeLerp = fLifeTimeLerp;
    Out[0].vProjPos = Out[0].vPosition;
    

    Out[1].vPosition = float4(In[0].vPosition.xyz - vRight + vUp, 1.f);
    Out[1].vPosition = mul(Out[1].vPosition, matVP);
    Out[1].vTexcoord = float2(vAtlasUVRB.x, vAtlasUVLT.y);
    Out[1].vLifeTime = In[0].vLifeTime;
    Out[1].fLifeLerp = fLifeTimeLerp;
    Out[1].vProjPos = Out[1].vPosition;

    Out[2].vPosition = float4(In[0].vPosition.xyz - vRight - vUp, 1.f);
    Out[2].vPosition = mul(Out[2].vPosition, matVP);
    Out[2].vTexcoord = float2(vAtlasUVRB.x, vAtlasUVRB.y);
    Out[2].vLifeTime = In[0].vLifeTime;
    Out[2].fLifeLerp = fLifeTimeLerp;
    Out[2].vProjPos = Out[2].vPosition;

    Out[3].vPosition = float4(In[0].vPosition.xyz + vRight - vUp, 1.f);
    Out[3].vPosition = mul(Out[3].vPosition, matVP);
    Out[3].vTexcoord = float2(vAtlasUVLT.x, vAtlasUVRB.y);
    Out[3].vLifeTime = In[0].vLifeTime;
    Out[3].fLifeLerp = fLifeTimeLerp;
    Out[3].vProjPos = Out[3].vPosition;
	
    DataStream.Append(Out[0]);
    DataStream.Append(Out[1]);
    DataStream.Append(Out[2]);
    DataStream.RestartStrip();

    DataStream.Append(Out[0]);
    DataStream.Append(Out[2]);
    DataStream.Append(Out[3]);
    DataStream.RestartStrip();
}
struct PS_IN
{
	float4			vPosition : SV_POSITION;
	float2			vTexcoord : TEXCOORD0;
	float2			vLifeTime : TEXCOORD1;
    float			fLifeLerp : PSIZE;
};

struct PS_IN_BLEND
{
    float4 vPosition : SV_POSITION;
    float2 vTexcoord : TEXCOORD0;
    float2 vLifeTime : TEXCOORD1;
    float4 vProjPos : TEXCOORD2;
    float fLifeLerp : PSIZE;
};

struct PS_OUT
{
	float4			vColor : SV_TARGET0;
};

PS_OUT PS_MAIN(PS_IN In)
{
	PS_OUT		Out = (PS_OUT)0;

	Out.vColor = g_Texture.Sample(LinearSampler, In.vTexcoord);
    
    float3 vLerp = lerp(g_vStartColor, g_vEndColor, In.fLifeLerp);
    Out.vColor.r = Out.vColor.r * vLerp.r;
    Out.vColor.g = Out.vColor.g * vLerp.g;
    Out.vColor.b = Out.vColor.b * vLerp.b;
    // Out.vColor.a = Out.vColor.a * (1 - In.fLifeLerp);
	
    if (Out.vColor.a <= 0.2f)
        discard;
    
    if (Out.vColor.r == 0.f && Out.vColor.g == 0.f && Out.vColor.b == 0.f)
        discard;
	
    if (In.vLifeTime.y >= In.vLifeTime.x)
        discard;

	return Out;
}

PS_OUT PS_MAIN_BLEND(PS_IN_BLEND In)
{
    PS_OUT Out = (PS_OUT) 0;

    Out.vColor = g_Texture.Sample(LinearSampler, In.vTexcoord);
	    
    float2 vTexcoord;

    vTexcoord.x = (In.vProjPos.x / In.vProjPos.w) * 0.5f + 0.5f;
    vTexcoord.y = (In.vProjPos.y / In.vProjPos.w) * -0.5f + 0.5f;

    vector vDepthDesc = g_DepthTexture.Sample(LinearSampler, vTexcoord);
    
    float3 vLerp = lerp(g_vStartColor, g_vEndColor, In.fLifeLerp);
    Out.vColor.r = Out.vColor.r * vLerp.r;
    Out.vColor.g = Out.vColor.g * vLerp.g;
    Out.vColor.b = Out.vColor.b * vLerp.b;
    Out.vColor.a = Out.vColor.a * (vDepthDesc.y - In.vProjPos.w);
    
    if (Out.vColor.a <= 0.2f)
        discard;
	
    if (In.vLifeTime.y >= In.vLifeTime.x)
        discard;

    return Out;
}


technique11 DefaultTechnique
{
	pass DefaultPass
	{
		SetRasterizerState(RS_Default);
		SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = compile gs_5_0 GS_MAIN();
		PixelShader = compile ps_5_0 PS_MAIN();
	}

    pass AtlasPass
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = compile gs_5_0 GS_MAIN_ATLAS();
        PixelShader = compile ps_5_0 PS_MAIN();
    }

    pass BlendPass
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = compile gs_5_0 GS_MAIN_BLEND();
        PixelShader = compile ps_5_0 PS_MAIN_BLEND();
    }
}
