/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Quake III Arena source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
// tr_init.c -- functions that are not called every frame

#include "tr_local.h"
#include "tr_stl.h"
//#include "ghoul2/g2_local.h"
#include "tr_cache.h"
#include "tr_allocator.h"
#include "tr_weather.h"
#include <algorithm>

static size_t FRAME_UNIFORM_BUFFER_SIZE = 8 * 1024 * 1024;
static size_t FRAME_VERTEX_BUFFER_SIZE = 8 * 1024 * 1024;
static size_t FRAME_INDEX_BUFFER_SIZE = 2 * 1024 * 1024;

#if defined(_WIN32)
extern "C" {
	__declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif

glconfig_t  glConfig;
glconfigExt_t glConfigExt;
glRefConfig_t glRefConfig;
glstate_t	glState;
window_t	window;

// the limits apply to the sum of all scenes in a frame --
// the main view, all the 3D icons, etc
#define	DEFAULT_MAX_POLYS		600
#define	DEFAULT_MAX_POLYVERTS	3000

cvar_t  *r_ext_compressed_textures;
cvar_t  *r_ext_multitexture;
cvar_t  *r_ext_compiled_vertex_array;
cvar_t  *r_ext_texture_env_add;
cvar_t  *r_ext_texture_filter_anisotropic;
cvar_t	*r_ext_preferred_tc_method;
cvar_t  *r_ext_draw_range_elements;
cvar_t  *r_ext_multi_draw_arrays;
cvar_t  *r_ext_texture_float;
cvar_t  *r_arb_half_float_pixel;
cvar_t  *r_arb_vertex_type_2_10_10_10_rev;
cvar_t  *r_arb_buffer_storage;

cvar_t	*sv_mapname;
cvar_t	*sv_mapChecksum;
cvar_t	*se_language;
cvar_t	*r_ignore;				// used for debugging anything
cvar_t	*r_verbose;				// used for verbose debug spew
cvar_t	*r_znear;				// near Z clip plane
cvar_t	*r_zproj;				// z distance of projection plane
cvar_t	*r_stereoSeparation;	// separation of cameras for stereo rendering
cvar_t	*r_skipBackEnd;
cvar_t	*r_stereo;
cvar_t	*r_anaglyphMode;
cvar_t	*r_greyscale;
cvar_t	*r_measureOverdraw;		// enables stencil buffer overdraw measurement
cvar_t	*r_inGameVideo;			// controls whether in game video should be draw
cvar_t	*r_fastsky;				// controls whether sky should be cleared or drawn
cvar_t	*r_drawSun;				// controls drawing of sun quad
cvar_t	*r_dynamiclight;		// dynamic lights enabled/disabled
cvar_t	*r_norefresh;			// bypasses the ref rendering
cvar_t	*r_drawentities;		// disable/enable entity rendering
cvar_t	*r_drawworld;			// disable/enable world rendering
cvar_t	*r_speeds;				// various levels of information display
cvar_t	*r_detailTextures;		// enables/disables detail texturing stages
cvar_t	*r_novis;				// disable/enable usage of PVS
cvar_t	*r_nocull;
cvar_t	*r_facePlaneCull;		// enables culling of planar surfaces with back side test
cvar_t	*r_nocurves;
cvar_t	*r_showcluster;
cvar_t	*r_gamma;
cvar_t	*r_lodbias;				// push/pull LOD transitions
cvar_t	*r_lodscale;
cvar_t	*r_autolodscalevalue;
cvar_t	*r_nobind;						// turns off binding to appropriate textures
cvar_t	*r_singleShader;				// make most world faces use default shader
cvar_t	*r_roundImagesDown;
cvar_t	*r_colorMipLevels;				// development aid to see texture mip usage
cvar_t	*r_picmip;						// controls picmip values
cvar_t	*r_finish;
cvar_t	*r_textureMode;
cvar_t	*r_offsetFactor;
cvar_t	*r_offsetUnits;
cvar_t	*r_fullbright;					// avoid lightmap pass
cvar_t	*r_lightmap;					// render lightmaps only
cvar_t	*r_vertexLight;					// vertex lighting mode for better performance
cvar_t	*r_uiFullScreen;				// ui is running fullscreen
cvar_t	*r_logFile;						// number of frames to emit GL logs
cvar_t	*r_showtris;					// enables wireframe rendering of the world
cvar_t	*r_showsky;						// forces sky in front of all surfaces
cvar_t	*r_shownormals;					// draws wireframe normals
cvar_t	*r_clear;						// force screen clear every frame
cvar_t	*r_shadows;						// controls shadows: 0 = none, 1 = blur, 2 = stencil, 3 = black planar projection
cvar_t	*r_flares;						// light flares
cvar_t	*r_intensity;
cvar_t	*r_lockpvs;
cvar_t	*r_noportals;
cvar_t	*r_portalOnly;
cvar_t	*r_subdivisions;
cvar_t	*r_lodCurveError;
cvar_t  *r_srgb;
cvar_t	*r_debugLight;
cvar_t	*r_debugSort;
cvar_t	*r_ignoreGLErrors;
cvar_t	*r_overBrightBits;
cvar_t	*r_mapOverBrightBits;
cvar_t	*r_debugSurface;
cvar_t	*r_simpleMipMaps;
cvar_t	*r_texturebits;
cvar_t	*r_showImages;
cvar_t	*r_screenshotJpegQuality;
cvar_t	*r_surfaceSprites;
cvar_t	*r_allowExtensions;
cvar_t	*r_dynamicGlow;
cvar_t	*r_dynamicGlowPasses;
cvar_t	*r_dynamicGlowDelta;
cvar_t	*r_dynamicGlowIntensity;
cvar_t	*r_dynamicGlowSoft;
cvar_t	*r_dynamicGlowWidth;
cvar_t	*r_dynamicGlowHeight;

/*
Ghoul2 Insert Start
*/
#ifdef _DEBUG
cvar_t	*r_noPrecacheGLA;
#endif

cvar_t	*r_noGhoul2;
cvar_t	*r_Ghoul2AnimSmooth = 0;
cvar_t	*r_Ghoul2UnSqashAfterSmooth = 0;
cvar_t	*r_Ghoul2UnSqash;
cvar_t	*r_Ghoul2TimeBase = 0;
cvar_t	*r_Ghoul2NoLerp;
cvar_t	*r_Ghoul2NoBlend;
cvar_t	*r_Ghoul2BlendMultiplier = 0;
cvar_t	*broadsword = 0;
cvar_t	*broadsword_kickbones = 0;
cvar_t	*broadsword_kickorigin = 0;
cvar_t	*broadsword_playflop = 0;
cvar_t	*broadsword_dontstopanim = 0;
cvar_t	*broadsword_waitforshot = 0;
cvar_t	*broadsword_smallbbox = 0;
cvar_t	*broadsword_extra1 = 0;
cvar_t	*broadsword_extra2 = 0;
cvar_t	*broadsword_effcorr = 0;
cvar_t	*broadsword_ragtobase = 0;
cvar_t	*broadsword_dircap = 0;
/*
Ghoul2 Insert End
*/

cvar_t	*com_buildScript;

//
//GL2-specific
//

cvar_t	*r_flareSize;
cvar_t	*r_flareFade;
cvar_t	*r_flareCoeff;
cvar_t  *r_ext_framebuffer_multisample;
cvar_t  *r_arb_seamless_cube_map;
cvar_t  *r_mergeMultidraws;
cvar_t  *r_mergeLeafSurfaces;
cvar_t  *r_cameraExposure;
cvar_t  *r_floatLightmap;
cvar_t  *r_toneMap;
cvar_t  *r_forceToneMap;
cvar_t  *r_forceToneMapMin;
cvar_t  *r_forceToneMapAvg;
cvar_t  *r_forceToneMapMax;
cvar_t  *r_autoExposure;
cvar_t  *r_forceAutoExposure;
cvar_t  *r_forceAutoExposureMin;
cvar_t  *r_forceAutoExposureMax;
cvar_t	*r_externalGLSL;
cvar_t  *r_hdr;
cvar_t	*r_marksOnTriangleMeshes;
cvar_t	*r_saveFontData;
cvar_t  *r_ignoreDstAlpha;
cvar_t	*r_printShaders;
cvar_t	*r_drawBuffer;
cvar_t	*r_markcount;
cvar_t	*r_debugContext;
cvar_t	*r_aspectCorrectFonts;
cvar_t	*r_maxpolys;
cvar_t	*r_maxpolyverts;
cvar_t  *r_refraction;
cvar_t  *r_depthPrepass;
cvar_t  *r_ssao;
cvar_t  *r_normalMapping;
cvar_t  *r_specularMapping;
cvar_t  *r_deluxeMapping;
cvar_t	*r_deferredShading;
cvar_t  *r_parallaxMapping;
cvar_t  *r_cubeMapping;
cvar_t  *r_horizonFade;
cvar_t  *r_cubemapSize;
cvar_t  *r_pbr;
cvar_t  *r_pbrIBL;
cvar_t  *r_baseNormalX;
cvar_t  *r_baseNormalY;
cvar_t  *r_baseParallax;
cvar_t  *r_baseSpecular;
cvar_t  *r_baseGloss;
cvar_t  *r_dlightMode;
cvar_t  *r_pshadowDist;
cvar_t  *r_mergeLightmaps;
cvar_t  *r_imageUpsample;
cvar_t  *r_imageUpsampleMaxSize;
cvar_t  *r_imageUpsampleType;
cvar_t  *r_genNormalMaps;
cvar_t  *r_forceSun;
cvar_t  *r_forceSunMapLightScale;
cvar_t  *r_forceSunLightScale;
cvar_t  *r_forceSunAmbientScale;
cvar_t  *r_sunlightMode;
cvar_t  *r_drawSunRays;
cvar_t  *r_sunShadows;
cvar_t  *r_shadowFilter;
cvar_t  *r_shadowMapSize;
cvar_t  *r_shadowCascadeZNear;
cvar_t  *r_shadowCascadeZFar;
cvar_t  *r_shadowCascadeZBias;
cvar_t	*r_ambientScale;
cvar_t	*r_directedScale;
cvar_t	*r_bloom_threshold;


int max_polys;
int max_polyverts;


extern void	RB_SetGL2D(void);
static void R_Splash()
{
	const GLfloat black[] = { 0.0f, 0.0f, 0.0f, 1.0f };

	qglViewport(0, 0, glConfig.vidWidth, glConfig.vidHeight);
	qglClearBufferfv(GL_COLOR, 0, black);

	GLSL_InitSplashScreenShader();

	GL_Cull(CT_TWO_SIDED);

	image_t *pImage = R_FindImageFile("menu/splash", IMGTYPE_COLORALPHA, IMGFLAG_NONE);
	if (pImage)
		GL_Bind(pImage);

	GL_State(GLS_DEPTHTEST_DISABLE);
	GLSL_BindProgram(&tr.splashScreenShader);
	qglDrawArrays(GL_TRIANGLES, 0, 3);

	ri->WIN_Present(&window);
}

/*
** GLW_CheckForExtension

Cannot use strstr directly to differentiate between (for eg) reg_combiners and reg_combiners2
*/
bool GL_CheckForExtension(const char *ext)
{
	const char *ptr = Q_stristr(glConfigExt.originalExtensionString, ext);
	if (ptr == NULL)
		return false;
	ptr += strlen(ext);
	return ((*ptr == ' ') || (*ptr == '\0'));  // verify it's complete string.
}


void GLW_InitTextureCompression(void)
{
	bool newer_tc, old_tc;

	// Check for available tc methods.
	newer_tc = GL_CheckForExtension("ARB_texture_compression") && GL_CheckForExtension("EXT_texture_compression_s3tc");
	old_tc = GL_CheckForExtension("GL_S3_s3tc");

	if (old_tc)
	{
		Com_Printf("...GL_S3_s3tc available\n");
	}

	if (newer_tc)
	{
		Com_Printf("...GL_EXT_texture_compression_s3tc available\n");
	}

	if (!r_ext_compressed_textures->value)
	{
		// Compressed textures are off
		glConfig.textureCompression = TC_NONE;
		Com_Printf("...ignoring texture compression\n");
	}
	else if (!old_tc && !newer_tc)
	{
		// Requesting texture compression, but no method found
		glConfig.textureCompression = TC_NONE;
		Com_Printf("...no supported texture compression method found\n");
		Com_Printf(".....ignoring texture compression\n");
	}
	else
	{
		// some form of supported texture compression is avaiable, so see if the user has a preference
		if (r_ext_preferred_tc_method->integer == TC_NONE)
		{
			// No preference, so pick the best
			if (newer_tc)
			{
				Com_Printf("...no tc preference specified\n");
				Com_Printf(".....using GL_EXT_texture_compression_s3tc\n");
				glConfig.textureCompression = TC_S3TC_DXT;
			}
			else
			{
				Com_Printf("...no tc preference specified\n");
				Com_Printf(".....using GL_S3_s3tc\n");
				glConfig.textureCompression = TC_S3TC;
			}
		}
		else
		{
			// User has specified a preference, now see if this request can be honored
			if (old_tc && newer_tc)
			{
				// both are avaiable, so we can use the desired tc method
				if (r_ext_preferred_tc_method->integer == TC_S3TC)
				{
					Com_Printf("...using preferred tc method, GL_S3_s3tc\n");
					glConfig.textureCompression = TC_S3TC;
				}
				else
				{
					Com_Printf("...using preferred tc method, GL_EXT_texture_compression_s3tc\n");
					glConfig.textureCompression = TC_S3TC_DXT;
				}
			}
			else
			{
				// Both methods are not available, so this gets trickier
				if (r_ext_preferred_tc_method->integer == TC_S3TC)
				{
					// Preferring to user older compression
					if (old_tc)
					{
						Com_Printf("...using GL_S3_s3tc\n");
						glConfig.textureCompression = TC_S3TC;
					}
					else
					{
						// Drat, preference can't be honored
						Com_Printf("...preferred tc method, GL_S3_s3tc not available\n");
						Com_Printf(".....falling back to GL_EXT_texture_compression_s3tc\n");
						glConfig.textureCompression = TC_S3TC_DXT;
					}
				}
				else
				{
					// Preferring to user newer compression
					if (newer_tc)
					{
						Com_Printf("...using GL_EXT_texture_compression_s3tc\n");
						glConfig.textureCompression = TC_S3TC_DXT;
					}
					else
					{
						// Drat, preference can't be honored
						Com_Printf("...preferred tc method, GL_EXT_texture_compression_s3tc not available\n");
						Com_Printf(".....falling back to GL_S3_s3tc\n");
						glConfig.textureCompression = TC_S3TC;
					}
				}
			}
		}
	}
}

// Truncates the GL extensions string by only allowing up to 'maxExtensions' extensions in the string.
static const char *TruncateGLExtensionsString(const char *extensionsString, int maxExtensions)
{
	const char *p = extensionsString;
	const char *q;
	int numExtensions = 0;
	size_t extensionsLen = strlen(extensionsString);

	char *truncatedExtensions;

	while ((q = strchr(p, ' ')) != NULL && numExtensions <= maxExtensions)
	{
		p = q + 1;
		numExtensions++;
	}

	if (q != NULL)
	{
		// We still have more extensions. We'll call this the end

		extensionsLen = p - extensionsString - 1;
	}

	truncatedExtensions = (char *)R_Malloc(extensionsLen + 1, TAG_GP2, qfalse);
	Q_strncpyz(truncatedExtensions, extensionsString, extensionsLen + 1);

	return truncatedExtensions;
}

static const char *GetGLExtensionsString()
{
	GLint numExtensions;
	glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);
	size_t extensionStringLen = 0;

	for (int i = 0; i < numExtensions; i++)
	{
		extensionStringLen += strlen((const char *)qglGetStringi(GL_EXTENSIONS, i)) + 1;
	}

	char *extensionString = (char *)R_Malloc(extensionStringLen + 1, TAG_GP2, qfalse);
	char *p = extensionString;
	for (int i = 0; i < numExtensions; i++)
	{
		const char *extension = (const char *)qglGetStringi(GL_EXTENSIONS, i);
		while (*extension != '\0')
			*p++ = *extension++;

		*p++ = ' ';
	}

	*p = '\0';
	assert((p - extensionString) == extensionStringLen);

	return extensionString;
}

