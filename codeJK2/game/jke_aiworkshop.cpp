#include "g_local.h"
#include "cgame/cg_local.h"
#include "cgame/cg_media.h"

#define DRAWBOX_REFRESH	500	// How long it takes to redraw the boxes
#define BOX_VERTICES 8

qboolean inAIWorkshop = qfalse;

int drawBoxesTime = 0;
int selectedAI = ENTITYNUM_NONE;

extern stringID_table_t BSTable[];
extern stringID_table_t teamTable[];
extern stringID_table_t NPCClassTable[];
extern stringID_table_t RankTable[];
extern stringID_table_t MoveTypeTable[];
extern stringID_table_t FPTable[];

static void WorkshopDrawEntityInformation(gentity_t* ent, int x, const char* title) {
	int add = 6;
	vec4_t textcolor = { 0.4f, 0.4f, 0.8f, 1.0f };

	cgi_R_Font_DrawString(x, 30, title, textcolor, cgs.media.qhFontSmall, -1, 0.5f);
	if (ent->NPC->goalEntity) {
		cgi_R_Font_DrawString(x, 30 + add, va("goalEnt = %i", ent->NPC->goalEntity->s.number), textcolor, cgs.media.qhFontSmall, -1, 0.5f);
		add += 6;
	}
	cgi_R_Font_DrawString(x, 30 + add, va("bs = %s", BSTable[ent->NPC->behaviorState].name), textcolor, cgs.media.qhFontSmall, -1, 0.5f);
	add += 6;
	if (ent->NPC->combatMove) {
		cgi_R_Font_DrawString(x, 30 + add, "-- in combat move --", textcolor, cgs.media.qhFontSmall, -1, 0.5f);
		add += 6;
	}
	
	cgi_R_Font_DrawString(x, 30 + add, va("class = %s", NPCClassTable[ent->client->NPC_class].name), textcolor, cgs.media.qhFontSmall, -1, 0.5f);
	add += 6;
	cgi_R_Font_DrawString(x, 30 + add, va("rank = %s (%i)", RankTable[ent->NPC->rank].name, ent->NPC->rank), textcolor, cgs.media.qhFontSmall, -1, 0.5f);
	add += 6;

	if (ent->NPC->scriptFlags) {
		cgi_R_Font_DrawString(x, 30 + add, va("scriptFlags: %i", ent->NPC->scriptFlags), textcolor, cgs.media.qhFontSmall, -1, 0.5f);
		add += 6;
	}
	if (ent->NPC->aiFlags) {
		cgi_R_Font_DrawString(x, 30 + add, va("aiFlags: %i", ent->NPC->aiFlags), textcolor, cgs.media.qhFontSmall, -1, 0.5f);
		add += 6;
	}

	if (ent->client->noclip) {
		cgi_R_Font_DrawString(x, 30 + add, "cheat: NOCLIP", textcolor, cgs.media.qhFontSmall, -1, 0.5f);
		add += 6;
	}

	if (ent->flags & FL_GODMODE) {
		cgi_R_Font_DrawString(x, 30 + add, "cheat: GODMODE", textcolor, cgs.media.qhFontSmall, -1, 0.5f);
		add += 6;
	}

	if (ent->flags & FL_NOTARGET) {
		cgi_R_Font_DrawString(x, 30 + add, "cheat: NOTARGET", textcolor, cgs.media.qhFontSmall, -1, 0.5f);
		add += 6;
	}

	if (ent->flags & FL_UNDYING) {
		cgi_R_Font_DrawString(x, 30 + add, "cheat: UNDEAD", textcolor, cgs.media.qhFontSmall, -1, 0.5f);
		add += 6;
	}

	cgi_R_Font_DrawString(x, 30 + add, va("playerTeam: %s", teamTable[ent->client->playerTeam].name), textcolor, cgs.media.qhFontSmall, -1, 0.5f);
	add += 6;

	cgi_R_Font_DrawString(x, 30 + add, va("enemyTeam: %s", teamTable[ent->client->enemyTeam].name), textcolor, cgs.media.qhFontSmall, -1, 0.5f);
	add += 6;

	cgi_R_Font_DrawString(x, 30 + add, "-- currently active timers --", textcolor, cgs.media.qhFontSmall, -1, 0.5f);
	add += 6;
	auto timers = TIMER_List(ent);
	for (auto it = timers.begin(); it != timers.end(); ++it) {
		if (it->second >= 0) {
			cgi_R_Font_DrawString(x, 30 + add, va("%s : %i", it->first.c_str(), it->second), textcolor, cgs.media.qhFontSmall, -1, 0.5f);
			add += 6;
		}
	}
}

