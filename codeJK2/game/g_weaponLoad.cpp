/*
This file is part of Jedi Knight 2.

    Jedi Knight 2 is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    Jedi Knight 2 is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Jedi Knight 2.  If not, see <http://www.gnu.org/licenses/>.
*/
// Copyright 2001-2013 Raven Software

// g_weaponLoad.cpp 
// fills in memory struct with ext_dat\weapons.dat

// this is excluded from PCH usage 'cos it looks kinda scary to me, being game and ui.... -Ste

#include "g_local.h"
#include "../../lib/json/cJSON.h"
#include <unordered_map>

typedef struct {
	char	*name;
	void	(*func)(centity_t *cent, const struct weaponInfo_s *weapon );
} func_t;

// Bryar
void FX_BryarProjectileThink(  centity_t *cent, const struct weaponInfo_s *weapon );
void FX_BryarAltProjectileThink(  centity_t *cent, const struct weaponInfo_s *weapon );

// Blaster
void FX_BlasterProjectileThink( centity_t *cent, const struct weaponInfo_s *weapon );
void FX_BlasterAltFireThink( centity_t *cent, const struct weaponInfo_s *weapon );

// Bowcaster
void FX_BowcasterProjectileThink( centity_t *cent, const struct weaponInfo_s *weapon );

// Heavy Repeater
void FX_RepeaterProjectileThink( centity_t *cent, const struct weaponInfo_s *weapon );
void FX_RepeaterAltProjectileThink( centity_t *cent, const struct weaponInfo_s *weapon );

// DEMP2
void FX_DEMP2_ProjectileThink( centity_t *cent, const struct weaponInfo_s *weapon );
void FX_DEMP2_AltProjectileThink( centity_t *cent, const struct weaponInfo_s *weapon );

// Golan Arms Flechette
void FX_FlechetteProjectileThink( centity_t *cent, const struct weaponInfo_s *weapon );
void FX_FlechetteAltProjectileThink( centity_t *cent, const struct weaponInfo_s *weapon );

// Personal Rocket Launcher
void FX_RocketProjectileThink( centity_t *cent, const struct weaponInfo_s *weapon );
void FX_RocketAltProjectileThink( centity_t *cent, const struct weaponInfo_s *weapon );

// Emplaced weapon
void FX_EmplacedProjectileThink( centity_t *cent, const struct weaponInfo_s *weapon );

// Turret weapon
void FX_TurretProjectileThink( centity_t *cent, const struct weaponInfo_s *weapon );

// ATST Main weapon
void FX_ATSTMainProjectileThink( centity_t *cent, const struct weaponInfo_s *weapon );

// ATST Side weapons
void FX_ATSTSideMainProjectileThink( centity_t *cent, const struct weaponInfo_s *weapon );
void FX_ATSTSideAltProjectileThink( centity_t *cent, const struct weaponInfo_s *weapon );

// Table used to attach an extern missile function string to the actual cgame function
func_t	funcs[] = {
	{"bryar_func",			FX_BryarProjectileThink},
	{"bryar_alt_func",		FX_BryarAltProjectileThink},
	{"blaster_func",		FX_BlasterProjectileThink},
	{"blaster_alt_func",	FX_BlasterAltFireThink},
	{"bowcaster_func",		FX_BowcasterProjectileThink},
	{"repeater_func",		FX_RepeaterProjectileThink},
	{"repeater_alt_func",	FX_RepeaterAltProjectileThink},
	{"demp2_func",			FX_DEMP2_ProjectileThink},
	{"demp2_alt_func",		FX_DEMP2_AltProjectileThink},
	{"flechette_func",		FX_FlechetteProjectileThink},
	{"flechette_alt_func",	FX_FlechetteAltProjectileThink},
	{"rocket_func",			FX_RocketProjectileThink},
	{"rocket_alt_func",		FX_RocketAltProjectileThink},
	{"emplaced_func",		FX_EmplacedProjectileThink},
	{"turret_func",			FX_TurretProjectileThink},
	{"atstmain_func",		FX_ATSTMainProjectileThink},
	{"atst_side_alt_func",	FX_ATSTSideAltProjectileThink},
	{"atst_side_main_func",	FX_ATSTSideMainProjectileThink},
	{NULL,					NULL}
};