/*
** InitOpenGL
**
** This function is responsible for initializing a valid OpenGL subsystem.  This
** is done by calling GLimp_Init (which gives us a working OGL subsystem) then
** setting variables, checking GL constants, and reporting the gfx system config
** to the user.
*/
static void InitOpenGL(void)
{
	//
	// initialize OS specific portions of the renderer
	//
	// GLimp_Init directly or indirectly references the following cvars:
	//		- r_fullscreen
	//		- r_mode
	//		- r_(color|depth|stencil)bits
	//		- r_ignorehwgamma
	//		- r_gamma
	//

	if (glConfig.vidWidth == 0)
	{
		windowDesc_t windowDesc = {};
		memset(&glConfig, 0, sizeof(glConfig));

		windowDesc.api = GRAPHICS_API_OPENGL;
		windowDesc.gl.majorVersion = 3;
		windowDesc.gl.minorVersion = 2;
		windowDesc.gl.profile = GLPROFILE_CORE;
		if (r_debugContext->integer)
			windowDesc.gl.contextFlags = GLCONTEXT_DEBUG;

		window = ri->WIN_Init(&windowDesc, &glConfig);

		GLimp_InitCoreFunctions();

		Com_Printf("GL_RENDERER: %s\n", (char *)qglGetString(GL_RENDERER));

		// get our config strings
		glConfig.vendor_string = (const char *)qglGetString(GL_VENDOR);
		glConfig.renderer_string = (const char *)qglGetString(GL_RENDERER);
		glConfig.version_string = (const char *)qglGetString(GL_VERSION);
		glConfig.extensions_string = GetGLExtensionsString();

		glConfigExt.originalExtensionString = glConfig.extensions_string;
		glConfig.extensions_string = TruncateGLExtensionsString(glConfigExt.originalExtensionString, 128);

		// OpenGL driver constants
		qglGetIntegerv(GL_MAX_TEXTURE_SIZE, &glConfig.maxTextureSize);

		// Determine GPU IHV
		if (Q_stristr(glConfig.vendor_string, "ATI Technologies Inc."))
		{
			glRefConfig.hardwareVendor = IHV_AMD;
		}
		else if (Q_stristr(glConfig.vendor_string, "NVIDIA"))
		{
			glRefConfig.hardwareVendor = IHV_NVIDIA;
		}
		else if (Q_stristr(glConfig.vendor_string, "INTEL"))
		{
			glRefConfig.hardwareVendor = IHV_INTEL;
		}
		else
		{
			glRefConfig.hardwareVendor = IHV_UNKNOWN;
		}

		// stubbed or broken drivers may have reported 0...
		glConfig.maxTextureSize = Q_max(0, glConfig.maxTextureSize);

		// initialize extensions
		GLimp_InitExtensions();

		// Create the default VAO
		GLuint vao;
		qglGenVertexArrays(1, &vao);
		qglBindVertexArray(vao);
		tr.globalVao = vao;

		// set default state
		GL_SetDefaultState();

		R_Splash();	//get something on screen asap
	}
	else
	{
		// set default state
		GL_SetDefaultState();
	}
}

/*
==================
GL_CheckErrors
==================
*/
void GL_CheckErrs(const char *file, int line) {
#if defined(_DEBUG)
	GLenum	err;
	char	s[64];

	err = qglGetError();
	if (err == GL_NO_ERROR) {
		return;
	}
	if (r_ignoreGLErrors->integer) {
		return;
	}
	switch (err) {
	case GL_INVALID_ENUM:
		strcpy(s, "GL_INVALID_ENUM");
		break;
	case GL_INVALID_VALUE:
		strcpy(s, "GL_INVALID_VALUE");
		break;
	case GL_INVALID_OPERATION:
		strcpy(s, "GL_INVALID_OPERATION");
		break;
	case GL_OUT_OF_MEMORY:
		strcpy(s, "GL_OUT_OF_MEMORY");
		break;
	default:
		Com_sprintf(s, sizeof(s), "%i", err);
		break;
	}

	ri->Error(ERR_FATAL, "GL_CheckErrors: %s in %s at line %d", s, file, line);
#endif
}

/*
==============================================================================

SCREEN SHOTS

NOTE TTimo
some thoughts about the screenshots system:
screenshots get written in fs_homepath + fs_gamedir
vanilla q3 .. baseq3/screenshots/ *.tga
team arena .. missionpack/screenshots/ *.tga

two commands: "screenshot" and "screenshotJPEG"
we use statics to store a count and start writing the first screenshot/screenshot????.tga (.jpg) available
(with FS_FileExists / FS_FOpenFileWrite calls)
FIXME: the statics don't get a reinit between fs_game changes

==============================================================================
*/

/*
==================
RB_ReadPixels

Reads an image but takes care of alignment issues for reading RGB images.

Reads a minimum offset for where the RGB data starts in the image from
integer stored at pointer offset. When the function has returned the actual
offset was written back to address offset. This address will always have an
alignment of packAlign to ensure efficient copying.

Stores the length of padding after a line of pixels to address padlen

Return value must be freed with ri->Hunk_FreeTempMemory()
==================
*/

byte *RB_ReadPixels(int x, int y, int width, int height, size_t *offset, int *padlen)
{
	byte *buffer, *bufstart;
	int padwidth, linelen;
	GLint packAlign;

	qglGetIntegerv(GL_PACK_ALIGNMENT, &packAlign);

	linelen = width * 3;
	padwidth = PAD(linelen, packAlign);

	// Allocate a few more bytes so that we can choose an alignment we like
	buffer = (byte *)R_Malloc(padwidth * height + *offset + packAlign - 1, TAG_TEMP_WORKSPACE, qfalse);

	bufstart = (byte*)(PADP((intptr_t)buffer + *offset, packAlign));
	qglReadPixels(x, y, width, height, GL_RGB, GL_UNSIGNED_BYTE, bufstart);

	*offset = bufstart - buffer;
	*padlen = padwidth - linelen;

	return buffer;
}