//
// Draw textual information about the NPC in our crosshairs and also the currently selected one
//
void WorkshopDrawClientsideInformation() {
	if (inAIWorkshop == qfalse) {
		return;
	}

	// Draw the information for the NPC that is in our crosshairs
	if (cg.crosshairClientNum != ENTITYNUM_NONE && cg.crosshairClientNum != 0 && g_entities[cg.crosshairClientNum].client) {
		gentity_t* crossEnt = &g_entities[cg.crosshairClientNum];
		WorkshopDrawEntityInformation(crossEnt, 10, "Crosshair AI");
	}

	// Draw the information for the AI that we have selected
	if (selectedAI != ENTITYNUM_NONE) {
		gentity_t* selectedEnt = &g_entities[selectedAI];
		WorkshopDrawEntityInformation(selectedEnt, 500, "Selected AI");
	}
}

//
//	Draws a box around the bounding box of the entity
//
void WorkshopDrawEntBox(gentity_t* ent, int colorOverride = -1) {
	unsigned color = 0x666666; // G_SoundOnEnt(ent, "satan.mp3");
	if (colorOverride == -1 && selectedAI == ent->s.number) {
		// Draw a box around our goal ent
		if (ent->NPC->goalEntity != nullptr) {
			WorkshopDrawEntBox(ent->NPC->goalEntity, color);
		}
		color = 0x0000FFFF; // Yellow = selected AI
	}
	else if (colorOverride == -1 && ent->client->playerTeam == TEAM_ENEMY) {
		color = 0x000000FF; // Red = enemy
	}
	else if (colorOverride == -1 && ent->client->playerTeam == TEAM_PLAYER) {
		color = 0x0000FF00; // Green = ally
	}
	else if (colorOverride == -1 && ent->client->playerTeam == TEAM_NEUTRAL) {
		color = 0x00FF0000; // Blue = neutral
	}
	else if (colorOverride != -1) {
		color = colorOverride;
	}

	// This is going to look like a mess and I can't really comment on this, but I drew this out manually and it works.
	vec3_t vertices[BOX_VERTICES] = { 0 };
	for (int i = 0; i < BOX_VERTICES; i++) {
		VectorCopy(ent->currentOrigin, vertices[i]);
	}

	for (int i = 4; i < BOX_VERTICES; i++) {
		vertices[i][2] += ent->mins[2];
	}
	for (int i = 0; i < 4; i++) {
		vertices[i][2] += ent->maxs[2];
	}
	vertices[0][0] += ent->mins[0];
	vertices[1][0] += ent->mins[0];
	vertices[4][0] += ent->mins[0];
	vertices[5][0] += ent->mins[0];
	vertices[2][0] += ent->maxs[0];
	vertices[3][0] += ent->maxs[0];
	vertices[6][0] += ent->maxs[0];
	vertices[7][0] += ent->maxs[0];
	vertices[0][1] += ent->mins[1];
	vertices[2][1] += ent->mins[1];
	vertices[4][1] += ent->mins[1];
	vertices[6][1] += ent->mins[1];
	vertices[1][1] += ent->maxs[1];
	vertices[3][1] += ent->maxs[1];
	vertices[5][1] += ent->maxs[1];
	vertices[7][1] += ent->maxs[1];

	G_DebugLine(vertices[0], vertices[1], DRAWBOX_REFRESH, color, qtrue);
	G_DebugLine(vertices[2], vertices[3], DRAWBOX_REFRESH, color, qtrue);
	G_DebugLine(vertices[0], vertices[2], DRAWBOX_REFRESH, color, qtrue);
	G_DebugLine(vertices[1], vertices[3], DRAWBOX_REFRESH, color, qtrue);
	G_DebugLine(vertices[0], vertices[4], DRAWBOX_REFRESH, color, qtrue);
	G_DebugLine(vertices[1], vertices[5], DRAWBOX_REFRESH, color, qtrue);
	G_DebugLine(vertices[2], vertices[6], DRAWBOX_REFRESH, color, qtrue);
	G_DebugLine(vertices[3], vertices[7], DRAWBOX_REFRESH, color, qtrue);
	G_DebugLine(vertices[4], vertices[5], DRAWBOX_REFRESH, color, qtrue);
	G_DebugLine(vertices[6], vertices[7], DRAWBOX_REFRESH, color, qtrue);
	G_DebugLine(vertices[4], vertices[6], DRAWBOX_REFRESH, color, qtrue);
	G_DebugLine(vertices[5], vertices[7], DRAWBOX_REFRESH, color, qtrue);
}