// This is used as a fallback for each new field, in case they're using base files --eez
const int defaultDamage[] = {
	0,							// WP_NONE
	0,							// WP_SABER										// handled elsewhere
	BRYAR_PISTOL_DAMAGE,		// WP_BRYAR_PISTOL
	BLASTER_DAMAGE,				// WP_BLASTER
	DISRUPTOR_MAIN_DAMAGE,		// WP_DISRUPTOR
	BOWCASTER_DAMAGE,			// WP_BOWCASTER
	REPEATER_DAMAGE,			// WP_REPEATER
	DEMP2_DAMAGE,				// WP_DEMP2
	FLECHETTE_DAMAGE,			// WP_FLECHETTE
	ROCKET_DAMAGE,				// WP_ROCKET_LAUNCHER
	TD_DAMAGE,					// WP_THERMAL
	LT_DAMAGE,					// WP_TRIP_MINE
	FLECHETTE_MINE_DAMAGE,		// WP_DET_PACK									// HACK, this is what the code sez.
	STUN_BATON_DAMAGE,			// WP_STUN_BATON
	0,							// WP_MELEE										// handled by the melee attack function
	EMPLACED_DAMAGE,			// WP_EMPLACED
	BRYAR_PISTOL_DAMAGE,		// WP_BOT_LASER
	0,							// WP_TURRET									// handled elsewhere
	ATST_MAIN_DAMAGE,			// WP_ATST_MAIN
	ATST_SIDE_MAIN_DAMAGE,		// WP_ATST_SIDE
	EMPLACED_DAMAGE,			// WP_TIE_FIGHTER
	EMPLACED_DAMAGE,			// WP_RAPID_FIRE_CONC
	BRYAR_PISTOL_DAMAGE			// WP_BLASTER_PISTOL
};

const int defaultAltDamage[] = {
	0,							// WP_NONE
	0,							// WP_SABER										// handled elsewhere
	BRYAR_PISTOL_DAMAGE,		// WP_BRYAR_PISTOL
	BLASTER_DAMAGE,				// WP_BLASTER
	DISRUPTOR_ALT_DAMAGE,		// WP_DISRUPTOR
	BOWCASTER_DAMAGE,			// WP_BOWCASTER
	REPEATER_ALT_DAMAGE,		// WP_REPEATER
	DEMP2_ALT_DAMAGE,			// WP_DEMP2
	FLECHETTE_ALT_DAMAGE,		// WP_FLECHETTE
	ROCKET_DAMAGE,				// WP_ROCKET_LAUNCHER
	TD_ALT_DAMAGE,				// WP_THERMAL
	LT_DAMAGE,					// WP_TRIP_MINE
	FLECHETTE_MINE_DAMAGE,		// WP_DET_PACK									// HACK, this is what the code sez.
	STUN_BATON_ALT_DAMAGE,		// WP_STUN_BATON
	0,							// WP_MELEE										// handled by the melee attack function
	EMPLACED_DAMAGE,			// WP_EMPLACED
	BRYAR_PISTOL_DAMAGE,		// WP_BOT_LASER
	0,							// WP_TURRET									// handled elsewhere
	ATST_MAIN_DAMAGE,			// WP_ATST_MAIN
	ATST_SIDE_ALT_DAMAGE,		// WP_ATST_SIDE
	EMPLACED_DAMAGE,			// WP_TIE_FIGHTER
	0,							// WP_RAPID_FIRE_CONC							// repeater alt damage is used instead
	BRYAR_PISTOL_DAMAGE			// WP_BLASTER_PISTOL
};

