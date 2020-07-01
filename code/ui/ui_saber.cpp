/*
===========================================================================
Copyright (C) 2000 - 2013, Raven Software, Inc.
Copyright (C) 2001 - 2013, Activision, Inc.
Copyright (C) 2013 - 2015, OpenJK contributors

This file is part of the OpenJK source code.

OpenJK is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License version 2 as
published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, see <http://www.gnu.org/licenses/>.
===========================================================================
*/

/*
=======================================================================

USER INTERFACE SABER LOADING & DISPLAY CODE

=======================================================================
*/

// leave this at the top of all UI_xxxx files for PCH reasons...
//
#include "../server/exe_headers.h"
#include "ui_local.h"
#include "ui_shared.h"
#include "../ghoul2/G2.h"

#define MAX_SABER_DATA_SIZE 0x80000
char	SaberParms[MAX_SABER_DATA_SIZE];
qboolean	ui_saber_parms_parsed = qfalse;

extern vmCvar_t	ui_rgb_saber_red;
extern vmCvar_t	ui_rgb_saber_green;
extern vmCvar_t	ui_rgb_saber_blue;
extern vmCvar_t	ui_rgb_saber2_red;
extern vmCvar_t	ui_rgb_saber2_green;
extern vmCvar_t	ui_rgb_saber2_blue;

extern vmCvar_t	ui_saber_skin1;
extern vmCvar_t	ui_saber_skin2;
extern vmCvar_t	ui_saber_skin3;
extern vmCvar_t	ui_saber_skin4;
extern vmCvar_t	ui_saber_skin5;
extern vmCvar_t	ui_saber2_skin1;
extern vmCvar_t	ui_saber2_skin2;
extern vmCvar_t	ui_saber2_skin3;
extern vmCvar_t	ui_saber2_skin4;
extern vmCvar_t	ui_saber2_skin5;

extern vmCvar_t	ui_SFXSabers;
extern vmCvar_t	ui_SFXSabersGlowSize;
extern vmCvar_t	ui_SFXSabersCoreSize;

static qhandle_t redSaberGlowShader;
static qhandle_t redSaberCoreShader;
static qhandle_t orangeSaberGlowShader;
static qhandle_t orangeSaberCoreShader;
static qhandle_t yellowSaberGlowShader;
static qhandle_t yellowSaberCoreShader;
static qhandle_t greenSaberGlowShader;
static qhandle_t greenSaberCoreShader;
static qhandle_t blueSaberGlowShader;
static qhandle_t blueSaberCoreShader;
static qhandle_t purpleSaberGlowShader;
static qhandle_t purpleSaberCoreShader;
static qhandle_t rgbSaberGlowShader;
static qhandle_t rgbSaberCoreShader;
static qhandle_t blackSaberGlowShader;
static qhandle_t blackSaberCoreShader;
static qhandle_t rgbUnstableCoreShader;
static qhandle_t SaberBladeShader;
static qhandle_t blackSaberBladeShader;

void UI_CacheSaberGlowGraphics( void )
{//FIXME: these get fucked by vid_restarts
	redSaberGlowShader		= re.RegisterShader( "gfx/effects/sabers/red_glow" );
	redSaberCoreShader		= re.RegisterShader( "gfx/effects/sabers/red_line" );
	orangeSaberGlowShader		= re.RegisterShader( "gfx/effects/sabers/orange_glow" );
	orangeSaberCoreShader		= re.RegisterShader( "gfx/effects/sabers/orange_line" );
	yellowSaberGlowShader		= re.RegisterShader( "gfx/effects/sabers/yellow_glow" );
	yellowSaberCoreShader		= re.RegisterShader( "gfx/effects/sabers/yellow_line" );
	greenSaberGlowShader		= re.RegisterShader( "gfx/effects/sabers/green_glow" );
	greenSaberCoreShader		= re.RegisterShader( "gfx/effects/sabers/green_line" );
	blueSaberGlowShader		= re.RegisterShader( "gfx/effects/sabers/blue_glow" );
	blueSaberCoreShader		= re.RegisterShader( "gfx/effects/sabers/blue_line" );
	purpleSaberGlowShader		= re.RegisterShader( "gfx/effects/sabers/purple_glow" );
	purpleSaberCoreShader		= re.RegisterShader( "gfx/effects/sabers/purple_line" );
	rgbSaberGlowShader		= re.RegisterShader( "gfx/effects/sabers/rgb_glow" );
	rgbSaberCoreShader		= re.RegisterShader( "gfx/effects/sabers/rgb_line" );
	blackSaberGlowShader		= re.RegisterShader( "gfx/effects/sabers/black_glow" );
	blackSaberCoreShader		= re.RegisterShader( "gfx/effects/sabers/black_line" );
	rgbUnstableCoreShader		= re.RegisterShader("gfx/effects/sabers/jkg/blade_unstable");
	SaberBladeShader = re.RegisterShader( "SFX_Sabers/saber_blade" );
	blackSaberBladeShader = re.RegisterShader( "SFX_Sabers/black_blade" );
}

qboolean UI_ParseLiteral( const char **data, const char *string, qboolean silent )
{
	const char	*token;

	token = COM_ParseExt( data, qtrue );
	if ( token[0] == 0 )
	{
		if (!silent)
			ui.Printf( "unexpected EOF\n" );
		return qtrue;
	}

	if ( Q_stricmp( token, string ) )
	{
		if (!silent)
			ui.Printf( "required string '%s' missing\n", string );
		return qtrue;
	}

	return qfalse;
}

