#version 460
#extension GL_ARB_separate_shader_objects: enable

#define PI 3.1415926535

vec3 CalcDirLight(vec3 normal, vec3 viewDir);
vec3 CalcPointLight(vec3 normal, vec3 fragPos, vec3 viewDir);

layout(location=0) centroid in vec2 TexCoord;
layout(location=1) in vec3 fragPos;
layout(location=2) smooth in vec3 fragNormal;
layout(location=3) in vec3 viewPos;
layout(location =4) in vec4 light_space_pos;

layout(location=0) out vec4 outColor;

layout(binding=1) uniform sampler2D texSampler;
layout(binding=2) uniform sampler2D shadowMapSampler;

vec3 lightColor={3.0,3.0,3.0};
vec3 lightDir={-1.0,1.0,1.0};

const float constant=1.0;
const float linear=0.09;
const float quadratic=0.032;

const float near=0.1;
const float far=100.0;

float LinearizeDepth(float depth)
{
	float z=depth*2.0-1.0;
	return (2.0*near*far)/(far+near-z*(far-near));
}

vec3 reinhard(vec3 origin)
{
	return origin/(origin+vec3(1.0));
}

vec3 sigmod(vec3 origin)
{
	return 2*(vec3(1.0)/(vec3(1.0)+exp(-origin))-vec3(0.5));
}

float DistributionGGX(vec3 N,vec3 H,float a)
{
	float a2=a*a;
	float NdotH=max(dot(N,H),0.0);
	float NdotH2=NdotH*NdotH;

	float nom=a2;
	float denom=(NdotH2*(a2-1.0)+1.0);
	denom =PI* denom*denom;

	return nom/denom;
}

float GeometrySchlickGGX(float NdotV,float k)
{
	float nom=NdotV;
	float denom=NdotV*(1.0-k)+k;
	
	return nom/denom;
}

float GeometrySmith(vec3 N,vec3 V,vec3 L,float k)
{
	float NdotV=max(dot(N,V),0.0);
	float NdotL=max(dot(N,L),0.0);
	float ggx1=GeometrySchlickGGX(NdotV,k);
	float ggx2=GeometrySchlickGGX(NdotL,k);

	return ggx1*ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
	return F0+(1.0-F0)*pow(1.0-cosTheta,5.0);
}

void main()
{
	vec3 objectColor=(TexCoord.x==0&&TexCoord.y==0)?vec3(0.3,0.3,0.3):vec3(0.2,0.6,0.6);//vec3(texture(texSampler,TexCoord));

	float shadow = 1;

	vec3 light_space_ndc = light_space_pos.xyz/light_space_pos.w;

	vec2 shadow_map_coord = light_space_ndc.xy * 0.5 + 0.5;

	float closeDepth = texture(shadowMapSampler,shadow_map_coord).x;

	if(light_space_ndc.z > closeDepth)
		shadow = 0.0;

	lightDir=normalize(lightDir);
	//calculate ambient
	float ambientStrength=0.1;
	vec3 ambient = ambientStrength*lightColor*objectColor;
	//calculate diffuse
	vec3 norm=normalize(fragNormal);
	float diff=max(dot(norm,lightDir),0.0);
	vec3 diffuse=diff*lightColor*shadow*objectColor;
	//caculate specular
	vec3 viewDir=normalize(viewPos-fragPos);
	vec3 halfDir=normalize(lightDir+viewDir);
	float alpha=max(dot(viewDir,halfDir),0.0);
	float spec=pow(alpha,32);
	vec3 specular=0.1*spec*lightColor*shadow*objectColor;

	//float dis= length(lightPos-fragPos);
	//float attenuation = 1.0/(constant+linear*dis+quadratic*dis*dis);

	vec3 result=(ambient+diffuse+specular);//*attenuation;
	outColor=vec4(result,1.0);
	//outColor=vec4(norm,1.0);
}

vec3 CalcDirLight(vec3 normal, vec3 viewDir)
{
	return vec3(0.0,0.0,0.0);
}
vec3 CalcPointLight(vec3 normal, vec3 fragPos, vec3 viewDir)
{
	return vec3(0.0,0.0,0.0);
}

