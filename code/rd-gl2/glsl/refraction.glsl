/*[Vertex]*/
in vec3 attr_Position;
in vec3 attr_Normal;
in vec4 attr_TexCoord0;

uniform mat4 u_ModelViewProjectionMatrix;
uniform mat4 u_ModelMatrix;
uniform vec3 u_ViewOrigin;

uniform vec4 u_PrimaryLightOrigin;
uniform float u_PrimaryLightRadius;

out vec4 var_PrimaryLightDir;

out vec2 var_Tex1;
out vec2 fragpos;
out vec3 normal;
out vec3 position;
out vec3 viewDir;

void main()
{
	gl_Position 	= u_ModelViewProjectionMatrix * vec4(attr_Position, 1.0);
	var_Tex1 		= attr_TexCoord0.st;
	fragpos			= (gl_Position.xy / gl_Position.w) *0.5 + 0.5; //perspective divide/normalize

	position  		= (u_ModelMatrix * vec4(attr_Position , 1.0)).xyz;
	normal    		= attr_Normal;
	normal			= normal  * 2.0 - vec3(1.0);
	//normal    		= ((u_ModelMatrix * vec4(normal , 0.0)).xyz);
	viewDir			= u_ViewOrigin - position.xyz;

	var_PrimaryLightDir.xyz = u_PrimaryLightOrigin.xyz - (position * u_PrimaryLightOrigin.w);
	var_PrimaryLightDir.w = u_PrimaryLightRadius * u_PrimaryLightRadius;
}

/*[Fragment]*/
/*
* Liquid shader based on:
* "Seascape" by Alexander Alekseev aka TDM - 2014
* Contact: tdmaav@gmail.com
*/

const float etaR = 0.65;
const float etaG = 0.67; // Ratio of indices of refraction
const float etaB = 0.69;
const float fresnelPower = 2.0;
const float F = ((1.0 - etaG) * (1.0 - etaG)) / ((1.0 + etaG) * (1.0 + etaG));

uniform sampler2D	u_DiffuseMap;
uniform samplerCube u_CubeMap;
uniform vec4		u_CubeMapInfo;
uniform vec4 u_Color;

uniform sampler2D u_ShadowMap;
uniform vec3  u_PrimaryLightColor;
uniform vec3  u_PrimaryLightAmbient;

layout(std140) uniform Liquid
{
float		isLiquid;
float		height;
float		choppy;
float		speed;
float		freq;
float		depth;
float		time;
};

layout(std140) uniform Liquid2
{
float		water_color_r;
float		water_color_g;
float		water_color_b;
float		fog_color_r;
float		fog_color_g;
float		fog_color_b;
};

in vec2 fragpos;
in vec3 position;
in vec3 normal;
in vec3 viewDir;

in vec4 var_PrimaryLightDir;

out vec4 out_Color;
out vec4 out_Glow;

#define EPSILON_NRM (.2 / 1920)
const int NUM_STEPS = 8;
const float EPSILON = 1e-3;
const int ITER_GEOMETRY = 3;
const int ITER_FRAGMENT = 5;
const mat2 octave_m = mat2(1.6, 1.2, -1.2, 1.6);
float SEA_TIME = 1.0 + time * speed;

float hash(vec2 p) {
	float h = dot(p, vec2(127.1, 311.7));
	return fract(sin(h)*43758.5453123);
}
float noise(in vec2 p) {
	vec2 i = floor(p);
	vec2 f = fract(p);
	vec2 u = f*f*(3.0 - 2.0*f);
	return -1.0 + 2.0*mix(mix(hash(i + vec2(0.0, 0.0)),
		hash(i + vec2(1.0, 0.0)), u.x),
		mix(hash(i + vec2(0.0, 1.0)),
			hash(i + vec2(1.0, 1.0)), u.x), u.y);
}

float diffuse(vec3 n, vec3 l, float p) {
	return pow(dot(n, l) * 0.4 + 0.6, p);
}

vec3 specular(vec3 n, vec3 l, vec3 e, float s) {

	vec2 shadowTex = gl_FragCoord.xy * r_FBufScale;
	float shadowValue = texture(u_ShadowMap, shadowTex).r;

	// surfaces not facing the light are always shadowed
	shadowValue *= clamp(dot(n, -l), 0.0, 1.0);

	float nrm = (s + 8.0) / (M_PI * 8.0);
	vec3 lightColor = u_PrimaryLightColor * (pow(max(dot(reflect(e, n), -l), 0.0), s) * nrm);

	return (lightColor * shadowValue);
}

float sea_octave(vec2 uv, float choppy) {
	uv += noise(uv);
	vec2 wv = 1.0 - abs(sin(uv));
	vec2 swv = abs(cos(uv));
	wv = mix(wv, swv, wv);
	return pow(1.0 - pow(wv.x * wv.y, 0.65), choppy);
}


float map(vec3 p) {
	float freq = freq;
	float amp = height;
	float choppy = choppy;
	vec2 uv = p.xy; uv.x *= 0.75;

	float d, h = 0.0;
	for (int i = 0; i < ITER_GEOMETRY; i++) {
		d = sea_octave((uv + SEA_TIME)*freq, choppy);
		d += sea_octave((uv - SEA_TIME)*freq, choppy);
		h += d * amp;
		uv *= octave_m; freq *= 1.9; amp *= 0.22;
		choppy = mix(choppy, 1.0, 0.2);
	}
	return p.z - h;
}

