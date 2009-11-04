/**
 * @file g_local.h
 * @brief Local definitions for game module
 */

/*
All original material Copyright (C) 2002-2009 UFO: Alien Invasion.

Original file from Quake 2 v3.21: quake2-2.31/game/g_local.h
Copyright (C) 1997-2001 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#ifndef GAME_G_LOCAL_H
#define GAME_G_LOCAL_H

#include "q_shared.h"
#include "inv_shared.h"
#include "../shared/infostring.h"

/** no gettext support for game lib - but we must be able to mark the strings */
# define _(String) String
# define ngettext(x, y, cnt) x

/** @note define GAME_INCLUDE so that game.h does not define the
 * short, server-visible player_t and edict_t structures,
 * because we define the full size ones in this file */
#define	GAME_INCLUDE
#include "game.h"

/** the "gameversion" client command will print this plus compile date */
#define	GAMEVERSION	"baseufo"

#define MAX_SPOT_DIST	4096 /* 768 */

/**
 * Bitmask for all players
 */
#define PM_ALL			0xFFFFFFFF
/**
 * Bitmask for all teams
 */
#define TEAM_ALL		0xFFFFFFFF

#define G_PlayerToPM(player) ((player)->num < game.sv_maxplayersperteam ? 1 << ((player)->num) : 0)

/** server is running at 10 fps */
#define	SERVER_FRAME_SECONDS		0.1

/** memory tags to allow dynamic memory to be cleaned up */
#define	TAG_GAME	765			/* clear when unloading the game library */
#define	TAG_LEVEL	766			/* clear when loading a new level */

/** Macros for faster access to the inventory-container. */
#define CONTAINER(e, containerID) ((e)->i.c[(containerID)])
#define RIGHT(e) (e->i.c[gi.csi->idRight])
#define LEFT(e)  (e->i.c[gi.csi->idLeft])
#define EXTENSION(e)  (e->i.c[gi.csi->idExtension])
#define HEADGEAR(e)  (e->i.c[gi.csi->idHeadgear])
#define FLOOR(e) (e->i.c[gi.csi->idFloor])

#define INVDEF(containerID) (&gi.csi->ids[(containerID)])

/** Actor visibility constants */
#define ACTOR_VIS_100	1.0
#define ACTOR_VIS_50	0.5
#define ACTOR_VIS_10	0.1
#define ACTOR_VIS_0		0.0

#define G_TagMalloc(size, tag) gi.TagMalloc((size), (tag), __FILE__, __LINE__)
#define G_MemFree(ptr) gi.TagFree((ptr), __FILE__, __LINE__)

#define G_PLAYER_FROM_ENT(ent) (game.players + (ent)->pnum)

/** @brief this structure is left intact through an entire game
 * it should be initialized at game library load time */
typedef struct {
	player_t *players;			/* [maxplayers] */

	/* store latched cvars here that we want to get at often */
	int sv_maxplayersperteam;
	int sv_maxentities;
} game_locals_t;

/** @brief this structure is cleared as each map is entered */
typedef struct {
	int framenum;		/**< the current frame (10fps) */
	float time;			/**< seconds the game is running already
						 * calculated through framenum * SERVER_FRAME_SECONDS */

	char mapname[MAX_QPATH];	/**< the server name (base1, etc) */
	char nextmap[MAX_QPATH];	/**< @todo Spawn the new map after the current one was ended */
	qboolean routed;
	int maxteams; /**< the max team amount for multiplayer games for the current map */
	qboolean day;

	/* intermission state */
	float intermissionTime;		/**< the seconds to wait until the game will be closed.
								 * This value is relative to @c level.time
								 * @sa G_MatchDoEnd */
	int winningTeam;			/**< the team that won this match */
	float roundstartTime;		/**< the time the team started the round */

	/* round statistics */
	int numplayers;
	int activeTeam;
	int nextEndRound;

	int actualRound;

	int randomSpawn;

	byte num_alive[MAX_TEAMS];
	byte num_spawned[MAX_TEAMS];
	byte num_spawnpoints[MAX_TEAMS];
	byte num_2x2spawnpoints[MAX_TEAMS];
	byte num_kills[MAX_TEAMS][MAX_TEAMS];
	byte num_stuns[MAX_TEAMS][MAX_TEAMS];
} level_locals_t;


