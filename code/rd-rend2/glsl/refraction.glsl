/*[Vertex]*/
in vec3 attr_Position;
in vec3 attr_Normal;
in vec4 attr_TexCoord0;

uniform mat4 u_ModelViewProjectionMatrix;
uniform mat4 u_ModelMatrix;
uniform vec3 u_ViewOrigin;
uniform vec4 u_ViewInfo; // zfar / znear, zfar, width, height

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

	position  		= (u_ModelMatrix * vec4(attr_Position, 1.0)).xyz;
	normal    		= (u_ModelMatrix * vec4(attr_Normal,   0.0)).xyz;
	viewDir 		= u_ViewOrigin - position;
	
}

/*[Fragment]*/

const float etaR = 1.14;
const float etaG = 1.12;
const float etaB = 1.10;
const float fresnelPower = 2.0;
const float F = ((1.0 - etaG) * (1.0 - etaG)) / ((1.0 + etaG) * (1.0 + etaG));

uniform sampler2D u_DiffuseMap;
uniform sampler2D u_ColorMap;
uniform sampler2D depthTexture;
uniform sampler2D colorBufferTexture;
uniform vec4 u_Color;

in vec2 var_Tex1;
in vec2 fragpos;
in vec3 normal;
in vec3 position;
in vec3 viewDir;

out vec4 out_Color;
out vec4 out_Glow;

float zNear;
float zFar;
float screenWidth;
float screenHeight;
mat4 cameraToClipMatrix;
vec3 cameraSpacePosition;
vec3 cameraSpaceNormal;
float refractiveIndex;

//Z buffer is nonlinear by default, so we fix this here
float linearizeDepth(float depth)
{
	return (2.0 * zNear) / (zFar + zNear - depth * (zFar - zNear));
}

vec2 getScreenSpacePosition()
{
	return position.xy/vec2(screenWidth,screenHeight);
}

//Convert something in camera space to screen space
vec3 convertCameraSpaceToScreenSpace(in vec3 cameraSpace)
{
	vec4 clipSpace = cameraToClipMatrix * vec4(cameraSpace, 1);
	vec3 NDCSpace = clipSpace.xyz / clipSpace.w;
	vec3 screenSpace = 0.5 * NDCSpace + 0.5;
	return screenSpace;
}

vec4 ComputeRefraction()
{
	//Tweakable variables
	float initialStepAmount = .01;
	float stepRefinementAmount = .7;
	int maxRefinements = 3;
	
	//Screen space vector
	vec3 cameraSpaceViewDir = normalize(cameraSpacePosition);
	vec3 cameraSpaceVector = normalize(refract(cameraSpaceViewDir,cameraSpaceNormal,1.0/refractiveIndex));
	vec3 screenSpacePosition = convertCameraSpaceToScreenSpace(cameraSpacePosition);
	vec3 cameraSpaceVectorPosition = cameraSpacePosition + cameraSpaceVector;
	vec3 screenSpaceVectorPosition = convertCameraSpaceToScreenSpace(cameraSpaceVectorPosition);
	vec3 screenSpaceVector = initialStepAmount*normalize(screenSpaceVectorPosition - screenSpacePosition);
	
	//Jitter the initial ray
	vec3 oldPosition = screenSpacePosition + screenSpaceVector;
	vec3 currentPosition = oldPosition + screenSpaceVector;
	
	//State
	vec4 color = vec4(0,0,0,1);
	int count = 0;
	int numRefinements = 0;

	//Ray trace!
	while(count < 1000)
	{
		//Stop ray trace when it goes outside screen space
		if(currentPosition.x < 0 || currentPosition.x > 1 ||
		   currentPosition.y < 0 || currentPosition.y > 1 ||
		   currentPosition.z < 0 || currentPosition.z > 1)
			break;

		//intersections
		vec2 samplePos = currentPosition.xy;
		float currentDepth = linearizeDepth(currentPosition.z);
		float sampleDepth = linearizeDepth(texture(depthTexture, samplePos).x);
		float diff = currentDepth - sampleDepth;
		float error = 100.0; //different than reflection
		if(diff >= 0 && diff < error)
		{
			screenSpaceVector *= stepRefinementAmount;
			currentPosition = oldPosition;
			numRefinements++;
			if(numRefinements >= maxRefinements)
			{
				color = texture(colorBufferTexture, samplePos);
				break;
			}
		}

		//Step ray
		oldPosition = currentPosition;
		currentPosition = oldPosition + screenSpaceVector;
		count++;
	}
	return color;
}

void main()
{	
	vec3 i = normalize(viewDir);
	vec3 n = normalize(normal);
	vec4 color = vec4(1.0);
	
	float ratio = F + (1.0 - F) * pow(1.0 - dot(-i, n), fresnelPower);
	vec3 refractR = normalize(refract(i, n, etaR));
	vec3 refractG = normalize(refract(i, n, etaG));
	vec3 refractB = normalize(refract(i, n, etaB));
	
	vec3 refractColor;
	refractColor.r	= texture(u_DiffuseMap, fragpos + refractR.xy * 0.05).r;
	refractColor.g  = texture(u_DiffuseMap, fragpos + refractG.xy * 0.05).g;
	refractColor.b  = texture(u_DiffuseMap, fragpos + refractB.xy * 0.05).b;
	
	vec3 combinedColor = mix(refractColor, texture(u_ColorMap,var_Tex1).rgb, ratio);

	out_Color = vec4(combinedColor, 1.0);

}