float map_detailed(vec3 p) {
	float freq = freq;
	float amp = height;
	float choppy = choppy;
	vec2 uv = p.xy; uv.x *= 0.75;

	float d, h = 0.0;
	for (int i = 0; i < ITER_FRAGMENT; i++) {
		d = sea_octave((uv + SEA_TIME)*freq, choppy);
		d += sea_octave((uv - SEA_TIME)*freq, choppy);
		h += d * amp;
		uv *= octave_m; freq *= 1.9; amp *= 0.22;
		choppy = mix(choppy, 1.0, 0.2);
	}
	return p.z - h;
}


vec3 getNormal(vec3 p, float eps) {
	vec3 n;
	n.z = map_detailed(p);
	n.x = map_detailed(vec3(p.x + eps, p.y, p.z)) - n.z;
	n.y = map_detailed(vec3(p.x, p.y + eps, p.z)) - n.z;
	n.z = eps;
	return normalize(n);
}

float heightMapTracing(vec3 ori, vec3 dir, out vec3 p) {
	float tm = 0.0;
	float tx = 1000.0;
	float hx = map(ori + dir * tx);
	if (hx > 0.0) return tx;
	float hm = map(ori + dir * tm);
	float tmid = 0.0;
	for (int i = 0; i < NUM_STEPS; i++) {
		tmid = mix(tm, tx, hm / (hm - hx));
		p = ori + dir * tmid;
		float hmid = map(p);
		if (hmid < 0.0) {
			tx = tmid;
			hx = hmid;
		}
		else {
			tm = tmid;
			hm = hmid;
		}
	}
	return tmid;
}

vec3 getSkyColor(vec3 n) {
	vec3 i = normalize(viewDir);

	vec3 parallax = u_CubeMapInfo.xyz + u_CubeMapInfo.w * viewDir;
	vec3 cubeLightColor = texture(u_CubeMap, reflect(i, n) + parallax).rgb;
	return cubeLightColor;
}

vec3 getGroundColor(vec3 n) {
	vec3 i = normalize(viewDir);

	vec3 cubeLightColor = vec3(texture(u_CubeMap, refract(i, n, 0.948)).r, texture(u_CubeMap, refract(i, n, 0.95)).g, texture(u_CubeMap, refract(i, n, 0.952)).b);
	return cubeLightColor;
}

vec3 getSeaColor(vec3 p, vec3 n, vec3 l, vec3 eye, vec3 dist) {
	vec3 i = normalize(viewDir);

	float fresnel = clamp(1.0 - dot(n, -i), 0.0, 1.0);
	fresnel = pow(fresnel, 3.0) * 0.65;

	vec3 water_color = vec3(water_color_r, water_color_g, water_color_b);
	vec3 fog_color = vec3(fog_color_r, fog_color_g, fog_color_b);

	vec3 groundColor = vec3(.4, .3, .2);

	vec3 reflected = getSkyColor(n);
	vec3 refracted = fog_color + diffuse(n,l,80) * water_color * 0.12;
	//vec3 refracted = mix(groundColor, fog_color /*+ diffuse(n,l,80)*/ * water_color * 0.12, depth);
	//refracted.r = mix(texture(u_CubeMap, refract(eye, n, 0.948)).rbg, (fog_color + diffuse(n, l, 80.0) * water_color * 0.12), .1).r;
	//refracted.g = mix(texture(u_CubeMap, refract(eye, n, 0.95)).rbg, (fog_color + diffuse(n, l, 80.0) * water_color * 0.12), .1).g;
	//refracted.b = mix(texture(u_CubeMap, refract(eye, n, 0.952)).rbg, (fog_color + diffuse(n, l, 80.0) * water_color * 0.12), .1).b;

	vec3 color = mix(refracted, reflected, fresnel);

	float atten = max(1.0 - dot(dist, dist) * 0.001, 0.0);
	color += water_color * (p.z - height) * 0.18 * atten;

	color += specular(n, l, i, 60.0);

	return color;
}

void main()
{	
	vec3 n = normalize(normal);
	vec3 refractColor;
	
	float alpha;
	vec3 eye = normalize(viewDir);

	if (isLiquid == 1) 
	{
		// based on https://www.shadertoy.com/view/Ms2SD1
		eye.z -= 2.0;
		vec3 ori = position.xyz;
		
		vec3 p;
		heightMapTracing(ori, eye, p);
		vec3 dist = p - ori;

		n = getNormal(p, dot(viewDir, viewDir) * EPSILON_NRM);
		vec3 color = getSeaColor(p, -n, var_PrimaryLightDir.xyz, eye, dist);
		
		refractColor = vec3(pow(color, vec3(0.75)));

		alpha = .80;
		alpha += depth * 0.2;
	}
	else
	{
		
		float ratio = F + (1.0 - F) * pow(1.0 - dot(-eye, n), fresnelPower);
		vec3	refractR = normalize(refract(eye, n, etaR));
		vec3	refractG = normalize(refract(eye, n, etaG));
		vec3	refractB = normalize(refract(eye, n, etaB));

		refractR = refractR - eye;
		refractG = refractG - eye;
		refractB = refractB - eye;
		refractColor.r = texture(u_DiffuseMap, fragpos + (refractR.xy * 0.1)).r;
		refractColor.g = texture(u_DiffuseMap, fragpos + (refractG.xy * 0.1)).g;
		refractColor.b = texture(u_DiffuseMap, fragpos + (refractB.xy * 0.1)).b;

		vec3 combinedColor = mix(refractColor, u_Color.rgb, ratio);
		refractColor = combinedColor;
		alpha = 1.0;
	}

	out_Color = vec4((refractColor), alpha);
	out_Glow = vec4(0.0);
}