/**
 * @brief this is only used to hold entity field values that can be set from
 * the editor, but aren't actually present in edict_t during gameplay
 */
typedef struct {
	/* world vars */
	char *nextmap;	/**< the next map that is started after the current one has finished */
	int randomSpawn;	/**< spawn the actors on random spawnpoints */
} spawn_temp_t;

/** @brief used in shot probability calculations (pseudo shots) */
typedef struct {
	int enemyCount;			/**< shot would hit that much enemies */
	int friendCount;		/**< shot would hit that much friends */
	int civilian;			/**< shot would hit that much civilians */
	int self;				/**< @todo incorrect actor facing or shotOrg, or bug in trace code? */
	int damage;
	qboolean allow_self;
} shot_mock_t;

extern game_locals_t game;
extern level_locals_t level;
extern game_import_t gi;
extern game_export_t globals;

extern edict_t *g_edicts;

#define random()	((rand() & 0x7fff) / ((float)0x7fff))
#define crandom()	(2.0 * (random() - 0.5))

#define G_IsStunned(ent)	(((ent)->state & STATE_STUN) & ~STATE_DEAD)
/** @note This check also includes the IsStunned check - see the STATE_* bitmasks */
#define G_IsDead(ent)		(((ent)->state & STATE_DEAD))
#define G_IsActor(ent)		((ent)->type == ET_ACTOR || (ent)->type == ET_ACTOR2x2)
/** @note Every none solid (none-bmodel) edict that is visible for the client */
#define G_IsVisibleOnBattlefield(ent)	(G_IsActor((ent)) || (ent)->type == ET_ITEM || (ent)->type == ET_PARTICLE)
#define G_IsAI(ent)				(G_PLAYER_FROM_ENT((ent))->pers.ai)
#define G_IsAIPlayer(player)	((player)->pers.ai)
#define G_TeamToVisMask(team)	(1 << (team))
#define G_IsVisibleForTeam(ent, team) ((ent)->visflags & G_TeamToVisMask(team))

extern cvar_t *sv_maxentities;
extern cvar_t *password;
extern cvar_t *sv_needpass;
extern cvar_t *sv_dedicated;

extern cvar_t *logstats;
extern FILE *logstatsfile;

extern cvar_t *sv_filterban;

extern cvar_t *sv_maxvelocity;

extern cvar_t *sv_cheats;
extern cvar_t *sv_maxclients;
extern cvar_t *sv_reaction_leftover;
extern cvar_t *sv_shot_origin;
extern cvar_t *sv_send_edicts;
extern cvar_t *sv_maxplayersperteam;
extern cvar_t *sv_maxsoldiersperteam;
extern cvar_t *sv_maxsoldiersperplayer;
extern cvar_t *sv_enablemorale;
extern cvar_t *sv_roundtimelimit;

extern cvar_t *sv_maxteams;

extern cvar_t *sv_ai;
extern cvar_t *sv_teamplay;

extern cvar_t *ai_alien;
extern cvar_t *ai_civilian;
extern cvar_t *ai_equipment;
extern cvar_t *ai_numaliens;
extern cvar_t *ai_numcivilians;
extern cvar_t *ai_numactors;

extern cvar_t *mob_death;
extern cvar_t *mob_wound;
extern cvar_t *mof_watching;
extern cvar_t *mof_teamkill;
extern cvar_t *mof_civilian;
extern cvar_t *mof_enemy;
extern cvar_t *mor_pain;

/*everyone gets this times morale damage */
extern cvar_t *mor_default;

/* at this distance the following two get halfed (exponential scale) */
extern cvar_t *mor_distance;

