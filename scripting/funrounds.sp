/*  妹萌え! FunRounds!
 *
 *  Copyright (C) 2020 Mai Ooizumi
 * 
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option) 
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT 
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS 
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with 
 * this program. If not, see http://www.gnu.org/licenses/.
 * 
 * Email : mai@imomoe.moe
 * Website: https://maiooizumi.xyz/
 * Github: https://github.com/Mai-Ooizumi
 */

#pragma newdecls required
#pragma semicolon 1

#include <sourcemod>
#include <sdkhooks>
#include <sdktools>
#include <cstrike>
#include <csgoitems> // CSGOItems_
#include "funrounds/funrounds"
#include "funrounds/frsounds" // sounds
#include "funrounds/frcvars" // cvars & commands
#include "funrounds/frrounds" // round create & round kv
#include "funrounds/frchicken" // chicken mode & chicken spawner
#include "funrounds/frplayer" //respawn time/weapon control & player color
#include "funrounds/frghost" // ghost mode
#include "funrounds/frchrono" // teleport mode
#include "funrounds/frsuicide" // suicidebomber
#include "funrounds/frarms" // armrace , explosive grenade & bullet , zeus bullet
#include "funrounds/frshield" // EnergyShield
#include "funrounds/frboss" // Boss mode & chickeneater & knockback
#include "funrounds/frtk" //throwknife
#include "funrounds/frcheat" // aimbot autofire

#define PLUGIN_VERSION "2.0.0.0"
#define PLUGIN_AUTHOR "Mai Ooizumi"
#define DONATE_URL "https://afdian.net/@maiooizumi"

public Plugin myinfo = 
{
	name = "Fun Rounds!",
	author = PLUGIN_AUTHOR,
	description = "Fun rounds reborn",
	version = PLUGIN_VERSION,
	url = "https://maiooizumi.xyz"
};

int iDodgeBallCount[MAXENTITIES + 1] = 0;


public void OnPluginStart()
{
	BuildFRCvars();
	CreateConVar("fr_version", PLUGIN_VERSION, "FunRounds! Version", FCVAR_NOTIFY);
	HookEvent("bomb_beginplant", Fun_EventBombBeginPlant, EventHookMode_Pre);
	HookEvent("bomb_begindefuse", Fun_EventBombBeginDefuse, EventHookMode_Pre);
	HookEvent("bomb_pickup", Fun_EventBombPickUp);
	HookEvent("bomb_dropped", Fun_EventBombDrop);
	HookEvent("bomb_planted", Fun_EventBombPlanted);
	HookEvent("player_spawn", Fun_EventPlayerSpawn);
	HookEvent("inspect_weapon", Fun_EventInspectWeapon);
	HookEvent("round_start", Fun_EventRoundStart);
	HookEvent("round_freeze_end", Fun_EventRoundFreezeEnd, EventHookMode_Pre);
	HookEvent("round_end", Fun_EventRoundEnd);
	HookEvent("player_death", Fun_EventPlayerDeath, EventHookMode_Pre);
	HookEvent("player_hurt", Fun_EventPlayerHurt, EventHookMode_Pre);
	HookEvent("weapon_fire", Fun_EventWeaponFire, EventHookMode_Pre);
	HookEvent("item_pickup", Fun_EventItemPickup);
	HookEvent("bullet_impact", Fun_EventBulletImpact);
	HookEvent("player_blind", Fun_EventPlayerBlind);
	AddTempEntHook("Shotgun Shot", Fun_BulletShot);
	
	AddNormalSoundHook(SoundHook);
	AddCommandListener(Drop, "drop");
	FrGhostOnMapStart();
	FrChronoOnMapStart();
	FrSuicideBomberStart();
	FrBossStart();
	frThrowKnifeStart();
	LoadSounds();
	DeathExplodeInit();
	for (int i = 1; i <= MaxClients; ++i)
	{
		if(IsValidClient(i))
		{
			InitHooks(i);
		}
	}
	ResetRounds();
	TurnOffAllSettings();
}

// doesn't need ?
public void OnPluginEnd()
{
	TurnSettingsOff();
	TurnOffAllSettings();
}

public void OnClientPutInServer(int client)
{
	CreateTimer(15.0, Timer_PrintInfo, GetClientUserId(client));
	InitHooks(client);
}

void InitHooks(int client)
{
	SDKHook(client, SDKHook_TraceAttack, OnTraceAttack);
	SDKHook(client, SDKHook_OnTakeDamage, OnTakeDamage);
	SDKHook(client, SDKHook_WeaponDrop, OnWeaponDrop);
	SDKHook(client, SDKHook_PostThink, OnPostThink);
	SDKHook(client, SDKHook_Touch, OnTouch);
	SDKHook(client, SDKHook_PreThink, OnPreThink);
	SDKHook(client, SDKHook_SetTransmit, Hook_SetTransmit);
	SDKHook(client, SDKHook_WeaponSwitch, OnWeaponSwitch);
	SDKHook(client, SDKHook_WeaponCanUse, OnWeaponCanUse);
	SDKHook(client, SDKHook_WeaponEquip, OnWeaponEquip);
	InitPlayerBools(client);
	AddNormalSoundHook(SoundHook);
	
}

void InitPlayerBools(int client)
{
	RemovePlayerGhost(client);
	RemoveSuicide(client);
	RemoveBoss(client);
	SetCheats(client, bAimbot);
	frAimbotEnable[client] = true;
	frTriggeredSuicide[client] = false;
	frIsChronoClient[client] = false;
	frIsChronoFreeze[client] = false;
	frIsChicken[client] = false;
	KillTimerHandle(frAnimationsTimer[client]);
	frLastFlags[client] = 0;
	frFlyCounter[client] = 0;
	frCWasRunning[client] = false;
	frCWasIdle[client] = false;
	frCWasWalking[client] = false;
	frArmorValue[client] = 0;
}
public void OnClientPostAdminCheck(int client)
{
	if (!IsFakeClient(client) && !ClientTurnOffSounds[client])
	{
		PlayJoinSound(client);
	}
}

public void OnClientDisconnect(int client)
{
	SDKUnhook(client, SDKHook_OnTakeDamage, OnTakeDamage);
	SDKUnhook(client, SDKHook_TraceAttack, OnTraceAttack);
	SDKUnhook(client, SDKHook_WeaponDrop, OnWeaponDrop);
	SDKUnhook(client, SDKHook_Touch, OnTouch);
	SDKUnhook(client, SDKHook_PreThink, OnPreThink);
	SDKUnhook(client, SDKHook_PostThink, OnPostThink);
	SDKUnhook(client, SDKHook_SetTransmit, Hook_SetTransmit);
	SDKUnhook(client, SDKHook_WeaponSwitch, OnWeaponSwitch);
	SDKUnhook(client, SDKHook_WeaponCanUse, OnWeaponCanUse);
	SDKUnhook(client, SDKHook_WeaponEquip, OnWeaponEquip);
	InitPlayerBools(client);
}
public void OnConfigsExecuted()
{
	ServerSettings();
}
public void OnMapStart()
{
	FrGhostOnMapStart();
	FrChronoOnMapStart();
	FrSuicideBomberStart();
	FrBossStart();
	frThrowKnifeStart();
	LoadSounds();
	DeathExplodeInit();
	AddNormalSoundHook(SoundHook);
	//BeamIndex = PrecacheModel("materials/sprites/laserbeam.vmt");
	PrecacheSound("weapons/hegrenade/explode3.wav", true);
	PrecacheSound("*player/bhit_helmet-1.wav", true);
	PrecacheSound("*player/headshot1.wav", true);
	PrecacheSound("*player/headshot2.wav", true);
}
public void ServerSettings()
{
	if (cvPluginEnable.BoolValue)
	{
		//SetCvar("bot_quota", "0");
		//SetCvar("bot_quota_mode", "none");
		//SetCvar("sv_buy_status_override", "3");
		//SetCvar("mp_buytime", "0");
		SetCvar("mp_maxmoney", "0");
		SetCvar("mp_ct_default_secondary", "");
		SetCvar("mp_t_default_secondary", "");
		SetCvar("mp_free_armor", "0");
		SetCvar("sv_disable_immunity_alpha", "1");
		SetCvar("mp_give_player_c4", "0");
		SetCvar("mp_weapons_allow_map_placed", "0");
		SetCvar("sv_allow_thirdperson", "1");
		SetCvar("mp_drop_knife_enable", "1");
		SetCvar("mp_drop_grenade_enable", "1");
		SetCvar("sv_bounce", "100");
		SetCvar("sv_alltalk", "1");
		SetCvar("sv_deadtalk", "1");
		SetCvar("sv_full_alltalk", "1");
		SetCvar("mp_anyone_can_pickup_c4", "1");
		SetCvar("sv_enablebunnyhopping", "1");
	}
}

public Action Timer_PrintInfo(Handle timer, any userid)
{
	int client = GetClientOfUserId(userid);
	if (!IsValidClient(client)) return;
	PrintToChat(client, "[FunRounds!]\x3 \x4FunRounds!开源版本%s ", PLUGIN_VERSION);
	PrintToChat(client, "[FunRounds!]\x3 \x4作者为 %s", PLUGIN_AUTHOR);
	PrintToChat(client, "[FunRounds!]\x3 \x4您可以向开发者捐助！ %s", DONATE_URL);
	PrintToChat(client, "[FunRounds!]\x3 \x4输入!frmenu 或者按“.”查看服务器可用指令");
}
public void Fun_EventRoundStart(Event event, const char [] name, bool dontBroadcast)
{
	if (cvPluginEnable.BoolValue)
	{
		TurnOffAllSettings();
		if (GameRules_GetProp("m_bWarmupPeriod") == 1)
		{
			bSuicide = true;
			bFWS = true;
			bKnockback = true;
			bAimbot = true;
			SetCvar("sv_infinite_ammo", "1");
			SetAllAimbot(true);
			SetAimbotCvar(360.0, 8000.0, 1, false);
			return;
		}
		frRoundEnd = false;
		ChickenC4 = -1;
		SetCvar("mp_drop_knife_enable", "0");
		SetCvar("mp_drop_grenade_enable", "0");
		SetCvar("sv_full_alltalk", "1");
		CreateRound();
		RemoveAllSuicide();
		CreateTimer(0.1, Timer_CleanMapWeapons, _, TIMER_REPEAT);
		CreateTimer(1.2, Timer_PreStart);
	}
}
public Action Fun_EventPlayerSpawn(Event event, const char [] name, bool dontBroadcast)
{
	int client = GetClientOfUserId(event.GetInt("userid"));
	if (GameRules_GetProp("m_bWarmupPeriod") == 1)
	{
		RemoveWeapons(client);
		int prandom = Math_GetRandomInt(0, 24);
		int srandom = Math_GetRandomInt(0, 10);
		int orandom = Math_GetRandomInt(0, 21);
		if (IsValidClient(client) && IsPlayerAlive(client))
		{
			CSGOItems_GiveWeapon(client, WeaponPrimary[prandom], _, _, 1);
			CSGOItems_GiveWeapon(client, WeaponSecondary[srandom]);
			CSGOItems_GiveWeapon(client, OtherEquipment[orandom]);
			SetArmorValue(client, 100);
			GivePlayerItem(client, "weapon_knife");
		}
	}
	SetAimbot(client, bAimbot);
	RemoveSuicide(client);
	frTriggeredSuicide[client] = false;
	frArmorValue[client] = 0;
	if (bChrono) SetChrono(client);
	SetRespawnArms(client);
}
public Action Fun_EventRoundFreezeEnd(Event event, const char [] name, bool dontBroadcast)
{
	RoundStartSound();
	if (cvPluginEnable.BoolValue && GameRules_GetProp("m_bWarmupPeriod") == 0 && !frRoundEnd)
	{
		StartRound();
		SetCvar("mp_drop_knife_enable", "1");
		SetCvar("mp_drop_grenade_enable", "1");
		SpawnChickens();
	}
}