/*
==================
RB_TakeScreenshot
==================
*/
void RB_TakeScreenshot(int x, int y, int width, int height, char *fileName)
{
	byte *allbuf, *buffer;
	byte *srcptr, *destptr;
	byte *endline, *endmem;
	byte temp;

	int linelen, padlen;
	size_t offset = 18, memcount;

	allbuf = RB_ReadPixels(x, y, width, height, &offset, &padlen);
	buffer = allbuf + offset - 18;

	Com_Memset(buffer, 0, 18);
	buffer[2] = 2;		// uncompressed type
	buffer[12] = width & 255;
	buffer[13] = width >> 8;
	buffer[14] = height & 255;
	buffer[15] = height >> 8;
	buffer[16] = 24;	// pixel size

						// swap rgb to bgr and remove padding from line endings
	linelen = width * 3;

	srcptr = destptr = allbuf + offset;
	endmem = srcptr + (linelen + padlen) * height;

	while (srcptr < endmem)
	{
		endline = srcptr + linelen;

		while (srcptr < endline)
		{
			temp = srcptr[0];
			*destptr++ = srcptr[2];
			*destptr++ = srcptr[1];
			*destptr++ = temp;

			srcptr += 3;
		}

		// Skip the pad
		srcptr += padlen;
	}

	memcount = linelen * height;

	// gamma correct
	if (glConfig.deviceSupportsGamma)
		R_GammaCorrect(allbuf + offset, memcount);

	ri->FS_WriteFile(fileName, buffer, memcount + 18);

	R_Free(allbuf);
}

/*
==================
R_TakeScreenshotPNG
==================
*/
void RB_TakeScreenshotPNG(int x, int y, int width, int height, char *fileName) {
	byte *buffer;
	size_t offset = 0, memcount;
	int padlen;

	buffer = RB_ReadPixels(x, y, width, height, &offset, &padlen);
	memcount = (width * 3 + padlen) * height;

	// gamma correct
	if (glConfig.deviceSupportsGamma)
		R_GammaCorrect(buffer + offset, memcount);

	RE_SavePNG(fileName, buffer, width, height, 3);
	R_Free(buffer);
}

/*
==================
RB_TakeScreenshotJPEG
==================
*/

void RB_TakeScreenshotJPEG(int x, int y, int width, int height, char *fileName)
{
	byte *buffer;
	size_t offset = 0, memcount;
	int padlen;

	buffer = RB_ReadPixels(x, y, width, height, &offset, &padlen);
	memcount = (width * 3 + padlen) * height;

	// gamma correct
	if (glConfig.deviceSupportsGamma)
		R_GammaCorrect(buffer + offset, memcount);

	RE_SaveJPG(fileName, r_screenshotJpegQuality->integer, width, height, buffer + offset, padlen);
	R_Free(buffer);
}

/*
==================
RB_TakeScreenshotCmd
==================
*/
const void *RB_TakeScreenshotCmd(const void *data) {
	const screenshotCommand_t	*cmd;

	cmd = (const screenshotCommand_t *)data;

	// finish any 2D drawing if needed
	if (tess.numIndexes)
		RB_EndSurface();

	switch (cmd->format) {
	case SSF_JPEG:
		RB_TakeScreenshotJPEG(cmd->x, cmd->y, cmd->width, cmd->height, cmd->fileName);
		break;
	case SSF_TGA:
		RB_TakeScreenshot(cmd->x, cmd->y, cmd->width, cmd->height, cmd->fileName);
		break;
	case SSF_PNG:
		RB_TakeScreenshotPNG(cmd->x, cmd->y, cmd->width, cmd->height, cmd->fileName);
		break;
	}

	return (const void *)(cmd + 1);
}

/*
==================
R_TakeScreenshot
==================
*/
void R_TakeScreenshot(int x, int y, int width, int height, char *name, screenshotFormat_t format) {
	static char	fileName[MAX_OSPATH]; // bad things if two screenshots per frame?
	screenshotCommand_t	*cmd;

	cmd = (screenshotCommand_t *)R_GetCommandBuffer(sizeof(*cmd));
	if (!cmd) {
		return;
	}
	cmd->commandId = RC_SCREENSHOT;

	cmd->x = x;
	cmd->y = y;
	cmd->width = width;
	cmd->height = height;
	Q_strncpyz(fileName, name, sizeof(fileName));
	cmd->fileName = fileName;
	cmd->format = format;
}

/*
==================
R_ScreenshotFilename
==================
*/
void R_ScreenshotFilename(char *buf, int bufSize, const char *ext) {
	time_t rawtime;
	char timeStr[32] = { 0 }; // should really only reach ~19 chars

	time(&rawtime);
	strftime(timeStr, sizeof(timeStr), "%Y-%m-%d_%H-%M-%S", localtime(&rawtime)); // or gmtime

	Com_sprintf(buf, bufSize, "screenshots/shot%s%s", timeStr, ext);
}

/*
====================
R_LevelShot

levelshots are specialized 256*256 thumbnails for
the menu system, sampled down from full screen distorted images
====================
*/
#define LEVELSHOTSIZE 256
static void R_LevelShot(void) {
	char		checkname[MAX_OSPATH];
	byte		*buffer;
	byte		*source, *allsource;
	byte		*src, *dst;
	size_t		offset = 0;
	int			padlen;
	int			x, y;
	int			r, g, b;
	float		xScale, yScale;
	int			xx, yy;

	Com_sprintf(checkname, sizeof(checkname), "levelshots/%s.tga", tr.world->baseName);

	allsource = RB_ReadPixels(0, 0, glConfig.vidWidth, glConfig.vidHeight, &offset, &padlen);
	source = allsource + offset;

	buffer = (byte *)R_Malloc(LEVELSHOTSIZE * LEVELSHOTSIZE * 3 + 18, TAG_TEMP_WORKSPACE, qfalse);
	Com_Memset(buffer, 0, 18);
	buffer[2] = 2;		// uncompressed type
	buffer[12] = LEVELSHOTSIZE & 255;
	buffer[13] = LEVELSHOTSIZE >> 8;
	buffer[14] = LEVELSHOTSIZE & 255;
	buffer[15] = LEVELSHOTSIZE >> 8;
	buffer[16] = 24;	// pixel size

						// resample from source
	xScale = glConfig.vidWidth / (4.0*LEVELSHOTSIZE);
	yScale = glConfig.vidHeight / (3.0*LEVELSHOTSIZE);
	for (y = 0; y < LEVELSHOTSIZE; y++) {
		for (x = 0; x < LEVELSHOTSIZE; x++) {
			r = g = b = 0;
			for (yy = 0; yy < 3; yy++) {
				for (xx = 0; xx < 4; xx++) {
					src = source + 3 * (glConfig.vidWidth * (int)((y * 3 + yy)*yScale) + (int)((x * 4 + xx)*xScale));
					r += src[0];
					g += src[1];
					b += src[2];
				}
			}
			dst = buffer + 18 + 3 * (y * LEVELSHOTSIZE + x);
			dst[0] = b / 12;
			dst[1] = g / 12;
			dst[2] = r / 12;
		}
	}

	// gamma correct
	if ((tr.overbrightBits > 0) && glConfig.deviceSupportsGamma) {
		R_GammaCorrect(buffer + 18, LEVELSHOTSIZE * LEVELSHOTSIZE * 3);
	}

	ri->FS_WriteFile(checkname, buffer, LEVELSHOTSIZE * LEVELSHOTSIZE * 3 + 18);

	R_Free(buffer);
	R_Free(allsource);

	ri->Printf(PRINT_ALL, "Wrote %s\n", checkname);
}

/*
==================
R_ScreenShotTGA_f

screenshot
screenshot [silent]
screenshot [levelshot]
screenshot [filename]

Doesn't print the pacifier message if there is a second arg
==================
*/
void R_ScreenShotTGA_f(void) {
	char checkname[MAX_OSPATH] = { 0 };
	qboolean silent = qfalse;

	if (!strcmp(ri->Cmd_Argv(1), "levelshot")) {
		R_LevelShot();
		return;
	}

	if (!strcmp(ri->Cmd_Argv(1), "silent"))
		silent = qtrue;

	if (ri->Cmd_Argc() == 2 && !silent) {
		// explicit filename
		Com_sprintf(checkname, sizeof(checkname), "screenshots/%s.tga", ri->Cmd_Argv(1));
	}
	else {
		// timestamp the file
		R_ScreenshotFilename(checkname, sizeof(checkname), ".tga");

		if (ri->FS_FileExists(checkname)) {
			Com_Printf("ScreenShot: Couldn't create a file\n");
			return;
		}
	}

	R_TakeScreenshot(0, 0, glConfig.vidWidth, glConfig.vidHeight, checkname, SSF_TGA);

	if (!silent)
		ri->Printf(PRINT_ALL, "Wrote %s\n", checkname);
}

void R_ScreenShotPNG_f(void) {
	char checkname[MAX_OSPATH] = { 0 };
	qboolean silent = qfalse;

	if (!strcmp(ri->Cmd_Argv(1), "levelshot")) {
		R_LevelShot();
		return;
	}

	if (!strcmp(ri->Cmd_Argv(1), "silent"))
		silent = qtrue;

	if (ri->Cmd_Argc() == 2 && !silent) {
		// explicit filename
		Com_sprintf(checkname, sizeof(checkname), "screenshots/%s.png", ri->Cmd_Argv(1));
	}
	else {
		// timestamp the file
		R_ScreenshotFilename(checkname, sizeof(checkname), ".png");

		if (ri->FS_FileExists(checkname)) {
			Com_Printf("ScreenShot: Couldn't create a file\n");
			return;
		}
	}

	R_TakeScreenshot(0, 0, glConfig.vidWidth, glConfig.vidHeight, checkname, SSF_PNG);

	if (!silent)
		ri->Printf(PRINT_ALL, "Wrote %s\n", checkname);
}

void R_ScreenShotJPEG_f(void) {
	char checkname[MAX_OSPATH] = { 0 };
	qboolean silent = qfalse;

	if (!strcmp(ri->Cmd_Argv(1), "levelshot")) {
		R_LevelShot();
		return;
	}

	if (!strcmp(ri->Cmd_Argv(1), "silent"))
		silent = qtrue;

	if (ri->Cmd_Argc() == 2 && !silent) {
		// explicit filename
		Com_sprintf(checkname, sizeof(checkname), "screenshots/%s.jpg", ri->Cmd_Argv(1));
	}
	else {
		// timestamp the file
		R_ScreenshotFilename(checkname, sizeof(checkname), ".jpg");

		if (ri->FS_FileExists(checkname)) {
			Com_Printf("ScreenShot: Couldn't create a file\n");
			return;
		}
	}

	R_TakeScreenshot(0, 0, glConfig.vidWidth, glConfig.vidHeight, checkname, SSF_JPEG);

	if (!silent)
		ri->Printf(PRINT_ALL, "Wrote %s\n", checkname);
}

//============================================================================

/*
==================
R_ExportCubemaps
==================
*/
void R_ExportCubemaps(void)
{
	exportCubemapsCommand_t	*cmd;

	cmd = (exportCubemapsCommand_t	*)R_GetCommandBuffer(sizeof(*cmd));
	if (!cmd) {
		return;
	}
	cmd->commandId = RC_EXPORT_CUBEMAPS;
}