/* at this distance the following two get halfed (exponential scale) */
extern cvar_t *mor_victim;

/* at this distance the following two get halfed (exponential scale) */
extern cvar_t *mor_attacker;

/* how much the morale depends on the size of the damaged team */
extern cvar_t *mon_teamfactor;

extern cvar_t *mor_regeneration;
extern cvar_t *mor_shaken;
extern cvar_t *mor_panic;
extern cvar_t *mor_brave;

extern cvar_t *m_sanity;
extern cvar_t *m_rage;
extern cvar_t *m_rage_stop;
extern cvar_t *m_panic_stop;

extern cvar_t *g_ailua;
extern cvar_t *g_aidebug;
extern cvar_t *g_nodamage;
extern cvar_t *g_notu;
extern cvar_t *g_actorspeed;

extern cvar_t *flood_msgs;
extern cvar_t *flood_persecond;
extern cvar_t *flood_waitdelay;

extern cvar_t *difficulty;

/* fields are needed for spawning from the entity string */
#define FFL_SPAWNTEMP		1
#define FFL_NOSPAWN			2

/* g_stats.c */
void G_SendPlayerStats(const player_t *player);

/* g_inventory.c */
void G_WriteItem(item_t item, const invDef_t *container, int x, int y);
void G_ReadItem(item_t *item, invDef_t **container, int *x, int *y);
void G_InventoryToFloor(edict_t *ent);

/* g_morale */
void G_MoraleBehaviour(int team, qboolean quiet);

/* g_phys.c */
void G_PhysicsRun(void);
void G_PhysicsStep(edict_t *ent);

/* g_utils.c */
edict_t *G_Find(edict_t *from, int fieldofs, char *match);
edict_t *G_FindRadius(edict_t *from, vec3_t org, float rad, entity_type_t type);
edict_t *G_FindTargetEntity(const char *target);
const char* G_GetPlayerName(int pnum);
player_t* G_GetPlayerForTeam(int team);
int G_GetActiveTeam(void);
const char* G_GetWeaponNameForFiredef(const fireDef_t *fd);
void G_PrintActorStats(const edict_t *victim, const edict_t *attacker, const fireDef_t *fd);
void G_PrintStats(const char *buffer);
int G_TouchTriggers(edict_t *ent);
edict_t *G_Spawn(void);
edict_t *G_ParticleSpawn(vec3_t origin, int spawnflags, const char *particle);
void G_FreeEdict(edict_t *e);
qboolean G_UseEdict(edict_t *ent);

/* g_reaction.c */
qboolean G_ResolveReactionFire(edict_t *target, qboolean force, qboolean endTurn, qboolean doShoot);
void G_ReactToPreFire(edict_t *target);
void G_ReactToPostFire(edict_t *target);

void G_CompleteRecalcRouting(void);
void G_RecalcRouting(const edict_t * ent);
void G_GenerateEntList(const char *entList[MAX_EDICTS]);

/* g_client.c */
/** the visibile changed - if it was visible - it's (the edict) now invisible */
#define VIS_CHANGE	1
/** actor visible? */
#define VIS_YES		2
/** stop the current action if actor appears */
#define VIS_STOP	4

/** check whether edict is still visible - it maybe is currently visible but this
 * might have changed due to some action */
#define VT_PERISH		1
/** don't perform a frustum vis check via G_FrustumVis in G_Vis */
#define VT_NOFRUSTUM	2

#define MORALE_RANDOM( mod )	( (mod) * (1.0 + 0.3*crand()) )

#define MAX_DVTAB 32

void G_FlushSteps(void);
qboolean G_ClientUseEdict(player_t *player, edict_t *actor, edict_t *door);
qboolean G_ActionCheck(player_t *player, edict_t *ent, int TU, qboolean quiet);
void G_SendStats(edict_t *ent) __attribute__((nonnull));
edict_t *G_SpawnFloor(pos3_t pos);
int G_CheckVisTeam(int team, edict_t *check, qboolean perish, edict_t *ent);
edict_t *G_GetFloorItems(edict_t *ent) __attribute__((nonnull));
void G_SendState(unsigned int playerMask, const edict_t *ent);