//
//	Occurs every frame
//
void WorkshopThink() {
	if (!inAIWorkshop) {
		return;
	}

	// If the selected AI is dead, deselect it
	if (selectedAI != ENTITYNUM_NONE) {
		gentity_t* ent = &g_entities[selectedAI];
		if (ent->health <= 0) {
			selectedAI = ENTITYNUM_NONE;
		}
	}
	if (drawBoxesTime < level.time) {
		for (int i = 1; i < globals.num_entities; i++) {
			if (!PInUse(i)) {
				continue;
			}

			gentity_t* found = &g_entities[i];
			if (!found->client) {
				continue;
			}

			WorkshopDrawEntBox(found);
			drawBoxesTime = level.time + DRAWBOX_REFRESH - 50;	// -50 so we get an extra frame to draw, so the boxes don't blink
		}
	}
}

//
//	Workshop mode just got toggled by console command
//
void WorkshopToggle() {
	inAIWorkshop = !inAIWorkshop;

	if (inAIWorkshop) {
		// Workshop mode just got enabled
		gi.Printf("AI Workshop enabled\n");
	}
	else {
		// Workshop mode just got disabled
		gi.Printf("AI Workshop disabled\n");
	}
}

//////////////////////
//
// Below are various commands that we can do
//
//////////////////////

void WorkshopSelect_f(gentity_t* ent) {
	vec3_t		src, dest, vf;
	trace_t		trace;
	if (!inAIWorkshop) {
		gi.Printf("You need to be in the AI workshop to use this command.\n");
		return;
	}
	VectorCopy(ent->client->renderInfo.eyePoint, src);
	AngleVectors(ent->client->ps.viewangles, vf, NULL, NULL);//ent->client->renderInfo.eyeAngles was cg.refdef.viewangles, basically
	//extend to find end of use trace
	VectorMA(src, 32967.0f, vf, dest);

	//Trace ahead to find a valid target
	gi.trace(&trace, src, vec3_origin, vec3_origin, dest, ent->s.number, MASK_OPAQUE | CONTENTS_SOLID | CONTENTS_BODY | CONTENTS_CORPSE, G2_NOCOLLIDE, 0);
	if (trace.fraction == 1.0f || trace.entityNum < 1)
	{
		return;
	}

	gentity_t* target = &g_entities[trace.entityNum];
	if (target->client) {
		selectedAI = target->s.number;
	}
	else {
		gi.Printf("No NPC in crosshair\n");
	}
}

// Deselect currently selected NPC
void WorkshopDeselect_f(gentity_t* ent) {
	if (!inAIWorkshop) {
		gi.Printf("You need to be in the AI workshop to use this command.\n");
		return;
	}
	selectedAI = ENTITYNUM_NONE;
}

// View all behavior states
void Workshop_List_BehaviorState_f(gentity_t* ent) {
	if (inAIWorkshop == qfalse) {
		gi.Printf("You need to be in the AI workshop to use this command.\n");
		return;
	}
	for (int i = 0; i < NUM_BSTATES; i++) {
		gi.Printf(va("%s\n", BSTable[i].name));
	}
}

// View all teams
void Workshop_List_Team_f(gentity_t* ent) {
	if (inAIWorkshop == qfalse) {
		gi.Printf("You need to be in the AI workshop to use this command.\n");
		return;
	}
	for (int i = 0; i < TEAM_NUM_TEAMS; i++) {
		gi.Printf(va("%s\n", teamTable[i].name));
	}
}

// View all ranks
void Workshop_List_Ranks_f(gentity_t* ent) {
	if (inAIWorkshop == qfalse) {
		gi.Printf("You need to be in the AI workshop to use this command.\n");
		return;
	}
	for (int i = 0; i < RANK_MAX; i++) {
		gi.Printf(va("%s\n", RankTable[i].name));
	}
}

// View all classes
void Workshop_List_Classes_f(gentity_t* ent) {
	if (inAIWorkshop == qfalse) {
		gi.Printf("You need to be in the AI workshop to use this command.\n");
		return;
	}
	for (int i = 0; i < CLASS_NUM_CLASSES; i++) {
		gi.Printf(va("%s\n", NPCClassTable[i].name));
	}
}