public Action Fun_EventRoundEnd(Event event, const char [] name, bool dontBroadcast)
{
	frRoundEnd = true;
	TurnOffAllSettings();
}

public Action OnWeaponDrop(int client, int weapon)
{
	if (cvPluginEnable.BoolValue && CSGOItems_IsValidWeapon(weapon))
	{
		SetModelScaleRandom(weapon);
		char sWeapon[64];
		GetEntityClassname(weapon, sWeapon, sizeof(sWeapon));
		if (StrEqual(DefuseMode, "2") || StrEqual(DefuseMode, "carry"))
		{
			if (!client || !c4Planted) return Plugin_Continue;
			if (weapon == EntRefToEntIndex(c4weapon) && IsValidEntity(weapon) && IsValidEntity(plantedC4)) 
			{
				plyCarryingC4 = -1;
				carryingC4[client] = false;
				AcceptEntityInput(plantedC4, "ClearParent");
				AcceptEntityInput(secondaryC4, "ClearParent");
				SetEntityRenderMode(weapon, RENDER_NONE);
				ParentEntity(plantedC4, c4weapon, "");
				TeleportEntity(plantedC4, emptyVector, emptyVector, NULL_VECTOR);
			}
		}
		if (StrEqual(sWeapon, "weapon_c4"))
		{
			bC4Bounce[weapon] = true;
			FR_C4Bounce(weapon);
		}
	}
	return Plugin_Continue;
}
public void ParentEntity(int child, int parent, const char[] attachment) 
{
	SetVariantString("!activator");
	AcceptEntityInput(child, "SetParent", parent, child, 0);
	if(!StrEqual(attachment, "", false)) 
	{
		SetVariantString(attachment);
		AcceptEntityInput(child, "SetParentAttachment", child, child, 0);
	}
}
public Action ChickenDamaged(int victim, int &attacker, int &inflictor, float &damage, int &damagetype, int &weapon, float damageForce[3], float damagePosition[3])
{
	if (!cvPluginEnable.BoolValue) return Plugin_Continue;
	char aWeapon[64];
	char iWeapon[64];
	if (IsValidClient(attacker))
	{
		SetFF(true);
		SetEntPropFloat(victim, Prop_Data, "m_explodeDamage", 2000.0);
		SetEntPropFloat(victim, Prop_Data, "m_explodeRadius", 1000.0);
		RequestFrame(SetFF, false);
		
	}
	if (IsValidEntity(weapon)) GetEntityClassname(weapon, aWeapon, sizeof(aWeapon));
	GetEntityClassname(inflictor, iWeapon, sizeof(iWeapon));
	if (StrContains(iWeapon, "chicken") != -1 || StrContains(iWeapon, "hegrenade") != -1 || StrContains(aWeapon, "hegrenade") != -1 || StrContains(iWeapon, "breachcharge") != -1 || StrContains(aWeapon, "breachcharge") != -1 || StrContains(iWeapon, "inferno") != -1 || StrContains(aWeapon, "inferno") != -1)
		return Plugin_Stop;
	if (StrContains(iWeapon, "snowball") != -1 || StrContains(aWeapon, "snowball") != -1)
		if (!IsValidClient(attacker))
			return Plugin_Stop;
	if (damagetype & DMG_BURN || damagetype & DMG_IGNITE || damagetype & DMG_BLAST)
		return Plugin_Stop;
	
	EmitSoundToAll(ChickenkillSounds[0], victim, GetRandomInt(10, 120));
	return Plugin_Continue;
}
public Action OnTraceAttack(int victim, int &attacker, int &inflictor, float &damage, int &damagetype, int &ammotype, int hitbox, int hitgroup)
{
	if (IsValidClient(attacker) && IsPlayerAlive(attacker))
	{
		char sWeapon[64];
		if (hitbox == 1)
		{
			damagetype |= CS_DMG_HEADSHOT;
			return Plugin_Changed;
		}
		int iWeapon = GetEntPropEnt(attacker, Prop_Send, "m_hActiveWeapon");
		if (!IsValidEntity(iWeapon)) return Plugin_Continue;
		GetEntityClassname(iWeapon, sWeapon, sizeof(sWeapon));
		if (StrEqual(sWeapon, "weapon_taser") || StrEqual(sWeapon, "weapon_knife") || StrEqual(sWeapon, "weapon_bayonet") || StrEqual(sWeapon, "weapon_axe") || StrEqual(sWeapon, "weapon_spanner") || StrEqual(sWeapon, "weapon_fists") || StrEqual(sWeapon, "weapon_shield"))
		{
			float Pos[3], Ang[3];
			GetClientEyePosition(attacker, Pos); 
			GetClientEyeAngles(attacker, Ang); 
			TR_TraceRayFilter(Pos, Ang, MASK_SHOT, RayType_Infinite, TR_VictimOnly, victim);
			int hitgroups = TR_GetHitGroup();
			if (hitgroups == 1)
			{
				damagetype |= CS_DMG_HEADSHOT;
				damage *= 1.5;
				hitbox = 1;
				hitgroup = 1;
				return Plugin_Changed;
			}
		}
	}
	if (cvPluginEnable.BoolValue)
	{
		
	}
	return Plugin_Continue;
}
bool TR_VictimOnly(int entity, int contentsMask, int victim)
{
	return entity == victim; 
}
public Action OnTakeDamage(int victim, int &attacker, int &inflictor, float &damage, int &damagetype, int &weapon, float damageForce[3], float damagePosition[3])
{
	if (!cvPluginEnable.BoolValue) return Plugin_Continue;
	char iWeapon[64];
	GetEdictClassname(inflictor, iWeapon, sizeof(iWeapon));
	if (damagetype & DMG_BURN && IsValidClient(victim))
	{
		if (StrEqual(iWeapon, "inferno"))
		{
			IgniteEntity(victim, 15.0);
		}
		else
		{
			damage = (GetHealth(victim) + 0.0) * 0.01;
			if (damage < 1) damage = 1.0;
		}
	}
	char sWeapon[64];
	if (CSGOItems_IsValidWeapon(weapon))
		GetEdictClassname(weapon, sWeapon, sizeof(sWeapon));
	if (StrContains(sWeapon, "snowball") != -1 || StrContains(iWeapon, "snowball") != -1)
		if (!IsValidClient(attacker))
			return Plugin_Stop;
	//SuicideBomber
	if (frIsSuicideBomber[victim] && frIsGoingSuicide[victim])
	{
		damage *= 5.0;
	}
	//Spectator Damage
	if (IsValidClient(attacker) && GetClientTeam(attacker) == CS_TEAM_SPECTATOR)
			return Plugin_Stop;
			
	//Self Damage
	if (IsValidClient(attacker) && (victim == attacker) && StringToFloat(SelfDMG) == 0.0)
	{
		if (!StrEqual(iWeapon, "chicken") && !StrEqual(sWeapon, "chicken"))
			return Plugin_Stop;
	}
	//GodMode
	if (IsValidClient(victim) && (frGodMode[victim] || frEraseingVictim[victim]))
	{
		return Plugin_Stop;
	}
	//HeadShot Only
	if (StrEqual(HeadShot, "1"))
	{
		if (StrEqual(iWeapon, "chicken") || StrEqual(sWeapon, "chicken"))
		{
			damagetype |= CS_DMG_HEADSHOT;
		}
		if (IsValidClient(attacker) && !(damagetype & CS_DMG_HEADSHOT))
			return Plugin_Stop;
	}
	if (StrEqual(HeadShot, "-1"))
	{
		if (IsValidClient(attacker) && (damagetype & CS_DMG_HEADSHOT))
		return Plugin_Stop;
	}
	if (IsValidClient(victim) && IsValidClient(attacker) && !frGodMode[victim] && !(damagetype & DMG_FALL) && !(GetClientTeam(attacker) == CS_TEAM_SPECTATOR) && (victim != attacker))
	{
		damage *= frPlayerDamageMultiplier[attacker];
		//BossMode Anti FF
		if (!StrEqual(BossMode, "0"))
		{
			if (!CheckBoss(victim) && !CheckBoss(attacker) && IsBossAlive())
			{
				return Plugin_Stop;
			}
		}
		if (bEnergyShield)
		{
			int clientshield = GetEntProp(victim, Prop_Send, "m_ArmorValue");
			if (clientshield > 0)
			{
				int calcshield;
				if (GetEntProp(victim, Prop_Send, "m_bHasHeavyArmor") == 1) calcshield = RoundToFloor(clientshield - damage / 3);
				else calcshield = RoundToFloor(clientshield - damage / 1.5);
				if (calcshield <= 0) calcshield = 0;
				SetArmorValue(victim, calcshield);
				damage = 0.0;
			}
		}
		if (StrEqual(TeslaBullet, "1") && !frPlayerWeaponFired[attacker])
		{
			if (StrEqual(Health, "random"))
			{
				damage = GetHealth(victim) * 0.45;
				if (damage < 90) damage = 90.0;
			}
			else
			{
				damage = GetHealth(victim) * 0.45;
				if (damage < (StringToFloat(Health) * 0.45)) damage = StringToFloat(Health) * 0.45;
			}
			frPlayerWeaponFired[attacker] = true;
			CreateTimer(5.0, Timer_TeslaRechange, GetClientUserId(attacker));
		}
		if (StrEqual(DamageMode, "delay"))
		{
			CreateDamage(3.0, victim, attacker, inflictor, damage, damagetype, weapon, false);
			return Plugin_Stop;
		}
		//BladeMail
		if (!StrEqual(BladeMail, "0.0"))
		{
			float damageMultiplier = StringToFloat(BladeMail);
			float blademaildmg = damage * damageMultiplier;
			CreateDamage(0.0, attacker, victim, inflictor, blademaildmg, damagetype, weapon, false);
		}
		//Decoy Damage (DodgeBall)
		if (bDodgeBall || StrEqual(DamageMode, "dodgeball"))
		{
			if (StrEqual(iWeapon, "weapon_decoy") || StrEqual(iWeapon, "decoy_projectile"))
			{
				if (frIsSuicideBomber[victim] && frIsGoingSuicide[victim])
					damage = 600.0;
				else
					damage = 200.0;
			}
		}
		// Damage mode
		if (StrEqual(DamageMode, "random") || RandomDamage())
		{
			int dmgrandom = Math_GetRandomInt(0, 99);
			if (dmgrandom <= 5) SetHealth(attacker, GetHealth(attacker) + RoundToFloor(damage * Math_GetRandomInt(1, 10)));
			else if (5 < dmgrandom <= 15) damage *= GetRandomFloat(0.0, 0.8);
			else if (15 < dmgrandom <= 80) damage *= GetRandomFloat(0.8, 2.4);
			else if (80 < dmgrandom <= 90) damage *= GetRandomFloat(2.4, 5.0);
			else if (90 < dmgrandom <= 98) damage *= GetRandomFloat(5.0, 20.0);
			else damage = (GetHealth(victim) + 0.00) * 10;
		}
		if (StrEqual(DamageMode, "taser"))
		{
			if (StrEqual(sWeapon, "weapon_taser"))
			{
				damage *= 10.0;
			}
			if (StrEqual(sWeapon, "weapon_knife"))
			{
				damage *= 20.0;
			}
		}
		if (StrEqual(DamageMode, "noknife"))
		{
			if (StrEqual(sWeapon, "weapon_knife"))
				return Plugin_Stop;
		}
		if (frChronoGunStatus[attacker] && IsValidClient(victim) && !frEraseingVictim[victim] && StrEqual(sWeapon, "weapon_taser"))
		{
			EraseingPlayer(attacker, victim);
			damage = 0.0;
			return Plugin_Stop;
		}
		//DamageMode
		if (!StrEqual(DamageMode, "taser") && !StrEqual(DamageMode, "noknife") && !StrEqual(DamageMode, "random") && !StrEqual(DamageMode, "none") && !StrEqual(DamageMode, "") && !StrEqual(DamageMode, "random") && !StrEqual(DamageMode, "delay"))
		{
			float FDamage = StringToFloat(DamageMode);
			if (FDamage > 0.0)
			{
				damage *= FDamage;
			}
			else
			{
				damage *= 1.0;
			}
		}
		return Plugin_Changed;
	}
	return Plugin_Continue;
}
public Action Timer_DelayTakeDamage(Handle timer, DataPack data)
{
	data.Reset();
	int victim = data.ReadCell();
	int inflictor = data.ReadCell();
	int attacker = data.ReadCell();
	float damage = data.ReadFloat();
	int damagetype = data.ReadCell();
	int weapon = data.ReadCell();
	int force = data.ReadCell();
	if (!CSGOItems_IsValidWeapon(inflictor)) inflictor = 0;
	if (!CSGOItems_IsValidWeapon(weapon)) weapon = -1;
	if (!IsValidClient(attacker) || !IsPlayerAlive(victim)) return Plugin_Stop;
	if (force == 0 && frGodMode[victim]) return Plugin_Stop;
	SDKHooks_TakeDamage(victim, inflictor, attacker, damage, damagetype, weapon);
	return Plugin_Continue;
}