const int defaultSplashDamage[] = {
	0,									// WP_NONE
	0,									// WP_SABER
	0,									// WP_BRYAR_PISTOL
	0,									// WP_BLASTER
	0,									// WP_DISRUPTOR
	BOWCASTER_SPLASH_DAMAGE,			// WP_BOWCASTER
	0,									// WP_REPEATER
	0,									// WP_DEMP2
	0,									// WP_FLECHETTE
	ROCKET_SPLASH_DAMAGE,				// WP_ROCKET_LAUNCHER
	TD_SPLASH_DAM,						// WP_THERMAL
	LT_SPLASH_DAM,						// WP_TRIP_MINE
	FLECHETTE_MINE_SPLASH_DAMAGE,		// WP_DET_PACK									// HACK, this is what the code sez.
	0,									// WP_STUN_BATON
	0,									// WP_MELEE
	0,									// WP_EMPLACED
	0,									// WP_BOT_LASER
	0,									// WP_TURRET
	0,									// WP_ATST_MAIN
	ATST_SIDE_MAIN_SPLASH_DAMAGE,		// WP_ATST_SIDE
	0,									// WP_TIE_FIGHTER
	0,									// WP_RAPID_FIRE_CONC
	0									// WP_BLASTER_PISTOL
};

const float defaultSplashRadius[] = {
	0,									// WP_NONE
	0,									// WP_SABER
	0,									// WP_BRYAR_PISTOL
	0,									// WP_BLASTER
	0,									// WP_DISRUPTOR
	BOWCASTER_SPLASH_RADIUS,			// WP_BOWCASTER
	0,									// WP_REPEATER
	0,									// WP_DEMP2
	0,									// WP_FLECHETTE
	ROCKET_SPLASH_RADIUS,				// WP_ROCKET_LAUNCHER
	TD_SPLASH_RAD,						// WP_THERMAL
	LT_SPLASH_RAD,						// WP_TRIP_MINE
	FLECHETTE_MINE_SPLASH_RADIUS,		// WP_DET_PACK									// HACK, this is what the code sez.
	0,									// WP_STUN_BATON
	0,									// WP_MELEE
	0,									// WP_EMPLACED
	0,									// WP_BOT_LASER
	0,									// WP_TURRET
	0,									// WP_ATST_MAIN
	ATST_SIDE_MAIN_SPLASH_RADIUS,		// WP_ATST_SIDE
	0,									// WP_TIE_FIGHTER
	0,									// WP_RAPID_FIRE_CONC
	0									// WP_BLASTER_PISTOL
};

const int defaultAltSplashDamage[] = {
	0,									// WP_NONE
	0,									// WP_SABER										// handled elsewhere
	0,									// WP_BRYAR_PISTOL
	0,									// WP_BLASTER
	0,									// WP_DISRUPTOR
	BOWCASTER_SPLASH_DAMAGE,			// WP_BOWCASTER
	REPEATER_ALT_SPLASH_DAMAGE,			// WP_REPEATER
	DEMP2_ALT_DAMAGE,					// WP_DEMP2
	FLECHETTE_ALT_SPLASH_DAM,			// WP_FLECHETTE
	ROCKET_SPLASH_DAMAGE,				// WP_ROCKET_LAUNCHER
	TD_ALT_SPLASH_DAM,					// WP_THERMAL
	TD_ALT_SPLASH_DAM,					// WP_TRIP_MINE
	FLECHETTE_MINE_SPLASH_DAMAGE,		// WP_DET_PACK									// HACK, this is what the code sez.
	0,									// WP_STUN_BATON
	0,									// WP_MELEE										// handled by the melee attack function
	0,									// WP_EMPLACED
	0,									// WP_BOT_LASER
	0,									// WP_TURRET									// handled elsewhere
	0,									// WP_ATST_MAIN
	ATST_SIDE_ALT_SPLASH_DAMAGE,		// WP_ATST_SIDE
	0,									// WP_TIE_FIGHTER
	0,									// WP_RAPID_FIRE_CONC
	0									// WP_BLASTER_PISTOL
};