// View all movetypes
void Workshop_List_Movetypes_f(gentity_t* ent) {
	if (inAIWorkshop == qfalse) {
		gi.Printf("You need to be in the AI workshop to use this command.\n");
		return;
	}
	for (int i = 0; i <= MT_FLYSWIM; i++) {
		gi.Printf(va("%s\n", MoveTypeTable[i].name));
	}
}

// View all forcepowers
void Workshop_List_ForcePowers_f(gentity_t* ent) {
	if (inAIWorkshop == qfalse) {
		gi.Printf("You need to be in the AI workshop to use this command.\n");
		return;
	}
	for (int i = 0; i <= FP_SABER_OFFENSE; i++) {
		gi.Printf(va("%s\n", FPTable[i].name));
	}
}

// List all scriptflags
#define PRINTFLAG(x) gi.Printf("" #x " = %i\n", #x)
void Workshop_List_Scriptflags_f(gentity_t* ent) {
	if (inAIWorkshop == qfalse) {
		gi.Printf("You need to be in the AI workshop to use this command.\n");
		return;
	}
	PRINTFLAG(SCF_CROUCHED);
	PRINTFLAG(SCF_WALKING);
	PRINTFLAG(SCF_MORELIGHT);
	PRINTFLAG(SCF_LEAN_RIGHT);
	PRINTFLAG(SCF_LEAN_LEFT);
	PRINTFLAG(SCF_RUNNING);
	PRINTFLAG(SCF_ALT_FIRE);
	PRINTFLAG(SCF_NO_RESPONSE);
	PRINTFLAG(SCF_FFDEATH);
	PRINTFLAG(SCF_NO_COMBAT_TALK);
	PRINTFLAG(SCF_CHASE_ENEMIES);
	PRINTFLAG(SCF_LOOK_FOR_ENEMIES);
	PRINTFLAG(SCF_FACE_MOVE_DIR);
	PRINTFLAG(SCF_IGNORE_ALERTS);
	PRINTFLAG(SCF_DONT_FIRE);
	PRINTFLAG(SCF_DONT_FLEE);
	PRINTFLAG(SCF_FORCED_MARCH);
	PRINTFLAG(SCF_NO_GROUPS);
	PRINTFLAG(SCF_FIRE_WEAPON);
	PRINTFLAG(SCF_NO_MIND_TRICK);
	PRINTFLAG(SCF_USE_CP_NEAREST);
	PRINTFLAG(SCF_NO_FORCE);
	PRINTFLAG(SCF_NO_FALLTODEATH);
	PRINTFLAG(SCF_NO_ACROBATICS);
	PRINTFLAG(SCF_USE_SUBTITLES);
	PRINTFLAG(SCF_NO_ALERT_TALK);
}

// List all aiflags
void Workshop_List_AIFlags_f(gentity_t* ent) {
	if (inAIWorkshop == qfalse) {
		gi.Printf("You need to be in the AI workshop to use this command.\n");
		return;
	}
	PRINTFLAG(NPCAI_CHECK_WEAPON);
	PRINTFLAG(NPCAI_BURST_WEAPON);
	PRINTFLAG(NPCAI_MOVING);
	PRINTFLAG(NPCAI_TOUCHED_GOAL);
	PRINTFLAG(NPCAI_PUSHED);
	PRINTFLAG(NPCAI_NO_COLL_AVOID);
	PRINTFLAG(NPCAI_BLOCKED);
	PRINTFLAG(NPCAI_OFF_PATH);
	PRINTFLAG(NPCAI_IN_SQUADPOINT);
	PRINTFLAG(NPCAI_STRAIGHT_TO_DESTPOS);
	PRINTFLAG(NPCAI_NO_SLOWDOWN);
	PRINTFLAG(NPCAI_LOST);
	PRINTFLAG(NPCAI_SHIELDS);
	PRINTFLAG(NPCAI_GREET_ALLIES);
	PRINTFLAG(NPCAI_FORM_TELE_NAV);
	PRINTFLAG(NPCAI_ENROUTE_TO_HOMEWP);
	PRINTFLAG(NPCAI_MATCHPLAYERWEAPON);
	PRINTFLAG(NPCAI_DIE_ON_IMPACT);
}