void DelayDamage(DataPack data)
{
	data.Reset();
	int victim = data.ReadCell();
	int inflictor = data.ReadCell();
	int attacker = data.ReadCell();
	float damage = data.ReadFloat();
	int damagetype = data.ReadCell();
	int weapon = data.ReadCell();
	int force = data.ReadCell();
	if (!CSGOItems_IsValidWeapon(inflictor)) inflictor = 0;
	if (!CSGOItems_IsValidWeapon(weapon)) weapon = -1;
	if (!IsValidClient(attacker) || !IsPlayerAlive(victim)) return;
	if (force == 0 && frGodMode[victim]) return;
	SDKHooks_TakeDamage(victim, inflictor, attacker, damage, damagetype, weapon);
}

void CreateDamage(float time, int victim, int attacker, int inflictor, float damage, int damagetype = DMG_GENERIC, int weapon = -1, bool force = false)
{
	DataPack Damage = new DataPack();
	Damage.Reset(true);
	if (time == 0.0)
		RequestFrame(DelayDamage, Damage);
	else
		CreateDataTimer(time, Timer_DelayTakeDamage, Damage);
	Damage.WriteCell(victim);
	Damage.WriteCell(inflictor);
	Damage.WriteCell(attacker);
	Damage.WriteFloat(damage);
	Damage.WriteCell(damagetype);
	Damage.WriteCell(weapon);
	if (force) Damage.WriteCell(1);
	else Damage.WriteCell(0);
}

public Action Hook_SetTransmit(int entity, int client)
{
	if (cvPluginEnable.BoolValue)
	{
		if ((client == entity) || !IsPlayerAlive(client))
		{
			return Plugin_Continue;
		}
		if (PlayerIsEnemy(entity, client) && !frIsPlayerVisible[entity] && frIsGhostClient[entity] && bGhost)
		{
			return Plugin_Handled;
		}
	}
	return Plugin_Continue;
}
public Action Hook_WeaponSetTransmit(int entity, int client)
{
	if (!cvPluginEnable.BoolValue) return Plugin_Continue;
	return Plugin_Handled;
}
public Action OnTouch(int client, int entity)
{
	if (cvPluginEnable.BoolValue)
	{
		if (bWallHang)
		{
			int flags = GetEntityFlags(client);
			if (!(flags & FL_ONGROUND) && IsValidEntity(entity) && IsEntitySolid(entity))
			{
				int buttons = GetClientButtons(client);
				if (!IsPlayerWallHanged[client] && (buttons & IN_JUMP))
				{
					SetEntityMoveType(client, MOVETYPE_NONE);
					IsPlayerWallHanged[client] = true;
				}
			}
		}
	}
}
public Action OnPreThink(int client)
{
	if (cvPluginEnable.BoolValue)
	{
		if (bNoScope && !IsFakeClient(client))
		{
			SetNoScope(GetEntPropEnt(client, Prop_Send, "m_hActiveWeapon"));
			return Plugin_Changed;
		}
		if (frIsChronoClient[client] && (frIsChronoFreeze[client] || frEraseingVictim[client]))
		{
			SetEntPropFloat(client, Prop_Send, "m_flNextAttack", GetGameTime() + 1.0);
			return Plugin_Changed;
		}
		int buttons = GetClientButtons(client);
		if (IsPlayerWallHanged[client])
		{
			if (!(buttons & IN_JUMP))
			{
				SetEntityMoveType(client, MOVETYPE_WALK);
				IsPlayerWallHanged[client] = false;
			}
			return Plugin_Changed;
		}
		if (IsAimbotavailable(client))
		{
			int weapon = GetEntPropEnt(client, Prop_Send, "m_hActiveWeapon");
			if (CSGOItems_IsValidWeapon(weapon))
			{
				SetAimbotViewPunch(client, weapon);
			}
		}
		if (bOneShot)
		{
			int weapon = GetEntPropEnt(client, Prop_Send, "m_hActiveWeapon");
			char cWeapon[64];
			CSGOItems_GetWeaponClassNameByWeapon(weapon, cWeapon, sizeof(cWeapon));
			if (CSGOItems_IsValidWeapon(weapon) && (buttons & IN_ATTACK) || (StrEqual(cWeapon, "weapon_revolver") && buttons & IN_ATTACK2))
			{
				int clip = GetEntProp(weapon, Prop_Send, "m_iClip1");
				int reserve = GetEntProp(weapon, Prop_Send, "m_iPrimaryReserveAmmoCount");
				int offset = FindDataMapInfo(client, "m_iAmmo");
				if (clip != -1 && clip > 1)
				{
					SetEntData(client, offset, 1, true);
					CSGOItems_SetWeaponAmmo(weapon, 1, 1);
				}
					
				if (reserve < 1)
					CSGOItems_SetWeaponAmmo(weapon, 1);
			}
		}
	}
	return Plugin_Continue;
}
public Action OnWeaponSwitch(int client, int weapon)
{
	if (!cvPluginEnable.BoolValue) return Plugin_Continue;
	if (frIsChicken[client])
	{
		SetWeaponVisibility(client, weapon, false);
		CreateFakeWeapon(client, weapon);
	}
	else
	{
		SetWeaponVisibility(client, weapon, true);
	}
	if (bFWS)
	{
		DataPack fws = new DataPack();
		fws.Reset(true);
		CreateDataTimer(0.0, Timer_InstantSwitch, fws);
		fws.WriteCell(client);
		fws.WriteCell(weapon);
	}
	if (bOneShot)
	{
		if (IsValidClient(client) && CSGOItems_IsValidWeapon(weapon) && (GetEntProp(weapon, Prop_Send, "m_iClip1") > 0))
			CSGOItems_SetWeaponAmmo(weapon, 1, 1);
	}
	if (StrEqual(DefuseMode, "2") || StrEqual(DefuseMode, "carry"))
	{
		if(!client || !c4Planted) return Plugin_Continue;
		int c4client = plyCarryingC4;
		if(client != c4client) return Plugin_Continue;
		if(!IsValidEntity(plantedC4)) return Plugin_Continue;
		if(weapon == EntRefToEntIndex(c4weapon)) 
		{
			AcceptEntityInput(plantedC4, "ClearParent");
			ParentEntity(plantedC4, client, "legacy_weapon_bone");
			float pos[3], ang[3];
			pos[0] = 1.2;
			pos[1] = -3.0;
			pos[2] = -0.5;
			ang[0] = 0.0;
			ang[1] = 95.0;
			ang[2] = 155.0;
			TeleportEntity(plantedC4, pos, ang, NULL_VECTOR);
			SetEntityRenderColor(weapon, 255, 255, 255, 0);
			SetEntityRenderMode(weapon, RENDER_NONE);
		} else 
		{
			AcceptEntityInput(plantedC4, "ClearParent");
			ParentEntity(plantedC4, client, "c4");
			float pos[3];
			pos[2] = -2.0;
			TeleportEntity(plantedC4, pos, NULL_VECTOR, NULL_VECTOR);
		}
	}
	return Plugin_Continue;
}
public Action OnWeaponCanUse(int client, int weapon)
{
	if (frIsGoingSuicide[client]) return Plugin_Stop;
	char classname[64];
	GetEntityClassname(weapon, classname, sizeof(classname));
	if (StrEqual(classname, "weapon_melee") && !HasWeapon(client, classname))
		EquipPlayerWeapon(client, weapon);
	return Plugin_Continue;
}
public Action OnWeaponEquip(int client, int weapon) 
{
	char Weapons[48];
	CSGOItems_GetWeaponClassNameByWeapon(weapon, Weapons, sizeof(Weapons));
	if (IsValidClient(client) && IsPlayerAlive(client) && IsFakeClient(client) && CSGOItems_IsValidWeapon(weapon) && StrContains(Weapons, "weapon_") && !StrEqual(Weapons, "weapon_c4") && CSGOItems_GetWeaponSlotByWeapon(weapon) != CS_SLOT_PRIMARY && CSGOItems_GetWeaponSlotByWeapon(weapon) != CS_SLOT_SECONDARY)
	{
		if (botweapon[weapon]) return Plugin_Continue;
		if (CSGOItems_GetWeaponSlotByWeapon(weapon) == CS_SLOT_KNIFE && StrContains(Weapons, "weapon_knife")) return Plugin_Continue;
		botweapon[weapon] = true;
		DataPack BotWeapon = new DataPack();
		BotWeapon.Reset(true);
		CreateDataTimer(0.1, Timer_BotUseWeapon, BotWeapon);
		BotWeapon.WriteCell(client);
		BotWeapon.WriteCell(weapon);
	}
	if (!client || !c4Planted || !cvPluginEnable.BoolValue) return Plugin_Continue;
	if ((StrEqual(DefuseMode, "2") || StrEqual(DefuseMode, "carry")) && weapon == EntRefToEntIndex(c4weapon) && IsValidEntity(plantedC4))
	{
		plyCarryingC4 = client;
		carryingC4[client] = true;
		AcceptEntityInput(plantedC4, "ClearParent");
		ParentEntity(plantedC4, client, "c4");
		float pos[3];
		pos[2] = -2.0;
		TeleportEntity(plantedC4, pos, NULL_VECTOR, NULL_VECTOR);
		SetEntityRenderMode(weapon, RENDER_NONE);
		float fPos[3];
		GetClientAbsOrigin(client, fPos);
		TeleportEntity(secondaryC4, fPos, emptyVector, NULL_VECTOR);
		ParentEntity(secondaryC4, client, "");
	}
	return Plugin_Continue;
}
public Action OnReload(int weapon)
{
	if (!cvPluginEnable.BoolValue) return Plugin_Continue;
	if (CSGOItems_IsValidWeapon(weapon))
	{
		int client = GetEntPropEnt(weapon, Prop_Data, "m_hOwnerEntity");
		int offset = FindDataMapInfo(client, "m_iAmmo");
		if (bOneShot)
		{
			if (!IsFakeClient(client))
			{
				SetEntData(client, offset, 1, true);
				CSGOItems_SetWeaponAmmo(weapon, 1, 0);
			}
		}
		if (GetEntProp(weapon, Prop_Send, "m_iPrimaryReserveAmmoCount") > 0)
		{
			if (!StrEqual(InfAmmo, "0"))
			{
				SetEntData(client, offset, 999, true);
				CSGOItems_SetWeaponAmmo(weapon, 999);
			}
			char classname[64];
			GetEntityClassname(weapon, classname, sizeof(classname));
			if (StrEqual(classname, "weapon_nova") || StrEqual(classname, "weapon_xm1014") || StrEqual(classname, "weapon_sawedoff"))
			{
				return Plugin_Continue;
			}
			if (StrEqual(classname, "weapon_revolver"))
			{
				CSGOItems_SetWeaponAmmo(weapon, -1, 0);
				SetEntProp(weapon, Prop_Send, "m_iClip1", 0);
				SetEntProp(weapon, Prop_Send, "m_iClip2", 0);
				return Plugin_Changed;
			}
			else if (GetEntProp(weapon, Prop_Send, "m_iClip1") > 1)
			{
				CSGOItems_SetWeaponAmmo(weapon, -1, 1);
				SetEntProp(weapon, Prop_Send, "m_iClip1", 1);
				return Plugin_Changed;
			}
		}
	}
	return Plugin_Continue;
}
public Action Timer_InstantSwitch(Handle timer, DataPack data)
{
	if (!cvPluginEnable.BoolValue || !bFWS) return;
	data.Reset();
	int client = data.ReadCell();
	int weapon = data.ReadCell();
	if (IsValidClient(client) && IsPlayerAlive(client) && CSGOItems_IsValidWeapon(weapon))
		InstantSwitchWeapon(client, weapon);
}
void InstantSwitchWeapon(int client, int weapon)
{
	SetEntPropEnt(client, Prop_Send, "m_hActiveWeapon", weapon);
	SetEntPropFloat(weapon, Prop_Send, "m_flNextPrimaryAttack", GetGameTime());
	SetEntPropFloat(client, Prop_Send, "m_flNextAttack", GetGameTime());
	SetEntProp(GetEntPropEnt(client, Prop_Send, "m_hViewModel"), Prop_Send, "m_nSequence", 0);
}
public void OnPostThink(int client)
{
	if (!cvPluginEnable.BoolValue) return;
	if (IsValidClient(client) && IsPlayerAlive(client) && frIsChicken[client])
	{
		SetViewModel(client, false);
		int Flags = GetEntityFlags(client);
		if (Flags & FL_ONGROUND)
		{
			SetChickenSpeed(client);
		}
	}
}
public Action OnPlayerRunCmd(int client, int &buttons, int &impulse, float vel[3], float angles[3], int &weapon, int &subtype, int &cmdnum, int &tickcount, int &seed, int mouse[2])
{
	if (IsValidClient(client) && GetEntPropFloat(client, Prop_Data, "m_flDuckSpeed") < 8.0)
		SetEntPropFloat(client, Prop_Send, "m_flDuckSpeed", 8.0);
	if (0 < GetEntPropEnt(client, Prop_Send, "m_hGroundEntity") < MaxClients)
		SetEntPropEnt(client, Prop_Send, "m_hGroundEntity", 0);
	if (IsValidClient(client) && IsPlayerAlive(client) && frIsGoingSuicide[client])
	{
		if (buttons & IN_ATTACK || buttons & IN_ATTACK2 || buttons & IN_ATTACK3)
		{
			buttons &= ~IN_ATTACK;
			return Plugin_Changed;
		}
	}
	if (cvPluginEnable.BoolValue)
	{
		//Ghost
		if (bGhost)
		{
			int flags = GetEntityFlags(client);
			bool isPlayerInvisible = ((IsPlayerNotMoving(buttons)) || IsPlayerWalking(buttons, flags)) && !IsPlayerAttacking(buttons);
			if (isPlayerInvisible)
			{
				FrGhostVisibility(client, false);
			}
			else
			{
				FrGhostVisibility(client, true);
			}
		}
	
		//Chicken
		if (StrEqual(Chicken, "1"))
		{
			if (!IsValidClient(client) || !IsPlayerAlive(client) || !frIsChicken[client]) return Plugin_Continue;
			if (cvFrChicken.BoolValue)
			{
				if (vel[1] != 0)
					vel[1] = 0.0;
				if (!(buttons & IN_BACK) || !(buttons & IN_USE))
				{
					if (vel[0] < 0)
					vel[0] = 0.0;
				}
			}
			frCIsWalking[client] = (buttons & IN_SPEED) || (buttons & IN_DUCK);
			frCIsMoving[client] = vel[0] > 0.0 || vel[0] < 0.0;
			if ((buttons & IN_JUMP) && !(GetEntityFlags(client) & FL_ONGROUND))
			{
				SlowPlayerFall(client);
			}
			if ((buttons & IN_MOVELEFT))
				PlayRandomIdleSound(client);
			else if (buttons & IN_MOVERIGHT)
				PlayRandomPanicSound(client);
			else if ((buttons & IN_BACK) && !(buttons & IN_USE))
				PlayChickenSound(client);
			return Plugin_Changed;
		}
		if (StrEqual(BackWards, "2") && !IsFakeClient(client) || (StrEqual(BackWards, "1") && IsFakeClient(client)))
		{
			vel[1] = -vel[1]; // X (left/right)
			vel[0] = -vel[0]; // Y (forward/backward)
			if (buttons & IN_MOVELEFT)
			{
				buttons &= ~IN_MOVELEFT;
				buttons |= IN_MOVERIGHT;
			}
			else if (buttons & IN_MOVERIGHT)
			{
				buttons &= ~IN_MOVERIGHT;
				buttons |= IN_MOVELEFT;
			}
			else if (buttons & IN_FORWARD)
			{
				buttons &= ~IN_FORWARD;
				buttons |= IN_BACK;
			}
			else if (buttons & IN_BACK)
			{
				buttons &= ~IN_BACK;
				buttons |= IN_FORWARD;
			}
			return Plugin_Changed;
		}
		if (StrEqual(BackWards, "3") && !IsFakeClient(client))
		{
			if (vel[0] > 0)
				vel[0] = 0.0;
			vel[1] = 0.0;
			if (buttons & IN_MOVELEFT)
			{
				buttons &= ~IN_MOVELEFT;
			}
			else if (buttons & IN_MOVERIGHT)
			{
				buttons &= ~IN_MOVERIGHT;
			}
			else if (buttons & IN_FORWARD)
			{
				buttons &= ~IN_FORWARD;
			}
			return Plugin_Changed;
		}
		//Chrono
		if (bChrono)
		{
			if (!IsValidClient(client) || !IsPlayerAlive(client) || !frIsChronoClient[client]) return Plugin_Continue;
			//vel[1] = 0.0;
			//vel[0] = 0.0;
			if (frIsChronoFreeze[client])
			{
				if (buttons & IN_ATTACK || buttons & IN_ATTACK2 || buttons & IN_ATTACK3 || buttons & IN_RELOAD || buttons & IN_MOVELEFT || buttons & IN_MOVERIGHT || buttons & IN_FORWARD || buttons & IN_BACK)
				{
					buttons |= IN_USE;
				}
			}
			if (frEraseingVictim[client])
			{
				if (buttons & FL_FROZEN)
				{
					buttons &= ~FL_FROZEN;
				}
				buttons |= FL_FROZEN;
			}
			return Plugin_Changed;
		}
		if (StrEqual(TeslaBullet, "1"))
		{
			if (!IsValidClient(client) || !IsPlayerAlive(client) || !frPlayerWeaponFired[client]) return Plugin_Continue;
			if (buttons & IN_ATTACK || buttons & IN_ATTACK2) buttons &= ~IN_ATTACK;
			return Plugin_Changed;
		}
		
		if (bAutoFire || IsFakeClient(client))
		{
			if (IsValidClient(client) && IsPlayerAlive(client))
			{
				frTriggerDetect(client, buttons);
			}
		}
		if (frIsPlayerCanHook[client])
		{
			if (buttons & IN_USE && !frIsHooked[client])
				GrappleHookAction(client);
			else if (!(buttons & IN_USE) && frIsHooked[client])
				GrappleHookOff(client);
		}
		if (IsAimbotavailable(client))
		{
			if (IsValidClient(client) && IsPlayerAlive(client))
			{
				AutoAimCheck(client, buttons);
			}
		}
		if (IsValidClient(client) && IsPlayerAlive(client))
		{
			ApplyAutoStrafe(client, buttons, vel, angles);
		}
			
		return Plugin_Changed;
	}
	return Plugin_Continue;
}
public Action Fun_EventBombBeginPlant(Event event, const char [] name, bool dontBroadcast)
{
	if (!cvPluginEnable.BoolValue) return;
	int client = GetClientOfUserId(event.GetInt("userid"));
	if (IsValidClient(client))
		CreateTimer(0.0, Timer_Plant, GetClientUserId(client));
}
public Action Fun_EventBombBeginDefuse(Event event, const char [] name, bool dontBroadcast)
{
	if (!cvPluginEnable.BoolValue) return;
	int client = GetClientOfUserId(event.GetInt("userid"));
	if (IsValidClient(client))
	{
		CreateTimer(0.0, Timer_Defuse, GetClientUserId(client));
	}
}

