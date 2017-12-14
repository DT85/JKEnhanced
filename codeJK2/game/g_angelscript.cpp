#include "g_local.h"
#include <angelscript.h>
#include <scriptbuilder.h>
#include <scriptstdstring.h>
#include <scripthandle.h>

static asIScriptEngine *scrpEngine = NULL;
static asIScriptContext *scrpContext = NULL;

void X_MessageCallback(const asSMessageInfo *msg, void *param)
{
	char *type;
	if( msg->type == asMSGTYPE_WARNING ) 
		type = "^3WARNING";
	else if( msg->type == asMSGTYPE_INFORMATION ) 
		type = "^5INFO";
	else
		type = "^1ERROR";
	Com_Printf("%s: %s (%d: %d) : %s\n", type, msg->section, msg->row, msg->col, msg->message);
}

void X_Com_Print(string& in) {
	Com_Printf("%s", in.c_str());
}

void AS_SetArgBasedOnType(void* arg, const string& argument, int argNum) {
	if(!argument.compare("byte") || !argument.compare("int8") || !argument.compare("uint8") || !argument.compare("bool"))
		scrpContext->SetArgByte(argNum, *(byte*)arg);
	else if(!argument.compare("double"))
		scrpContext->SetArgDouble(argNum, *(double*)arg);
	else if(!argument.compare("dword") || !argument.compare("int") || !argument.compare("uint"))
		scrpContext->SetArgDWord(argNum, *(DWORD*)arg);
	else if(!argument.compare("float"))
		scrpContext->SetArgFloat(argNum, *(float*)arg);
	else if(!argument.compare("int16") || !argument.compare("uint16"))
		scrpContext->SetArgWord(argNum, *(short*)arg);
	else if(argument.find_first_of('@') != argument.npos || argument.find_first_of('&') != argument.npos)
		scrpContext->SetArgAddress(argNum, arg);
	else
		scrpContext->SetArgObject(argNum, arg);
}

void G_ExecuteASVoid(const char* funcDeclaration, int numArgs, ...) {
	string s = funcDeclaration;
	auto paramStart = s.find_first_of('(')+1;
	auto paramEnd = s.find_last_of(')')-paramStart;
	string params = s.substr(paramStart, paramEnd);
	vector<string> arguments;

	asIScriptFunction *func = scrpEngine->GetModule(0)->GetFunctionByDecl(funcDeclaration);
	scrpContext->Prepare(func);
	auto argSplitter = params.find_first_of(',');
	while(argSplitter != params.npos) {
		string thisArgument = params.substr(0, argSplitter-1);
		arguments.push_back(thisArgument);
		params+=argSplitter;
		argSplitter = params.find_first_of(',')+1;
	}
	arguments.push_back(params);

	va_list		argptr;
	va_start(argptr, numArgs);

	void* arg;
	for(int i = 2; i < numArgs+2; i++) {
		arg = va_arg(argptr, void*);
		AS_SetArgBasedOnType(&arg, arguments.at(i-2), i-2);
	}
	scrpContext->Execute();
}

int G_ExecuteASInteger(const char* funcDeclaration, int numArgs, ...) {
	string s = funcDeclaration;
	auto paramStart = s.find_first_of('(')+1;
	auto paramEnd = s.find_last_of(')')-paramStart;
	string params = s.substr(paramStart, paramEnd);
	vector<string> arguments;

	asIScriptFunction *func = scrpEngine->GetModule(0)->GetFunctionByDecl(funcDeclaration);
	scrpContext->Prepare(func);
	auto argSplitter = params.find_first_of(',');
	while(argSplitter != params.npos) {
		string thisArgument = params.substr(0, argSplitter-1);
		arguments.push_back(thisArgument);
		params+=argSplitter;
		argSplitter = params.find_first_of(',')+1;
	}
	arguments.push_back(params);

	va_list		argptr;
	va_start(argptr, numArgs);

	void* arg;
	for(int i = 2; i < numArgs+2; i++) {
		arg = va_arg(argptr, void*);
		AS_SetArgBasedOnType(&arg, arguments.at(i-2), i-2);
	}
	scrpContext->Execute();
	return scrpContext->GetReturnDWord();
}