// Set a raw timer
void Workshop_Set_Timer_f(gentity_t* ent) {
	if (inAIWorkshop == qfalse) {
		gi.Printf("You need to be in the AI workshop to use this command.\n");
		return;
	}
	if (selectedAI == ENTITYNUM_NONE) {
		gi.Printf("You need to select an NPC first with workshop_select\n");
		return;
	}
	if (gi.argc() != 3) {
		gi.Printf("usage: workshop_set_timer <timer name> <milliseconds>\n");
		return;
	}
	const char* timerName = gi.argv(1);
	int ms = atoi(gi.argv(2));
	TIMER_Set(&g_entities[selectedAI], timerName, ms);
}

// View all timers, including ones which are dead
void Workshop_View_Timers_f(gentity_t* ent) {
	if (inAIWorkshop == qfalse) {
		gi.Printf("You need to be in the AI workshop to use this command.\n");
		return;
	}
	if (selectedAI == ENTITYNUM_NONE) {
		gi.Printf("You need to select an NPC first with workshop_select\n");
		return;
	}
	auto timers = TIMER_List(&g_entities[selectedAI]);
	for (auto it = timers.begin(); it != timers.end(); ++it) {
		gi.Printf("%s\t\t:\t\t%i\n", it->first.c_str(), it->second);
	}
}

// Set Behavior State
void Workshop_Set_BehaviorState_f(gentity_t* ent) {
	if (inAIWorkshop == qfalse) {
		gi.Printf("You need to be in the AI workshop to use this command.\n");
		return;
	}
	if (selectedAI == ENTITYNUM_NONE) {
		gi.Printf("You need to select an NPC first with workshop_select\n");
		return;
	}
	if (gi.argc() != 2) {
		gi.Printf("usage: workshop_set_bstate <bstate>\n");
		return;
	}
	int bState = GetIDForString(BSTable, gi.argv(1));
	if (bState == -1) {
		gi.Printf("Invalid bstate\n");
		return;
	}
	g_entities[selectedAI].NPC->behaviorState = (bState_t)bState;
}

// Set goal entity
void Workshop_Set_GoalEntity_f(gentity_t* ent) {
	if (inAIWorkshop == qfalse) {
		gi.Printf("You need to be in the AI workshop to use this command.\n");
		return;
	}
	if (selectedAI == ENTITYNUM_NONE) {
		gi.Printf("You need to select an NPC first with workshop_select\n");
		return;
	}

	if (gi.argc() == 2) {
		// There's a second argument. Use that.
		if (!Q_stricmp(gi.argv(1), "me")) {
			g_entities[selectedAI].NPC->goalEntity = g_entities;
			return;
		}
		else {
			g_entities[selectedAI].NPC->goalEntity = &g_entities[atoi(gi.argv(1))];
		}
		if (g_entities[selectedAI].NPC->lastGoalEntity != nullptr) {
			gi.Printf("New goal entity: %i (last goal entity was %i)\n", g_entities[selectedAI].NPC->goalEntity->s.number, g_entities[selectedAI].NPC->lastGoalEntity->s.number);
		}
		else {
			gi.Printf("New goal entity: %i\n", g_entities[selectedAI].NPC->goalEntity->s.number);
		}
	}

	vec3_t		src, dest, vf;
	trace_t		trace;
	VectorCopy(ent->client->renderInfo.eyePoint, src);
	AngleVectors(ent->client->ps.viewangles, vf, NULL, NULL);//ent->client->renderInfo.eyeAngles was cg.refdef.viewangles, basically
	//extend to find end of use trace
	VectorMA(src, 32967.0f, vf, dest);

	//Trace ahead to find a valid target
	gi.trace(&trace, src, vec3_origin, vec3_origin, dest, ent->s.number, MASK_OPAQUE | CONTENTS_SOLID | CONTENTS_BODY | CONTENTS_ITEM | CONTENTS_CORPSE, G2_NOCOLLIDE, 0);
	if (trace.fraction == 1.0f || trace.entityNum < 1 || trace.entityNum == ENTITYNUM_NONE || trace.entityNum == ENTITYNUM_WORLD)
	{
		gi.Printf("Invalid entity\n");
		return;
	}

	gentity_t* target = &g_entities[trace.entityNum];
	gentity_t* selected = &g_entities[selectedAI];
	selected->NPC->lastGoalEntity = selected->NPC->goalEntity;
	selected->NPC->goalEntity = target;
	if (selected->NPC->lastGoalEntity != nullptr) {
		gi.Printf("New goal entity: %i (last goal entity was %i)\n", selected->NPC->goalEntity->s.number, selected->NPC->lastGoalEntity->s.number);
	}
	else {
		gi.Printf("New goal entity: %i\n", selected->NPC->goalEntity->s.number);
	}
}