public Action Fun_EventBombPickUp(Event event, const char [] name, bool dontBroadcast)
{
	int client = GetClientOfUserId(event.GetInt("userid"));
	int c4ent = FindEntityByClassname(MAXPLAYERS + 1, "weapon_c4");
	if (!cvPluginEnable.BoolValue) return;
	if ((client == plyCarryingC4) || (c4ent == c4weapon) || (c4ent == secondaryC4) || carryingC4[client]) return;
	if (cvSuicideBomber.BoolValue && bSuicide && IsValidClient(client) && !frIsSuicideBomber[client])
	{
		if (GetClientTeam(client) == CS_TEAM_T)
		{
			PrintToChat(client, "[SM] 你身上有ISIS成员的圣战武器！当你 \x06自爆时\x01, 你会杀死你身边小范围内的所有玩家。 拿出C4炸弹喊出安拉胡阿克巴并按 \x06F\x01 键来自爆！");
			frIsSuicideBomber[client] = true;
		} else if (GetClientTeam(client) == CS_TEAM_CT)
		{
			PrintToChat(client, "[SM] 你身上有ISIS成员的“圣战”武器！ \x06你是反恐精英，可以选择销毁这个邪恶的武器！\x01 代价是你会杀死你身边小范围内的所有玩家并壮烈牺牲。 拿出C4炸弹按 \x06F\x01 键来销毁！");
			frIsSuicideBomber[client] = true;
		}
	}
	if (frC4Bounce[c4ent] != INVALID_HANDLE && (c4ent != -1))
	{
		KillTimer(frC4Bounce[c4ent]);
		frC4Bounce[c4ent] = INVALID_HANDLE;
		bC4Bounce[c4ent] = false;
	}
}
public Action Fun_EventBombDrop(Event event, const char [] name, bool dontBroadcast)
{
	if (!cvPluginEnable.BoolValue) return;
	int client = GetClientOfUserId(event.GetInt("userid"));
	if (IsValidClient(client) && frIsSuicideBomber[client]) frIsSuicideBomber[client] = false;
/*
	int c4ent = FindEntityByClassname(MAXPLAYERS + 1, "weapon_c4");
	if (IsValidEntity(c4ent) && !c4Planted)
	{
		bC4Bounce[c4ent] = true;
		FR_C4Bounce(c4ent);
	}	
*/
}
public Action Fun_EventBombPlanted(Event event, const char [] name, bool dontBroadcast)
{
	if (!cvPluginEnable.BoolValue) return;
	int client = GetClientOfUserId(event.GetInt("userid"));
	int c4ent = FindEntityByClassname(MAXPLAYERS + 1, "planted_c4");
	SetBombTimer(c4ent);
	if (IsValidClient(client) && frIsSuicideBomber[client]) frIsSuicideBomber[client] = false;
	if (IsValidClient(client) && GetClientTeam(client) == CS_TEAM_CT)
	{
		SetCvar("mp_c4_cannot_be_defused", "1");
	}
	if (IsValidEntity(c4ent))
	{
		SetModelScaleRandom(c4ent);
		if (StrEqual(DefuseMode, "1") || StrEqual(DefuseMode, "chicken"))
		{
			int chicken = CreateEntityByName("chicken");
			if (chicken != -1)
			{
				float pos[3];
				GetEntPropVector(client, Prop_Data, "m_vecOrigin", pos);
				pos[2] += -15.0;
				DispatchSpawn(chicken);
				SetEntProp(chicken, Prop_Data, "m_takedamage", 0);
				SetEntProp(chicken, Prop_Send, "m_fEffects", 0);
				TeleportEntity(chicken, pos, NULL_VECTOR, NULL_VECTOR);
				TeleportEntity(c4ent, NULL_VECTOR, view_as<float>( { 0.0, 0.0, 0.0 } ), NULL_VECTOR);
				SetVariantString("!activator");
				AcceptEntityInput(c4ent, "SetParent", chicken, c4ent, 0);
				SetModelScaleRandom(chicken);
				ChickenC4 = chicken;
				pos[2] += 15.0;
				TeleportEntity(chicken, pos, NULL_VECTOR, NULL_VECTOR);
			}
		}
		if (StrEqual(DefuseMode, "2") || StrEqual(DefuseMode, "carry"))
		{
			c4Planted = true;
			if(IsValidClient(client))
			{
				int c42 = CreateEntityByName("planted_c4");
				float bombTime = GetEntPropFloat(c4ent, Prop_Send, "m_flTimerLength");
				SetEntProp(c42, Prop_Send, "m_bBombTicking", true);
				SetEntPropFloat(c42, Prop_Send, "m_flTimerLength", bombTime);
				SetEntPropFloat(c42, Prop_Send, "m_flC4Blow", GetEntPropFloat(c4ent, Prop_Send, "m_flC4Blow"));
				DispatchSpawn(c42);
				ActivateEntity(c42);
				char g_sOutput[32];
				Format(g_sOutput, sizeof(g_sOutput), "OnUser1 !self:ClearParent::%f:1", bombTime - 0.3);
				SetVariantString(g_sOutput);
				AcceptEntityInput(c42, "AddOutput");
				Format(g_sOutput, sizeof(g_sOutput), "OnUser2 !self:Kill::%f:1", bombTime - 0.2);
				SetVariantString(g_sOutput);
				AcceptEntityInput(c42, "AddOutput");
				AcceptEntityInput(c42, "FireUser1");
				AcceptEntityInput(c42, "FireUser2");
				SetEntityRenderMode(c42, RENDER_NONE);
				secondaryC4 = EntIndexToEntRef(c42);
				SDKHook(c42, SDKHook_SetTransmit, OnShouldDisplay);
				plantedC4 = EntIndexToEntRef(c4ent);
				SDKHook(c4ent, SDKHook_SetTransmit, OnShouldDisplay);
				int c4wep = CreateEntityByName("weapon_c4"); 
				DispatchSpawn(c4wep);
				SetEntityRenderMode(c4wep, RENDER_NONE);
				c4weapon = EntIndexToEntRef(c4wep);
				EquipPlayerWeapon(client, c4wep);
				carryingC4[client] = true;
				plyCarryingC4 = client;
			}
		}
	}
}
public Action OnShouldDisplay(int ent, int client)
{
	if(!c4Planted) return Plugin_Continue;
	int c4client = plyCarryingC4;
	if(client != c4client && ent == EntRefToEntIndex(plantedC4)) return Plugin_Continue;
	if(client == c4client && ent == EntRefToEntIndex(secondaryC4)) return Plugin_Continue;
	return Plugin_Handled;
}
public Action Fun_EventInspectWeapon(Event event, const char [] name, bool dontBroadcast)
{
	if (!cvPluginEnable.BoolValue) return;
	int client = GetClientOfUserId(event.GetInt("userid"));
	int weapon = GetEntPropEnt(client, Prop_Send, "m_hActiveWeapon");
	if ((client == plyCarryingC4) || (weapon == c4weapon) || (weapon == secondaryC4) || carryingC4[client]) return;
	if (!CSGOItems_IsValidWeapon(weapon)) return;
	char Weapons[64];
	GetEdictClassname(weapon, Weapons, sizeof(Weapons));
	if (cvSuicideBomber.BoolValue && bSuicide && IsPlayerAlive(client) && frIsSuicideBomber[client] && !frIsGoingSuicide[client])
	{
		if (StrEqual(Weapons, "weapon_c4"))
			BomberGoSuicide(client);
	}
	if (frIsChronoClient[client] && !frIsChronoFreeze[client] && IsPlayerAlive(client))
	{
		if (StrEqual(Weapons, "weapon_taser"))
		{
			frChronoGunStatus[client] = !frChronoGunStatus[client];
			PrintToChat(client, "[SM] 武器已切换为%s", frChronoGunStatus[client] ? "抹消枪" : "传送枪");
			PrintHintText(client, "武器已切换为%s", frChronoGunStatus[client] ? "抹消枪" : "传送枪");
		}
	}
}
public Action Fun_EventPlayerHurt(Event event, const char [] name, bool dontBroadcast)
{
	
	int client = GetClientOfUserId(event.GetInt("userid"));
	int attacker = GetClientOfUserId(event.GetInt("attacker"));
	if (!IsValidClient(attacker)) return;
	int dmg_health = event.GetInt("dmg_health");
	int dmg_armor = event.GetInt("dmg_armor");
	int attackerH = GetEntProp(attacker, Prop_Data, "m_iHealth");
	int attackerA = GetEntProp(attacker, Prop_Send, "m_ArmorValue");
	int hitgroup = event.GetInt("hitgroup");
	char cWeapon[64];
	event.GetString("weapon", cWeapon, sizeof(cWeapon));
	
	if (hitgroup == 1 && StrEqual(cWeapon, "weapon_taser") || StrEqual(cWeapon, "weapon_knife") || StrEqual(cWeapon, "weapon_bayonet") || StrEqual(cWeapon, "weapon_axe") || StrEqual(cWeapon, "weapon_spanner") || StrEqual(cWeapon, "weapon_fists") || StrEqual(cWeapon, "weapon_shield") || StrContains(cWeapon, "grenade") || StrContains(cWeapon, "flashbang") || StrContains(cWeapon, "snowball") || StrContains(cWeapon, "decoy") || StrEqual(cWeapon, "weapon_molotov"))
	{
		event.SetBool("headshot", true);
	}
	if (StrEqual(cWeapon, "weapon_knife") || StrEqual(cWeapon, "weapon_bayonet") || StrEqual(cWeapon, "weapon_axe") || StrEqual(cWeapon, "weapon_spanner") || StrEqual(cWeapon, "weapon_fists") || StrEqual(cWeapon, "weapon_shield"))
	{
		float Pos[3], Ang[3];
		GetClientEyePosition(attacker, Pos); 
		GetClientEyeAngles(attacker, Ang); 
		TR_TraceRayFilter(Pos, Ang, MASK_SHOT, RayType_Infinite, TR_VictimOnly, client);
		int hitgroups = TR_GetHitGroup();
		if (hitgroups == 1)
		{
			event.SetBool("headshot", true);
			if (GetEntProp(client, Prop_Send, "m_bHasHelmet") == 1) EmitSoundToAll("*player/bhit_helmet-1.wav", client);
			else
			{
				if (Math_GetRandomInt(0, 1) == 1) EmitSoundToAll("*player/headshot1.wav", client);
				else EmitSoundToAll("*player/headshot2.wav", client);
			}
		}
	}
	if (!cvPluginEnable.BoolValue) return;
	if (bVampire)
	{
		if (IsValidClient(attacker) && IsPlayerAlive(attacker) && (attacker != client))
		{
			int GiveHealth = attackerH + dmg_health;
			int GiveArmor = attackerA + dmg_armor;
			SetHealth(attacker, GiveHealth);
			if (attackerA < 1) GivePlayerItem(attacker, "item_assaultsuit");
			SetArmorValue(attacker, GiveArmor);
			
		}
	}
	if (IsValidClient(client) && IsPlayerAlive(client) && frGodMode[client] && (attacker != client))
	{
		SetHealth(client, GetHealth(client) + dmg_health);
		SetArmorValue(client, GetEntProp(client, Prop_Send, "m_ArmorValue") + dmg_armor);
		return;
	}
	if (GetEntProp(client, Prop_Send, "m_bHasHelmet") == 1 && (hitgroup == 1 || event.GetBool("headshot")) && attacker != client)
	{
			if ((!StrEqual(FriendlyFire, "0") && !PlayerIsEnemy(client, attacker)) || (PlayerIsEnemy(client, attacker) && IsValidClient(attacker)))
				SetEntProp(client, Prop_Send, "m_bHasHelmet", 0);
	}
	CheckHealthPercent(client);
	CheckArmorValue(client);
}
public Action Fun_EventPlayerDeath(Event event, const char [] name, bool dontBroadcast)
{
	int client = GetClientOfUserId(event.GetInt("userid"));
	int attacker = GetClientOfUserId(event.GetInt("attacker"));
	char Weapons[64];
	event.GetString("weapon", Weapons, sizeof(Weapons));
	if (IsValidClient(client))
		CreateDeathExplode(client);
	if (cvPluginEnable.BoolValue)
	{
		char aname[64];
		carryingC4[client] = false;
		if (StrEqual(Weapons, "chicken"))
		{
			if(StrEqual(HeadShot, "1")) event.SetBool("headshot", true);
		}
		if (IsValidClient(client) && !IsFakeClient(client))
		{
			ClientCommand(client, "firstperson");
			SetFov(client, 90);
		}
		if (StrEqual(Weapons, "inferno"))
		{
			int Ragdoll = GetEntPropEnt(client, Prop_Send, "m_hRagdoll");
			AcceptEntityInput(Ragdoll, "Ignite");
		}
		if (frIsChicken[client])
			ChickenStatus(client, false);
		//Ghost
		RemovePlayerGhost(client);
		frIsPlayerVisible[client] = false;
		//GodMode
		if (IsValidClient(attacker) && IsPlayerAlive(attacker) && !IsFakeClient(attacker))
		{
			frGodMode[attacker] = true;
			CreateTimer(0.5, RemoveGodMode, GetClientUserId(attacker));
		}
		//Suicide Bomber
		if (cvSuicideBomber.BoolValue && bSuicide)
		{
			if (frKilledBySuicide[client] && frDiedByBomb[client] != -1)
			{
				if (StrEqual(HeadShot, "1")) event.SetBool("headshot", true);
				event.BroadcastDisabled = true; // hide origin display
				Event display = CreateEvent("player_death", true); //create fake display
				display.SetInt("userid", event.GetInt("userid"));
				display.SetInt("attacker", GetClientUserId(frDiedByBomb[client]));
				display.SetInt("assister", event.GetInt("assister"));
				display.SetBool("assistedflash", event.GetBool("assistedflash"));
				display.SetString("weapon", "c4");	// set weapon icon
				event.GetString("weapon_itemid", Weapons, sizeof Weapons);
				display.SetString("weapon_itemid", Weapons);
				event.GetString("weapon_fauxitemid", Weapons, sizeof Weapons);
				display.SetString("weapon_fauxitemid", Weapons);
				event.GetString("weapon_originalowner_xuid", Weapons, sizeof Weapons);
				display.SetString("weapon_originalowner_xuid", Weapons);
				display.SetBool("headshot", false);
				display.SetInt("dominated", event.GetInt("dominated"));
				display.SetInt("revenge", event.GetInt("revenge"));
				display.SetInt("wipe", event.GetInt("wipe"));
				display.SetInt("penetrated", event.GetInt("penetrated"));
				display.SetBool("noreplay", event.GetBool("noreplay"));
				display.SetBool("noscope", event.GetBool("noscope"));
				display.SetBool("thrusmoke", event.GetBool("thrusmoke"));
				display.SetBool("attackerblind", event.GetBool("attackerblind"));
				for(int i = 1; i <= MaxClients; ++i) if(IsClientInGame(i) && !IsFakeClient(i))
				{
					display.FireToClient(i);
				}
				display.Cancel();
				//set attacker and weapon
				event.SetInt("attacker", GetClientUserId(frDiedByBomb[client]));
				event.SetString("weapon", "c4");
				RemoveSuicide(client);
				int Ragdoll = GetEntPropEnt(client, Prop_Send, "m_hRagdoll");
				AcceptEntityInput(Ragdoll, "Ignite");
				return Plugin_Changed;
			}
			if (frIsSuicideBomber[client])
			{
				if (frIsGoingSuicide[client] && !frKilledBySuicide[client] && GetClientTeam(client) == CS_TEAM_T)
				{
					int Ragdoll = GetEntPropEnt(client, Prop_Send, "m_hRagdoll");
					AcceptEntityInput(Ragdoll, "Ignite");
					GetClientName(client, aname, sizeof(aname));
					PrintToChatAll("[SM] \x06%s\x01 得不到七十二个处女了", aname);
				}
				RemoveSuicide(client);
			}
		}
		//ArmRace
		if (StrEqual(ArmRace, "1"))
		{
			if (IsValidClient(attacker) && IsPlayerAlive(attacker))
				EquipRandomWeapon(attacker, _, true);
		}
		if (!StrEqual(TeammateKill, "0"))
		{
			if(IsValidClient(attacker) && IsPlayerAlive(attacker) && IsValidClient(client))
			{
				if (GetClientTeam(attacker) == GetClientTeam(client))
				{
					++frTKs[attacker];
					if (StringToInt(TeammateKill) == frTKs[attacker])
					{
						frTKs[attacker] = 0;
						CreateExplosion(attacker);
						CreateDamage(0.0, attacker, attacker, 0, (GetHealth(attacker) + 0.00) * 10, CS_DMG_HEADSHOT, client, true);
					}
				}
				frTKs[client] = 0;
			}
		}
		//Chrono Erase Gun
		if (bChrono)
		{
			if (frEraseing[client] != -1)
			{
				RemoveEraseing(client);
			}
			if (frEraseingVictim[client])
			{
				EmitSoundToAll(frEraseKillSound, client, SNDCHAN_USER_BASE);
				RequestFrame(EraseRagdoll, GetEntPropEnt(client, Prop_Send, "m_hRagdoll"));
			}
		}
		//BossEvolve
		if (bBossEvolve && IsValidClient(attacker) && IsPlayerAlive(attacker))
		{
			int health = GetHealth(attacker);
			int armor = GetArmorValue(attacker);
			if (health < 150)
				SetHealth(attacker, 300);
			else
				SetHealth(attacker, (health + 100) * 2);
			if (armor < 100)
				SetArmorValue(client, 200, true, true);
			else
				SetArmorValue(client, (armor * 2), true);
		}
		if (!StrEqual(BossMode, "0") && IsBossAlive())
		{
			CheckBossAlive();
			if (!IsBossAlive())
			{
				RestorePlayersTeam();
				DisableRespawn();
				SetFFA(true);
				PrintToChatAll("[SM] \x3 \x4最后的BOSS被消灭了！但人们开始了混战！");
			}
		}
		carryingC4[client] = false;
		frBuyedHealthshot[client] = false;
		frAteChicken[client] = false;
		frAteChickenCount[client] = 0;
		frPlayerDamageMultiplier[client] = 1.0;
		frPlayerSpeedMultiplier[client] = 1.0;
		frArmorValue[client] = 0;
		ClearChickenTimer(client);
		RemoveChrono(client);
		RemoveBoss(client);
		SetCheats(client, false);
	}
	PlayKillSounds(client, attacker, Weapons, event.GetBool("headshot"));
	return Plugin_Continue;
}
public Action Fun_EventWeaponFire(Event event, const char [] name, bool dontBroadcast)
{
	if (!cvPluginEnable.BoolValue) return;
	int client = GetClientOfUserId(event.GetInt("userid"));
	char Weapons[64];
	event.GetString("weapon", Weapons, sizeof(Weapons));
	if (StringToFloat(SelfDMG) > 0.0 && !StrEqual(SelfDMG, "0.0"))
	{
		if (GetHealth(client) <= 1)
			CreateDamage(0.0, client, client, client, 1.00, CS_DMG_HEADSHOT, client, true);
	}
	if (StrEqual(Weapons, "weapon_taser") && frIsChronoClient[client] && !frChronoGunStatus[client])
	{
		RequestFrame(FrTeleport, client);
	}
	if (cvKnockBack.BoolValue && bKnockback && frKnockBack[client])
	{
		int slot = CSGOItems_GetActiveWeaponSlot(client);
		if (StrEqual(Weapons, "weapon_nova") || StrEqual(Weapons, "weapon_mag7") || StrEqual(Weapons, "weapon_sawedoff") || StrEqual(Weapons, "weapon_xm1014") || StrEqual(Weapons, "weapon_ssg08"))
		{
			Knockback(client, 700.0);
		}
		else if (StrEqual(Weapons, "weapon_awp") || StrEqual(Weapons, "weapon_scar20") || StrEqual(Weapons, "weapon_g3sg1"))
			Knockback(client, 1300.0);
		else if (StrEqual(Weapons, "weapon_revolver") || StrEqual(Weapons, "weapon_deagle"))
			Knockback(client, 450.0);
		else if (StrEqual(Weapons, "weapon_fists"))
			Knockback(client, 700.0);
		else if (StrEqual(Weapons, "weapon_tablet"))
			Knockback(client, 1500.0);
		else if (slot < 2 || slot > 4)
			Knockback(client, 135.0);
	}
	if (StrEqual(InfAmmo, "1"))
	{
		int buttons = GetClientButtons(client);
		int weapon = GetEntPropEnt(client, Prop_Send, "m_hActiveWeapon");
		if (CSGOItems_IsValidWeapon(weapon) && (buttons & IN_ATTACK || buttons & IN_ATTACK2))
		{
			int slot = CSGOItems_GetWeaponSlotByWeapon(CSGOItems_GetActiveWeapon(client));
			if (slot != CS_SLOT_GRENADE)
				CSGOItems_SetWeaponAmmo(weapon, 999, GetEntProp(weapon, Prop_Send, "m_iClip1") + 1);
		}
	}
	if (frThrowKnifePlayer[client])
	{
		if (StrContains(Weapons, "knife", false) != -1 || StrContains(Weapons, "bayonet", false) != -1)
		{
			RequestFrame(ThrowingKnife, client);
		}
	}
	if (IsValidClient(client) && IsPlayerAlive(client) && IsAimbotavailable(client))
	{
		SetClientEyePos(client);
	}
	
}

