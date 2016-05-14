#include "g_local.h"
#include "cgame/cg_local.h"
#include "cgame/cg_media.h"

#define DRAWBOX_REFRESH	500	// How long it takes to redraw the boxes
#define BOX_VERTICES 8

qboolean inAIWorkshop = qfalse;

int drawBoxesTime = 0;
int selectedAI = ENTITYNUM_NONE;

extern stringID_table_t BSTable[];

static void WorkshopDrawEntityInformation(gentity_t* ent, int x, const char* title) {
	int add = 5;
	vec4_t textcolor = { 0.4f, 0.4f, 0.8f, 1.0f };

	cgi_R_Font_DrawString(x, 30, title, textcolor, cgs.media.qhFontSmall, -1, 0.4f);
	if (ent->NPC->goalEntity) {
		cgi_R_Font_DrawString(x, 30 + add, va("goalEnt = %i", ent->NPC->goalEntity->s.number), textcolor, cgs.media.qhFontSmall, -1, 0.4f);
		add += 5;
	}
	cgi_R_Font_DrawString(x, 30 + add, va("bs = %s", BSTable[ent->NPC->behaviorState].name), textcolor, cgs.media.qhFontSmall, -1, 0.4f);
	add += 5;
	if (ent->NPC->combatMove) {
		cgi_R_Font_DrawString(x, 30 + add, "-- in combat move --", textcolor, cgs.media.qhFontSmall, -1, 0.4f);
		add += 5;
	}
	if (ent->NPC->scriptFlags) {
		cgi_R_Font_DrawString(x, 30 + add, va("scriptFlags: %i", ent->NPC->scriptFlags), textcolor, cgs.media.qhFontSmall, -1, 0.4f);
		add += 5;
	}

	cgi_R_Font_DrawString(x, 30 + add, "-- currently active timers --", textcolor, cgs.media.qhFontSmall, -1, 0.4f);
	add += 5;
	auto timers = TIMER_List(ent);
	for (auto it = timers.begin(); it != timers.end(); ++it) {
		if (it->second >= 0) {
			cgi_R_Font_DrawString(x, 30 + add, va("%s : %i", it->first.c_str(), it->second), textcolor, cgs.media.qhFontSmall, -1, 0.4f);
			add += 5;
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

// List all scriptflags
void Workshop_List_Scriptflags_f(gentity_t* ent) {
	if (inAIWorkshop == qfalse) {
		gi.Printf("You need to be in the AI workshop to use this command.\n");
		return;
	}
	gi.Printf("SCF_CROUCHED = 1\n");
	gi.Printf("SCF_WALKING = 2\n");
	gi.Printf("SCF_MORELIGHT = 4\n");
	gi.Printf("SCF_LEAN_RIGHT = 8\n");
	gi.Printf("SCF_LEAN_LEFT = 16\n");
	gi.Printf("SCF_RUNNING = 32\n");
	gi.Printf("SCF_ALT_FIRE = 64\n");
	gi.Printf("SCF_NO_RESPONSE = 128\n");
	gi.Printf("SCF_FFDEATH = 256\n");
	gi.Printf("SCF_NO_COMBAT_TALK = 512\n");
	gi.Printf("SCF_CHASE_ENEMIES = 1024\n");
	gi.Printf("SCF_LOOK_FOR_ENEMIES = 2048\n");
	gi.Printf("SCF_FACE_MOVE_DIR = 4096\n");
	gi.Printf("SCF_IGNORE_ALERTS = 8192\n");
	gi.Printf("SCF_DONT_FIRE = 16384\n");
	gi.Printf("SCF_DONT_FLEE = 32768\n");
	gi.Printf("SCF_FORCED_MARCH = 65536\n");
	gi.Printf("SCF_NO_GROUPS = 131072\n");
	gi.Printf("SCF_FIRE_WEAPON = 262144\n");
	gi.Printf("SCF_NO_MIND_TRICK = 524288\n");
	gi.Printf("SCF_USE_CP_NEAREST = 1048576\n");
	gi.Printf("SCF_NO_FORCE = 2097152\n");
	gi.Printf("SCF_NO_FALLTODEATH = 4194304\n");
	gi.Printf("SCF_NO_ACROBATICS = 8388608\n");
	gi.Printf("SCF_USE_SUBTITLES = 16777216\n");
	gi.Printf("SCF_NO_ALERT_TALK = 33554432\n");
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

// Set weapon
extern stringID_table_t WPTable[];
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
		gi.Printf("usage: workshop_set_weapon <weapon name>\n");
		return;
	}
	int weaponNum = GetIDForString(WPTable, gi.argv(1));
	if (weaponNum == -1) {
		gi.Printf("Invalid weapon number. Maybe try WP_NONE, or WP_BLASTER for instance?\n");
	}
	g_entities[selectedAI].s.weapon = weaponNum;
}

void Workshop_Commands_f(gentity_t* ent) {
	gi.Printf("aiworkshop\n");
	gi.Printf("workshop_commands\n");
	gi.Printf("workshop_select\n");
	gi.Printf("workshop_list_bstates\n");
	gi.Printf("workshop_list_scriptflags\n");
	gi.Printf("workshop_set_timer\n");
	gi.Printf("workshop_view_timers\n");
	gi.Printf("workshop_set_bstate\n");
	gi.Printf("workshop_set_goalent\n");
	gi.Printf("workshop_set_scriptflags\n");
	gi.Printf("workshop_set_weapon\n");
}