qboolean UI_SaberParseParm( const char *saberName, const char *parmname, char *saberData )
{
	const char	*token;
	const char	*value;
	const char	*p;

	if ( !saberName || !saberName[0] )
	{
		return qfalse;
	}

	//try to parse it out
	p = SaberParms;
	COM_BeginParseSession();

	// look for the right saber
	while ( p )
	{
		token = COM_ParseExt( &p, qtrue );
		if ( token[0] == 0 )
		{
			COM_EndParseSession(  );
			return qfalse;
		}

		if ( !Q_stricmp( token, saberName ) )
		{
			break;
		}

		SkipBracedSection( &p );
	}
	if ( !p )
	{
		COM_EndParseSession(  );
		return qfalse;
	}

	if ( UI_ParseLiteral( &p, "{", qfalse ) )
	{
		COM_EndParseSession(  );
		return qfalse;
	}

	// parse the saber info block
	while ( 1 )
	{
		token = COM_ParseExt( &p, qtrue );
		if ( !token[0] )
		{
			ui.Printf( S_COLOR_RED"ERROR: unexpected EOF while parsing '%s'\n", saberName );
			COM_EndParseSession(  );
			return qfalse;
		}

		if ( !Q_stricmp( token, "}" ) )
		{
			break;
		}

		if ( !Q_stricmp( token, parmname ) )
		{
			if ( COM_ParseString( &p, &value ) )
			{
				continue;
			}
			strcpy( saberData, value );
			COM_EndParseSession(  );
			return qtrue;
		}

		SkipRestOfLine( &p );
		continue;
	}

	COM_EndParseSession(  );
	return qfalse;
}

qboolean UI_SaberProperNameForSaber( const char *saberName, char *saberProperName )
{
	return UI_SaberParseParm( saberName, "name", saberProperName );
}

qboolean UI_SaberModelForSaber( const char *saberName, char *saberModel )
{
	return UI_SaberParseParm( saberName, "saberModel", saberModel );
}

qboolean UI_SaberSkinForSaber( const char *saberName, char *saberSkin, qboolean secondSaber )
{
	qboolean isCustomSaber = qfalse;
	for (int i = 0; i < uiInfo.customSabersCount; i++)
	{
		if (uiInfo.customSabers[i].SaberName && uiInfo.customSabers[i].SaberName[0] && !Q_stricmp(saberName, uiInfo.customSabers[i].SaberName))
		{
			isCustomSaber = qtrue;
			break;
		}
	}
	if (isCustomSaber)
	{
		char skinRoot[MAX_QPATH];
		if (!UI_SaberModelForSaber(saberName, skinRoot))
		{
			return qfalse;
		}
		
		int l = strlen(skinRoot);
		while (l > 0 && skinRoot[l] != '/')
		{ //parse back to first /
			l--;
		}
		
		if (skinRoot[l] == '/')
		{
			l++;
			skinRoot[l] = 0;
			
			{
				Q_strcat(skinRoot, MAX_QPATH, "|_");
				Q_strcat(skinRoot, MAX_QPATH, "|");
				if (secondSaber)
				{
					if (Cvar_VariableString("ui_saber2_skin1") && Cvar_VariableString("ui_saber2_skin1")[0])
					{
						Q_strcat(skinRoot, MAX_QPATH, Cvar_VariableString("ui_saber2_skin1"));
					}
					Q_strcat(skinRoot, MAX_QPATH, "|");
					if (Cvar_VariableString("ui_saber2_skin2") && Cvar_VariableString("ui_saber2_skin2")[0])
					{
						Q_strcat(skinRoot, MAX_QPATH, Cvar_VariableString("ui_saber2_skin2"));
					}
					Q_strcat(skinRoot, MAX_QPATH, "|");
					if (Cvar_VariableString("ui_saber2_skin3") && Cvar_VariableString("ui_saber2_skin3")[0])
					{
						Q_strcat(skinRoot, MAX_QPATH, Cvar_VariableString("ui_saber2_skin3"));
					}
					Q_strcat(skinRoot, MAX_QPATH, "|");
					if (Cvar_VariableString("ui_saber2_skin4") && Cvar_VariableString("ui_saber2_skin4")[0])
					{
						Q_strcat(skinRoot, MAX_QPATH, Cvar_VariableString("ui_saber2_skin4"));
					}
					Q_strcat(skinRoot, MAX_QPATH, "|");
					if (Cvar_VariableString("ui_saber2_skin5") && Cvar_VariableString("ui_saber2_skin5")[0])
					{
						Q_strcat(skinRoot, MAX_QPATH, Cvar_VariableString("ui_saber2_skin5"));
					}
				}
				else
				{
					if (Cvar_VariableString("ui_saber_skin1") && Cvar_VariableString("ui_saber_skin1")[0])
					{
						Q_strcat(skinRoot, MAX_QPATH, Cvar_VariableString("ui_saber_skin1"));
					}
					Q_strcat(skinRoot, MAX_QPATH, "|");
					if (Cvar_VariableString("ui_saber_skin2") && Cvar_VariableString("ui_saber_skin2")[0])
					{
						Q_strcat(skinRoot, MAX_QPATH, Cvar_VariableString("ui_saber_skin2"));
					}
					Q_strcat(skinRoot, MAX_QPATH, "|");
					if (Cvar_VariableString("ui_saber_skin3") && Cvar_VariableString("ui_saber_skin3")[0])
					{
						Q_strcat(skinRoot, MAX_QPATH, Cvar_VariableString("ui_saber_skin3"));
					}
					Q_strcat(skinRoot, MAX_QPATH, "|");
					if (Cvar_VariableString("ui_saber_skin4") && Cvar_VariableString("ui_saber_skin4")[0])
					{
						Q_strcat(skinRoot, MAX_QPATH, Cvar_VariableString("ui_saber_skin4"));
					}
					Q_strcat(skinRoot, MAX_QPATH, "|");
					if (Cvar_VariableString("ui_saber_skin5") && Cvar_VariableString("ui_saber_skin5")[0])
					{
						Q_strcat(skinRoot, MAX_QPATH, Cvar_VariableString("ui_saber_skin5"));
					}
				}
			}
			Q_strncpyz(saberSkin, skinRoot, MAX_QPATH);
			return qtrue;
			
		}
	}
	return UI_SaberParseParm( saberName, "customSkin", saberSkin );
}

qboolean UI_SaberTypeForSaber( const char *saberName, char *saberType )
{
	return UI_SaberParseParm( saberName, "saberType", saberType );
}