public Action Fun_EventItemPickup(Event event, const char [] name, bool dontBroadcast)
{
	if (!cvPluginEnable.BoolValue) return;
	int client = GetClientOfUserId(event.GetInt("userid"));
	if (frIsGoingSuicide[client]) DropPlayerWeapons(client);
}
public Action Fun_EventBulletImpact(Event event, const char[] name, bool dontBroadcast)
{
	if (cvPluginEnable.BoolValue)
	{
		int client = GetClientOfUserId(event.GetInt("userid"));
		float impact_pos[3];
		impact_pos[0] = event.GetFloat("x");
		impact_pos[1] = event.GetFloat("y");
		impact_pos[2] = event.GetFloat("z");
		if (!StrEqual(TeslaBullet, "0"))
		{
			FireTeslaBullet(client, impact_pos);
		}
		float expbulletcd = StringToFloat(ExplodeBullet);
		if ((frExplodeBulletClient[client] || (!StrEqual(ExplodeBullet, "-1") && expbulletcd >= 0.0 )) && !frExplodeBulletCD[client])
		{
			int weapon = GetEntPropEnt(client, Prop_Send, "m_hActiveWeapon");
			char classname[32];
			GetEntityClassname(weapon, classname, sizeof(classname));
			CS_CreateExplosion(client, Math_GetRandomInt(75, 180), Math_GetRandomInt(225, 450), impact_pos, classname);
			if (expbulletcd > 0.0)
			{
				frExplodeBulletCD[client] = true;
				CreateTimer(expbulletcd, Timer_ExplosiveRechange, GetClientUserId(client));
			}
		}
	}
}
public Action Fun_BulletShot(const char[] te_name, const int[] Players, int numClients, float delay)
{
	if (cvPluginEnable.BoolValue)
	{
		int client = TE_ReadNum("m_iPlayer") + 1;
		if (!StrEqual(TeslaBullet, "0"))
		{
			float origin[3];
			TE_ReadVector("m_vecOrigin", origin);
			g_fLastAngles[client][0] = TE_ReadFloat("m_vecAngles[0]");
			g_fLastAngles[client][1] = TE_ReadFloat("m_vecAngles[1]");
			g_fLastAngles[client][2] = 0.0;
			
			float impact_pos[3];
			Handle trace = TR_TraceRayFilterEx(origin, g_fLastAngles[client], MASK_SHOT, RayType_Infinite, TR_DontHitSelf, client);
			if (TR_DidHit(trace))
			{
				TR_GetEndPosition(impact_pos, trace);
			}
			delete trace;
		}
	}
}