qboolean G_IsLivingActor(const edict_t *ent) __attribute__((nonnull));
void G_CheckForceEndRound(void);
void G_ActorDie(edict_t *ent, int state, edict_t *attacker);
int G_ClientAction(player_t * player);
void G_ClientEndRound(player_t * player, qboolean quiet);
void G_ClientTeamInfo(player_t * player);
int G_ClientGetTeamNum(const player_t * player);
int G_ClientGetTeamNumPref(const player_t * player);
void G_PlayerPrintf(const player_t * player, int printlevel, const char *fmt, ...) __attribute__((format(printf, 3, 4)));
void G_ResetClientData(void);

void G_ClientCommand(player_t * player);
void G_ClientUserinfoChanged(player_t * player, char *userinfo);
void G_ClientBegin(player_t * player);
qboolean G_ClientSpawn(player_t * player);
qboolean G_ClientConnect(player_t * player, char *userinfo);
void G_ClientDisconnect(player_t * player);

void G_ActorReload(int entnum, shoot_types_t st, qboolean quiet);
qboolean G_ClientCanReload(player_t *player, int entnum, shoot_types_t st);
void G_ClientGetWeaponFromInventory(player_t *player, int entnum, qboolean quiet);
void G_ClientMove(player_t * player, int visTeam, const int num, pos3_t to, qboolean stop, qboolean quiet);
void G_MoveCalc(int team, pos3_t from, int actorSize, byte crouchingState, int distance);
void G_ActorInvMove(int num, const invDef_t * from, invList_t *fItem, const invDef_t * to, int tx, int ty, qboolean checkaction, qboolean quiet);
void G_ClientStateChange(player_t * player, int num, int reqState, qboolean checkaction);
int G_ActorDoTurn(edict_t * ent, byte dir);

void G_SendInvisible(player_t *player);
void G_GiveTimeUnits(int team);

void G_AppearPerishEvent(unsigned int player_mask, qboolean appear, edict_t * check, edict_t *ent);
unsigned int G_VisToPM(unsigned int vis_mask);
void G_SendInventory(unsigned int player_mask, edict_t * ent);
unsigned int G_TeamToPM(int team);

void G_SpawnEntities(const char *mapname, qboolean day, const char *entities);
qboolean G_RunFrame(void);

#ifdef DEBUG
void G_InvList_f(const player_t *player);
#endif

/* g_vis.c */
qboolean G_FrustumVis(const edict_t *from, const vec3_t point);
float G_ActorVis(const vec3_t from, const edict_t *check, qboolean full);
void G_ClearVisFlags(int team);
int G_CheckVis(edict_t *check, qboolean perish);
int G_CheckVisPlayer(player_t* player, qboolean perish);
int G_TestVis(int team, edict_t * check, int flags);
float G_Vis(int team, const edict_t * from, const edict_t * check, int flags);

/* g_combat.c */
qboolean G_ClientShoot(player_t *player, const int entnum, pos3_t at, int type, int firemode, shot_mock_t *mock, qboolean allowReaction, int z_align);
void G_ResetReactionFire(int team);
qboolean G_ReactToMove(edict_t *target, qboolean mock);
void G_ReactToEndTurn(void);

/* g_ai.c */
extern edict_t *ai_waypointList;
void G_AddToWayPointList(edict_t *ent);
void AI_Run(void);
void AI_ActorThink(player_t *player, edict_t *ent);
player_t *AI_CreatePlayer(int team);
qboolean AI_CheckUsingDoor(const edict_t *ent, const edict_t *door);

/* g_svcmds.c */
void ServerCommand(void);
qboolean SV_FilterPacket(const char *from);