/*
==================
R_ExportCubemaps_f
==================
*/
void R_ExportCubemaps_f(void)
{
	R_ExportCubemaps();
}

//============================================================================

/*
** GL_SetDefaultState
*/
void GL_SetDefaultState(void)
{
	qglClearDepth(1.0f);

	qglCullFace(GL_FRONT);

	// initialize downstream texture unit if we're running
	// in a multitexture environment
	GL_SelectTexture(1);
	GL_TextureMode(r_textureMode->string);
	GL_SelectTexture(0);

	GL_TextureMode(r_textureMode->string);

	//qglShadeModel( GL_SMOOTH );
	qglDepthFunc(GL_LEQUAL);

	Com_Memset(&glState, 0, sizeof(glState));

	//
	// make sure our GL state vector is set correctly
	//
	glState.glStateBits = GLS_DEPTHTEST_DISABLE | GLS_DEPTHMASK_TRUE;
	glState.maxDepth = 1.0f;
	qglDepthRange(0.0f, 1.0f);

	qglUseProgram(0);

	qglBindBuffer(GL_ARRAY_BUFFER, 0);
	qglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	qglPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	qglDepthMask(GL_TRUE);
	qglDisable(GL_DEPTH_TEST);
	qglEnable(GL_SCISSOR_TEST);
	qglEnable(GL_PROGRAM_POINT_SIZE);
	qglDisable(GL_CULL_FACE);
	qglDisable(GL_BLEND);

	qglEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
}

/*
================
R_PrintLongString

Workaround for ri->Printf's 1024 characters buffer limit.
================
*/
void R_PrintLongString(const char *string) {
	char buffer[1024];
	const char *p;
	int size = strlen(string);

	p = string;
	while (size > 0)
	{
		Q_strncpyz(buffer, p, sizeof(buffer));
		ri->Printf(PRINT_ALL, "%s", buffer);
		p += 1023;
		size -= 1023;
	}
}

/*
================
GfxInfo_f
================
*/
static void GfxInfo_f(void)
{
	const char *enablestrings[] =
	{
		"disabled",
		"enabled"
	};
	const char *fsstrings[] =
	{
		"windowed",
		"fullscreen"
	};
	const char *noborderstrings[] =
	{
		"",
		"noborder "
	};

	int fullscreen = ri->Cvar_VariableIntegerValue("r_fullscreen");
	int noborder = ri->Cvar_VariableIntegerValue("r_noborder");

	ri->Printf(PRINT_ALL, "\nGL_VENDOR: %s\n", glConfig.vendor_string);
	ri->Printf(PRINT_ALL, "GL_RENDERER: %s\n", glConfig.renderer_string);
	ri->Printf(PRINT_ALL, "GL_VERSION: %s\n", glConfig.version_string);
	ri->Printf(PRINT_ALL, "GL_EXTENSIONS: ");
	R_PrintLongString(glConfigExt.originalExtensionString);
	ri->Printf(PRINT_ALL, "\n");
	ri->Printf(PRINT_ALL, "GL_MAX_TEXTURE_SIZE: %d\n", glConfig.maxTextureSize);
	ri->Printf(PRINT_ALL, "\nPIXELFORMAT: color(%d-bits) Z(%d-bit) stencil(%d-bits)\n", glConfig.colorBits, glConfig.depthBits, glConfig.stencilBits);
	ri->Printf(PRINT_ALL, "MODE: %d, %d x %d %s%s hz:",
		ri->Cvar_VariableIntegerValue("r_mode"),
		glConfig.vidWidth, glConfig.vidHeight,
		fullscreen == 0 ? noborderstrings[noborder == 1] : noborderstrings[0],
		fsstrings[fullscreen == 1]);
	if (glConfig.displayFrequency)
	{
		ri->Printf(PRINT_ALL, "%d\n", glConfig.displayFrequency);
	}
	else
	{
		ri->Printf(PRINT_ALL, "N/A\n");
	}
	if (glConfig.deviceSupportsGamma)
	{
		ri->Printf(PRINT_ALL, "GAMMA: hardware w/ %d overbright bits\n", tr.overbrightBits);
	}
	else
	{
		ri->Printf(PRINT_ALL, "GAMMA: software w/ %d overbright bits\n", tr.overbrightBits);
	}

	ri->Printf(PRINT_ALL, "texturemode: %s\n", r_textureMode->string);
	ri->Printf(PRINT_ALL, "picmip: %d\n", r_picmip->integer);
	ri->Printf(PRINT_ALL, "texture bits: %d\n", r_texturebits->integer);

	if (r_vertexLight->integer)
	{
		ri->Printf(PRINT_ALL, "HACK: using vertex lightmap approximation\n");
	}
	int displayRefresh = ri->Cvar_VariableIntegerValue("r_displayRefresh");
	if (displayRefresh) {
		ri->Printf(PRINT_ALL, "Display refresh set to %d\n", displayRefresh);
	}

	if (r_finish->integer) {
		ri->Printf(PRINT_ALL, "Forcing glFinish\n");
	}

	ri->Printf(PRINT_ALL, "Dynamic Glow: %s\n", enablestrings[r_dynamicGlow->integer != 0]);
}

/*
================
GfxMemInfo_f
================
*/
void GfxMemInfo_f(void)
{
	switch (glRefConfig.memInfo)
	{
	case MI_NONE:
	{
		ri->Printf(PRINT_ALL, "No extension found for GPU memory info.\n");
	}
	break;
	case MI_NVX:
	{
		int value;

		qglGetIntegerv(GL_GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX, &value);
		ri->Printf(PRINT_ALL, "GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX: %ikb\n", value);

		qglGetIntegerv(GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX, &value);
		ri->Printf(PRINT_ALL, "GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX: %ikb\n", value);

		qglGetIntegerv(GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &value);
		ri->Printf(PRINT_ALL, "GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX: %ikb\n", value);

		qglGetIntegerv(GL_GPU_MEMORY_INFO_EVICTION_COUNT_NVX, &value);
		ri->Printf(PRINT_ALL, "GPU_MEMORY_INFO_EVICTION_COUNT_NVX: %i\n", value);

		qglGetIntegerv(GL_GPU_MEMORY_INFO_EVICTED_MEMORY_NVX, &value);
		ri->Printf(PRINT_ALL, "GPU_MEMORY_INFO_EVICTED_MEMORY_NVX: %ikb\n", value);
	}
	break;
	case MI_ATI:
	{
		// GL_ATI_meminfo
		int value[4];

		qglGetIntegerv(GL_VBO_FREE_MEMORY_ATI, &value[0]);
		ri->Printf(PRINT_ALL, "VBO_FREE_MEMORY_ATI: %ikb total %ikb largest aux: %ikb total %ikb largest\n", value[0], value[1], value[2], value[3]);

		qglGetIntegerv(GL_TEXTURE_FREE_MEMORY_ATI, &value[0]);
		ri->Printf(PRINT_ALL, "TEXTURE_FREE_MEMORY_ATI: %ikb total %ikb largest aux: %ikb total %ikb largest\n", value[0], value[1], value[2], value[3]);

		qglGetIntegerv(GL_RENDERBUFFER_FREE_MEMORY_ATI, &value[0]);
		ri->Printf(PRINT_ALL, "RENDERBUFFER_FREE_MEMORY_ATI: %ikb total %ikb largest aux: %ikb total %ikb largest\n", value[0], value[1], value[2], value[3]);
	}
	break;
	}
}

static void R_CaptureFrameData_f()
{
	int argc = ri->Cmd_Argc();
	if (argc <= 1)
	{
		ri->Printf(PRINT_ALL, "Usage: %s <multi|single>\n", ri->Cmd_Argv(0));
		return;
	}


	const char *cmd = ri->Cmd_Argv(1);
	if (Q_stricmp(cmd, "single") == 0)
		tr.numFramesToCapture = 1;
	else if (Q_stricmp(cmd, "multi") == 0)
		tr.numFramesToCapture = atoi(ri->Cmd_Argv(1));

	int len = ri->FS_FOpenFileByMode("rend2.log", &tr.debugFile, FS_APPEND);
	if (len == -1 || !tr.debugFile)
	{
		ri->Printf(PRINT_ERROR, "Failed to open rend2 log file\n");
		tr.numFramesToCapture = 0;
	}
}

typedef struct consoleCommand_s {
	const char	*cmd;
	xcommand_t	func;
} consoleCommand_t;

static consoleCommand_t	commands[] = {
	{ "imagelist",			R_ImageList_f },
	{ "shaderlist",			R_ShaderList_f },
	{ "skinlist",			R_SkinList_f },
	{ "fontlist",			R_FontList_f },
	{ "screenshot",			R_ScreenShotJPEG_f },
	{ "screenshot_png",		R_ScreenShotPNG_f },
	{ "screenshot_tga",		R_ScreenShotTGA_f },
	{ "gfxinfo",			GfxInfo_f },
	{ "gfxmeminfo",			GfxMemInfo_f },
	{ "modellist",			R_Modellist_f },
	{ "vbolist",			R_VBOList_f },
	{ "capframes",			R_CaptureFrameData_f },
	{ "exportCubemaps",		R_ExportCubemaps_f },
};

static const size_t numCommands = ARRAY_LEN(commands);