const float defaultAltSplashRadius[] = {
	0,							// WP_NONE
	0,							// WP_SABER										// handled elsewhere
	0,							// WP_BRYAR_PISTOL
	0,							// WP_BLASTER
	0,							// WP_DISRUPTOR
	BOWCASTER_SPLASH_RADIUS,	// WP_BOWCASTER
	REPEATER_ALT_SPLASH_RADIUS,	// WP_REPEATER
	DEMP2_ALT_SPLASHRADIUS,		// WP_DEMP2
	FLECHETTE_ALT_SPLASH_RAD,	// WP_FLECHETTE
	ROCKET_SPLASH_RADIUS,		// WP_ROCKET_LAUNCHER
	TD_ALT_SPLASH_RAD,			// WP_THERMAL
	LT_SPLASH_RAD,				// WP_TRIP_MINE
	FLECHETTE_ALT_SPLASH_RAD,	// WP_DET_PACK									// HACK, this is what the code sez.
	0,							// WP_STUN_BATON
	0,							// WP_MELEE										// handled by the melee attack function
	0,							// WP_EMPLACED
	0,							// WP_BOT_LASER
	0,							// WP_TURRET									// handled elsewhere
	0,							// WP_ATST_MAIN
	ATST_SIDE_ALT_SPLASH_RADIUS,// WP_ATST_SIDE
	0,							// WP_TIE_FIGHTER
	0,							// WP_RAPID_FIRE_CONC
	0							// WP_BLASTER_PISTOL
};

static void PARSE_Classname(cJSON* json, weaponData_t *wp) {
	Q_strncpyz(wp->classname, cJSON_ToString(json), sizeof(wp->classname));
}

static void PARSE_WeaponModel(cJSON* json, weaponData_t *wp) {
	Q_strncpyz(wp->weaponMdl, cJSON_ToString(json), sizeof(wp->weaponMdl));
}

static void PARSE_FiringSound(cJSON* json, weaponData_t *wp) {
	Q_strncpyz(wp->firingSnd, cJSON_ToString(json), sizeof(wp->firingSnd));
}

static void PARSE_AltFiringSound(cJSON* json, weaponData_t *wp) {
	Q_strncpyz(wp->altFiringSnd, cJSON_ToString(json), sizeof(wp->altFiringSnd));
}

static void PARSE_StopSound(cJSON* json, weaponData_t *wp) {
	Q_strncpyz(wp->stopSnd, cJSON_ToString(json), sizeof(wp->stopSnd));
}

static void PARSE_ChargeSound(cJSON* json, weaponData_t *wp) {
	Q_strncpyz(wp->chargeSnd, cJSON_ToString(json), sizeof(wp->chargeSnd));
}

static void PARSE_AltChargeSound(cJSON* json, weaponData_t *wp) {
	Q_strncpyz(wp->altChargeSnd, cJSON_ToString(json), sizeof(wp->altChargeSnd));
}

static void PARSE_SelectSound(cJSON* json, weaponData_t *wp) {
	Q_strncpyz(wp->selectSnd, cJSON_ToString(json), sizeof(wp->selectSnd));
}

static void PARSE_AmmoIndex(cJSON* json, weaponData_t *wp) {
	char ammoName[32];
	Q_strncpyz(ammoName, cJSON_ToString(json), sizeof(ammoName));
	if(!Q_stricmp(ammoName, "AMMO_NONE"))
		wp->ammoIndex = AMMO_NONE;
	else if(!Q_stricmp(ammoName, "AMMO_FORCE"))
		wp->ammoIndex = AMMO_FORCE;
	else if(!Q_stricmp(ammoName, "AMMO_BLASTER"))
		wp->ammoIndex = AMMO_BLASTER;
	else if(!Q_stricmp(ammoName, "AMMO_POWERCELL"))
		wp->ammoIndex = AMMO_POWERCELL;
	else if(!Q_stricmp(ammoName, "AMMO_METAL_BOLTS"))
		wp->ammoIndex = AMMO_METAL_BOLTS;
	else if(!Q_stricmp(ammoName, "AMMO_ROCKETS"))
		wp->ammoIndex = AMMO_ROCKETS;
	else if(!Q_stricmp(ammoName, "AMMO_EMPLACED"))
		wp->ammoIndex = AMMO_EMPLACED;
	else if(!Q_stricmp(ammoName, "AMMO_THERMAL"))
		wp->ammoIndex = AMMO_THERMAL;
	else if(!Q_stricmp(ammoName, "AMMO_TRIPMINE"))
		wp->ammoIndex = AMMO_TRIPMINE;
	else if(!Q_stricmp(ammoName, "AMMO_DETPACK"))
		wp->ammoIndex = AMMO_DETPACK;
	else
		wp->ammoIndex = AMMO_NONE;
}