/* g_match.c */
qboolean G_MatchIsRunning(void);
void G_MatchEndTrigger(int team, int timeGap);
void G_MatchEndCheck(void);
qboolean G_MatchDoEnd(void);

/* g_trigger.c */
edict_t* G_TriggerSpawn(edict_t *owner);
void SP_trigger_hurt(edict_t *ent);
void SP_trigger_touch(edict_t *ent);

/* g_func.c */
void SP_func_rotating(edict_t *ent);
void SP_func_door(edict_t *ent);
void SP_func_breakable(edict_t *ent);

/*============================================================================ */

/** @brief e.g. used for breakable objects */
typedef enum {
	MAT_GLASS,		/* default */
	MAT_METAL,
	MAT_ELECTRICAL,
	MAT_WOOD,

	MAT_MAX
} edictMaterial_t;

typedef struct {
	/* actor movement */
	int			contentFlags[MAX_DVTAB];
	int			visflags[MAX_DVTAB];
	byte		steps;
	int			currentStep;

	int			state;
} moveinfo_t;

/** @brief client data that stays across multiple level loads */
typedef struct {
	char userinfo[MAX_INFO_STRING];
	char netname[16];

	/** the number of the team for this player
	 * 0 is reserved for civilians and critters */
	int team;
	qboolean ai;				/**< client controlled by ai */

	/** ai specific data */
	edict_t *last; /**< set to the last actor edict that was handled for the ai in their think function */

	float		flood_locktill;		/**< locked from talking */
	float		flood_when[10];		/**< when messages were said */
	int			flood_whenhead;		/**< head pointer for when said */
} client_persistant_t;

/** @brief this structure is cleared on each PutClientInServer(),
 * except for 'client->pers'
 * @note shared between game and server - but server doesn't know all the fields */
struct player_s {
	/* known to server */
	qboolean inuse;
	int num;
	int ping;

	/* private to game */
	qboolean spawned;			/**< already spawned? */
	qboolean began;				/**< the player sent his 'begin' already */
	qboolean ready;				/**< ready to end his round */

	qboolean autostand;			/**< autostand for long walks */

	client_persistant_t pers;
};

/**
 * @brief not the first on the team
 * @sa groupMaster and groupChain
 */
#define FL_GROUPSLAVE	0x00000008
/**
 * @brief If an edict is destroyable (like ET_BREAKABLE, ET_DOOR [if health set]
 * or maybe a ET_MISSION [if health set])
 * @note e.g. misc_mission, func_breakable, func_door
 */
#define FL_DESTROYABLE	0x00000004
/**
 * @brief Trigger the edict at spawn.
 */
#define FL_TRIGGERED	0x00000100

/**
 * @brief Everything that is not in the bsp tree is an edict, the spawnpoints,
 * the actors, the misc_models, the weapons and so on.
 */
struct edict_s {
	qboolean inuse;
	int linkcount;

	int number;			/**< the number in the global edict array */

	vec3_t origin;		/**< the position in the world */
	vec3_t angles;		/**< the rotation in the world */

	/** @todo move these fields to a server private sv_entity_t */
	link_t area;				/**< linked to a division node or leaf */

	/* tracing info SOLID_BSP, SOLID_BBOX, ... */
	solid_t solid;

	vec3_t mins, maxs; /**< position of min and max points - relative to origin */
	vec3_t absmin, absmax; /**< position of min and max points - relative to world's origin */
	vec3_t size;

	edict_t *child;	/**< e.g. the trigger for this edict */
	edict_t *owner;	/**< e.g. the door model in case of func_door */
	/*
	 * type of objects the entity will not pass through
	 * can be any of MASK_* or CONTENTS_*
	 */
	int clipmask;
	int modelindex;	 /**< inline model index */

	/*================================ */
	/* don't change anything above here - the server expects the fields in that order */
	/*================================ */

	int mapNum;		/**< entity number in the map file */
	const char *model;	/**< inline model (start with * and is followed by the model numberer)
						 * or misc_model model name (e.g. md2) */

