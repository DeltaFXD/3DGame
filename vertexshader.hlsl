cbuffer cBuffer : register(b0)
{
    float4x4 mat;
};

struct ModeConstant
{
    uint mode;
};

struct VS_INPUT
{
    float3 inPos : POSITION;
    float2 inTexCoord : TEXCOORD;
};

struct VS_OUTPUT
{
    float4 outPosition : SV_POSITION;
    float2 outTexCoord : TEXCOORD;
    uint mode : MODE;
    uint select : SELECTOR;
};

ConstantBuffer<ModeConstant> modeConstant : register(b1, space0);

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;
    
    output.outPosition = mul(float4(input.inPos, 1.0f), mat);
    output.outTexCoord = input.inTexCoord;
    output.mode = modeConstant.mode;
    if (modeConstant.mode == 0)
    {
        if (input.inPos.y > 7.0f)
        {
            output.select = 0;
        }
        else if (input.inPos.y > 2.0f)
        {
            output.select = 1;
        }
        else if (input.inPos.y > -2.5f)
        {
            output.select = 2;
        }
        else
        {
            output.select = 3;
        }
    }
    else
    {
        output.select = 0;   
    }
    
    return output;
}