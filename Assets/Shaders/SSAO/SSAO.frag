#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable


layout (location = 0) in vec3 inUV;


layout (binding = 1) uniform highp  sampler2D  positionColor;
layout (binding = 2) uniform highp  sampler2D  packedTexture; //packed normal, diffuse, specular
layout (binding = 3) uniform highp  sampler2D texNoise;
layout (location = 4) out float FragColor;


layout (binding = 0) uniform UBO 
{
	vec3 samples[64];
	mat4 projection;
} ubo;



// parameters (you'd probably want to use them as uniforms to more easily tweak the effect)
int kernelSize = 64;
float radius = 1.0;


// c_precision of 128 fits within 7 base-10 digits
const float c_precision = 128.0;
const float c_precisionp1 = c_precision + 1.0;
/*
	param value 3-component encoded float
	returns normalized RGB value
*/
vec3 float2color(float value) 
{
    vec3 color;
    color.r = mod(value, c_precisionp1) / c_precision;
    color.b = mod(floor(value / c_precisionp1), c_precisionp1) / c_precision;
    color.g = floor(value / (c_precisionp1 * c_precisionp1)) / c_precision;
    return color;
}

//http://stackoverflow.com/questions/4200224/random-noise-functions-for-glsl
float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

// tile noise texture over screen based on screen dimensions divided by noise size
const vec2 noiseScale = vec2(1280.0f/4.0f, 720.0f/4.0f); 

void main()
{
    // Get input for SSAO algorithm
    vec4 fragPos = texture(positionColor, inUV.st).rgba;

	//Get normal
	vec3 packedData = texture(packedTexture, inUV.st).rgb;
    vec3 normal = normalize(float2color(packedData.y));

	float n =  rand( inUV.st * noiseScale);
	vec3 randomVec = vec3(n,n,n);
    //vec3 randomVec = texture(texNoise, inUV.st * noiseScale).xyz;

    // Create TBN change-of-basis matrix: from tangent-space to view-space
    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));

    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);

    // Iterate over the sample kernel and calculate occlusion factor
    float occlusion = 0.0;
    for(int i = 0; i < kernelSize; ++i)
    {
        // get sample position
        vec3 Sample = vec3(TBN * ubo.samples[i]); // From tangent to view-space
        Sample = fragPos.xyz + Sample * radius; 
        
        // project sample position (to sample texture) (to get position on screen/texture)
        vec4 offset = vec4(Sample, 1.0);
        offset = ubo.projection * offset; // from view to clip-space
        offset.xyz /= offset.w; // perspective divide
        offset.xyz = offset.xyz * 0.5 + 0.5; // transform to range 0.0 - 1.0
        
        // get sample depth
        float sampleDepth = -texture(positionColor, offset.xy).w; // Get depth value of kernel sample
        
        // range check & accumulate
       float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth ));
        occlusion += (sampleDepth >= Sample.z ? 1.0 : 0.0) * rangeCheck;           
    }
    occlusion = 1.0 - (occlusion / kernelSize);
    
    FragColor = occlusion;
}