/**
 * =============================================================================
 * CS2Fixes
 * Copyright (C) 2023 Source2ZE
 * =============================================================================
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License, version 3.0, as published by the
 * Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once
#include "platform.h"
#include "stdint.h"
#include "utils/module.h"
#include "utlstring.h"
#include "gameconfig.h"

namespace modules
{
	inline CModule *engine;
	inline CModule *tier0;
	inline CModule *server;
	inline CModule *schemasystem;
	inline CModule *vscript;
	inline CModule *client;
#ifdef _WIN32
	inline CModule *hammer;
#endif
}

class CEntityInstance;
class CBasePlayerController;
class Z_CBaseEntity;

namespace addresses
{
	void Initialize(CGameConfig *g_GameConfig);

	inline void(FASTCALL *NetworkStateChanged)(int64 chainEntity, int64 offset, int64 a3);
	inline void(FASTCALL *StateChanged)(void *networkTransmitComponent, CEntityInstance *ent, int64 offset, int16 a4, int16 a5);
	inline void(FASTCALL *UTIL_ClientPrintAll)(int msg_dest, const char *msg_name, const char *param1, const char *param2, const char *param3, const char *param4);
	inline void(FASTCALL *ClientPrint)(CBasePlayerController *player, int msg_dest, const char *msg_name, const char *param1, const char *param2, const char *param3, const char *param4);
	inline void(FASTCALL* SetGroundEntity)(Z_CBaseEntity* ent, Z_CBaseEntity* ground);
}

namespace offsets
{
#ifdef _WIN32
	inline constexpr int GameEntitySystem = 0x58;
	inline constexpr int Teleport = 148;
	// PhysDisableEntityCollisions called on entities in two different scene
	inline constexpr int CollisionRulesChanged = 173;
#else
	inline constexpr int GameEntitySystem = 0x50;
	inline constexpr int Teleport = 147;
	// PhysDisableEntityCollisions called on entities in two different scene
	inline constexpr int CollisionRulesChanged = 172;
#endif
}