int UI_SaberNumBladesForSaber( const char *saberName )
{
	char	numBladesString[8]={0};
	UI_SaberParseParm( saberName, "numBlades", numBladesString );
	int numBlades = atoi( numBladesString );
	if ( numBlades < 1 )
	{
		numBlades = 1;
	}
	else if ( numBlades > 8 )
	{
		numBlades = 8;
	}
	return numBlades;
}

qboolean UI_SaberShouldDrawBlade( const char *saberName, int bladeNum )
{
	int bladeStyle2Start = 0, noBlade = 0;
	char	bladeStyle2StartString[8]={0};
	char	noBladeString[8]={0};
	UI_SaberParseParm( saberName, "bladeStyle2Start", bladeStyle2StartString );
	if ( bladeStyle2StartString[0] )
	{
		bladeStyle2Start = atoi( bladeStyle2StartString );
	}
	if ( bladeStyle2Start
		&& bladeNum >= bladeStyle2Start )
	{//use second blade style
		UI_SaberParseParm( saberName, "noBlade2", noBladeString );
		if ( noBladeString[0] )
		{
			noBlade = atoi( noBladeString );
		}
	}
	else
	{//use first blade style
		UI_SaberParseParm( saberName, "noBlade", noBladeString );
		if ( noBladeString[0] )
		{
			noBlade = atoi( noBladeString );
		}
	}
	return ((qboolean)(noBlade==0));
}

float UI_SaberBladeLengthForSaber( const char *saberName, int bladeNum )
{
	char	lengthString[8]={0};
	float	length = 40.0f;
	UI_SaberParseParm( saberName, "saberLength", lengthString );
	if ( lengthString[0] )
	{
		length = atof( lengthString );
		if ( length < 0.0f )
		{
			length = 0.0f;
		}
	}

	UI_SaberParseParm( saberName, va("saberLength%d", bladeNum+1), lengthString );
	if ( lengthString[0] )
	{
		length = atof( lengthString );
		if ( length < 0.0f )
		{
			length = 0.0f;
		}
	}

	return length;
}

float UI_SaberBladeRadiusForSaber( const char *saberName, int bladeNum )
{
	char	radiusString[8]={0};
	float	radius = 3.0f;
	UI_SaberParseParm( saberName, "saberRadius", radiusString );
	if ( radiusString[0] )
	{
		radius = atof( radiusString );
		if ( radius < 0.0f )
		{
			radius = 0.0f;
		}
	}

	UI_SaberParseParm( saberName, va("saberRadius%d", bladeNum+1), radiusString );
	if ( radiusString[0] )
	{
		radius = atof( radiusString );
		if ( radius < 0.0f )
		{
			radius = 0.0f;
		}
	}

	return radius;
}

qboolean UI_SaberValidForPlayerInSP(const char* saberName)
{
	char allowed[8] = { 0 };
	if (!UI_SaberParseParm(saberName, "notInMP", allowed))
	{//not defined, default is yes
		return qtrue;
	}
	if (!allowed[0])
	{//not defined, default is yes
		return qtrue;
	}
	else
	{//return value
		return ((qboolean)(atoi(allowed) == 0));
	}
}

qboolean UI_IsSaberTwoHanded(const char* saberName)
{
	int twoHanded;
	char	twoHandedString[8] = { 0 };
	UI_SaberParseParm(saberName, "twoHanded", twoHandedString);
	if (!twoHandedString[0])
	{//not defined defaults to "no"
		return qfalse;
	}
	twoHanded = atoi(twoHandedString);
	return ((qboolean)(twoHanded != 0));
}

void UI_SaberGetHiltInfo(const char* singleHilts[MAX_SABER_HILTS], const char* staffHilts[MAX_SABER_HILTS])
{
	int	numSingleHilts = 0, numStaffHilts = 0;
	const char* saberName;
	const char* token;
	const char* p;

	//go through all the loaded sabers and put the valid ones in the proper list
	p = SaberParms;
	COM_BeginParseSession();

	// look for a saber
	while (p)
	{
		token = COM_ParseExt(&p, qtrue);
		if (token[0] == 0)
		{//invalid name
			continue;
		}
		saberName = String_Alloc(token);
		//see if there's a "{" on the next line
		SkipRestOfLine(&p);

		if (UI_ParseLiteral(&p, "{", qtrue))
		{//nope, not a name, keep looking
			continue;
		}

		//this is a saber name
		if (!UI_SaberValidForPlayerInSP(saberName))
		{
			SkipBracedSection(&p);
			continue;
		}

		if (UI_IsSaberTwoHanded(saberName))
		{
			if (numStaffHilts < MAX_SABER_HILTS - 1)//-1 because we have to NULL terminate the list
			{
				staffHilts[numStaffHilts++] = saberName;
			}
			else
			{
				Com_Printf("WARNING: too many two-handed sabers, ignoring saber '%s'\n", saberName);
			}
		}
		else
		{
			if (numSingleHilts < MAX_SABER_HILTS - 1)//-1 because we have to NULL terminate the list
			{
				singleHilts[numSingleHilts++] = saberName;
			}
			else
			{
				Com_Printf("WARNING: too many one-handed sabers, ignoring saber '%s'\n", saberName);
			}
		}
		//skip the whole braced section and move on to the next entry
		SkipBracedSection(&p);
	}
	//null terminate the list so the UI code knows where to stop listing them
	singleHilts[numSingleHilts] = NULL;
	staffHilts[numStaffHilts] = NULL;
	COM_EndParseSession();
}