/*
===============
R_Register
===============
*/
void R_Register(void)
{
	//
	// latched and archived variables
	//
	r_allowExtensions = ri->Cvar_Get("r_allowExtensions", "1", CVAR_ARCHIVE | CVAR_LATCH);
	r_ext_compressed_textures = ri->Cvar_Get("r_ext_compress_textures", "0", CVAR_ARCHIVE | CVAR_LATCH);
	r_ext_multitexture = ri->Cvar_Get("r_ext_multitexture", "1", CVAR_ARCHIVE | CVAR_LATCH);
	r_ext_compiled_vertex_array = ri->Cvar_Get("r_ext_compiled_vertex_array", "1", CVAR_ARCHIVE | CVAR_LATCH);
	r_ext_texture_env_add = ri->Cvar_Get("r_ext_texture_env_add", "1", CVAR_ARCHIVE | CVAR_LATCH);
	r_ext_preferred_tc_method = ri->Cvar_Get("r_ext_preferred_tc_method", "0", CVAR_ARCHIVE | CVAR_LATCH);
	r_ext_draw_range_elements = ri->Cvar_Get("r_ext_draw_range_elements", "1", CVAR_ARCHIVE | CVAR_LATCH);
	r_ext_multi_draw_arrays = ri->Cvar_Get("r_ext_multi_draw_arrays", "1", CVAR_ARCHIVE | CVAR_LATCH);
	r_ext_texture_float = ri->Cvar_Get("r_ext_texture_float", "1", CVAR_ARCHIVE | CVAR_LATCH);
	r_arb_half_float_pixel = ri->Cvar_Get("r_arb_half_float_pixel", "1", CVAR_ARCHIVE | CVAR_LATCH);
	r_ext_framebuffer_multisample = ri->Cvar_Get("r_ext_framebuffer_multisample", "0", CVAR_ARCHIVE | CVAR_LATCH);
	r_arb_seamless_cube_map = ri->Cvar_Get("r_arb_seamless_cube_map", "0", CVAR_ARCHIVE | CVAR_LATCH);
	r_arb_vertex_type_2_10_10_10_rev = ri->Cvar_Get("r_arb_vertex_type_2_10_10_10_rev", "1", CVAR_ARCHIVE | CVAR_LATCH);
	r_arb_buffer_storage = ri->Cvar_Get("r_arb_buffer_storage", "0", CVAR_ARCHIVE | CVAR_LATCH);
	r_ext_texture_filter_anisotropic = ri->Cvar_Get("r_ext_texture_filter_anisotropic", "16", CVAR_ARCHIVE);

	r_dynamicGlow = ri->Cvar_Get("r_dynamicGlow", "1", CVAR_ARCHIVE);
	r_dynamicGlowPasses = ri->Cvar_Get("r_dynamicGlowPasses", "5", CVAR_ARCHIVE);
	r_dynamicGlowDelta = ri->Cvar_Get("r_dynamicGlowDelta", "0.8f", CVAR_ARCHIVE);
	r_dynamicGlowIntensity = ri->Cvar_Get("r_dynamicGlowIntensity", "1.13f", CVAR_ARCHIVE);
	r_dynamicGlowSoft = ri->Cvar_Get("r_dynamicGlowSoft", "1", CVAR_ARCHIVE);
	r_dynamicGlowWidth = ri->Cvar_Get("r_dynamicGlowWidth", "320", CVAR_ARCHIVE | CVAR_LATCH);
	r_dynamicGlowHeight = ri->Cvar_Get("r_dynamicGlowHeight", "240", CVAR_ARCHIVE | CVAR_LATCH);
	r_bloom_threshold = ri->Cvar_Get("r_bloom_threshold", "0", CVAR_ARCHIVE);
	r_debugContext = ri->Cvar_Get("r_debugContext", "0", CVAR_LATCH);
	r_picmip = ri->Cvar_Get("r_picmip", "0", CVAR_ARCHIVE | CVAR_LATCH);
	ri->Cvar_CheckRange(r_picmip, 0, 16, qtrue);
	r_roundImagesDown = ri->Cvar_Get("r_roundImagesDown", "1", CVAR_ARCHIVE | CVAR_LATCH);
	r_colorMipLevels = ri->Cvar_Get("r_colorMipLevels", "0", CVAR_LATCH);
	r_detailTextures = ri->Cvar_Get("r_detailtextures", "1", CVAR_ARCHIVE | CVAR_LATCH);
	r_texturebits = ri->Cvar_Get("r_texturebits", "32", CVAR_ARCHIVE | CVAR_LATCH);
	r_overBrightBits = ri->Cvar_Get("r_overBrightBits", "0", CVAR_ARCHIVE | CVAR_LATCH);
	r_simpleMipMaps = ri->Cvar_Get("r_simpleMipMaps", "1", CVAR_ARCHIVE | CVAR_LATCH);
	r_vertexLight = ri->Cvar_Get("r_vertexLight", "0", CVAR_ARCHIVE | CVAR_LATCH);
	r_uiFullScreen = ri->Cvar_Get("r_uifullscreen", "0", 0);
	r_subdivisions = ri->Cvar_Get("r_subdivisions", "2", CVAR_ARCHIVE | CVAR_LATCH);
	ri->Cvar_CheckRange(r_subdivisions, 4, 80, qfalse);
	r_stereo = ri->Cvar_Get("r_stereo", "0", CVAR_ARCHIVE | CVAR_LATCH);
	r_greyscale = ri->Cvar_Get("r_greyscale", "0", CVAR_ARCHIVE | CVAR_LATCH);
	ri->Cvar_CheckRange(r_greyscale, 0, 1, qfalse);
	r_externalGLSL = ri->Cvar_Get("r_externalGLSL", "0", CVAR_LATCH);
	r_hdr = ri->Cvar_Get("r_hdr", "1", CVAR_ARCHIVE | CVAR_LATCH);
	r_floatLightmap = ri->Cvar_Get("r_floatLightmap", "0", CVAR_ARCHIVE | CVAR_LATCH);
	r_toneMap = ri->Cvar_Get("r_toneMap", "1", CVAR_ARCHIVE);
	r_forceToneMap = ri->Cvar_Get("r_forceToneMap", "0", CVAR_CHEAT);
	r_forceToneMapMin = ri->Cvar_Get("r_forceToneMapMin", "-8.0", CVAR_CHEAT);
	r_forceToneMapAvg = ri->Cvar_Get("r_forceToneMapAvg", "-2.0", CVAR_CHEAT);
	r_forceToneMapMax = ri->Cvar_Get("r_forceToneMapMax", "0.0", CVAR_CHEAT);
	r_autoExposure = ri->Cvar_Get("r_autoExposure", "1", CVAR_ARCHIVE);
	r_forceAutoExposure = ri->Cvar_Get("r_forceAutoExposure", "0", CVAR_CHEAT);
	r_forceAutoExposureMin = ri->Cvar_Get("r_forceAutoExposureMin", "-2.0", CVAR_CHEAT);
	r_forceAutoExposureMax = ri->Cvar_Get("r_forceAutoExposureMax", "2.0", CVAR_CHEAT);
	r_cameraExposure = ri->Cvar_Get("r_cameraExposure", "0", CVAR_CHEAT);
	r_srgb = ri->Cvar_Get("r_srgb", "0", CVAR_ARCHIVE | CVAR_LATCH);
	r_refraction = ri->Cvar_Get("r_refraction", "1", CVAR_ARCHIVE | CVAR_LATCH);
	r_depthPrepass = ri->Cvar_Get("r_depthPrepass", "1", CVAR_ARCHIVE);
	r_ssao = ri->Cvar_Get("r_ssao", "0", CVAR_LATCH | CVAR_ARCHIVE);
	r_normalMapping = ri->Cvar_Get("r_normalMapping", "1", CVAR_ARCHIVE | CVAR_LATCH);
	r_specularMapping = ri->Cvar_Get("r_specularMapping", "1", CVAR_ARCHIVE | CVAR_LATCH);
	r_deluxeMapping = ri->Cvar_Get("r_deluxeMapping", "1", CVAR_ARCHIVE | CVAR_LATCH);
	r_deferredShading = ri->Cvar_Get("r_deferredShading", "0", CVAR_ARCHIVE | CVAR_LATCH);
	r_parallaxMapping = ri->Cvar_Get("r_parallaxMapping", "1", CVAR_ARCHIVE | CVAR_LATCH);
	r_cubeMapping = ri->Cvar_Get("r_cubeMapping", "1", CVAR_ARCHIVE | CVAR_LATCH);
	r_horizonFade = ri->Cvar_Get("r_horizonFade", "3", CVAR_ARCHIVE | CVAR_LATCH);
	r_cubemapSize = ri->Cvar_Get("r_cubemapSize", "256", CVAR_ARCHIVE | CVAR_LATCH);
	r_pbr = ri->Cvar_Get("r_pbr", "1", CVAR_ARCHIVE | CVAR_LATCH);
	r_pbrIBL = ri->Cvar_Get("r_pbrIBL", "1", CVAR_ARCHIVE | CVAR_LATCH);
	r_baseNormalX = ri->Cvar_Get("r_baseNormalX", "1.0", CVAR_ARCHIVE | CVAR_LATCH);
	r_baseNormalY = ri->Cvar_Get("r_baseNormalY", "1.0", CVAR_ARCHIVE | CVAR_LATCH);
	r_baseParallax = ri->Cvar_Get("r_baseParallax", "0.05", CVAR_ARCHIVE | CVAR_LATCH);
	r_baseSpecular = ri->Cvar_Get("r_baseSpecular", "0.04", CVAR_ARCHIVE | CVAR_LATCH);
	r_baseGloss = ri->Cvar_Get("r_baseGloss", "0.45", CVAR_ARCHIVE | CVAR_LATCH);
	r_dlightMode = ri->Cvar_Get("r_dlightMode", "1", CVAR_ARCHIVE | CVAR_LATCH);
	r_pshadowDist = ri->Cvar_Get("r_pshadowDist", "128", CVAR_ARCHIVE);
	r_mergeLightmaps = ri->Cvar_Get("r_mergeLightmaps", "0", CVAR_ARCHIVE | CVAR_LATCH);
	r_imageUpsample = ri->Cvar_Get("r_imageUpsample", "0", CVAR_ARCHIVE | CVAR_LATCH);
	r_imageUpsampleMaxSize = ri->Cvar_Get("r_imageUpsampleMaxSize", "1024", CVAR_ARCHIVE | CVAR_LATCH);
	r_imageUpsampleType = ri->Cvar_Get("r_imageUpsampleType", "1", CVAR_ARCHIVE | CVAR_LATCH);
	r_genNormalMaps = ri->Cvar_Get("r_genNormalMaps", "1", CVAR_ARCHIVE | CVAR_LATCH);
	r_forceSun = ri->Cvar_Get("r_forceSun", "0", CVAR_CHEAT);
	r_forceSunMapLightScale = ri->Cvar_Get("r_forceSunMapLightScale", "1.0", CVAR_CHEAT);
	r_forceSunLightScale = ri->Cvar_Get("r_forceSunLightScale", "1.0", CVAR_CHEAT);
	r_forceSunAmbientScale = ri->Cvar_Get("r_forceSunAmbientScale", "0.5", CVAR_CHEAT);
	r_drawSunRays = ri->Cvar_Get("r_drawSunRays", "1", CVAR_ARCHIVE | CVAR_LATCH);
	r_sunlightMode = ri->Cvar_Get("r_sunlightMode", "1", CVAR_ARCHIVE | CVAR_LATCH);
	r_sunShadows = ri->Cvar_Get("r_sunShadows", "1", CVAR_ARCHIVE | CVAR_LATCH);
	r_shadowFilter = ri->Cvar_Get("r_shadowFilter", "1", CVAR_ARCHIVE | CVAR_LATCH);
	r_shadowMapSize = ri->Cvar_Get("r_shadowMapSize", "1024", CVAR_ARCHIVE | CVAR_LATCH);
	r_shadowCascadeZNear = ri->Cvar_Get("r_shadowCascadeZNear", "8", CVAR_ARCHIVE | CVAR_LATCH);
	r_shadowCascadeZFar = ri->Cvar_Get("r_shadowCascadeZFar", "3072", CVAR_ARCHIVE | CVAR_LATCH);
	r_shadowCascadeZBias = ri->Cvar_Get("r_shadowCascadeZBias", "64", CVAR_ARCHIVE | CVAR_LATCH);
	r_ignoreDstAlpha = ri->Cvar_Get("r_ignoreDstAlpha", "0", CVAR_ARCHIVE | CVAR_LATCH);

	//
	// temporary latched variables that can only change over a restart
	//
	r_fullbright = ri->Cvar_Get("r_fullbright", "0", CVAR_LATCH | CVAR_CHEAT);
	r_mapOverBrightBits = ri->Cvar_Get("r_mapOverBrightBits", "0", CVAR_LATCH);
	r_intensity = ri->Cvar_Get("r_intensity", "1", CVAR_LATCH);
	r_singleShader = ri->Cvar_Get("r_singleShader", "0", CVAR_CHEAT | CVAR_LATCH);

	//
	// archived variables that can change at any time
	//
	r_lodCurveError = ri->Cvar_Get("r_lodCurveError", "250", CVAR_ARCHIVE | CVAR_CHEAT);
	r_lodbias = ri->Cvar_Get("r_lodbias", "0", CVAR_ARCHIVE);
	r_flares = ri->Cvar_Get("r_flares", "1", CVAR_ARCHIVE);
	r_znear = ri->Cvar_Get("r_znear", "4", CVAR_CHEAT);
	ri->Cvar_CheckRange(r_znear, 0.001f, 200, qfalse);
	r_autolodscalevalue = ri->Cvar_Get("r_autolodscalevalue", "0", CVAR_ROM);
	r_zproj = ri->Cvar_Get("r_zproj", "64", CVAR_ARCHIVE);
	r_stereoSeparation = ri->Cvar_Get("r_stereoSeparation", "64", CVAR_ARCHIVE);
	r_ignoreGLErrors = ri->Cvar_Get("r_ignoreGLErrors", "1", CVAR_ARCHIVE);
	r_fastsky = ri->Cvar_Get("r_fastsky", "0", CVAR_ARCHIVE);
	r_inGameVideo = ri->Cvar_Get("r_inGameVideo", "1", CVAR_ARCHIVE);
	r_drawSun = ri->Cvar_Get("r_drawSun", "0", CVAR_ARCHIVE);
	r_dynamiclight = ri->Cvar_Get("r_dynamiclight", "1", CVAR_ARCHIVE);
	r_finish = ri->Cvar_Get("r_finish", "0", CVAR_ARCHIVE);
	r_textureMode = ri->Cvar_Get("r_textureMode", "GL_LINEAR_MIPMAP_LINEAR", CVAR_ARCHIVE);
	r_markcount = ri->Cvar_Get("r_markcount", "100", CVAR_ARCHIVE);
	r_gamma = ri->Cvar_Get("r_gamma", "1", CVAR_ARCHIVE);
	r_facePlaneCull = ri->Cvar_Get("r_facePlaneCull", "1", CVAR_ARCHIVE);
	r_ambientScale = ri->Cvar_Get("r_ambientScale", "0.6", CVAR_CHEAT);
	r_directedScale = ri->Cvar_Get("r_directedScale", "1", CVAR_CHEAT);
	r_anaglyphMode = ri->Cvar_Get("r_anaglyphMode", "0", CVAR_ARCHIVE);
	r_mergeMultidraws = ri->Cvar_Get("r_mergeMultidraws", "1", CVAR_ARCHIVE);
	r_mergeLeafSurfaces = ri->Cvar_Get("r_mergeLeafSurfaces", "1", CVAR_ARCHIVE);

	//
	// temporary variables that can change at any time
	//
	r_showImages = ri->Cvar_Get("r_showImages", "0", CVAR_TEMP);

	r_debugLight = ri->Cvar_Get("r_debuglight", "0", CVAR_TEMP);
	r_debugSort = ri->Cvar_Get("r_debugSort", "0", CVAR_CHEAT);
	r_printShaders = ri->Cvar_Get("r_printShaders", "0", 0);
	r_saveFontData = ri->Cvar_Get("r_saveFontData", "0", 0);
	r_nocurves = ri->Cvar_Get("r_nocurves", "0", CVAR_CHEAT);
	r_drawworld = ri->Cvar_Get("r_drawworld", "1", CVAR_CHEAT);
	r_lightmap = ri->Cvar_Get("r_lightmap", "0", 0);
	r_portalOnly = ri->Cvar_Get("r_portalOnly", "0", CVAR_CHEAT);
	r_flareSize = ri->Cvar_Get("r_flareSize", "40", CVAR_CHEAT);
	r_flareFade = ri->Cvar_Get("r_flareFade", "7", CVAR_CHEAT);
	r_flareCoeff = ri->Cvar_Get("r_flareCoeff", FLARE_STDCOEFF, CVAR_CHEAT);
	r_skipBackEnd = ri->Cvar_Get("r_skipBackEnd", "0", CVAR_CHEAT);
	r_measureOverdraw = ri->Cvar_Get("r_measureOverdraw", "0", CVAR_CHEAT);
	r_lodscale = ri->Cvar_Get("r_lodscale", "5", CVAR_CHEAT);
	r_norefresh = ri->Cvar_Get("r_norefresh", "0", CVAR_CHEAT);
	r_drawentities = ri->Cvar_Get("r_drawentities", "1", CVAR_CHEAT);
	r_ignore = ri->Cvar_Get("r_ignore", "1", CVAR_CHEAT);
	r_nocull = ri->Cvar_Get("r_nocull", "0", CVAR_CHEAT);
	r_novis = ri->Cvar_Get("r_novis", "0", CVAR_CHEAT);
	r_showcluster = ri->Cvar_Get("r_showcluster", "0", CVAR_CHEAT);
	r_speeds = ri->Cvar_Get("r_speeds", "0", CVAR_CHEAT);
	r_verbose = ri->Cvar_Get("r_verbose", "0", CVAR_CHEAT);
	r_logFile = ri->Cvar_Get("r_logFile", "0", CVAR_CHEAT);
	r_debugSurface = ri->Cvar_Get("r_debugSurface", "0", CVAR_CHEAT);
	r_nobind = ri->Cvar_Get("r_nobind", "0", CVAR_CHEAT);
	r_showtris = ri->Cvar_Get("r_showtris", "0", CVAR_CHEAT);
	r_showsky = ri->Cvar_Get("r_showsky", "0", CVAR_CHEAT);
	r_shownormals = ri->Cvar_Get("r_shownormals", "0", CVAR_CHEAT);
	r_clear = ri->Cvar_Get("r_clear", "0", CVAR_CHEAT);
	r_offsetFactor = ri->Cvar_Get("r_offsetfactor", "-1", CVAR_CHEAT);
	r_offsetUnits = ri->Cvar_Get("r_offsetunits", "-2", CVAR_CHEAT);
	r_drawBuffer = ri->Cvar_Get("r_drawBuffer", "GL_BACK", CVAR_CHEAT);
	r_lockpvs = ri->Cvar_Get("r_lockpvs", "0", CVAR_CHEAT);
	r_noportals = ri->Cvar_Get("r_noportals", "0", CVAR_CHEAT);
	r_shadows = ri->Cvar_Get("cg_shadows", "4", 0);
	r_marksOnTriangleMeshes = ri->Cvar_Get("r_marksOnTriangleMeshes", "1", CVAR_ARCHIVE);
	com_buildScript = ri->Cvar_Get("com_buildScript", "0", 0);
	r_screenshotJpegQuality = ri->Cvar_Get("r_screenshotJpegQuality", "90", CVAR_ARCHIVE);
	r_surfaceSprites = ri->Cvar_Get("r_surfaceSprites", "1", CVAR_ARCHIVE);
	r_aspectCorrectFonts = ri->Cvar_Get("r_aspectCorrectFonts", "0", CVAR_ARCHIVE);
	r_maxpolys = ri->Cvar_Get("r_maxpolys", XSTRING(DEFAULT_MAX_POLYS), 0);
	r_maxpolyverts = ri->Cvar_Get("r_maxpolyverts", XSTRING(DEFAULT_MAX_POLYVERTS), 0);

	/*
	Ghoul2 Insert Start
	*/
#ifdef _DEBUG
	r_noPrecacheGLA = ri->Cvar_Get("r_noPrecacheGLA", "0", CVAR_CHEAT);
#endif
	r_noGhoul2 = ri->Cvar_Get("r_noghoul2", "0", CVAR_CHEAT);
	r_Ghoul2AnimSmooth = ri->Cvar_Get("r_ghoul2animsmooth", "0.25", 0);
	r_Ghoul2UnSqash = ri->Cvar_Get("r_ghoul2unsquash", "1", 0);
	r_Ghoul2TimeBase = ri->Cvar_Get("r_ghoul2timebase", "2", 0);
	r_Ghoul2NoLerp = ri->Cvar_Get("r_ghoul2nolerp", "0", 0);
	r_Ghoul2NoBlend = ri->Cvar_Get("r_ghoul2noblend", "0", 0);
	r_Ghoul2BlendMultiplier = ri->Cvar_Get("r_ghoul2blendmultiplier", "1", 0);
	r_Ghoul2UnSqashAfterSmooth = ri->Cvar_Get("r_ghoul2unsquashaftersmooth", "1", 0);
	broadsword = ri->Cvar_Get("broadsword", "0", CVAR_ARCHIVE);
	broadsword_kickbones = ri->Cvar_Get("broadsword_kickbones", "1", 0);
	broadsword_kickorigin = ri->Cvar_Get("broadsword_kickorigin", "1", 0);
	broadsword_dontstopanim = ri->Cvar_Get("broadsword_dontstopanim", "0", 0);
	broadsword_waitforshot = ri->Cvar_Get("broadsword_waitforshot", "0", 0);
	broadsword_playflop = ri->Cvar_Get("broadsword_playflop", "1", 0);
	broadsword_smallbbox = ri->Cvar_Get("broadsword_smallbbox", "0", 0);
	broadsword_extra1 = ri->Cvar_Get("broadsword_extra1", "0", 0);
	broadsword_extra2 = ri->Cvar_Get("broadsword_extra2", "0", 0);
	broadsword_effcorr = ri->Cvar_Get("broadsword_effcorr", "1", 0);
	broadsword_ragtobase = ri->Cvar_Get("broadsword_ragtobase", "2", 0);
	broadsword_dircap = ri->Cvar_Get("broadsword_dircap", "64", 0);
	/*
	Ghoul2 Insert End
	*/

	sv_mapname = ri->Cvar_Get("mapname", "nomap", CVAR_SERVERINFO | CVAR_ROM);
	sv_mapChecksum = ri->Cvar_Get("sv_mapChecksum", "", CVAR_ROM);
	se_language = ri->Cvar_Get("se_language", "english", CVAR_ARCHIVE | CVAR_NORESTART);

	for (size_t i = 0; i < numCommands; i++)
		ri->Cmd_AddCommand(commands[i].cmd, commands[i].func);
}

