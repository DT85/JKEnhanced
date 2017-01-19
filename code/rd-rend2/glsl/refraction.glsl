/*[Vertex]*/
in vec3 attr_Position;
in vec3 attr_Normal;
in vec4 attr_TexCoord0;

uniform mat4 u_ModelViewProjectionMatrix;
uniform mat4 u_ModelMatrix;
uniform vec3 u_ViewOrigin;

out vec2 var_Tex1;
out vec2 fragpos;
out vec3 normal;
out vec3 position;
out vec3 viewDir;

void main()
{
	gl_Position 	= u_ModelViewProjectionMatrix * vec4(attr_Position, 1.0);
	var_Tex1 		= attr_TexCoord0.st;
	fragpos 		= (gl_Position.xy / gl_Position.w )* 0.5 + 0.5; //perspective divide/normalize

	position  		= (u_ModelMatrix * vec4(attr_Position , 1.0)).xyz;
	normal    		= attr_Normal;
	normal			= normal  * 2.0 - vec3(1.0);
	normal    		= ((u_ModelMatrix * vec4(normal , 0.0)).xyz);
	viewDir			= u_ViewOrigin - position;
}

/*[Fragment]*/

const float etaR = 0.65;
const float etaG = 0.67; // Ratio of indices of refraction
const float etaB = 0.69;
const float fresnelPower = 2.0;
const float F = ((1.0 - etaG) * (1.0 - etaG)) / ((1.0 + etaG) * (1.0 + etaG));

uniform sampler2D u_DiffuseMap;
uniform vec4 u_Color;

in vec2 fragpos;
in vec3 normal;
in vec3 viewDir;

out vec4 out_Color;

void main()
{	
	vec3 i = normalize(viewDir);
	vec3 n = normalize(normal);

	
	float ratio = F + (1.0 - F) * pow(1.0 - dot(-i, n), fresnelPower);
	vec3	refractR = normalize(refract(i, n, etaR));
	vec3	refractG = normalize(refract(i, n, etaG));
	vec3	refractB = normalize(refract(i, n, etaB));
	
	refractR = refractR - i;
	refractG = refractG - i; 
	refractB = refractB - i;
	vec3 refractColor;
	refractColor.r	= texture(u_DiffuseMap, fragpos + (refractR.xy * 0.1)).r;
	refractColor.g  = texture(u_DiffuseMap, fragpos + (refractG.xy * 0.1)).g;
	refractColor.b  = texture(u_DiffuseMap, fragpos + (refractB.xy * 0.1)).b;
	
	vec3 combinedColor = mix(refractColor, u_Color.rgb, ratio);

	out_Color = vec4(refractColor , u_Color.a);
}