void UI_SaberLoadParms( void )
{
	int			len, totallen, saberExtFNLen, fileCnt, i;
	char		*buffer, *holdChar, *marker;
	char		saberExtensionListBuf[2048];			//	The list of file names read in

	//ui.Printf( "UI Parsing *.sab saber definitions\n" );

	ui_saber_parms_parsed = qtrue;
	UI_CacheSaberGlowGraphics();

	//set where to store the first one
	totallen = 0;
	marker = SaberParms;
	marker[0] = '\0';

	//now load in the sabers
	fileCnt = ui.FS_GetFileList("ext_data/sabers", ".sab", saberExtensionListBuf, sizeof(saberExtensionListBuf) );

	holdChar = saberExtensionListBuf;
	for ( i = 0; i < fileCnt; i++, holdChar += saberExtFNLen + 1 )
	{
		saberExtFNLen = strlen( holdChar );

		len = ui.FS_ReadFile( va( "ext_data/sabers/%s", holdChar), (void **) &buffer );

		if ( len == -1 )
		{
			ui.Printf( "UI_SaberLoadParms: error reading %s\n", holdChar );
		}
		else
		{
			if ( totallen && *(marker-1) == '}' )
			{//don't let it end on a } because that should be a stand-alone token
				strcat( marker, " " );
				totallen++;
				marker++;
			}
			len = COM_Compress( buffer );

			if ( totallen + len >= MAX_SABER_DATA_SIZE ) {
				Com_Error( ERR_FATAL, "UI_SaberLoadParms: ran out of space before reading %s\n(you must make the .npc files smaller)", holdChar );
			}
			strcat( marker, buffer );
			ui.FS_FreeFile( buffer );

			totallen += len;
			marker += len;
		}
	}
}

void UI_DoSFXSaber( vec3_t blade_muz, vec3_t blade_dir, float lengthMax, float radius, saber_colors_t color, saber_crystals_t crystals, int whichSaber )
{
	vec3_t	mid;
	float	radiusmult, effectradius, coreradius;
	float	blade_len;
	float	effectalpha = 0.8f;
	float	AngleScale = 1.0f;
	
	qhandle_t	glow = 0;
	refEntity_t saber;
	
	blade_len = lengthMax;
	
	if ( blade_len < 0.5f )
	{
		return;
	}
	
	switch( color )
	{
		case SABER_RED:
			glow = redSaberGlowShader;
			break;
		case SABER_ORANGE:
			glow = orangeSaberGlowShader;
			break;
		case SABER_YELLOW:
			glow = yellowSaberGlowShader;
			break;
		case SABER_GREEN:
			glow = greenSaberGlowShader;
			break;
		case SABER_PURPLE:
			glow = purpleSaberGlowShader;
			break;
		case SABER_BLUE:
			glow = blueSaberGlowShader;
			break;
		default://SABER_RGB
			glow = rgbSaberGlowShader;
			break;
	}
	
	if ( crystals & SABER_CRYSTAL_BLACK )
	{
		glow = blackSaberGlowShader;
	}
	
	VectorMA( blade_muz, blade_len * 0.5f, blade_dir, mid );
	
	memset( &saber, 0, sizeof( refEntity_t ));
	
	if (blade_len < lengthMax)
	{
		radiusmult = 0.5 + ((blade_len / lengthMax)/2);
	}
	else
	{
		radiusmult = 1.0;
	}
	
	effectradius	= ((radius * 1.6) + Q_flrand(-1.0f, 1.0f) * 0.1f)*radiusmult*ui_SFXSabersGlowSize.value;
	coreradius		= ((radius * 0.4) + Q_flrand(-1.0f, 1.0f) * 0.1f)*radiusmult*ui_SFXSabersCoreSize.value;
	
	if ( crystals & SABER_CRYSTAL_UNSTABLE )
	{
		coreradius *= 0.9;
	}
	
	{
		//saber.renderfx = rfx;
		if(blade_len-((effectradius*AngleScale)/2) > 0)
		{
			saber.radius = effectradius*AngleScale;
			saber.saberLength = (blade_len - (saber.radius/2));
			VectorCopy( blade_muz, saber.origin );
			VectorCopy( blade_dir, saber.axis[0] );
			saber.reType = RT_SABER_GLOW;
			saber.customShader = glow;
			saber.shaderRGBA[0] = 0xff * effectalpha;
			saber.shaderRGBA[1] = 0xff * effectalpha;
			saber.shaderRGBA[2] = 0xff * effectalpha;
			saber.shaderRGBA[3] = 0xff * effectalpha;
			
			if (color >= SABER_RGB)
			{
				if (whichSaber == 0)
				{
					saber.shaderRGBA[0] = ui_rgb_saber_red.integer * effectalpha;
					saber.shaderRGBA[1] = ui_rgb_saber_green.integer * effectalpha;
					saber.shaderRGBA[2] = ui_rgb_saber_blue.integer * effectalpha;
				}
				else
				{
					saber.shaderRGBA[0] = ui_rgb_saber2_red.integer * effectalpha;
					saber.shaderRGBA[1] = ui_rgb_saber2_green.integer * effectalpha;
					saber.shaderRGBA[2] = ui_rgb_saber2_blue.integer * effectalpha;
				}
			}
			
			DC->addRefEntityToScene( &saber );
		}
		
		// Do the hot core
		VectorMA( blade_muz, blade_len, blade_dir, saber.origin );
		VectorMA( blade_muz, -1, blade_dir, saber.oldorigin );
		
		saber.customShader = SaberBladeShader;
		if ( crystals & SABER_CRYSTAL_BLACK )
		{
			saber.customShader = blackSaberBladeShader;
		}
		else if ( crystals & SABER_CRYSTAL_UNSTABLE )
		{
			saber.customShader = rgbUnstableCoreShader;
		}
		
		saber.reType = RT_LINE;
		
		saber.radius = coreradius;
		
		saber.shaderTexCoord[0] = saber.shaderTexCoord[1] = 1.0f;
		saber.shaderRGBA[0] = saber.shaderRGBA[1] = saber.shaderRGBA[2] = saber.shaderRGBA[3] = 0xff;
		
		DC->addRefEntityToScene( &saber );
	}
}