public Action Fun_EventPlayerBlind(Event event, const char[] name, bool dontBroadcast)
{
	int client = GetClientOfUserId(event.GetInt("userid"));
	if (IsValidClient(client) && IsPlayerAlive(client)) PlayBlindSound(client);
}

void GiveWeapons()
{
	if (StrEqual(Weapon, "none") || StrEqual(WeaponFilter, "none")) return;
	
	for (int client = 1; client <= MaxClients; ++client)
	{
		if (IsValidClient(client) && IsPlayerAlive(client))
		{
			if (StrEqual(NoKnife, "0") && !StrEqual(Weapon, "none"))
				EquipPlayerWeapon(client, GivePlayerItem(client, "weapon_knife"));
		}
	}
	int team;
	
	if (StrEqual(WeaponFilter, "all"))
		team = 1;
	if (StrEqual(WeaponFilter, "t"))
		team = 2;
	if (StrEqual(WeaponFilter, "ct"))
		team = 3;
	if (0 < team < 4) GivePlayersWeapon(team);
}
void GivePlayersWeapon(int Team)
{
	char bit[10][200];
	int Sum = ExplodeString(Weapon, ";", bit, sizeof(bit), sizeof(bit[]));
	if (StrEqual(Weapon, "random"))
	{
		for (int client = 1; client <= MaxClients; ++client)
		{
			if (IsValidClient(client) && IsPlayerAlive(client) && TeamCheck(client, Team))
			{
				EquipRandomWeapon(client, 2);
			}
		}
	}
	for (int i = 0; i < Sum; ++i)
	{
		if (StrEqual(bit[i], "weapon_primary_random") || StrEqual(bit[i], "weapon_secondary_random") || StrEqual(bit[i], "other_equipment_random"))
		{
			if (StrEqual(bit[i], "weapon_primary_random"))
			{
				int random = Math_GetRandomInt(0, 24);
				CurrentRoundWeapon[0] = random;
				Weapon_Random(0, random, Team);
			}
			if (StrEqual(bit[i], "weapon_secondary_random"))
			{
				int random = Math_GetRandomInt(0, 10);
				CurrentRoundWeapon[1] = random;
				Weapon_Random(1, random, Team);
			}
			if (StrEqual(bit[i], "other_equipment_random"))
			{
				int random = Math_GetRandomInt(0, 21);
				CurrentRoundWeapon[2] = random;
				Weapon_Random(2, random, Team);
			}
		}
		else
		{
			for (int client = 1; client <= MaxClients; ++client)
			{
				if (IsValidClient(client) && IsPlayerAlive(client) && TeamCheck(client, Team))
				{
					if (StrEqual(bit[i], "weapon_knife"))
						GivePlayerItem(client, "weapon_knife");
					else if (StrContains(bit[i], "weapon_") != -1)
						EquipPlayerWeapon(client, CSGOItems_GiveWeapon(client, bit[i]));
				}
			}
		}
	}
}

void SetSpeed(int client, float value = 1.0, bool ignoreGlobalSpeed = false)
{
	if (IsValidClient(client) && IsPlayerAlive(client))
	{
		if (!ignoreGlobalSpeed)
		{
			value *= StringToFloat(PSpeed);
		}
		if (value <= 0.0) value = 0.1;
		if (frIsGhostClient[client]) value *= 1.05;
		SetEntPropFloat(client, Prop_Data, "m_flLaggedMovementValue", value * frPlayerSpeedMultiplier[client]);
	}
}