float G_ExecuteASFloat(const char* funcDeclaration, int numArgs, ...) {
	string s = funcDeclaration;
	auto paramStart = s.find_first_of('(')+1;
	auto paramEnd = s.find_last_of(')')-paramStart;
	string params = s.substr(paramStart, paramEnd);
	vector<string> arguments;

	asIScriptFunction *func = scrpEngine->GetModule(0)->GetFunctionByDecl(funcDeclaration);
	scrpContext->Prepare(func);
	auto argSplitter = params.find_first_of(',');
	while(argSplitter != params.npos) {
		string thisArgument = params.substr(0, argSplitter-1);
		arguments.push_back(thisArgument);
		params+=argSplitter;
		argSplitter = params.find_first_of(',')+1;
	}
	arguments.push_back(params);

	va_list		argptr;
	va_start(argptr, numArgs);

	void* arg;
	for(int i = 2; i < numArgs+2; i++) {
		arg = va_arg(argptr, void*);
		AS_SetArgBasedOnType(&arg, arguments.at(i-2), i-2);
	}
	scrpContext->Execute();
	return scrpContext->GetReturnFloat();
}

string* G_ExecuteASString(const char* funcDeclaration, int numArgs, ...) {
	string s = funcDeclaration;
	auto paramStart = s.find_first_of('(')+1;
	auto paramEnd = s.find_last_of(')')-paramStart;
	string params = s.substr(paramStart, paramEnd);
	vector<string> arguments;

	asIScriptFunction *func = scrpEngine->GetModule(0)->GetFunctionByDecl(funcDeclaration);
	scrpContext->Prepare(func);
	auto argSplitter = params.find_first_of(',');
	while(argSplitter != params.npos) {
		string thisArgument = params.substr(0, argSplitter-1);
		arguments.push_back(thisArgument);
		params+=argSplitter;
		argSplitter = params.find_first_of(',')+1;
	}
	arguments.push_back(params);

	va_list		argptr;
	va_start(argptr, numArgs);

	void* arg;
	for(int i = 2; i < numArgs+2; i++) {
		arg = va_arg(argptr, void*);
		AS_SetArgBasedOnType(&arg, arguments.at(i-2), i-2);
	}
	scrpContext->Execute();
	return (string*)scrpContext->GetReturnObject();
}

bool TryNewDempRoutine(gentity_t *ent) {
	if(ent->s.weapon != WP_DEMP2) {
		return false;
	}
	G_ExecuteASVoid("void DEMP2Test(gentity_t@ ent)", 1, ent);
	return true;
}

// ================================================================================================================
// Routines which are called by the game
// ================================================================================================================

extern vec3_t	wpFwd, wpVright, wpUp;
extern vec3_t	wpMuzzle;
extern void CalcMuzzlePoint( gentity_t *const ent, vec3_t wpFwd, vec3_t right, vec3_t wpUp, vec3_t muzzlePoint, float lead_in );
extern void WP_TraceSetStart( const gentity_t *ent, vec3_t start, const vec3_t mins, const vec3_t maxs );
extern gentity_t *CreateMissile( vec3_t org, vec3_t dir, float vel, int life, gentity_t *owner, qboolean altFire );

int GetWeaponDamage(int weapon) {
	return weaponData[weapon].damage;
}

void VectorSet3(vec3_t in, float x, float y, float z) {
	in[0] = x; in[1] = y; in[2] = z;
}

float sinCallback(float in) {
	return sin(in);
}

float cosCallback(float in) {
	return cos(in);
}