	/** only used locally in game, not by server */

	int type;
	int visflags;

	int contentFlags;			/**< contents flags of the brush the actor is walking in */

	pos3_t pos;
	byte dir;					/**< direction the player looks at */

	int TU;						/**< remaining timeunits for actors or timeunits needed to 'use' this entity */
	int HP;						/**< remaining healthpoints */
	int STUN;					/**< The stun damage received in a mission.
							 * @sa g_combat.c:G_Damage
							 * @todo How is this handled after mission-end?
							 * @todo How is this checked to determine the stun-state? (I've found HP<=STUN in g_combat.c:G_Damage)
							 */
	int morale;					/**< the current morale value */

	int state;					/**< the player state - dead, shaken.... */

	int reaction_minhit;		/**< acceptable odds for reaction shots */

	int team;					/**< player of which team? */
	int pnum;					/**< the actual player slot */
	/** the models (hud) */
	int body;
	int head;
	int frame;					/**< frame of the model to show */

	char *group;				/**< this can be used to trigger a group of entities
								 * e.g. for two-part-doors - set the group to the same
								 * string for each door part and they will open both
								 * if you open one */

	/** delayed reaction fire */
	edict_t *reactionTarget;
	float reactionTUs;
	qboolean reactionNoDraw;

	/** client actions - interact with the world */
	edict_t *clientAction;

	/** only set this for the attacked edict - to know who's shooting */
	edict_t *reactionAttacker;

	int	reactionFired;	/**< A simple counter that tells us how many times an actor has fired as a reaction (in the current turn). */

	/** here are the character values */
	character_t chr;

	/** this is the inventory */
	inventory_t i;

	int spawnflags;	/**< set via mapeditor */
	const char *classname;

	float angle;	/**< entity yaw - set via mapeditor - -1=up; -2=down */

	int speed;	/**< speed of entities - e.g. rotating or actors */
	const char *target;	/**< name of the entity to trigger or move towards */
	const char *targetname;	/**< name pointed to by target */
	const char *item;	/**< the item id that must be placed to e.g. the func_mission to activate the use function */
	const char *particle;
	const char *noise;	/**< sounds - e.g. for func_door */
	edictMaterial_t material;	/**< material value (e.g. for func_breakable) */
	int count;		/**< general purpose 'amount' variable - set via mapeditor often */
	int time;		/**< general purpose 'rounds' variable - set via mapeditor often */
	int sounds;		/**< type of sounds to play - e.g. doors */
	int dmg;		/**< damage done by entity */
	/** @sa memcpy in Grid_CheckForbidden */
	int fieldSize;	/* ACTOR_SIZE_* */
	qboolean hiding;		/**< for ai actors - when they try to hide after the performed their action */

	/** function to call when triggered - this function should only return true when there is
	 * a client action associated with it */
	qboolean (*touch)(edict_t * self, edict_t * activator);
	float nextthink;
	void (*think)(edict_t *self);
	/** general use function that is called when the triggered client action is executed
	 * or when the server has to 'use' the entity */
	qboolean (*use)(edict_t *self);
	qboolean (*destroy)(edict_t *self);

	/** e.g. doors */
	moveinfo_t		moveinfo;

	/** flags will have FL_GROUPSLAVE set when the edict is part of a chain,
	 * but not the master - you can use the groupChain pointer to get all the
	 * edicts in the particular chain - and start out for the on that doesn't
	 * have the above mentioned flag set.
	 * @sa G_FindEdictGroups */
	edict_t *groupChain;
	edict_t *groupMaster;	/**< first entry in the list */
	int flags;		/**< FL_* */

	pos3_t *forbiddenListPos; /**< this is used for e.g. misc_models with the solid flag set - this will
								* hold a list of grid positions that are blocked by the aabb of the model */
	int forbiddenListSize;		/**< amount of entries in the forbiddenListPos */
};

#endif /* GAME_G_LOCAL_H */