void RemoveAllWeapon()
{
	for (int client = 1; client < MaxClients; ++client)
	{
		if (IsValidClient(client) && IsPlayerAlive(client)) //maybe need  GameRules_GetProp("m_bFreezePeriod") == 1
		{
			DisarmArmor(client);
			RemoveWeapons(client);
			RemoveNades(client);
		}
	}
}
void RemoveWeapons(int client)
{
	CSGOItems_RemoveAllWeapons(client);
	DisarmPlayerWeapons(client);
	if (StrEqual(NoKnife, "0")) EquipPlayerWeapon(client, GivePlayerItem(client, "weapon_knife"));
}
void RemoveNades(int client)
{
	while (RemoveWeaponBySlot(client, 3))
	{
	}
	for (int nades = 0; nades < 6; ++nades)
	{
		SetEntProp(client, Prop_Send, "m_iAmmo", 0, _, GrenadesAll[nades]);
	}
}
void EnableThirdPerson()
{
	for (int client = 1; client <= MaxClients; ++client)
	{
		if (IsValidClient(client) && IsPlayerAlive(client) && !IsFakeClient(client))
		{
			ClientCommand(client, "thirdperson");
			KillTimerHandle(frThirdperson);
			frThirdperson = CreateTimer(0.1, Timer_Thirdperson, _, TIMER_REPEAT);
			/* maybe need
			 * SetEntProp(client, Prop_Send, "m_iObserverMode", 1);
			 * SetEntPropEnt(client, Prop_Send, "m_hObserverTarget", 0);
			 * SetEntProp(client, Prop_Send, "m_bDrawViewmodel", 0);
			 * SetEntProp(client, Prop_Send, "m_iFOV", 120);
			 */
		}
	}
}
public Action Timer_Thirdperson(Handle timer)
{
	for (int client = 1; client <= MaxClients; ++client)
	{
		if (IsValidClient(client) && IsPlayerAlive(client) && !IsFakeClient(client) && StrEqual(ThirdPerson, "1"))
		{
			ClientCommand(client, "thirdperson");
		}
	}
}

public void OnEntityCreated(int entity, const char [] classname)
{
	if (StrContains(classname, "chicken") != -1 && IsValidEntity(entity))
	{
		//SetModelScaleRandom(entity);
		HookSingleEntityOutput(entity, "OnBreak", OnChickenDeath);
		SDKHook(entity, SDKHook_Use, OnEntityUse);
		SDKHook(entity, SDKHook_OnTakeDamage, ChickenDamaged);
		//SetEntPropFloat(entity, Prop_Data, "m_flLaggedMovementValue", 5.0);
	}
	if (StrEqual(classname, "weapon_hegrenade"))
	{
		SDKHook(entity, SDKHook_Spawn, OnEntitySpawned);
	}
	if (StrContains(classname, "_projectile") != -1)
	{
		SDKHook(entity, SDKHook_Spawn, OnEntitySpawned);
		SDKHook(entity, SDKHook_StartTouch, OnEntityTouch);
	}
	if (StrContains(classname, "weapon_") != -1)
	{
		SDKHookEx(entity, SDKHook_Reload, OnReload);
	}
	if (StrEqual(classname, "func_bomb_target", false))
    {
        SDKHookEx(entity, SDKHook_TouchPost, TouchPost);
    }
}
void OnChickenDeath(const char[] output, int caller, int activator, float delay)
{
	AcceptEntityInput(caller, "Deactivate");
	AcceptEntityInput(caller, "Kill");
	EmitSoundToAll("weapons/hegrenade/explode3.wav", caller);
}
public Action OnEntityUse(int entity, int activator, int caller, UseType type, float value)
{
	EmitSoundToAll(ChickenkillSounds[0], activator, Math_GetRandomInt(10, 120));
	if (entity == ChickenC4)
	{
		PrintToChat(activator, "[SM] 背C4的鸡不能用来吃！");
		return Plugin_Stop;
	}
	//ChickenEater
	if (cvChickenEater.BoolValue && bChickenEater)
	{
		if (IsValidClient(activator) && IsPlayerAlive(activator))
		{
			if (!StrEqual(Chicken, "0")) return Plugin_Stop;
			if (!frAteChicken[activator])
			{
				EatChicken(entity, activator);
				KillTimerHandle(frChickenEating[activator]);
				frChickenEating[activator] = CreateTimer(15.0, Timer_EatingChicken, GetClientUserId(activator), TIMER_REPEAT);
				PrintToChat(activator, "[SM] \x3 \x4提示：您已经吃饱了，消化需要一定时间，强行吃的话可能会掉血甚至撑死");
				return Plugin_Handled;
			}
			int random = Math_GetRandomInt(1, 100) + (frAteChickenCount[activator] * 5);
			if (random <= 50)
			{
				EatChicken(entity, activator);
			}
			else if (50 < random <= 85)
			{
				AcceptEntityInput(entity, "Deactivate");
				AcceptEntityInput(entity, "Kill");
				++frAteChickenCount[activator];
				int damage = RoundToFloor(GetHealth(activator) * GetRandomFloat(0.10, 0.25)) * frAteChickenCount[activator];
				if (damage < 1) damage = 1;
				SDKHooks_TakeDamage(activator, entity, activator, damage + 0.0, CS_DMG_HEADSHOT);
				PrintToChat(activator, "[SM] \x3 \x4由于吃得太撑掉了 %i 血", damage);
			}
			else if (85 < random < 95)
			{
				AcceptEntityInput(entity, "Deactivate");
				AcceptEntityInput(entity, "Kill");
				CreateDamage(0.0, activator, activator, entity, (GetHealth(activator) + 0.00) * 10, CS_DMG_HEADSHOT, entity, true);
				char name[128];
				GetClientName(activator, name, sizeof(name));
				PrintToChatAll("[SM] \x3 \x4%s 吃鸡撑死了", name);
			}
			else
			{
				SetEntPropFloat(entity, Prop_Data, "m_explodeDamage", 2000.0);
				SetEntPropFloat(entity, Prop_Data, "m_explodeRadius", 1000.0);
				AcceptEntityInput(entity, "Break");
				RemoveEntity(entity);
				CreateDamage(0.0, activator, activator, entity, (GetHealth(activator) + 0.00) * 10, CS_DMG_HEADSHOT, entity, true);
				char name[128];
				GetClientName(activator, name, sizeof(name));
				PrintToChatAll("[SM] \x3 \x4%s 吃鸡时不小心按到了小鸡的自爆开关，让我们为他默哀3秒钟", name);
			}
		}
	}
	else
		PrintToChat(activator, "[SM] 吃鸡功能当前不可用");
	return Plugin_Stop;
}
public Action TouchPost(int entity, int client)
{
	if (IsValidClient(client) && StrEqual(DefuseMode, "none"))
		SetEntProp(client, Prop_Send, "m_bInBombZone", 1);
}
public Action OnEntitySpawned(int entity)
{
	int client = GetEntPropEnt(entity, Prop_Send, "m_hOwnerEntity");
	char classname[64];
	GetEntityClassname(entity, classname, sizeof(classname));
	if (bInfNade)
	{
		if (IsValidClient(client) && IsPlayerAlive(client))
		{
			int nadeslot = GetPlayerWeaponSlot(client, 3);
			if (nadeslot > 0)
			{
				CSGOItems_RemoveWeapon(client, nadeslot);
				RemovePlayerItem(client, nadeslot);
				AcceptEntityInput(nadeslot, "Kill");
				//RemoveEdict(nadeslot);
			}
			//EquipPlayerWeapon(client, GivePlayerItem(client, InfNade));
			RequestFrame(GiveNade, client);
		}
	}
	if (bDodgeBall)
	{
		if (!client || !IsValidClient(client) || !IsPlayerAlive(client))
		{
			AcceptEntityInput(entity, "Kill");
		}
	}
	if (StrEqual(classname, "weapon_hegrenade"))
	{
		SetEntProp(entity, Prop_Data, "m_takedamage", 2);
		SDKHook(entity, SDKHook_OnTakeDamage, OnGrenadeTakeDamage);
	}
}

public Action OnGrenadeTakeDamage(int victim, int &attacker, int &inflictor, float &damage, int &damagetype, int &weapon, float damageForce[3], float damagePosition[3])
{
	if (IsValidEntity(victim) && IsValidClient(attacker) && IsClientInGame(attacker))
	{
		CreateGrenadeExplosion(victim, attacker);
	}
	return Plugin_Continue;
}

public Action OnEntityTouch(int entity, int other)
{
	if (bDodgeBall)
	{
		int client = GetEntPropEnt(other, Prop_Send, "m_hOwnerEntity");
		if (!IsValidClient(client))
		{
			KillEntity(entity);
			return;
		}
		++iDodgeBallCount[entity];
		if (iDodgeBallCount[entity] >= 2)
			KillEntity(entity);
	}
}
public Action CS_OnTerminateRound(float& delay, CSRoundEndReason& reason)
{
	int winner = GetWinner(view_as<int>(reason));
	RoundEndSound(winner);
}

stock bool KillEntity(int entity)
{
	if (IsValidEntity(entity))
	{
		AcceptEntityInput(entity, "kill");
		iDodgeBallCount[entity] = 0;
	}
}
stock void SetNoScope(int weapon)
{
	if (IsValidEdict(weapon))
	{
		char classname[MAX_NAME_LENGTH];
		GetEdictClassname(weapon, classname, sizeof(classname));
		if (StrEqual(classname[7], "ssg08") || StrEqual(classname[7], "aug") || StrEqual(classname[7], "sg550") || StrEqual(classname[7], "sg552") || StrEqual(classname[7], "sg556") || StrEqual(classname[7], "awp") || StrEqual(classname[7], "scar20") || StrEqual(classname[7], "g3sg1"))
		{
			SetEntDataFloat(weapon, FindSendPropInfo("CBaseCombatWeapon", "m_flNextSecondaryAttack"), GetGameTime() + 9999.9);
		}
	}
}
public Action RemoveGodMode(Handle timer, any userid)
{
	int client = GetClientOfUserId(userid);
	frGodMode[client] = false;
}
public Action Timer_SpeedChange(Handle timer)
{
	if (!bSpeedChange) return Plugin_Stop;
	int motion = Math_GetRandomInt(0, 5);
	float rSpeed = Math_GetRandomInt(1, 10) + 0.0;
	float rGravity = GetRandomFloat(0.001, 4.000);
	for (int client = 1; client < MaxClients; ++client)
	{
		if (IsValidClient(client) && IsPlayerAlive(client))
		{
			if (motion == 0)
			{
				SetSpeed(client, 0.3, true);
			}
			if (motion == 1)
			{
				SetSpeed(client, 0.7, true);
			}
			if (motion >= 2)
			{
				SetSpeed(client, rSpeed, true);
			}
			SetEntityGravity(client, rGravity);
		}
	}
	return Plugin_Continue;
}
public Action SoundHook(int clients[64], int &numClients, char sample[PLATFORM_MAX_PATH], int &entity, int &channel, float &volume, int &level, int &pitch, int &flags)
{
	if (IsValidClient(entity) && (StrContains(sample, "physics") != -1 || StrContains(sample, "footsteps") != -1))
	{
		if (IsPlayerAlive(entity) && frIsChicken[entity])
		{
			return Plugin_Handled;
		}
	}
	return Plugin_Continue;
}
public Action Timer_CleanMapWeapons(Handle timer)
{
	static int count = 0;
	if (count >= 5)
	{
		count = 0;
		return Plugin_Stop;
	}
	for (int client = 1; client <= MaxClients; ++client)
	{
		if (IsValidClient(client) && IsPlayerAlive(client))
		{
			DisarmPlayerWeapons(client);
		}
	}
	char classname[128];
	for(int entity = MaxClients; entity < GetMaxEntities(); ++entity)
	{
		if(IsValidEntity(entity))
		{
			GetEntityClassname(entity, classname, sizeof(classname));
			if ((StrContains(classname, "weapon_", false) != -1) && (GetEntProp(entity, Prop_Data, "m_iState") == 0) && (GetEntProp(entity, Prop_Data, "m_spawnflags") != 1))
			{
				AcceptEntityInput(entity, "Kill");
				//RemoveEntity(entity);
			}
		}
	}
	++count;
	return Plugin_Continue;
}