static void PARSE_AmmoLow(cJSON* json, weaponData_t *wp) {
	wp->ammoLow = cJSON_ToInteger(json);
}

static void PARSE_EnergyPerShot(cJSON* json, weaponData_t* wp) {
	wp->energyPerShot = cJSON_ToInteger(json);
}

static void PARSE_FireTime(cJSON* json, weaponData_t* wp) {
	wp->fireTime = cJSON_ToInteger(json);
}

static void PARSE_Range(cJSON* json, weaponData_t* wp) {
	wp->range = cJSON_ToInteger(json);
}

static void PARSE_AltEnergyPerShot(cJSON* json, weaponData_t* wp) {
	wp->altEnergyPerShot = cJSON_ToInteger(json);
}

static void PARSE_AltFireTime(cJSON* json, weaponData_t* wp) {
	wp->altFireTime = cJSON_ToInteger(json);
}

static void PARSE_AltRange(cJSON* json, weaponData_t* wp) {
	wp->altRange = cJSON_ToInteger(json);
}

static void PARSE_WeaponIcon(cJSON* json, weaponData_t* wp) {
	Q_strncpyz(wp->weaponIcon, cJSON_ToString(json), sizeof(wp->weaponIcon));
}

static void PARSE_NumBarrels(cJSON* json, weaponData_t* wp) {
	wp->numBarrels = cJSON_ToInteger(json);
}

static void PARSE_MissileModel(cJSON* json, weaponData_t* wp) {
	Q_strncpyz(wp->missileMdl, cJSON_ToString(json), sizeof(wp->missileMdl));
}

static void PARSE_MissileSound(cJSON* json, weaponData_t* wp) {
	Q_strncpyz(wp->missileSound, cJSON_ToString(json), sizeof(wp->missileSound));
}

static void PARSE_MissileDLight(cJSON* json, weaponData_t* wp) {
	wp->missileDlight = cJSON_ToNumber(json);
}

static void PARSE_MissileDLightColor(cJSON* json, weaponData_t* wp) {
	cJSON_ReadFloatArrayv(json, 3, &wp->missileDlightColor[0],
		&wp->missileDlightColor[1], &wp->missileDlightColor[2]);
}

static void PARSE_AltMissileModel(cJSON* json, weaponData_t* wp) {
	Q_strncpyz(wp->alt_missileMdl, cJSON_ToString(json), sizeof(wp->alt_missileMdl));
}

static void PARSE_AltMissileSound(cJSON* json, weaponData_t* wp) {
	Q_strncpyz(wp->alt_missileSound, cJSON_ToString(json), sizeof(wp->alt_missileSound));
}

static void PARSE_AltMissileDLight(cJSON* json, weaponData_t* wp) {
	wp->alt_missileDlight = cJSON_ToNumber(json);
}

static void PARSE_AltMissileDLightColor(cJSON* json, weaponData_t* wp) {
	cJSON_ReadFloatArrayv(json, 3, &wp->alt_missileDlightColor[0],
		&wp->alt_missileDlightColor[1], &wp->alt_missileDlightColor[2]);
}

static void PARSE_MissileHitSound(cJSON* json, weaponData_t* wp) {
	Q_strncpyz(wp->missileHitSound, cJSON_ToString(json), sizeof(wp->missileHitSound));
}

static void PARSE_AltMissileHitSound(cJSON* json, weaponData_t* wp) {
	Q_strncpyz(wp->altmissileHitSound, cJSON_ToString(json), sizeof(wp->altmissileHitSound));
}

static void PARSE_Func(cJSON* json, weaponData_t* wp) {
	for(int i = 0; funcs[i].func; i++) {
		if(!Q_stricmp(cJSON_ToString(json), funcs[i].name)) {
			wp->func = funcs[i].func;
			break;
		}
	}
}

static void PARSE_AltFunc(cJSON* json, weaponData_t* wp) {
	for(int i = 0; funcs[i].func; i++) {
		if(!Q_stricmp(cJSON_ToString(json), funcs[i].name)) {
			wp->altfunc = funcs[i].func;
			break;
		}
	}
}