void R_InitQueries(void)
{
	if (r_drawSunRays->integer)
		qglGenQueries(ARRAY_LEN(tr.sunFlareQuery), tr.sunFlareQuery);
}

void R_ShutDownQueries(void)
{
	if (r_drawSunRays->integer)
		qglDeleteQueries(ARRAY_LEN(tr.sunFlareQuery), tr.sunFlareQuery);
}

void RE_SetLightStyle(int style, int color);

static void R_InitBackEndFrameData()
{
	GLuint timerQueries[MAX_GPU_TIMERS*MAX_FRAMES];
	qglGenQueries(MAX_GPU_TIMERS*MAX_FRAMES, timerQueries);

	GLuint ubos[MAX_FRAMES];
	qglGenBuffers(MAX_FRAMES, ubos);

	for (int i = 0; i < MAX_FRAMES; i++)
	{
		gpuFrame_t *frame = backEndData->frames + i;
		const GLbitfield mapBits = GL_MAP_WRITE_BIT | GL_MAP_COHERENT_BIT | GL_MAP_PERSISTENT_BIT;

		frame->ubo = ubos[i];
		frame->uboWriteOffset = 0;
		qglBindBuffer(GL_UNIFORM_BUFFER, frame->ubo);

		// TODO: persistently mapped UBOs
		qglBufferData(GL_UNIFORM_BUFFER, FRAME_UNIFORM_BUFFER_SIZE,
			nullptr, GL_DYNAMIC_DRAW);

		frame->dynamicVbo = R_CreateVBO(nullptr, FRAME_VERTEX_BUFFER_SIZE,
			VBO_USAGE_DYNAMIC);
		frame->dynamicVboCommitOffset = 0;
		frame->dynamicVboWriteOffset = 0;

		frame->dynamicIbo = R_CreateIBO(nullptr, FRAME_INDEX_BUFFER_SIZE,
			VBO_USAGE_DYNAMIC);
		frame->dynamicIboCommitOffset = 0;
		frame->dynamicIboWriteOffset = 0;

		if (glRefConfig.immutableBuffers)
		{
			R_BindVBO(frame->dynamicVbo);
			frame->dynamicVboMemory = qglMapBufferRange(GL_ARRAY_BUFFER, 0,
				frame->dynamicVbo->vertexesSize, mapBits);

			R_BindIBO(frame->dynamicIbo);
			frame->dynamicIboMemory = qglMapBufferRange(GL_ELEMENT_ARRAY_BUFFER, 0,
				frame->dynamicIbo->indexesSize, mapBits);
		}
		else
		{
			frame->dynamicVboMemory = nullptr;
			frame->dynamicIboMemory = nullptr;
		}

		for (int j = 0; j < MAX_GPU_TIMERS; j++)
		{
			gpuTimer_t *timer = frame->timers + j;
			timer->queryName = timerQueries[i*MAX_GPU_TIMERS + j];
		}
	}

	backEndData->currentFrame = backEndData->frames;
}

