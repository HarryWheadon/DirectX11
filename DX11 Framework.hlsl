//--------------------------------------------------------------------------------------
// File: DX11 Framework.hlsl
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------

//Shader Variables
Texture2D texDiffuse : register(t0);
SamplerState sampLinear : register(s0);

cbuffer ConstantBuffer : register( b0 )
{
	matrix World;
	matrix View;
	matrix Projection;

    float4 AmbLight;
    float4 AmbMaterial;

    float4 DiffuseMaterial;
    float4 DiffuseLight;
    
    float4 SpecularMaterial;
    float4 SpecularLight;
    float SpecularPower;
    float3 EyeWorldPos;
   
    float3 directionToLight;
}

//--------------------------------------------------------------------------------------
struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
    float3 Normal : NORMAL;
    float3 PosW : POSITION0;
    float2 TexCoord : TEXCOORD0;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT VS( float3 Pos : POSITION, float3 Normal : NORMAL, float2 TexCoord : TEXCOORD)
{
    float4 pos4 = float4(Pos, 1.0f);
    
    VS_OUTPUT output = (VS_OUTPUT)0;
    
    output.Pos = mul( pos4, World);
    output.PosW = output.Pos;
    output.Pos = mul( output.Pos, View );
    output.Pos = mul( output.Pos, Projection );
    
    float3 NormalW = mul(float4(Normal, 0), World);
    NormalW = normalize(NormalW);
    output.Normal = NormalW;
    output.TexCoord = TexCoord;
    
    return output;
}
//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS(VS_OUTPUT input) : SV_Target
{
    input.Normal = normalize(input.Normal);
    float3 ViewerDir = normalize(EyeWorldPos - input.PosW.xyz);
    
    //Diffuse Light
    float DiffuseAmount = max(dot(normalize(directionToLight), input.Normal), 0);
    float4 diffuse = (DiffuseAmount * (DiffuseMaterial * DiffuseLight));
    
    //Ambient Light
    float4 ambient = (AmbLight * AmbMaterial);
    
    //Specular Light
    float ReflectDir = reflect(-normalize(directionToLight), input.Normal);
    float SpecularAmount = pow(max(dot(ReflectDir, ViewerDir), 0), SpecularPower);
    float4 specular = SpecularAmount * (SpecularMaterial * SpecularLight);
    
    //Add texture colour
    float4 textureColour = texDiffuse.Sample(sampLinear, input.TexCoord);
    float4 TotalColor;
    TotalColor = textureColour * (diffuse + ambient + specular);
    
    return TotalColor;
}