static void PARSE_MuzzleEffect(cJSON* json, weaponData_t* wp) {
	Q_strncpyz(wp->mMuzzleEffect, cJSON_ToString(json), sizeof(wp->mMuzzleEffect));
}

static void PARSE_AltMuzzleEffect(cJSON* json, weaponData_t* wp) {
	Q_strncpyz(wp->mAltMuzzleEffect, cJSON_ToString(json), sizeof(wp->mAltMuzzleEffect));
}

static void PARSE_Damage(cJSON* json, weaponData_t* wp) {
	wp->damage = cJSON_ToInteger(json);
}

static void PARSE_AltDamage(cJSON* json, weaponData_t* wp) {
	wp->altDamage = cJSON_ToInteger(json);
}

static void PARSE_SplashDamage(cJSON* json, weaponData_t* wp) {
	wp->splashDamage = cJSON_ToInteger(json);
}

static void PARSE_SplashRadius(cJSON* json, weaponData_t* wp) {
	wp->splashRadius = cJSON_ToInteger(json);
}

static void PARSE_AltSplashDamage(cJSON* json, weaponData_t* wp) {
	wp->altSplashDamage = cJSON_ToInteger(json);
}

static void PARSE_AltSplashRadius(cJSON* json, weaponData_t* wp) {
	wp->altSplashRadius = cJSON_ToInteger(json);
}

static void PARSE_NPCDamageModifier(cJSON* json, weaponData_t* wp) {
	wp->npcDamageModifier = cJSON_ToNumber(json);
}

static void PARSE_ProjectileVelocity(cJSON* json, weaponData_t* wp) {
	wp->projectileVelocity = cJSON_ToNumber(json);
}

static void PARSE_AltProjectileVelocity(cJSON* json, weaponData_t* wp) {
	wp->altProjectileVelocity = cJSON_ToNumber(json);
}

// -----------------------------------------------

static unordered_map<string, int> weapons;

#define SPAIR(x)	make_pair("#x", x)
static void WP_PopulateWeaponList() {
	weapons.insert(SPAIR(WP_NONE));
	weapons.insert(SPAIR(WP_SABER));
	weapons.insert(SPAIR(WP_BRYAR_PISTOL));
	weapons.insert(SPAIR(WP_BLASTER));
	weapons.insert(SPAIR(WP_DISRUPTOR));
	weapons.insert(SPAIR(WP_BOWCASTER));
	weapons.insert(SPAIR(WP_REPEATER));
	weapons.insert(SPAIR(WP_DEMP2));
	weapons.insert(SPAIR(WP_FLECHETTE));
	weapons.insert(SPAIR(WP_ROCKET_LAUNCHER));
	weapons.insert(SPAIR(WP_THERMAL));
	weapons.insert(SPAIR(WP_TRIP_MINE));
	weapons.insert(SPAIR(WP_DET_PACK));
	weapons.insert(SPAIR(WP_STUN_BATON));
	weapons.insert(SPAIR(WP_MELEE));
	weapons.insert(SPAIR(WP_EMPLACED_GUN));
	weapons.insert(SPAIR(WP_BOT_LASER));
	weapons.insert(SPAIR(WP_TURRET));
	weapons.insert(SPAIR(WP_ATST_MAIN));
	weapons.insert(SPAIR(WP_ATST_SIDE));
	weapons.insert(SPAIR(WP_TIE_FIGHTER));
	weapons.insert(SPAIR(WP_RAPID_FIRE_CONC));
	weapons.insert(SPAIR(WP_BLASTER_PISTOL));
}

typedef void (*pfParse)(cJSON*, weaponData_t*);
static unordered_map<string, pfParse> parsers;

