/**
 * @file cl_game_skirmish.h
 * @brief Skirmish game type headers
 */

/*
Copyright (C) 2002-2010 UFO: Alien Invasion.

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

#ifndef CL_GAME_SKIRMISH_H
#define CL_GAME_SKIRMISH_H

const mapDef_t* GAME_SK_MapInfo(int step);
void GAME_SK_InitStartup(const cgame_import_t *import);
void GAME_SK_Shutdown(void);
void GAME_SK_Results(struct dbuffer *msg, int winner, int *numSpawned, int *numAlive, int numKilled[][MAX_TEAMS], int numStunned[][MAX_TEAMS]);

#endif