void UI_DoSaber( vec3_t origin, vec3_t dir, float length, float lengthMax, float radius, saber_colors_t color, saber_crystals_t crystals, int whichSaber )
{
	vec3_t		mid, rgb={1,1,1};
	qhandle_t	blade = 0, glow = 0;
	refEntity_t saber;
	float radiusmult;

	if ( length < 0.5f )
	{
		// if the thing is so short, just forget even adding me.
		return;
	}

	// Find the midpoint of the saber for lighting purposes
	VectorMA( origin, length * 0.5f, dir, mid );

	switch( color )
	{
		case SABER_RED:
			glow = redSaberGlowShader;
			blade = redSaberCoreShader;
			VectorSet( rgb, 1.0f, 0.2f, 0.2f );
			break;
		case SABER_ORANGE:
			glow = orangeSaberGlowShader;
			blade = orangeSaberCoreShader;
			VectorSet( rgb, 1.0f, 0.5f, 0.1f );
			break;
		case SABER_YELLOW:
			glow = yellowSaberGlowShader;
			blade = yellowSaberCoreShader;
			VectorSet( rgb, 1.0f, 1.0f, 0.2f );
			break;
		case SABER_GREEN:
			glow = greenSaberGlowShader;
			blade = greenSaberCoreShader;
			VectorSet( rgb, 0.2f, 1.0f, 0.2f );
			break;
		case SABER_BLUE:
			glow = blueSaberGlowShader;
			blade = blueSaberCoreShader;
			VectorSet( rgb, 0.2f, 0.4f, 1.0f );
			break;
		case SABER_PURPLE:
			glow = purpleSaberGlowShader;
			blade = purpleSaberCoreShader;
			VectorSet( rgb, 0.9f, 0.2f, 1.0f );
			break;
		default:
			glow = rgbSaberGlowShader;
			blade = rgbSaberCoreShader;
			break;
	}
	
	if ( crystals & SABER_CRYSTAL_BLACK )
	{
		glow = blackSaberGlowShader;
		blade = blackSaberCoreShader;
	}
	else if ( crystals & SABER_CRYSTAL_UNSTABLE )
	{
		glow = rgbSaberGlowShader;
		blade = rgbUnstableCoreShader;
	}

	memset( &saber, 0, sizeof( refEntity_t ));

	// Saber glow is it's own ref type because it uses a ton of sprites, otherwise it would eat up too many
	//	refEnts to do each glow blob individually
	saber.saberLength = length;

	// Jeff, I did this because I foolishly wished to have a bright halo as the saber is unleashed.
	// It's not quite what I'd hoped tho.  If you have any ideas, go for it!  --Pat
	if (length < lengthMax )
	{
		radiusmult = 1.0 + (2.0 / length);		// Note this creates a curve, and length cannot be < 0.5.
	}
	else
	{
		radiusmult = 1.0;
	}

	float radiusRange = radius * 0.075f;
	float radiusStart = radius-radiusRange;

	saber.radius = (radiusStart + Q_flrand(-1.0f, 1.0f) * radiusRange)*radiusmult;
	//saber.radius = (2.8f + Q_flrand(-1.0f, 1.0f) * 0.2f)*radiusmult;


	VectorCopy( origin, saber.origin );
	VectorCopy( dir, saber.axis[0] );
	saber.reType = RT_SABER_GLOW;
	saber.customShader = glow;
	saber.shaderRGBA[0] = saber.shaderRGBA[1] = saber.shaderRGBA[2] = saber.shaderRGBA[3] = 0xff;
	//saber.renderfx = rfx;
	
	if (color >= SABER_RGB)
	{
		if (whichSaber == 0)
		{
			saber.shaderRGBA[0] = ui_rgb_saber_red.integer;
			saber.shaderRGBA[1] = ui_rgb_saber_green.integer;
			saber.shaderRGBA[2] = ui_rgb_saber_blue.integer;
		}
		else
		{
			saber.shaderRGBA[0] = ui_rgb_saber2_red.integer;
			saber.shaderRGBA[1] = ui_rgb_saber2_green.integer;
			saber.shaderRGBA[2] = ui_rgb_saber2_blue.integer;
		}
	}
	else if ( (crystals & SABER_CRYSTAL_UNSTABLE) && !(crystals & SABER_CRYSTAL_BLACK) )
	{
		saber.shaderRGBA[0] = 0xff * rgb[0];
		saber.shaderRGBA[1] = 0xff * rgb[1];
		saber.shaderRGBA[2] = 0xff * rgb[2];
	}

	DC->addRefEntityToScene( &saber );

	// Do the hot core
	VectorMA( origin, length, dir, saber.origin );
	VectorMA( origin, -1, dir, saber.oldorigin );
	saber.customShader = blade;
	saber.reType = RT_LINE;
	radiusStart = radius/3.0f;
	saber.radius = (radiusStart + Q_flrand(-1.0f, 1.0f) * radiusRange)*radiusmult;
//	saber.radius = (1.0 + Q_flrand(-1.0f, 1.0f) * 0.2f)*radiusmult;

	DC->addRefEntityToScene( &saber );
}

saber_colors_t TranslateSaberColor( const char *name )
{
	if ( !Q_stricmp( name, "red" ) )
	{
		return SABER_RED;
	}
	if ( !Q_stricmp( name, "orange" ) )
	{
		return SABER_ORANGE;
	}
	if ( !Q_stricmp( name, "yellow" ) )
	{
		return SABER_YELLOW;
	}
	if ( !Q_stricmp( name, "green" ) )
	{
		return SABER_GREEN;
	}
	if ( !Q_stricmp( name, "blue" ) )
	{
		return SABER_BLUE;
	}
	if ( !Q_stricmp( name, "purple" ) )
	{
		return SABER_PURPLE;
	}
	if ( !Q_stricmp( name, "random" ) )
	{
		return ((saber_colors_t)(Q_irand( SABER_ORANGE, SABER_PURPLE )));
	}
	float colors[3];
	Q_parseSaberColor(name, colors);
	int colourArray[3];
	for (int i = 0; i < 3; i++)
	{
		colourArray[i] = (int)(colors[i] * 255);
	}
	return (saber_colors_t)((colourArray[0]) + (colourArray[1] << 8) + (colourArray[2] << 16) + (1 << 24));
}

