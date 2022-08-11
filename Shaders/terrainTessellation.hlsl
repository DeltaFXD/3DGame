cbuffer cBuffer : register(b0)
{
    float4x4 world;
    float4x4 viewProj;
    float3 eyePos;
};


struct ModeConstant
{
    uint mode;
};

ConstantBuffer<ModeConstant> modeConstant : register(b1, space0);

struct VS_INPUT
{
    float3 inPos : POSITION;
    float3 inNormal : NORMAL;
    float2 inTexCoord : TEXCOORD;
};

struct VS_OUTPUT
{
    float3 outPosition : POSITION;
    float3 outNormal : NORMAL;
    float2 outTextCoord : TEXCOORD;
};


VS_OUTPUT VS(VS_INPUT input)
{
    VS_OUTPUT output;
    
    output.outPosition = input.inPos;
    output.outTextCoord = input.inTexCoord;
    output.outNormal = input.inNormal;
    
    return output;
}

struct PatchTess
{
    float EdgeTess[4] : SV_TessFactor;
    float InsideTess[2] : SV_InsideTessFactor;
};

PatchTess ConstantHS(InputPatch<VS_OUTPUT, 4> patch, uint patchID : SV_PrimitiveID)
{
    PatchTess pt;

    float3 centerL = 0.25f * (patch[0].outPosition + patch[1].outPosition + patch[2].outPosition + patch[3].outPosition);

    float d = distance(centerL, eyePos);

    // Tessellate the patch based on distance from the eye such that
    // the tessellation is 0 if d >= d1 and 64 if d <= d0.  The interval
    // [d0, d1] defines the range we tessellate in.

    const float d0 = 0.0f;
    const float d1 = 100.0f;
    float tess = 1.0f * saturate((d1 - d) / (d1 - d0));
    tess = max(tess, modeConstant.mode);
    
    // Uniformly tessellate the patch.

    pt.EdgeTess[0] = tess;
    pt.EdgeTess[1] = tess;
    pt.EdgeTess[2] = tess;
    pt.EdgeTess[3] = tess;

    pt.InsideTess[0] = tess;
    pt.InsideTess[1] = tess;

    return pt;
}

struct HS_OUTPUT
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float2 texCoord : TEXCOORD;
};

[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(4)]
[patchconstantfunc("ConstantHS")]
[maxtessfactor(16.0f)]
HS_OUTPUT HS(InputPatch<VS_OUTPUT, 4> p,
           uint i : SV_OutputControlPointID,
           uint patchId : SV_PrimitiveID)
{
    HS_OUTPUT hout;

    hout.pos = p[i].outPosition;
    hout.normal = p[i].outNormal;
    hout.texCoord = p[i].outTextCoord;

    return hout;
}

struct DS_OUTPUT
{
    float4 posH : SV_POSITION;
    float3 normal : NORMAL;
    float2 texCoord : TEXCOORD;
    float col : COLOR;
    uint select : SELECTOR;
};

// The domain shader is called for every vertex created by the tessellator.
// It is like the vertex shader after tessellation.
[domain("quad")]
DS_OUTPUT DS(PatchTess patchTess,
             float2 uv : SV_DomainLocation,
             const OutputPatch<HS_OUTPUT, 4> quad)
{
    DS_OUTPUT dout;

    // Bilinear interpolation.
    float3 v1 = lerp(quad[0].pos, quad[1].pos, uv.x);
    float3 v2 = lerp(quad[2].pos, quad[3].pos, uv.x);
    float3 p = lerp(v1, v2, uv.y);
    
    float3 n1 = lerp(quad[0].normal, quad[1].normal, uv.x);
    float3 n2 = lerp(quad[2].normal, quad[3].normal, uv.x);
    float3 n = lerp(n1, n2, uv.y);
    
    float2 tv1 = lerp(quad[0].texCoord, quad[1].texCoord, uv.x);
    float2 tv2 = lerp(quad[2].texCoord, quad[3].texCoord, uv.x);
    float2 tp = lerp(tv1, tv2, uv.y);
   
    //p.y = 0.3f * (p.z * sin(p.x) + p.x * cos(p.z));
    dout.col = p.y;
    
    float4 posW = mul(float4(p, 1.0f), world);
    dout.posH = mul(posW, viewProj);
    dout.texCoord = tp;
    //TEMP
    dout.normal = n;
    
    if (p.y > 7.0f)
    {
        dout.select = 0;
    }
    else if (p.y > 2.0f)
    {
        dout.select = 1;
    }
    else if (p.y > -2.5f)
    {
        dout.select = 2;
    }
    else
    {
        dout.select = 3;
    }
    
    return dout;
}