// Set scriptflags
void Workshop_Set_Scriptflags_f(gentity_t* ent) {
	if (inAIWorkshop == qfalse) {
		gi.Printf("You need to be in the AI workshop to use this command.\n");
		return;
	}
	if (selectedAI == ENTITYNUM_NONE) {
		gi.Printf("You need to select an NPC first with workshop_select\n");
		return;
	}
	if (gi.argc() != 2) {
		gi.Printf("usage: workshop_set_scriptflags <script flags>\n");
		return;
	}
	int scriptFlags = atoi(gi.argv(1));
	if (scriptFlags < 0) {
		gi.Printf("Invalid scriptflags.\n");
		return;
	}
	g_entities[selectedAI].NPC->scriptFlags = scriptFlags;
}

// Set aiflags
void Workshop_Set_Aiflags_f(gentity_t* ent) {
	if (inAIWorkshop == qfalse) {
		gi.Printf("You need to be in the AI workshop to use this command.\n");
		return;
	}
	if (selectedAI == ENTITYNUM_NONE) {
		gi.Printf("You need to select an NPC first with workshop_select\n");
		return;
	}
	if (gi.argc() != 2) {
		gi.Printf("usage: workshop_set_aiflags <ai flags>\n");
		return;
	}
	int scriptFlags = atoi(gi.argv(1));
	if (scriptFlags < 0) {
		gi.Printf("Invalid scriptflags.\n");
		return;
	}
	g_entities[selectedAI].NPC->aiFlags = scriptFlags;
}

// Set weapon
extern stringID_table_t WPTable[];
extern void ChangeWeapon(gentity_t *ent, int newWeapon);
extern void G_CreateG2AttachedWeaponModel(gentity_t *ent, const char *psWeaponModel);
void Workshop_Set_Weapon_f(gentity_t* ent) {
	if (inAIWorkshop == qfalse) {
		gi.Printf("You need to be in the AI workshop to use this command.\n");
		return;
	}
	if (selectedAI == ENTITYNUM_NONE) {
		gi.Printf("You need to select an NPC first with workshop_select\n");
		return;
	}
	if (gi.argc() != 2) {
		gi.Printf("usage: workshop_set_weapon <weapon name or 'me'>\n");
		return;
	}
	int weaponNum;
	if (!Q_stricmp(gi.argv(1), "me")) {
		weaponNum = g_entities[0].s.weapon;
	}
	else {
		weaponNum = GetIDForString(WPTable, gi.argv(1));
		if (weaponNum == -1) {
			gi.Printf("Invalid weapon number. Maybe try WP_NONE, or WP_BLASTER for instance?\n");
		}
	}
	gentity_t* selected = &g_entities[selectedAI];
	selected->client->ps.weapon = weaponNum;
	selected->client->ps.weaponstate = WEAPON_RAISING;
	ChangeWeapon(selected, weaponNum);
	if (weaponNum == WP_SABER)
	{
		selected->client->ps.saberActive = qfalse;
		selected->client->ps.saberModel = "models/weapons2/saber/saber_w.glm";
		if (selected->client->NPC_class == CLASS_DESANN)
		{//longer saber
			selected->client->ps.saberLengthMax = 48;
		}
		else if (selected->client->NPC_class == CLASS_REBORN)
		{//shorter saber
			selected->client->ps.saberLengthMax = 32;
		}
		else
		{//standard saber length
			selected->client->ps.saberLengthMax = 40;
		}
		G_CreateG2AttachedWeaponModel(selected, selected->client->ps.saberModel);
	}
	else
	{
		G_CreateG2AttachedWeaponModel(selected, weaponData[weaponNum].weaponMdl);
	}
}

// Set team
void Workshop_Set_Team_f(gentity_t* ent) {
	if (inAIWorkshop == qfalse) {
		gi.Printf("You need to be in the AI workshop to use this command.\n");
		return;
	}
	if (selectedAI == ENTITYNUM_NONE) {
		gi.Printf("You need to select an NPC first with workshop_select\n");
		return;
	}
	if (gi.argc() != 2) {
		gi.Printf("usage: workshop_set_team <team name>\n");
		return;
	}
	int team = GetIDForString(teamTable, gi.argv(1));
	if (team == -1) {
		gi.Printf("invalid team\n");
	}
	g_entities[selectedAI].client->playerTeam = (team_t)team;
}

