#define NUM_WAVES 64

#define FBM

#ifdef FBM
#define SEED 0
#define SEEDA 1253.2131f
#define FREQ 1.0f
#define FREQM 1.18f
#define AMP 1.0f
#define AMPM 0.82f
#define SPD 2.0f
#define SPDR 1.07f
#define MXP 1.0f
#define OFF 1.0f

#endif


cbuffer MatrixBuffer : register(b0)
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

cbuffer CameraBuffer : register(b2)
{
    float3 cameraPosition;
    float padding;
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
    float3 viewDirection: TEXCOORD0;
};

#ifndef FBM
float Sine(float4 p, Wave w)
{
    float2 point2d = float2(p.x, p.z);
    float2 d = normalize(w.direction);
    return w.fapsProperties.y * sin(dot(d, point2d)*w.fapsProperties.x + w.time.x);
}

float3 CalculateNormal(Wave w, float3 v)
{
    float2 pos = float2(v.x, v.z);
    float2 d = normalize(w.direction);
    float2 n = d * w.fapsProperties.x * w.fapsProperties.y * cos(dot(d, pos) * w.fapsProperties.x + w.time.x);

    return float3(n.x, n.y, 0.0f);
}
#else
float2 CalculateHeight(Wave w, float3 v)
{


}
#endif


PixelInput WaterVertexShader(VertexInput input)
{
    PixelInput output;
    input.position.w = 1.0f;    
    float4 worldPosition = mul(input.position, worldMatrix);

    float height = 0.0f;
    float3 normal = float3(0.0f, 0.0f, 0.0f);

#ifndef FBM
    for (int wi = 0; wi < NUM_WAVES; wi++){

        height += Sine(worldPosition, waves[wi]);
        normal += CalculateNormal(waves[wi], worldPosition);

    }
   // height /= NUM_WAVES;
   // height = clamp(height, -0.5f, 0.5f);

#else
    float ampSum = 0.0f;
    float3 v = worldPosition;
    float a = AMP;
    float f = FREQ;
    float seed = SEED;
    float spd = SPD;

    for (int wi = 0; wi < NUM_WAVES; wi++)
    {
        Wave w = waves[wi];
        float d = normalize(float2(cos(seed), sin(seed)));

        float x = dot(d, v.xz) * f + w.time.x * spd;

        float wave = a * exp(MXP * sin(x) - OFF);
        float dx = MXP * wave * cos(x);

        height += wave;

        v.xz += d * -dx * a;

        ampSum += a;
        f *= FREQM;
        a *= AMPM;
        spd *= SPDR;
        seed += SEEDA;
    }

    height /= ampSum;
        
#endif
    float4 newPos = float4(input.position.x, input.position.y + height, input.position.z, input.position.w);
    output.position = newPos;

    output.position = mul(output.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);

    output.normal = normalize(normal);
    output.worldPosition = mul(newPos, worldMatrix);
    output.viewDirection = normalize(cameraPosition.xyz - worldPosition.xyz);

	return output;
}