saberType_t TranslateSaberType( const char *name )
{
	if ( !Q_stricmp( name, "SABER_SINGLE" ) )
	{
		return SABER_SINGLE;
	}
	if ( !Q_stricmp( name, "SABER_STAFF" ) )
	{
		return SABER_STAFF;
	}
	if ( !Q_stricmp( name, "SABER_BROAD" ) )
	{
		return SABER_BROAD;
	}
	if ( !Q_stricmp( name, "SABER_PRONG" ) )
	{
		return SABER_PRONG;
	}
	if ( !Q_stricmp( name, "SABER_DAGGER" ) )
	{
		return SABER_DAGGER;
	}
	if ( !Q_stricmp( name, "SABER_ARC" ) )
	{
		return SABER_ARC;
	}
	if ( !Q_stricmp( name, "SABER_SAI" ) )
	{
		return SABER_SAI;
	}
	if ( !Q_stricmp( name, "SABER_CLAW" ) )
	{
		return SABER_CLAW;
	}
	if ( !Q_stricmp( name, "SABER_LANCE" ) )
	{
		return SABER_LANCE;
	}
	if ( !Q_stricmp( name, "SABER_STAR" ) )
	{
		return SABER_STAR;
	}
	if ( !Q_stricmp( name, "SABER_TRIDENT" ) )
	{
		return SABER_TRIDENT;
	}
	if ( !Q_stricmp( name, "SABER_SITH_SWORD" ) )
	{
		return SABER_SITH_SWORD;
	}
	return SABER_SINGLE;
}

