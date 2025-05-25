
sampler LinearSampler = sampler_state
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = wrap;
	AddressV = wrap;
};

sampler LinearSampler_Clamp = sampler_state
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = clamp;
    AddressV = clamp;
};

sampler MirrorSampler = sampler_state
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = mirror;
    AddressV = mirror;
};

RasterizerState RS_Default
{
    FillMode = SOLID;
	CullMode = BACK;
};
RasterizerState RS_Wireframe
{
	FillMode = WIREFRAME;
};

RasterizerState RS_Cull_CW
{
	FillMode = SOLID;
	CullMode = FRONT;
};

DepthStencilState DSS_Default
{
	DepthEnable = TRUE;
	DepthWriteMask = ALL;
	DepthFunc = LESS_EQUAL;

};

DepthStencilState DSS_Skip_Z
{
	DepthEnable = FALSE;
	DepthWriteMask = ZERO;		
};

BlendState BS_Default
{
	BlendEnable[0] = FALSE;
};

BlendState BS_AlphaBlend
{
	BlendEnable[0] = TRUE;	

	BlendOp = Add;
	SrcBlend = SRC_ALPHA;
	DestBlend = INV_SRC_ALPHA;

	/*
	SrcColor(rgba), DestColor(rgba)

	SrcColor.rgb * SrcColor.a + DestColor.rgb * (1.f - SrcColor.a)
	*/		
};

BlendState BS_Blend
{
	BlendEnable[0] = TRUE;
	BlendOp[0] = Add;
	SrcBlend[0] = ONE;
	DestBlend[0] = ONE;

	/*
	SrcColor(rgba), DestColor(rgba)

	SrcColor.rgb * 1 + DestColor.rgb * 1


	*/
};