// Set enemyteam
void Workshop_Set_Enemyteam_f(gentity_t* ent) {
	if (inAIWorkshop == qfalse) {
		gi.Printf("You need to be in the AI workshop to use this command.\n");
		return;
	}
	if (selectedAI == ENTITYNUM_NONE) {
		gi.Printf("You need to select an NPC first with workshop_select\n");
		return;
	}
	if (gi.argc() != 2) {
		gi.Printf("usage: workshop_set_enemyteam <team name>\n");
		return;
	}
	int team = GetIDForString(teamTable, gi.argv(1));
	if (team == -1) {
		gi.Printf("invalid team\n");
	}
	g_entities[selectedAI].client->enemyTeam = (team_t)team;
}

// Set class
void Workshop_Set_Class_f(gentity_t* ent) {
	if (inAIWorkshop == qfalse) {
		gi.Printf("You need to be in the AI workshop to use this command.\n");
		return;
	}
	if (selectedAI == ENTITYNUM_NONE) {
		gi.Printf("You need to select an NPC first with workshop_select\n");
		return;
	}
	if (gi.argc() != 2) {
		gi.Printf("usage: workshop_set_class <class name>\n");
		return;
	}
	int team = GetIDForString(NPCClassTable, gi.argv(1));
	if (team == -1) {
		gi.Printf("invalid class\n");
	}
	g_entities[selectedAI].client->NPC_class = (class_t)team;
}

// Set rank
void Workshop_Set_Rank_f(gentity_t* ent) {
	if (inAIWorkshop == qfalse) {
		gi.Printf("You need to be in the AI workshop to use this command.\n");
		return;
	}
	if (selectedAI == ENTITYNUM_NONE) {
		gi.Printf("You need to select an NPC first with workshop_select\n");
		return;
	}
	if (gi.argc() != 2) {
		gi.Printf("usage: workshop_set_rank <rank name>\n");
		return;
	}
	int team = GetIDForString(RankTable, gi.argv(1));
	if (team == -1) {
		gi.Printf("invalid rank\n");
	}
	g_entities[selectedAI].NPC->rank = (rank_t)team;
}

// Set leader
void Workshop_Set_Leader_f(gentity_t* ent) {
	if (inAIWorkshop == qfalse) {
		gi.Printf("You need to be in the AI workshop to use this command.\n");
		return;
	}
	if (selectedAI == ENTITYNUM_NONE) {
		gi.Printf("You need to select an NPC first with workshop_select\n");
		return;
	}

	if (gi.argc() == 2) {
		// There's a second argument. Use that.
		if (!Q_stricmp(gi.argv(1), "me")) {
			g_entities[selectedAI].client->leader = g_entities;
			g_entities[selectedAI].client->team_leader = g_entities;
			return;
		}
		else {
			g_entities[selectedAI].client->leader = &g_entities[atoi(gi.argv(1))];
			g_entities[selectedAI].client->team_leader = &g_entities[atoi(gi.argv(1))];
		}
	}

	vec3_t		src, dest, vf;
	trace_t		trace;
	VectorCopy(ent->client->renderInfo.eyePoint, src);
	AngleVectors(ent->client->ps.viewangles, vf, NULL, NULL);//ent->client->renderInfo.eyeAngles was cg.refdef.viewangles, basically
	//extend to find end of use trace
	VectorMA(src, 32967.0f, vf, dest);

	//Trace ahead to find a valid target
	gi.trace(&trace, src, vec3_origin, vec3_origin, dest, ent->s.number, MASK_OPAQUE | CONTENTS_SOLID | CONTENTS_BODY | CONTENTS_ITEM | CONTENTS_CORPSE, G2_NOCOLLIDE, 0);
	if (trace.fraction == 1.0f || trace.entityNum < 1 || trace.entityNum == ENTITYNUM_NONE || trace.entityNum == ENTITYNUM_WORLD)
	{
		gi.Printf("Invalid entity\n");
		return;
	}

	gentity_t* target = &g_entities[trace.entityNum];
	gentity_t* selected = &g_entities[selectedAI];
	selected->client->leader = target;
}