public Action Drop(int client, const char[] command, int argc)
{
	if (IsValidClient(client) && IsPlayerAlive(client))
	{
		char Weapons[128];
		int weapon = GetEntPropEnt(client, Prop_Send, "m_hActiveWeapon");
		if (CSGOItems_IsValidWeapon(weapon))
		{
			GetEdictClassname(weapon, Weapons, sizeof(Weapons));
			
			//if (StrEqual(Weapons, "weapon_hegrenade", false) || StrEqual(Weapons, "weapon_flashbang", false) || StrEqual(Weapons, "weapon_smokegrenade", false) || StrEqual(Weapons, "weapon_incgrenade", false) || StrEqual(Weapons, "weapon_molotov", false) || StrEqual(Weapons, "weapon_decoy", false) || StrEqual(Weapons, "weapon_tagrenade", false))
			if (StrEqual(Weapons, "weapon_fists"))
			{
				if (GetEntProp(weapon, Prop_Data, "m_nSequence") != 2)
				{
					SDKHooks_DropWeapon(client, weapon, NULL_VECTOR, NULL_VECTOR);
					SetModelScaleRandom(weapon);
					return Plugin_Handled;
				}
			}			
		}
	}
	return Plugin_Continue;
}

void CheckHealthPercent(int client)
{
	if (StrEqual(Health, "random") || bSpeedChange || frIsGoingSuicide[client] || frRoundEnd || bBossEvolve) return;
	float HealthPercent = GetPlayerHealthPercent(client);
	if (HealthPercent < 10)
	{
		frPlayerDamageMultiplier[client] = 0.6925;
		frPlayerSpeedMultiplier[client] = 0.385;
		
	}
	else if (10 <= HealthPercent < 55)
	{
		frPlayerDamageMultiplier[client] = 0.86;
		frPlayerSpeedMultiplier[client] = 0.68;
	}
	else if (55 <= HealthPercent < 133)
	{
		frPlayerDamageMultiplier[client] = 1.0;
		frPlayerSpeedMultiplier[client] = 1.0;
	}
	else
	{
		int Multiplier = RoundToFloor(HealthPercent / 33) - 4;
		frPlayerDamageMultiplier[client] = 1.0 + Multiplier * 0.02;
		frPlayerSpeedMultiplier[client] = 1.0 + Multiplier * 0.01;
		if (frPlayerDamageMultiplier[client] > 6.0) frPlayerDamageMultiplier[client] == 6.0;
		if (frPlayerSpeedMultiplier[client] > 5.0) frPlayerSpeedMultiplier[client] == 5.0;
	}
	SetSpeed(client);
}
float GetPlayerHealthPercent(int client)
{
	return (GetHealth(client) + 0.00) / StringToFloat(Health) * 100;
}

public Action Timer_HealthLeak(Handle timer)
{
	if (!frRoundEnd)
	{
		int BaseHealth = StringToInt(Health);
		for (int client = 1; client <= MaxClients; ++client)
		{
			if (IsValidClient(client) && IsPlayerAlive(client))
			{
				int hp = GetHealth(client);
				if (hp > BaseHealth)
				{
					hp = RoundToCeil((hp + 0.0) * 0.92);
					if (hp < BaseHealth) hp = BaseHealth;
					SetHealth(client, hp);
				}
			}
		}
	}
}

public Action Timer_BotUseWeapon(Handle timer, DataPack data)
{
	data.Reset();
	int client = data.ReadCell();
	int weapon = data.ReadCell();
	if (IsValidClient(client) && IsPlayerAlive(client) && IsFakeClient(client) && CSGOItems_IsValidWeapon(weapon) && botweapon[weapon])
	{
		char Weapons[48];
		CSGOItems_GetWeaponClassNameByWeapon(weapon, Weapons, sizeof(Weapons));
		EquipPlayerWeapon(client, weapon);
		FakeClientCommand(client, "use %s", Weapons);
		SetActiveWeapon(client, weapon);
		botweapon[weapon] = false;
	}
}

public Action Command_NadeBug(int client, int args)
{
	if (IsValidClient(client) && IsPlayerAlive(client))
	{
		if (bInfNade)
		{
			int nade = GetPlayerWeaponSlot(client, 3);
			if (nade > 0)
			{
				PrintToChat(client, "[SM] 您现在有投掷物，不需要使用此命令");
				return Plugin_Handled;
			}
			else
			{
				RemoveWeaponBySlot(client, 3);
				RequestFrame(GiveNade, client);
				return Plugin_Handled;
			}
		}
		else
		{
			PrintToChat(client, "[SM] 当前非投掷物相关模式，不需要使用此命令");
			return Plugin_Handled;
		}
	}
	return Plugin_Handled;
}


public Action Timer_EatingChicken(Handle timer, any userid)
{
	int client = GetClientOfUserId(userid);
	if (IsValidClient(client) && IsPlayerAlive(client))
	{
		if (frAteChickenCount[client] > 0)
		{
			--frAteChickenCount[client];
			if (frAteChickenCount[client] == 0)
			{
				frAteChicken[client] = false;
				PrintToChat(client, "[SM] \x3 \x4你饿了，想再吃一只鸡。");
				RequestFrame(ClearChickenTimer, client);
			}
			else
			{
				PrintToChat(client, "[SM] \x3 \x4看起来你消化掉了一只鸡。");
			}
		}
	}
}
void ClearChickenTimer(int client)
{
	KillTimerHandle(frChickenEating[client]);
}

public Action Command_FRCommands(int client, int args)
{
	Menu menu = new Menu(FrMenu, MenuAction_End);
	menu.SetTitle("可用命令列表:");
	if (cvEnableBuyC4.BoolValue && bBuyC4 && (GetClientTeam(client) == CS_TEAM_T))
		menu.AddItem("sm_c4", "购买c4 !c4");
	if (cvSuicideBomber.BoolValue && bSuicide && IsPlayerAlive(client))
	{
		if (frIsSuicideBomber[client] && !frIsGoingSuicide[client])
			menu.AddItem("fr_suicide", "开启自爆");
		else if (!frIsSuicideBomber[client] && GetClientTeam(client) == CS_TEAM_T && (CS_GetClientContributionScore(client) >= 5 || GetMoney(client) >= 10000))
			menu.AddItem("fr_suicide", "自动购买c4并开启自爆");
	}
	if (bChrono)
		menu.AddItem("sm_tpbug", "!tpbug 瞬移模式中不小心瞬移到空气墙中使用");
	if (bInfNade)
		menu.AddItem("sm_nade", "!nade 在投掷物相关模式中没有投掷物时使用");
	if (cvKnockBack.BoolValue && cvPluginEnable.BoolValue)
		menu.AddItem("sm_knock", "开关开枪击退效果 !knock");
	menu.AddItem("sm_healthshot", "购买医疗针 !healthshot 4000金钱或者2分数");
	menu.AddItem("sm_heavyarmor", "购买重甲 !heavyarmor 16000金钱");
	menu.AddItem("sm_chicken", "获得一只鸡 !chicken 1分数或者1金钱");
	menu.AddItem("sm_aimbot", "开关自瞄模式中的自瞄效果 !aimbot");
	menu.AddItem("sm_nvg", "开关夜视仪 !nvg");
	menu.AddItem("sm_shield", "购买盾牌 1000金钱或者2分数 !shield");
	menu.AddItem("sm_bump", "购买弹射地雷 !bump 500金钱或者2分数");
	menu.AddItem("sm_tablet", "购买特训助手 !tablet");
	menu.AddItem("fr_togglesound", "开启/关闭特色音效");
	
	menu.ExitButton = true;
	menu.Display(client, 20);
	
	return Plugin_Handled;
}

stock void SetArmorValue(int client, int ap = -1, bool helmet = false, bool heavy = false)
{
	if (IsValidClient(client) && IsPlayerAlive(client))
	{
		if (heavy)
		{
			SetEntProp(client, Prop_Send, "m_bHasHeavyArmor", 1);
			SetEntProp(client, Prop_Send, "m_bWearingSuit", 1);
		}
		if (ap > 255)
		{
			frArmorValue[client] = ap - 255;
			ap = 255;
		}
		if (!helmet)
		{
			helmet = IsPlayerHasHelmet(client);
		}
		SetPlayerArmor(client, ap, helmet);
	}
}

stock int GetArmorValue(int client)
{
	int ap = -1;
	if (IsValidClient(client) && IsPlayerAlive(client))
	{
		ap = GetEntProp(client, Prop_Send, "m_ArmorValue");
		if (frArmorValue[client] > 0)
			ap += frArmorValue[client];
	}
	return ap;
}


void CheckArmorValue(int client)
{
	int ap = GetEntProp(client, Prop_Send, "m_ArmorValue");
	if (frArmorValue[client] > 0)
	{
		ap = 255 + (255 - ap + frArmorValue[client]);
	}
	SetArmorValue(client, ap);
}


//Dead Player Explode
void DeathExplodeInit()
{
	PreCacheSoundAndDownload(ExplodeSound);
	ExplosionSprite = PrecacheModel("sprites/blueglow2.vmt");
	AddFileToDownloadsTable("materials/sprites/blueglow2.vtf");
	AddFileToDownloadsTable("materials/sprites/blueglow2.vmt");
	SmokeSprite = PrecacheModel("sprites/steam2.vmt");
	AddFileToDownloadsTable("materials/sprites/steam2.vtf");
	AddFileToDownloadsTable("materials/sprites/steam2.vmt");
}

void CreateDeathExplode(int client)
{
	float vec[3];
	GetClientAbsOrigin(client, vec);
	
	static float normalvec[3] =  { 0.0, 0.0, 1.0 };
	TE_SetupExplosion(vec, ExplosionSprite, 5.0, 1, 0, 50, 40, normalvec);
	TE_SendToAll();
		
	TE_SetupSmoke(vec, SmokeSprite, 10.0, 3);
	TE_SendToAll();
	EmitAmbientSound(ExplodeSound, vec, client);
}

public Action CS_OnBuyCommand(int client, const char[] weapon)
{
	if (IsValidClient(client) && IsPlayerAlive(client))
	{
		int money = GetMoney(client);
		int price = 0;
		char classname[PLATFORM_MAX_PATH];
		if (StrEqual(weapon, "defuser") && GetClientTeam(client) == CS_TEAM_CT && GetEntProp(client, Prop_Send, "m_bHasDefuser") == 0)
		{
			price = 400;
			if (money >= price)
			{
				SetMoney(client, money - price);
				GivePlayerItem(client, "item_defuser");
			}
		}
		if (StrContains(weapon, "vest", false))
		{
			int ap = GetArmorValue(client);
			if (StrEqual(weapon, "vesthelm"))
			{
				if (IsPlayerHasHelmet(client))
				{
					if (ap < 100)
					{
						price = 650;
						ap = 100;
					}
					if (money >= price)
					{
						 SetMoney(client, money - price);
						 SetArmorValue(client, ap, true);
					}
				}
				else
				{
					if (ap < 100)
					{
						ap = 100;
						price = 1000;
					}
					else price = 350;
					if (money >= price)
					{
						 SetMoney(client, money - price);
						 SetArmorValue(client, ap, true);
					}
				}
				
			}
			else
			{
				if (ap < 100)
				{
					ap = 100;
					price = 650;
				}
				if (money >= price)
				{
					 SetMoney(client, money - price);
					 SetArmorValue(client, ap, true);
				}
			}
		}
		for (int num = 0; num < 36; ++num)
		{
			if (StrEqual(frBuyMenuWeapons[num], weapon))
			{
				Format(classname, sizeof(classname), "weapon_%s", weapon);
				price = CS_GetWeaponPrice(client, CS_AliasToWeaponID(classname));
				if (money >= price)
				{
					SetMoney(client, money - price);
					GivePlayerItem(client, classname);
					return Plugin_Stop;
				}
			}
		}
		return Plugin_Stop;
	}
	return Plugin_Continue;
}