#define SPAIR2(x)	make_pair("##x##", (pfParse)PARSE_##x##)
static void WP_PopulateParseList() {
	parsers.insert(SPAIR2(Classname));
	parsers.insert(SPAIR2(WeaponModel));
	parsers.insert(SPAIR2(FiringSound));
	parsers.insert(SPAIR2(AltFiringSound));
	parsers.insert(SPAIR2(StopSound));
	parsers.insert(SPAIR2(ChargeSound));
	parsers.insert(SPAIR2(AltChargeSound));
	parsers.insert(SPAIR2(SelectSound));
	parsers.insert(SPAIR2(AmmoIndex));
	parsers.insert(SPAIR2(AmmoLow));
	parsers.insert(SPAIR2(EnergyPerShot));
	parsers.insert(SPAIR2(FireTime));
	parsers.insert(SPAIR2(Range));
	parsers.insert(SPAIR2(AltEnergyPerShot));
	parsers.insert(SPAIR2(AltFireTime));
	parsers.insert(SPAIR2(AltRange));
	parsers.insert(SPAIR2(WeaponIcon));
	parsers.insert(SPAIR2(NumBarrels));
	parsers.insert(SPAIR2(MissileModel));
	parsers.insert(SPAIR2(MissileSound));
	parsers.insert(SPAIR2(MissileDLight));
	parsers.insert(SPAIR2(MissileDLightColor));
	parsers.insert(SPAIR2(AltMissileModel));
	parsers.insert(SPAIR2(AltMissileSound));
	parsers.insert(SPAIR2(AltMissileDLight));
	parsers.insert(SPAIR2(AltMissileDLightColor));
	parsers.insert(SPAIR2(MissileHitSound));
	parsers.insert(SPAIR2(AltMissileHitSound));
	parsers.insert(SPAIR2(Func));
	parsers.insert(SPAIR2(AltFunc));
	parsers.insert(SPAIR2(MuzzleEffect));
	parsers.insert(SPAIR2(AltMuzzleEffect));
	parsers.insert(SPAIR2(Damage));
	parsers.insert(SPAIR2(AltDamage));
	parsers.insert(SPAIR2(SplashDamage));
	parsers.insert(SPAIR2(SplashRadius));
	parsers.insert(SPAIR2(AltSplashDamage));
	parsers.insert(SPAIR2(AltSplashRadius));
	parsers.insert(SPAIR2(NPCDamageModifier));
	parsers.insert(SPAIR2(ProjectileVelocity));
	parsers.insert(SPAIR2(AltProjectileVelocity));
}

//--------------------------------------------
static void WP_ParseParms(const char *buffer)
{
	char error[MAX_STRING_CHARS];

	cJSON* json = cJSON_ParsePooled(buffer, error, sizeof(error));
	if(json == NULL)
	{
		Com_Printf("^1%s\n", error);
		return;
	}

	for(auto listPtr = cJSON_GetFirstItem(json); listPtr; listPtr = cJSON_GetNextItem(json)) {
		for(auto wpnPtr = cJSON_GetFirstItem(listPtr); wpnPtr; wpnPtr = cJSON_GetNextItem(listPtr)) {
			auto key = cJSON_GetItemKey(wpnPtr);
			auto it2 = weapons.find(key);
			if(it2 == weapons.end()) {
				Com_Printf("^3WARNING: Invalid weapon %s in weapon parameter file\n");
				continue;
			}

			weaponData_t x;
			for(auto parmPtr = cJSON_GetFirstItem(wpnPtr); parmPtr; parmPtr = cJSON_GetNextItem(listPtr)) {
				auto parmKey = cJSON_GetItemKey(parmPtr);
				auto it = parsers.find(parmKey);
				if(it == parsers.end()) {
					Com_Printf("^3WARNING: Nonfunctioning key %s found in weapon parameter file\n");
					continue;
				}
				it->second(parmPtr, &x);
			}

			memcpy(&weaponData[it2->second], &x, sizeof(weaponData_t));
		}
	}
}

//--------------------------------------------
extern cvar_t *g_weaponFile;
void WP_LoadWeaponParms (void)
{
	char *buffer;
	int len;

	len = gi.FS_ReadFile(g_weaponFile->string,(void **) &buffer);

	// initialise the data area
	memset(weaponData, 0, WP_NUM_WEAPONS * sizeof(weaponData_t));	
	WP_PopulateWeaponList();
	WP_PopulateParseList();

	WP_ParseParms(buffer);

	gi.FS_FreeFile( buffer );	//let go of the buffer
}