static void R_ShutdownBackEndFrameData()
{
	if (!backEndData)
		return;

	for (int i = 0; i < MAX_FRAMES; i++)
	{
		gpuFrame_t *frame = backEndData->frames + i;

		qglDeleteBuffers(1, &frame->ubo);

		if (glRefConfig.immutableBuffers)
		{
			R_BindVBO(frame->dynamicVbo);
			R_BindIBO(frame->dynamicIbo);
			qglUnmapBuffer(GL_ARRAY_BUFFER);
			qglUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
		}

		for (int j = 0; j < MAX_GPU_TIMERS; j++)
		{
			gpuTimer_t *timer = frame->timers + j;
			qglDeleteQueries(1, &timer->queryName);
		}
	}
}

// need to do this hackery so ghoul2 doesn't crash the game because of ITS hackery...
//
void R_ClearStuffToStopGhoul2CrashingThings(void)
{
	memset(&tr, 0, sizeof(tr));
}

/*
===============
R_Init
===============
*/
void R_Init(void) {
	byte *ptr;
	int i;

	ri->Printf(PRINT_ALL, "----- R_Init -----\n");

	ShaderEntryPtrs_Clear();

	// clear all our internal state
	Com_Memset(&tr, 0, sizeof(tr));
	Com_Memset(&backEnd, 0, sizeof(backEnd));
	Com_Memset(&tess, 0, sizeof(tess));


	//
	// init function tables
	//
	for (i = 0; i < FUNCTABLE_SIZE; i++)
	{
		tr.sinTable[i] = sin(DEG2RAD(i * 360.0f / ((float)(FUNCTABLE_SIZE - 1))));
		tr.squareTable[i] = (i < FUNCTABLE_SIZE / 2) ? 1.0f : -1.0f;
		tr.sawToothTable[i] = (float)i / FUNCTABLE_SIZE;
		tr.inverseSawToothTable[i] = 1.0f - tr.sawToothTable[i];

		if (i < FUNCTABLE_SIZE / 2)
		{
			if (i < FUNCTABLE_SIZE / 4)
			{
				tr.triangleTable[i] = (float)i / (FUNCTABLE_SIZE / 4);
			}
			else
			{
				tr.triangleTable[i] = 1.0f - tr.triangleTable[i - FUNCTABLE_SIZE / 4];
			}
		}
		else
		{
			tr.triangleTable[i] = -tr.triangleTable[i - FUNCTABLE_SIZE / 2];
		}
	}

	R_InitFogTable();

	R_ImageLoader_Init();
	R_NoiseInit();
	R_Register();

	max_polys = Q_min(r_maxpolys->integer, DEFAULT_MAX_POLYS);
	max_polyverts = Q_min(r_maxpolyverts->integer, DEFAULT_MAX_POLYVERTS);

	ptr = (byte*)R_Hunk_Alloc(
		sizeof(*backEndData) + 
		sizeof(srfPoly_t)* max_polys + 
		sizeof(polyVert_t)* max_polyverts + 
		sizeof(Allocator) +	
		PER_FRAME_MEMORY_BYTES, 
		qtrue);

	backEndData = (backEndData_t *)ptr;
	ptr = (byte *)(backEndData + 1);

	backEndData->polys = (srfPoly_t *)ptr;
	ptr += sizeof(*backEndData->polys) * max_polys;

	backEndData->polyVerts = (polyVert_t *)ptr;
	ptr += sizeof(*backEndData->polyVerts) * max_polyverts;

	backEndData->perFrameMemory = new(ptr) Allocator(ptr + sizeof(*backEndData->perFrameMemory), PER_FRAME_MEMORY_BYTES);

	R_InitNextFrame();

	for (int i = 0; i < MAX_LIGHT_STYLES; i++)
	{
		RE_SetLightStyle(i, -1);
	}

	InitOpenGL();

	R_InitVBOs();

	R_InitBackEndFrameData();
	R_InitImages();
	
	FBO_Init();

	GLSL_LoadGPUShaders();

	R_InitShaders();

	R_InitSkins();

	R_InitFonts();

	R_ModelInit();

	R_InitDecals();

	R_InitQueries();

	R_InitWeatherSystem();

#if defined(_DEBUG)
	GLenum err = qglGetError();
	if (err != GL_NO_ERROR)
		ri->Printf(PRINT_ALL, "glGetError() = 0x%x\n", err);
#endif

	RestoreGhoul2InfoArray();

	// print info
	GfxInfo_f();
	ri->Printf(PRINT_ALL, "----- finished R_Init -----\n");
}

/*
===============
RE_Shutdown
===============
*/
void RE_Shutdown(qboolean destroyWindow, qboolean restarting) {

	ri->Printf(PRINT_ALL, "RE_Shutdown( %i )\n", destroyWindow);

	for (size_t i = 0; i < numCommands; i++)
		ri->Cmd_RemoveCommand(commands[i].cmd);

	R_ShutdownBackEndFrameData();

	R_ShutdownWeatherSystem();

	R_ShutdownFonts();
	if (tr.registered) {
		R_IssuePendingRenderCommands();
		R_ShutDownQueries();
		FBO_Shutdown();
		R_DeleteTextures();
		R_ShutdownVBOs();
		GLSL_ShutdownGPUShaders();

		if (destroyWindow && restarting)
		{
			ri->Z_Free((void *)glConfig.extensions_string);
			ri->Z_Free((void *)glConfigExt.originalExtensionString);

			qglDeleteVertexArrays(1, &tr.globalVao);
			SaveGhoul2InfoArray();
		}
	}

	// shut down platform specific OpenGL stuff
	if (destroyWindow) {
		ri->WIN_Shutdown();
	}

	tr.registered = qfalse;
	backEndData = NULL;
}

/*
=============
RE_EndRegistration

Touch all images to make sure they are resident
=============
*/
void RE_EndRegistration(void) {
	R_IssuePendingRenderCommands();
	/*if (!ri->Sys_LowPhysicalMemory()) {
		RB_ShowImages();
	}*/
}

// HACK
//extern qboolean gG2_GBMNoReconstruct;
//extern qboolean gG2_GBMUseSPMethod;
//static void G2API_BoltMatrixReconstruction(qboolean reconstruct) { gG2_GBMNoReconstruct = (qboolean)!reconstruct; }
//static void G2API_BoltMatrixSPMethod(qboolean spMethod) { gG2_GBMUseSPMethod = spMethod; }

//static float GetDistanceCull(void) { return tr.distanceCull; }

/*static void GetRealRes(int *w, int *h) {
	*w = glConfig.vidWidth;
	*h = glConfig.vidHeight;
}*/

void RE_GetLightStyle(int style, color4ub_t color)
{
	if (style >= MAX_LIGHT_STYLES)
	{
		Com_Error(ERR_FATAL, "RE_GetLightStyle: %d is out of range", style);
		return;
	}

	byteAlias_t *baDest = (byteAlias_t *)&color, *baSource = (byteAlias_t *)&styleColors[style];
	baDest->i = baSource->i;
}

void RE_SetLightStyle(int style, int color)
{
	if (style >= MAX_LIGHT_STYLES)
	{
		Com_Error(ERR_FATAL, "RE_SetLightStyle: %d is out of range", style);
		return;
	}

	byteAlias_t *ba = (byteAlias_t *)&styleColors[style];
	if (ba->i != color) {
		ba->i = color;
	}
}

// STUBS, REPLACEME
void stub_RE_GetBModelVerts(int bModel, vec3_t *vec, float *normal) {}
void stub_RE_WorldEffectCommand(const char *cmd) {}
void stub_RE_AddWeatherZone(vec3_t mins, vec3_t maxs) {}
//static void RE_SetRefractionProperties(float distortionAlpha, float distortionStretch, qboolean distortionPrePost, qboolean distortionNegate) { }
qboolean stub_RE_ProcessDissolve(void) { return qfalse; }
qboolean stub_RE_InitDissolve(qboolean bForceCircularExtroWipe) { return qfalse; }
bool stub_R_IsShaking(vec3_t pos) { return qfalse; }
void stub_R_InitWorldEffects(void){}
bool stub_R_GetWindVector(vec3_t windVector, vec3_t atpoint) { return qfalse; }
bool stub_R_GetWindGusting(vec3_t atpoint){ return qfalse; }
bool stub_R_IsOutside(vec3_t pos) { return qfalse; }
float stub_R_IsOutsideCausingPain(vec3_t pos) { return qfalse; }
float stub_R_GetChanceOfSaberFizz(){return qfalse;}
qboolean stub_RE_GetLighting(const vec3_t origin, vec3_t ambientLight, vec3_t directedLight, vec3_t lightDir) { return qfalse; }
bool stub_R_SetTempGlobalFogColor(vec3_t color) { return qfalse; }
void stub_RE_GetScreenShot(byte *buffer, int w, int h){}

void C_LevelLoadBegin(const char *psMapName, ForceReload_e eForceReload, qboolean bAllowScreenDissolve)
{
	static char sPrevMapName[MAX_QPATH] = { 0 };
	bool bDeleteModels = eForceReload == eForceReload_MODELS || eForceReload == eForceReload_ALL;

	if (bDeleteModels)
		CModelCache->DeleteAll();
	/*else if (ri->Cvar_VariableIntegerValue("sv_pure"))
		CModelCache->DumpNonPure();*/

	tr.numBSPModels = 0;

	/* If we're switching to the same level, don't increment current level */
	if (Q_stricmp(psMapName, sPrevMapName))
	{
		Q_strncpyz(sPrevMapName, psMapName, sizeof(sPrevMapName));
		tr.currentLevel++;
	}
}

