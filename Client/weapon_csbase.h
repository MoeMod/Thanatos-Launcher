#ifndef WEAPON_CSBASE_H
#define WEAPON_CSBASE_H

#ifdef _WIN32
#pragma once
#endif

#include "weapons.h"

extern int ClassnameToWeaponID(const char *classname);
extern int AliasToWeaponID(const char *alias);
extern const char *WeaponIDToAlias(int id);
extern const char *GetTranslatedWeaponAlias(const char *alias);
extern bool IsPrimaryWeapon(int id);
extern bool IsSecondaryWeapon(int id);
extern int GetShellForAmmoType(const char *ammoname);

#define SHIELD_VIEW_MODEL "models/weapons/v_shield.mdl"
#define SHIELD_WORLD_MODEL "models/weapons/w_shield.mdl"

#define BULLET_PLAYER_50AE "BULLET_PLAYER_50AE"
#define BULLET_PLAYER_762MM "BULLET_PLAYER_762MM"
#define BULLET_PLAYER_556MM "BULLET_PLAYER_556MM"
#define BULLET_PLAYER_556MM_BOX "BULLET_PLAYER_556MM_BOX"
#define BULLET_PLAYER_338MAG "BULLET_PLAYER_338MAG"
#define BULLET_PLAYER_9MM "BULLET_PLAYER_9MM"
#define BULLET_PLAYER_BUCKSHOT "BULLET_PLAYER_BUCKSHOT"
#define BULLET_PLAYER_45ACP "BULLET_PLAYER_45ACP"
#define BULLET_PLAYER_357SIG "BULLET_PLAYER_357SIG"
#define BULLET_PLAYER_57MM "BULLET_PLAYER_57MM"
#define AMMO_TYPE_HEGRENADE "AMMO_TYPE_HEGRENADE"
#define AMMO_TYPE_FLASHBANG "AMMO_TYPE_FLASHBANG"
#define AMMO_TYPE_SMOKEGRENADE "AMMO_TYPE_SMOKEGRENADE"

#endif