void UI_SaberDrawBlade( itemDef_t *item, char *saberName, int saberModel, saberType_t saberType, vec3_t origin, float curYaw, int bladeNum )
{
	char bladeColorString[MAX_QPATH];
	vec3_t	angles={0};
	int whichSaber = 0;
	int secondSaberModel;
	
	if ( item->flags&(ITF_ISANYSABER) && item->flags&(ITF_ISCHARACTER) )
	{	//it's bolted to a dude!
		angles[YAW] = curYaw;
	}
	else
	{
		angles[PITCH] = curYaw;
		angles[ROLL] = 90;
	}

	if ( saberModel >= item->ghoul2.size() )
	{//uhh... invalid index!
		return;
	}

	if (Cvar_VariableString( "ui_char_head_model" )[0])
	{
		secondSaberModel = 3;
	}
	else
	{
		secondSaberModel = 2;
	}

	if ( (item->flags&ITF_ISSABER) && saberModel < secondSaberModel )
	{
		whichSaber = 0;
	}
	else//if ( item->flags&ITF_ISSABER2 ) - presumed
	{
		whichSaber = 1;
	}
	
	if ( whichSaber == 0 )
	{
		DC->getCVarString( "ui_saber_color", bladeColorString, sizeof(bladeColorString) );
	}
	else//if ( whichSaber == 1 ) - presumed
	{
		DC->getCVarString( "ui_saber2_color", bladeColorString, sizeof(bladeColorString) );
	}
	saber_colors_t bladeColor = TranslateSaberColor( bladeColorString );

	float bladeLength = UI_SaberBladeLengthForSaber( saberName, bladeNum );
	float bladeRadius = UI_SaberBladeRadiusForSaber( saberName, bladeNum );
	vec3_t	bladeOrigin={0};
	vec3_t	axis[3]={};
	mdxaBone_t	boltMatrix;
	qboolean tagHack = qfalse;

	char *tagName = va( "*blade%d", bladeNum+1 );
	int bolt = DC->g2_AddBolt( &item->ghoul2[saberModel], tagName );

	if ( bolt == -1 )
	{
		tagHack = qtrue;
		//hmm, just fall back to the most basic tag (this will also make it work with pre-JKA saber models
		bolt = DC->g2_AddBolt( &item->ghoul2[saberModel], "*flash" );
		if ( bolt == -1 )
		{//no tag_flash either?!!
			bolt = 0;
		}
	}

	DC->g2_GetBoltMatrix( item->ghoul2, saberModel, bolt, &boltMatrix, angles, origin, uiInfo.uiDC.realTime, NULL, vec3_origin );//NULL was cgs.model_draw

	// work the matrix axis stuff into the original axis and origins used.
	DC->g2_GiveMeVectorFromMatrix(boltMatrix, ORIGIN, bladeOrigin);
	DC->g2_GiveMeVectorFromMatrix(boltMatrix, NEGATIVE_X, axis[0]);//front (was NEGATIVE_Y, but the md3->glm exporter screws up this tag somethin' awful)
	DC->g2_GiveMeVectorFromMatrix(boltMatrix, NEGATIVE_Y, axis[1]);//right
	DC->g2_GiveMeVectorFromMatrix(boltMatrix, POSITIVE_Z, axis[2]);//up

	float scale = DC->xscale;

	if ( tagHack )
	{
		switch ( saberType )
		{
		case SABER_SINGLE:
		case SABER_DAGGER:
		case SABER_LANCE:
			break;
		case SABER_STAFF:
			if ( bladeNum == 1 )
			{
				VectorScale( axis[0], -1, axis[0] );
				VectorMA( bladeOrigin, 16*scale, axis[0], bladeOrigin );
			}
			break;
		case SABER_BROAD:
			if ( bladeNum == 0 )
			{
				VectorMA( bladeOrigin, -1*scale, axis[1], bladeOrigin );
			}
			else if ( bladeNum == 1 )
			{
				VectorMA( bladeOrigin, 1*scale, axis[1], bladeOrigin );
			}
			break;
		case SABER_PRONG:
			if ( bladeNum == 0 )
			{
				VectorMA( bladeOrigin, -3*scale, axis[1], bladeOrigin );
			}
			else if ( bladeNum == 1 )
			{
				VectorMA( bladeOrigin, 3*scale, axis[1], bladeOrigin );
			}
			break;
		case SABER_ARC:
			VectorSubtract( axis[1], axis[2], axis[1] );
			VectorNormalize( axis[1] );
			switch ( bladeNum )
			{
			case 0:
				VectorMA( bladeOrigin, 8*scale, axis[0], bladeOrigin );
				VectorScale( axis[0], 0.75f, axis[0] );
				VectorScale( axis[1], 0.25f, axis[1] );
				VectorAdd( axis[0], axis[1], axis[0] );
				break;
			case 1:
				VectorScale( axis[0], 0.25f, axis[0] );
				VectorScale( axis[1], 0.75f, axis[1] );
				VectorAdd( axis[0], axis[1], axis[0] );
				break;
			case 2:
				VectorMA( bladeOrigin, -8*scale, axis[0], bladeOrigin );
				VectorScale( axis[0], -0.25f, axis[0] );
				VectorScale( axis[1], 0.75f, axis[1] );
				VectorAdd( axis[0], axis[1], axis[0] );
				break;
			case 3:
				VectorMA( bladeOrigin, -16*scale, axis[0], bladeOrigin );
				VectorScale( axis[0], -0.75f, axis[0] );
				VectorScale( axis[1], 0.25f, axis[1] );
				VectorAdd( axis[0], axis[1], axis[0] );
				break;
			}
			break;
		case SABER_SAI:
			if ( bladeNum == 1 )
			{
				VectorMA( bladeOrigin, -3*scale, axis[1], bladeOrigin );
			}
			else if ( bladeNum == 2 )
			{
				VectorMA( bladeOrigin, 3*scale, axis[1], bladeOrigin );
			}
			break;
		case SABER_CLAW:
			switch ( bladeNum )
			{
			case 0:
				VectorMA( bladeOrigin, 2*scale, axis[0], bladeOrigin );
				VectorMA( bladeOrigin, 2*scale, axis[2], bladeOrigin );
				break;
			case 1:
				VectorMA( bladeOrigin, 2*scale, axis[0], bladeOrigin );
				VectorMA( bladeOrigin, 2*scale, axis[2], bladeOrigin );
				VectorMA( bladeOrigin, 2*scale, axis[1], bladeOrigin );
				break;
			case 2:
				VectorMA( bladeOrigin, 2*scale, axis[0], bladeOrigin );
				VectorMA( bladeOrigin, 2*scale, axis[2], bladeOrigin );
				VectorMA( bladeOrigin, -2*scale, axis[1], bladeOrigin );
				break;
			}
			break;
		case SABER_STAR:
			switch ( bladeNum )
			{
			case 0:
				VectorMA( bladeOrigin, 8*scale, axis[0], bladeOrigin );
				break;
			case 1:
				VectorScale( axis[0], 0.33f, axis[0] );
				VectorScale( axis[2], 0.67f, axis[2] );
				VectorAdd( axis[0], axis[2], axis[0] );
				VectorMA( bladeOrigin, 8*scale, axis[0], bladeOrigin );
				break;
			case 2:
				VectorScale( axis[0], -0.33f, axis[0] );
				VectorScale( axis[2], 0.67f, axis[2] );
				VectorAdd( axis[0], axis[2], axis[0] );
				VectorMA( bladeOrigin, 8*scale, axis[0], bladeOrigin );
				break;
			case 3:
				VectorScale( axis[0], -1, axis[0] );
				VectorMA( bladeOrigin, 8*scale, axis[0], bladeOrigin );
				break;
			case 4:
				VectorScale( axis[0], -0.33f, axis[0] );
				VectorScale( axis[2], -0.67f, axis[2] );
				VectorAdd( axis[0], axis[2], axis[0] );
				VectorMA( bladeOrigin, 8*scale, axis[0], bladeOrigin );
				break;
			case 5:
				VectorScale( axis[0], 0.33f, axis[0] );
				VectorScale( axis[2], -0.67f, axis[2] );
				VectorAdd( axis[0], axis[2], axis[0] );
				VectorMA( bladeOrigin, 8*scale, axis[0], bladeOrigin );
				break;
			}
			break;
		case SABER_TRIDENT:
			switch ( bladeNum )
			{
			case 0:
				VectorMA( bladeOrigin, 24*scale, axis[0], bladeOrigin );
				break;
			case 1:
				VectorMA( bladeOrigin, -6*scale, axis[1], bladeOrigin );
				VectorMA( bladeOrigin, 24*scale, axis[0], bladeOrigin );
				break;
			case 2:
				VectorMA( bladeOrigin, 6*scale, axis[1], bladeOrigin );
				VectorMA( bladeOrigin, 24*scale, axis[0], bladeOrigin );
				break;
			case 3:
				VectorMA( bladeOrigin, -32*scale, axis[0], bladeOrigin );
				VectorScale( axis[0], -1, axis[0] );
				break;
			}
			break;
		case SABER_SITH_SWORD:
			//no blade
			break;
		default:
			break;
		}
	}
	if ( saberType == SABER_SITH_SWORD )
	{//draw no blade
		return;
	}

	if (ui_SFXSabers.integer)
	{
		UI_DoSFXSaber( bladeOrigin, axis[0], bladeLength, bladeRadius, bladeColor, SABER_CRYSTAL_DEFAULT, whichSaber);
	}
	else
	{
		UI_DoSaber( bladeOrigin, axis[0], bladeLength, bladeLength, bladeRadius, bladeColor, SABER_CRYSTAL_DEFAULT, whichSaber );
	}
}