void CalculateMuzzlePoint_AS(CScriptHandle ent2, vec3_t fwd, vec3_t right, vec3_t up, vec3_t muzzle, float lead_in) {
	gentity_t *ent;
	ent2.Cast((void**)&ent, ent2.GetTypeId());
	Com_Printf("one\n");
}

// ================================================================================================================

void G_InitAngelscript() {
	scrpEngine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	scrpEngine->SetMessageCallback(asFUNCTION(X_MessageCallback), 0, asCALL_CDECL);
	scrpEngine->SetEngineProperty(asEP_ALLOW_UNSAFE_REFERENCES, 1);
	scrpContext = scrpEngine->CreateContext();
	RegisterStdString(scrpEngine);
	//RegisterStdStringUtils(scrpEngine);	// NOT LINKED

	// Global object types
	scrpEngine->RegisterObjectType("vec3_t", sizeof(vec3_t), asOBJ_VALUE | asOBJ_APP_CLASS | asOBJ_POD);
	scrpEngine->RegisterObjectProperty("vec3_t", "float x", 0);
	scrpEngine->RegisterObjectProperty("vec3_t", "float y", 4);
	scrpEngine->RegisterObjectProperty("vec3_t", "float z", 8);

	scrpEngine->RegisterObjectType("entityState_t", sizeof(entityState_t), asOBJ_VALUE | asOBJ_APP_CLASS | asOBJ_POD);
	scrpEngine->RegisterObjectProperty("entityState_t", "int weapon", asOFFSET(entityState_t, weapon));

	scrpEngine->RegisterObjectType("playerState_t", sizeof(playerState_t), asOBJ_VALUE | asOBJ_APP_CLASS | asOBJ_POD);
	scrpEngine->RegisterObjectProperty("playerState_t", "int accuracy_shots", asOFFSET(playerState_t, persistant[PERS_ACCURACY_SHOTS]));
	scrpEngine->RegisterObjectProperty("playerState_t", "vec3_t viewangles", asOFFSET(playerState_t, viewangles));

	scrpEngine->RegisterObjectType("gclient_t", sizeof(gclient_t), asOBJ_VALUE | asOBJ_APP_CLASS | asOBJ_POD);
	scrpEngine->RegisterObjectProperty("gclient_t", "playerState_t ps", asOFFSET(gclient_t, ps));

	scrpEngine->RegisterObjectType("gentity_t", sizeof(gentity_t), asOBJ_REF | asOBJ_NOCOUNT);
	scrpEngine->RegisterObjectProperty("gentity_t", "gclient_t client", asOFFSET(gentity_t, client));
	scrpEngine->RegisterObjectProperty("gentity_t", "entityState_t s", asOFFSET(gentity_t, s));
	scrpEngine->RegisterObjectProperty("gentity_t", "vec3_t maxs", asOFFSET(gentity_t, maxs));
	scrpEngine->RegisterObjectProperty("gentity_t", "vec3_t mins", asOFFSET(gentity_t, mins));
	scrpEngine->RegisterObjectProperty("gentity_t", "int damage", asOFFSET(gentity_t, damage));
	scrpEngine->RegisterObjectProperty("gentity_t", "int dflags", asOFFSET(gentity_t, dflags));
	scrpEngine->RegisterObjectProperty("gentity_t", "int methodOfDeath", asOFFSET(gentity_t, methodOfDeath));
	scrpEngine->RegisterObjectProperty("gentity_t", "int clipmask", asOFFSET(gentity_t, clipmask));
	scrpEngine->RegisterObjectProperty("gentity_t", "int bounceCount", asOFFSET(gentity_t, bounceCount));

	// Global functions

	// VECTOR FUNCTIONS

	//scrpEngine->RegisterGlobalFunction("void VectorSet(vec3_t in, float fx, float fy, float fz)", asFUNCTION(VectorSet3), asCALL_CDECL);
	//scrpEngine->RegisterGlobalFunction("void VectorCopy(vec3_t in, vec3_t out)", asFUNCTION(VectorCopy), asCALL_CDECL);
	//scrpEngine->RegisterGlobalFunction("void VectorScale(vec3_t in, float scale, vec3_t out)", asFUNCTION(VectorScale), asCALL_CDECL);
	//scrpEngine->RegisterGlobalFunction("void AngleVectors(vec3_t angles, vec3_t fwd, vec3_t right, vec3_t up)", asFUNCTION(AngleVectors), asCALL_CDECL);
	scrpEngine->RegisterGlobalFunction("void CalcMuzzlePoint(gentity_t@ ent, vec3_t fwd, vec3_t right, vec3_t up, vec3_t muzzle, float lead_in)", asFUNCTION(CalcMuzzlePoint), asCALL_CDECL);

	// COM FUNCTIONS

	scrpEngine->RegisterGlobalFunction("void Com_Print(string &in)", asFUNCTION(X_Com_Print), asCALL_CDECL);
	scrpEngine->RegisterGlobalFunction("int Com_Clampi(int min, int max, int value)", asFUNCTION(Com_Clampi), asCALL_CDECL);
	scrpEngine->RegisterGlobalFunction("float Com_Clamp(float min, float max, float value)", asFUNCTION(Com_Clamp), asCALL_CDECL);
	scrpEngine->RegisterGlobalFunction("int Com_AbsClampi(int min, int max, int value)", asFUNCTION(Com_AbsClampi), asCALL_CDECL);
	scrpEngine->RegisterGlobalFunction("float Com_AbdClamp(float min, float max, float value)", asFUNCTION(Com_AbsClamp), asCALL_CDECL);


	// Q_ FUNCTIONS
	scrpEngine->RegisterGlobalFunction("int Q_irand(int min, int max)", asFUNCTION(Q_irand), asCALL_CDECL);
	scrpEngine->RegisterGlobalFunction("float Q_flrand(float min, float max)", asFUNCTION(Q_flrand), asCALL_CDECL);

	// MISC
	scrpEngine->RegisterGlobalFunction("int GetWeaponDamage(int weapon)", asFUNCTION(GetWeaponDamage), asCALL_CDECL);

	scrpEngine->RegisterGlobalFunction("float sin(float val)", asFUNCTION(sinCallback), asCALL_CDECL);
	scrpEngine->RegisterGlobalFunction("float cos(float val)", asFUNCTION(cosCallback), asCALL_CDECL);
	scrpEngine->RegisterGlobalFunction("void WP_TraceSetStart(gentity_t@ ent, vec3_t start, vec3_t mins, vec3_t maxs)", asFUNCTION(WP_TraceSetStart), asCALL_CDECL);
	scrpEngine->RegisterGlobalFunction("gentity_t@ CreateMissile(vec3_t start, vec3_t forward, int velocity, int life, gentity_t@ owner)", asFUNCTION(CreateMissile), asCALL_CDECL);
	//scrpEngine->RegisterGlobalFunction("void SetEntityClassname(gentity_t@ ent, string classname)", asFUNCTION(SetEntityClassname), asCALL_CDECL);

	// Global Properties
	// WEAPON STUFF
	scrpEngine->RegisterGlobalProperty("vec3_t wpFwd", &wpFwd);
	scrpEngine->RegisterGlobalProperty("vec3_t wpVright", &wpVright);
	scrpEngine->RegisterGlobalProperty("vec3_t wpUp", &wpUp);
	scrpEngine->RegisterGlobalProperty("vec3_t wpMuzzle", &wpMuzzle);

	scrpEngine->RegisterGlobalProperty("vec3_t vec3_origin", (void*)&vec3_origin);

	scrpEngine->RegisterEnum("weaponTypes_e");
	scrpEngine->RegisterEnumValue("weaponTypes_e", "WP_NONE", WP_NONE);
	scrpEngine->RegisterEnumValue("weaponTypes_e", "WP_SABER",				WP_SABER);
	scrpEngine->RegisterEnumValue("weaponTypes_e", "WP_BRYAR_PISTOL",		WP_BRYAR_PISTOL);
	scrpEngine->RegisterEnumValue("weaponTypes_e", "WP_BLASTER",			WP_BLASTER);
	scrpEngine->RegisterEnumValue("weaponTypes_e", "WP_DISRUPTOR",			WP_DISRUPTOR);
	scrpEngine->RegisterEnumValue("weaponTypes_e", "WP_BOWCASTER",			WP_BOWCASTER);
	scrpEngine->RegisterEnumValue("weaponTypes_e", "WP_REPEATER",			WP_REPEATER);
	scrpEngine->RegisterEnumValue("weaponTypes_e", "WP_DEMP2",				WP_DEMP2);
	scrpEngine->RegisterEnumValue("weaponTypes_e", "WP_FLECHETTE",			WP_FLECHETTE);
	scrpEngine->RegisterEnumValue("weaponTypes_e", "WP_ROCKET_LAUNCHER",	WP_ROCKET_LAUNCHER);
	scrpEngine->RegisterEnumValue("weaponTypes_e", "WP_THERMAL",			WP_THERMAL);
	scrpEngine->RegisterEnumValue("weaponTypes_e", "WP_TRIP_MINE",			WP_TRIP_MINE);
	scrpEngine->RegisterEnumValue("weaponTypes_e", "WP_DET_PACK",			WP_DET_PACK);
	scrpEngine->RegisterEnumValue("weaponTypes_e", "WP_STUN_BATON",			WP_STUN_BATON);
	scrpEngine->RegisterEnumValue("weaponTypes_e", "WP_MELEE",				WP_MELEE);
	scrpEngine->RegisterEnumValue("weaponTypes_e", "WP_EMPLACED_GUN",		WP_EMPLACED_GUN);
	scrpEngine->RegisterEnumValue("weaponTypes_e", "WP_BOT_LASER",			WP_BOT_LASER);
	scrpEngine->RegisterEnumValue("weaponTypes_e", "WP_TURRET",				WP_TURRET);
	scrpEngine->RegisterEnumValue("weaponTypes_e", "WP_ATST_MAIN",			WP_ATST_MAIN);
	scrpEngine->RegisterEnumValue("weaponTypes_e", "WP_ATST_SIDE",			WP_ATST_SIDE);
	scrpEngine->RegisterEnumValue("weaponTypes_e", "WP_TIE_FIGHTER",		WP_TIE_FIGHTER);
	scrpEngine->RegisterEnumValue("weaponTypes_e", "WP_RAPID_FIRE_CONC",	WP_RAPID_FIRE_CONC);
	scrpEngine->RegisterEnumValue("weaponTypes_e", "WP_BLASTER_PISTOL",		WP_BLASTER_PISTOL);

	// Compile the scripts
	char		asExtensionListBuf[4096];			//	The list of file names read in
	int numFiles = gi.FS_GetFileList("scripts/as", ".as", asExtensionListBuf, sizeof(asExtensionListBuf));
	char *holdChar = asExtensionListBuf;
	int length;
	asIScriptModule *mod = scrpEngine->GetModule(0, asGM_ALWAYS_CREATE);
	for(int i = 0; i < numFiles; i++, holdChar += length + 1) {
		length = strlen(holdChar);

		char *buffer;
		int len = gi.FS_ReadFile(va("scripts/as/%s", holdChar), (void**)&buffer);
		if ( len == -1 ) {
			gi.Printf( "error reading file %s\n", holdChar ); // FIXME: clean this message up a little bit
			continue;
		}
		mod->AddScriptSection("script", buffer, len);
	}
	mod->Build();
}

// ================================================================================================================

void G_ShutdownAngelscript() {
	if(!scrpContext)
		return;
	scrpContext->Release();
	if(!scrpEngine)
		return;
	scrpEngine->Release();
}