// Set movetype
void Workshop_Set_Movetype_f(gentity_t* ent) {
	if (inAIWorkshop == qfalse) {
		gi.Printf("You need to be in the AI workshop to use this command.\n");
		return;
	}
	if (selectedAI == ENTITYNUM_NONE) {
		gi.Printf("You need to select an NPC first with workshop_select\n");
		return;
	}
	if (gi.argc() != 2) {
		gi.Printf("usage: workshop_set_movetype <move type>\n");
		return;
	}
	int team = GetIDForString(MoveTypeTable, gi.argv(1));
	if (team == -1) {
		gi.Printf("invalid movetype\n");
	}
	g_entities[selectedAI].NPC->stats.moveType = (movetype_t)team;
}

// Set forcepower
void Workshop_Set_Forcepower_f(gentity_t* ent) {
	if (inAIWorkshop == qfalse) {
		gi.Printf("You need to be in the AI workshop to use this command.\n");
		return;
	}
	if (selectedAI == ENTITYNUM_NONE) {
		gi.Printf("You need to select an NPC first with workshop_select\n");
		return;
	}
	if (gi.argc() != 3) {
		gi.Printf("usage: workshop_set_forcepower <force power> <rank>\n");
		return;
	}
	int power = GetIDForString(FPTable, gi.argv(1));
	if (power == -1) {
		gi.Printf("invalid force power\n");
	}
	int rank = atoi(gi.argv(2));
	if (rank < 0) {
		gi.Printf("invalid rank\n");
	}

	gentity_t* selected = &g_entities[selectedAI];
	if (rank == 0) {
		selected->client->ps.forcePowersKnown ^= (1 << power);
	}
	else {
		selected->client->ps.forcePowersKnown |= (1 << power);
	}
	selected->client->ps.forcePowerLevel[power] = rank;
}

// Godmode for the NPC
void Workshop_God_f(gentity_t* ent) {
	if (inAIWorkshop == qfalse) {
		gi.Printf("You need to be in the AI workshop to use this command.\n");
		return;
	}
	if (selectedAI == ENTITYNUM_NONE) {
		gi.Printf("You need to select an NPC first with workshop_select\n");
		return;
	}
	gentity_t* selected = &g_entities[selectedAI];
	if (selected->flags & FL_GODMODE) {
		gi.Printf("godmode OFF\n");
		selected->flags ^= FL_GODMODE;
	}
	else {
		gi.Printf("godmode ON\n");
		selected->flags |= FL_GODMODE;
	}
}

// Notarget mode for the NPC
void Workshop_Notarget_f(gentity_t* ent) {
	if (inAIWorkshop == qfalse) {
		gi.Printf("You need to be in the AI workshop to use this command.\n");
		return;
	}
	if (selectedAI == ENTITYNUM_NONE) {
		gi.Printf("You need to select an NPC first with workshop_select\n");
		return;
	}
	gentity_t* selected = &g_entities[selectedAI];
	if (selected->flags & FL_NOTARGET) {
		gi.Printf("notarget OFF\n");
		selected->flags ^= FL_NOTARGET;
	}
	else {
		gi.Printf("notarget ON\n");
		selected->flags |= FL_NOTARGET;
	}
}

void Workshop_Commands_f(gentity_t* ent) {
	gi.Printf("aiworkshop\n");
	gi.Printf("workshop_commands\n");
	gi.Printf("workshop_select\n");
	gi.Printf("workshop_deselect\n");
	gi.Printf("workshop_list_bstates\n");
	gi.Printf("workshop_list_scriptflags\n");
	gi.Printf("workshop_list_teams\n");
	gi.Printf("workshop_list_aiflags\n");
	gi.Printf("workshop_list_classes\n");
	gi.Printf("workshop_list_ranks\n");
	gi.Printf("workshop_list_movetypes\n");
	gi.Printf("workshop_list_forcepowers\n");
	gi.Printf("workshop_view_timers\n");
	gi.Printf("workshop_set_timer\n");
	gi.Printf("workshop_set_bstate\n");
	gi.Printf("workshop_set_goalent\n");
	gi.Printf("workshop_set_scriptflags\n");
	gi.Printf("workshop_set_weapon\n");
	gi.Printf("workshop_set_team\n");
	gi.Printf("workshop_set_enemyteam\n");
	gi.Printf("workshop_set_aiflags\n");
	gi.Printf("workshop_set_class\n");
	gi.Printf("workshop_set_rank\n");
	gi.Printf("workshop_set_leader\n");
	gi.Printf("workshop_set_movetype\n");
	gi.Printf("workshop_set_forcepower\n");
	gi.Printf("workshop_god\n");
	gi.Printf("workshop_notarget\n");
	
}