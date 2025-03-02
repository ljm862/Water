#define NUM_WAVES 4

cbuffer MatrixBuffer : register(b0)
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

struct Wave 
{
    float2 direction;
    float2 origin;
    float4 fapsProperties; // x=frequency, y=amplitude, z=phase, w=steepness 
    float4 time; // x=time, others are padding
};

cbuffer Waves : register(b1)
{
    Wave waves[NUM_WAVES];
};

struct VertexInput
{
    float4 position: POSITION;
    float2 tex: TEXCOORD0;
    float3 normal: NORMAL;
};

struct PixelInput
{
    float4 position: SV_POSITION;
    float4 worldPosition: POSITION;
    float3 normal: NORMAL;
};

float Sine(float4 p, Wave w)
{
    float2 point2d = float2(p.x, p.z);
    return w.fapsProperties.y * sin(dot(w.direction, point2d)*w.fapsProperties.x + w.fapsProperties.z * w.time.x);
}


PixelInput WaterVertexShader(VertexInput input)
{
    PixelInput output;
    input.position.w = 1.0f;

    output.worldPosition = mul(input.position, worldMatrix);

    float height = 0.0f;
    //float3 normal = float3(0.0f, 0.0f, 0.0f);

    for (int wi = 0; wi < NUM_WAVES; wi++){
        height += Sine(input.position, waves[wi]);
    }

    output.position = float4(input.position.x, height, input.position.z, input.position.w);
    output.position = mul(output.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);

    output.normal = input.normal;

	return output;
}