extern qboolean ItemParse_asset_model_go( itemDef_t *item, const char *name );
extern qboolean ItemParse_model_g2skin_go( itemDef_t *item, const char *skinName );
void UI_GetSaberForMenu( char *saber, int saberNum )
{
	char saberTypeString[MAX_QPATH]={0};
	saberType_t saberType = SABER_NONE;

	if ( saberNum == 0 )
	{
		DC->getCVarString( "g_saber", saber, MAX_QPATH );
	}
	else
	{
		DC->getCVarString( "g_saber2", saber, MAX_QPATH );
	}
	//read this from the sabers.cfg
	UI_SaberTypeForSaber( saber, saberTypeString );
	if ( saberTypeString[0] )
	{
		saberType = TranslateSaberType( saberTypeString );
	}

	switch ( uiInfo.movesTitleIndex )
	{
	case 0://MD_ACROBATICS:
		break;
	case 1://MD_SINGLE_FAST:
	case 2://MD_SINGLE_MEDIUM:
	case 3://MD_SINGLE_STRONG:
		if ( saberType != SABER_SINGLE )
		{
			Q_strncpyz(saber,"single_1",MAX_QPATH);
		}
		break;
	case 4://MD_DUAL_SABERS:
		if ( saberType != SABER_SINGLE )
		{
			Q_strncpyz(saber,"single_1",MAX_QPATH);
		}
		break;
	case 5://MD_SABER_STAFF:
		if ( saberType == SABER_SINGLE || saberType == SABER_NONE )
		{
			Q_strncpyz(saber,"dual_1",MAX_QPATH);
		}
		break;
	}

}

void UI_SaberDrawBlades( itemDef_t *item, vec3_t origin, float curYaw )
{
	//NOTE: only allows one saber type in view at a time
	char saber[MAX_QPATH];
	int saberNum = 0;
	int saberModel = 0;
	int	numSabers = 1;

	if ( (item->flags&ITF_ISCHARACTER)//hacked sabermoves sabers in character's hand
		&& uiInfo.movesTitleIndex == 4 /*MD_DUAL_SABERS*/ )
	{
		numSabers = 2;
	}

	for ( saberNum = 0; saberNum < numSabers; saberNum++ )
	{
		if ( (item->flags&ITF_ISCHARACTER) )//hacked sabermoves sabers in character's hand
		{
			UI_GetSaberForMenu( saber, saberNum );
			if (Cvar_VariableString( "ui_char_head_model" )[0])
			{
				saberModel = saberNum + 2;
			}
			else
			{
				saberModel = saberNum + 1;
			}
		}
		else if ( (item->flags&ITF_ISSABER) )
		{
			DC->getCVarString( "ui_saber", saber, sizeof(saber) );
			saberModel = 0;
		}
		else if ( (item->flags&ITF_ISSABER2) )
		{
			DC->getCVarString( "ui_saber2", saber, sizeof(saber) );
			saberModel = 0;
		}
		else
		{
			return;
		}
		if ( saber[0] )
		{
			int numBlades = UI_SaberNumBladesForSaber( saber );
			if ( numBlades )
			{//okay, here we go, time to draw each blade...
				char	saberTypeString[MAX_QPATH]={0};
				UI_SaberTypeForSaber( saber, saberTypeString );
				saberType_t saberType = TranslateSaberType( saberTypeString );
				for ( int curBlade = 0; curBlade < numBlades; curBlade++ )
				{
					if ( UI_SaberShouldDrawBlade( saber, curBlade ) )
					{
						UI_SaberDrawBlade( item, saber, saberModel, saberType, origin, curYaw, curBlade );
					}
				}
			}
		}
	}
}

void UI_SaberAttachToChar( itemDef_t *item )
{
	int	numSabers = 1;
 	int	saberNum = 0;

	if (Cvar_VariableString( "ui_char_head_model" )[0])
	{
		if ( item->ghoul2.size() > 3 && item->ghoul2[3].mModelindex >=0 )
		{//remove any extra models
			DC->g2_RemoveGhoul2Model(item->ghoul2, 3);
		}
		if ( item->ghoul2.size() > 2 && item->ghoul2[2].mModelindex >=0 )
		{//remove any extra models
			DC->g2_RemoveGhoul2Model(item->ghoul2, 2);
		}
	}
	else
	{
		if ( item->ghoul2.size() > 2 && item->ghoul2[2].mModelindex >=0 )
		{//remove any extra models
			DC->g2_RemoveGhoul2Model(item->ghoul2, 2);
		}
		if ( item->ghoul2.size() > 1 && item->ghoul2[1].mModelindex >=0)
		{//remove any extra models
			DC->g2_RemoveGhoul2Model(item->ghoul2, 1);
		}
	}

	if ( uiInfo.movesTitleIndex == 4 /*MD_DUAL_SABERS*/ )
	{
		numSabers = 2;
	}

	for ( saberNum = 0; saberNum < numSabers; saberNum++ )
	{
		//bolt sabers
		char modelPath[MAX_QPATH];
		char skinPath[MAX_QPATH];
		char saber[MAX_QPATH];

		UI_GetSaberForMenu( saber, saberNum );

		if ( UI_SaberModelForSaber( saber, modelPath ) )
		{//successfully found a model
			int g2Saber = DC->g2_InitGhoul2Model(item->ghoul2, modelPath, 0, 0, 0, 0, 0); //add the model
			if (g2Saber)
			{
				//get the customSkin, if any
				if ( UI_SaberSkinForSaber( saber, skinPath, (saberNum == 1)?qtrue:qfalse ) )
				{
					int g2skin = DC->registerSkin(skinPath);
					DC->g2_SetSkin( &item->ghoul2[g2Saber], 0, g2skin );//this is going to set the surfs on/off matching the skin file
				}
				else
				{
					DC->g2_SetSkin( &item->ghoul2[g2Saber], -1, 0 );//turn off custom skin
				}
				int boltNum;
				if ( saberNum == 0 )
				{
					boltNum = DC->g2_AddBolt(&item->ghoul2[0], "*r_hand");
				}
				else
				{
					boltNum = DC->g2_AddBolt(&item->ghoul2[0], "*l_hand");
				}
				re.G2API_AttachG2Model(&item->ghoul2[g2Saber], &item->ghoul2[0], boltNum, 0);
                re.G2API_SetTintType(&item->ghoul2[g2Saber], saberNum ? G2_TINT_SABER2 : G2_TINT_SABER);
			}
		}
	}
}