int C_GetLevel(void)
{
	return tr.currentLevel;
}

void C_LevelLoadEnd(void)
{
	CModelCache->LevelLoadEnd(qfalse);
	ri->SND_RegisterAudio_LevelLoadEnd(qfalse);
	ri->S_RestartMusic();
}

//bool inServer = false;
void RE_SVModelInit(void)
{
	tr.numModels = 0;
	tr.numShaders = 0;
	tr.numSkins = 0;
	R_InitImages();
	//inServer = true;
	R_InitShaders();
	//inServer = false;
	R_ModelInit();
}

/*
@@@@@@@@@@@@@@@@@@@@@
GetRefAPI

@@@@@@@@@@@@@@@@@@@@@
*/
extern qboolean R_inPVS(vec3_t p1, vec3_t p2);
extern void G2API_AnimateG2Models(CGhoul2Info_v &ghoul2, int AcurrentTime, CRagDollUpdateParams *params);
extern qboolean G2API_GetRagBonePos(CGhoul2Info_v &ghoul2, const char *boneName, vec3_t pos, vec3_t entAngles, vec3_t entPos, vec3_t entScale);
extern qboolean G2API_IKMove(CGhoul2Info_v &ghoul2, int time, sharedIKMoveParams_t *params);
extern qboolean G2API_RagEffectorGoal(CGhoul2Info_v &ghoul2, const char *boneName, vec3_t pos);
extern qboolean G2API_RagEffectorKick(CGhoul2Info_v &ghoul2, const char *boneName, vec3_t velocity);
extern qboolean G2API_RagForceSolve(CGhoul2Info_v &ghoul2, qboolean force);
extern qboolean G2API_RagPCJConstraint(CGhoul2Info_v &ghoul2, const char *boneName, vec3_t min, vec3_t max);
extern qboolean G2API_RagPCJGradientSpeed(CGhoul2Info_v &ghoul2, const char *boneName, const float speed);
extern qboolean G2API_SetBoneIKState(CGhoul2Info_v &ghoul2, int time, const char *boneName, int ikState, sharedSetBoneIKStateParams_t *params);
extern void G2API_SetRagDoll(CGhoul2Info_v &ghoul2, CRagDollParams *parms);
extern IGhoul2InfoArray &TheGhoul2InfoArray();

#ifdef JK2_MODE
unsigned int AnyLanguage_ReadCharFromString_JK2(char **text, qboolean *pbIsTrailingPunctuation) {
	return AnyLanguage_ReadCharFromString(text, pbIsTrailingPunctuation);
}
#endif

extern "C" Q_EXPORT refexport_t* QDECL GetRefAPI(int apiVersion, refimport_t *rimp) {
	static refexport_t	re;

	assert(rimp);
	ri = rimp;

	Com_Memset(&re, 0, sizeof(re));

	if (apiVersion != REF_API_VERSION) {
		ri->Printf(PRINT_ALL, "Mismatched REF_API_VERSION: expected %i, got %i\n",
			REF_API_VERSION, apiVersion);
		return NULL;
	}

	// the RE_ functions are Renderer Entry points

#define REX(x)	re.x = RE_##x

	REX(Shutdown);

	REX(BeginRegistration);
	REX(RegisterModel);
	REX(RegisterSkin);
	REX(GetAnimationCFG);
	REX(RegisterShader);
	REX(RegisterShaderNoMip);
	re.LoadWorld = RE_LoadWorldMap;
	re.R_LoadImage = R_LoadImage;

	re.RegisterMedia_LevelLoadBegin = C_LevelLoadBegin;
	re.RegisterMedia_LevelLoadEnd = C_LevelLoadEnd;
	re.RegisterMedia_GetLevel = C_GetLevel;
	re.RegisterImages_LevelLoadEnd = C_Images_LevelLoadEnd;
	re.RegisterModels_LevelLoadEnd = C_Models_LevelLoadEnd;

	REX(SetWorldVisData);

	REX(EndRegistration);

	REX(ClearScene);
	REX(ClearDecals);
	REX(AddRefEntityToScene);
	re.GetLighting = stub_RE_GetLighting;
	REX(AddPolyToScene);
	re.LightForPoint = R_LightForPoint;
	REX(AddDecalToScene);
	REX(AddLightToScene);
	REX(AddAdditiveLightToScene);
	REX(RenderScene);
	re.GetLighting = stub_RE_GetLighting;

	REX(SetColor);
	re.DrawStretchPic = RE_StretchPic;
	re.DrawRotatePic = RE_RotatePic;
	re.DrawRotatePic2 = RE_RotatePic2;
	re.RotatePic2RatioFix = RE_RotatePic2RatioFix;
	REX(LAGoggles);
	REX(Scissor);

	re.DrawStretchRaw = RE_StretchRaw;
	REX(UploadCinematic);

	REX(BeginFrame);
	REX(EndFrame);

	re.ProcessDissolve = stub_RE_ProcessDissolve;
	re.InitDissolve = stub_RE_InitDissolve;

	re.GetScreenShot = stub_RE_GetScreenShot;

	//REX(TempRawImage_ReadFromFile);  //JK2 only
	//REX(TempRawImage_CleanUp);  //JK2 only

	re.MarkFragments = R_MarkFragments;
	re.LerpTag = R_LerpTag;
	re.ModelBounds = R_ModelBounds;
	REX(GetLightStyle);
	REX(SetLightStyle);
	re.GetBModelVerts = stub_RE_GetBModelVerts;
	re.WorldEffectCommand = stub_RE_WorldEffectCommand;
	//REX(GetModelBounds);  //Not used by game code, do we really need it?

	re.SVModelInit = RE_SVModelInit;

	REX(RegisterFont);
	REX(Font_HeightPixels);
	REX(Font_StrLenPixels);
	REX(Font_DrawString);
	REX(Font_StrLenChars);
	re.FontRatioFix = RE_FontRatioFix;
	re.Language_IsAsian = Language_IsAsian;
	re.Language_UsesSpaces = Language_UsesSpaces;
	re.AnyLanguage_ReadCharFromString = AnyLanguage_ReadCharFromString;
#ifdef JK2_MODE
	re.AnyLanguage_ReadCharFromString2 = AnyLanguage_ReadCharFromString_JK2;
#endif
		
	re.R_InitWorldEffects = stub_R_InitWorldEffects;
	re.R_ClearStuffToStopGhoul2CrashingThings = R_ClearStuffToStopGhoul2CrashingThings;
	re.R_inPVS = R_inPVS;

	re.GetWindVector = stub_R_GetWindVector;
	re.GetWindGusting = stub_R_GetWindGusting;
	re.IsOutside = stub_R_IsOutside;
	re.IsOutsideCausingPain = stub_R_IsOutsideCausingPain;
	re.GetChanceOfSaberFizz = stub_R_GetChanceOfSaberFizz;
	re.IsShaking = stub_R_IsShaking;
	re.AddWeatherZone = stub_RE_AddWeatherZone;
	re.SetTempGlobalFogColor = stub_R_SetTempGlobalFogColor;

	REX(SetRangedFog);

	re.TheGhoul2InfoArray = TheGhoul2InfoArray;

	re.GetEntityToken = R_GetEntityToken;  //MP only, but need this for cubemaps...

#define G2EX(x)	re.G2API_##x = G2API_##x

	G2EX(AddBolt);
	G2EX(AddBoltSurfNum);
	G2EX(AddSurface);
	G2EX(AnimateG2Models);
	G2EX(AttachEnt);
	G2EX(AttachG2Model);
	G2EX(CollisionDetect);
	G2EX(CleanGhoul2Models);
	G2EX(CopyGhoul2Instance);
	G2EX(DetachEnt);
	G2EX(DetachG2Model);
	G2EX(GetAnimFileName);
	G2EX(GetAnimFileNameIndex);
	G2EX(GetAnimFileInternalNameIndex);
	G2EX(GetAnimIndex);
	G2EX(GetAnimRange);
	G2EX(GetAnimRangeIndex);
	G2EX(GetBoneAnim);
	G2EX(GetBoneAnimIndex);
	G2EX(GetBoneIndex);
	G2EX(GetBoltMatrix);
	G2EX(GetGhoul2ModelFlags);
	G2EX(GetGLAName);
	G2EX(GetParentSurface);
	G2EX(GetRagBonePos);
	G2EX(GetSurfaceIndex);
	G2EX(GetSurfaceName);
	G2EX(GetSurfaceRenderStatus);
	G2EX(GetTime);
	G2EX(GiveMeVectorFromMatrix);
	G2EX(HaveWeGhoul2Models);
	G2EX(IKMove);
	G2EX(InitGhoul2Model);
	G2EX(IsPaused);
	G2EX(ListBones);
	G2EX(ListSurfaces);
	G2EX(LoadGhoul2Models);
	G2EX(LoadSaveCodeDestructGhoul2Info);
	G2EX(PauseBoneAnim);
	G2EX(PauseBoneAnimIndex);
	G2EX(PrecacheGhoul2Model);
	G2EX(RagEffectorGoal);
	G2EX(RagEffectorKick);
	G2EX(RagForceSolve);
	G2EX(RagPCJConstraint);
	G2EX(RagPCJGradientSpeed);
	G2EX(RemoveBolt);
	G2EX(RemoveBone);
	G2EX(RemoveGhoul2Model);
	G2EX(RemoveSurface);
	G2EX(SaveGhoul2Models);
	G2EX(SetAnimIndex);
	G2EX(SetBoneAnim);
	G2EX(SetBoneAnimIndex);
	G2EX(SetBoneAngles);
	G2EX(SetBoneAnglesIndex);
	G2EX(SetBoneAnglesMatrix);
	G2EX(SetBoneIKState);
	G2EX(SetGhoul2ModelFlags);
	G2EX(SetGhoul2ModelIndexes);
	G2EX(SetLodBias);
	//G2EX(SetModelIndexes);
	G2EX(SetNewOrigin);
	G2EX(SetRagDoll);
	G2EX(SetRootSurface);
	G2EX(SetShader);
	G2EX(SetSkin);
	G2EX(SetSurfaceOnOff);
	G2EX(SetTime);
	G2EX(StopBoneAnim);
	G2EX(StopBoneAnimIndex);
	G2EX(StopBoneAngles);
	G2EX(StopBoneAnglesIndex);
#ifdef _G2_GORE
	G2EX(AddSkinGore);
	G2EX(ClearSkinGore);
#endif

#ifdef G2_PERFORMANCE_ANALYSIS
	re.G2Time_ReportTimers = G2Time_ReportTimers;
	re.G2Time_ResetTimers = G2Time_ResetTimers;
#endif

	//Swap_Init();

#ifdef JK2_MODE
	re.TempRawImage_CleanUp = []() {};
	re.TempRawImage_ReadFromFile = [](const char* psLocalFileNmae, int *piWidth, int *piHeight, byte* pbReSampleBuffer, qboolean qbVertFlip) -> byte* { return NULL; };
	re.SaveJPGToBuffer = [](byte* buffer, size_t bufSize, int quality, int image_width, int image_height, byte* image_buffer, int padding, bool flip_vertical) -> size_t { return 0; };
#endif

	return &re;
}
