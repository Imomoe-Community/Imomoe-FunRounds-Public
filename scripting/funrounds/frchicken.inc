char ChickenModel[] = "models/chicken/chicken.mdl";
char ChickenDeathSounds[][] =  { "ambient/creatures/chicken_death_01.wav", "ambient/creatures/chicken_death_02.wav", "ambient/creatures/chicken_death_03.wav" };
char frChickensec[][] =  { "ACT_WALK", "ACT_RUN", "ACT_IDLE", "ACT_JUMP", "ACT_GLIDE", "ACT_LAND", "ACT_HOP" };
char frChickenIdleSounds[][] =  { "ambient/creatures/chicken_idle_01.wav", "ambient/creatures/chicken_idle_02.wav", "ambient/creatures/chicken_idle_03.wav" };
char frChickenPanicSounds[][] =  { "ambient/creatures/chicken_panic_01.wav", "ambient/creatures/chicken_panic_02.wav", "ambient/creatures/chicken_panic_03.wav", "ambient/creatures/chicken_panic_04.wav" };

int frChickens[MAXPLAYERS + 1];
int frFeathersParticles[MAXPLAYERS + 1];
int frLastFlags[MAXPLAYERS + 1];
int frFlyCounter[MAXPLAYERS + 1];
int frCweapons[MAXPLAYERS];
int frChickenNumber;

bool frIsChicken[MAXPLAYERS + 1];
bool frCWasIdle[MAXPLAYERS + 1] = false;
bool frCWasRunning[MAXPLAYERS + 1] = false;
bool frCIsWalking[MAXPLAYERS + 1] = false;
bool frCWasWalking[MAXPLAYERS + 1] = false;
bool frCIsMoving[MAXPLAYERS + 1] = false;
bool frChickenSpawnOrigin = false;
bool frChickenSoundCD[MAXPLAYERS + 1] = false;

Handle frAnimationsTimer[MAXPLAYERS + 1];
Handle frFeathersTimer[MAXPLAYERS + 1];

char frPlayersModels[MAXPLAYERS + 1][PLATFORM_MAX_PATH];
const float frChickenRunSpeed = 102.0;
const float frChickenWalkSpeed = 6.5;
const float maxFallSpeed = -100.0;
void AllChickenStatus(bool status)
{
	for (int i = 1; i <= MaxClients; ++i)
	{
		if (IsValidClient(i) && !frIsChicken[i] && GetClientTeam(i) != CS_TEAM_SPECTATOR && GetClientTeam(i) != CS_TEAM_NONE && status == true)
			ChickenStatus(i, status);
		else if (IsValidClient(i) && frIsChicken[i])
			ChickenStatus(i, status);
	}
	NameStatus(status);
}
void ChickenStatus(int client, bool status)
{
	frIsChicken[client] = status;
	DisableFakeModel(client);
	if (status)
	{
		
		
		char ModelName[PLATFORM_MAX_PATH];
		GetEntPropString(client, Prop_Data, "m_ModelName", ModelName, sizeof(ModelName));
		frPlayersModels[client] = ModelName;
		SetEntityModel(client, ChickenModel);
		//SDKHook(client, SDKHook_SetTransmit, Hook_SetTransmit);
		SetEntityRenderMode(client, RENDER_NONE);
		CreateFakeModel(client);
		
	}
	else
	{
		//SDKUnhook(client, SDKHook_SetTransmit, Hook_SetTransmit);
		SetEntityRenderMode(client, RENDER_NORMAL);
		KillTimerHandle(frAnimationsTimer[client]);
		//SetClientViewEntity(client, client);
		//float eyepos[3];
		//GetClientEyePosition(client, eyepos);
		//TeleportEntity(client, NULL_VECTOR, eyepos, NULL_VECTOR);
		frLastFlags[client] = 0;
		frFlyCounter[client] = 0;
		frCWasRunning[client] = status;
		frCWasIdle[client] = status;
		frCWasWalking[client] = status;
		DeleteFakeWeapon(client);
		ChickenDeath(client);
		SetEntityModel(client, frPlayersModels[client]);
	}
	SetViewModel(client, !status);
}
void CreateFakeModel(int client)
{
	frChickens[client] = CreateEntityByName("prop_dynamic_override");
	if (IsValidEntity(frChickens[client]))
	{
		SetEntityModel(frChickens[client], ChickenModel);
		SetVariantString("!activator");
		AcceptEntityInput(frChickens[client], "SetParent", client, frChickens[client], 0);
		float pos[3], rot[3];
		TeleportEntity(frChickens[client], pos, rot, NULL_VECTOR);
		DispatchKeyValue(frChickens[client], "solid", "0");
		DispatchSpawn(frChickens[client]);
		ActivateEntity(frChickens[client]);
		SetVariantString(frChickensec[2]); 
		AcceptEntityInput(frChickens[client], "SetAnimation");
		//SetClientViewEntity(client, frChickens[client]);
		KillTimerHandle(frAnimationsTimer[client]);
		frAnimationsTimer[client] = CreateTimer(0.1, Timer_ChickenAnim, GetClientUserId(client), TIMER_REPEAT);
	}
}
void CreateFakeWeapon(int client, int weapon)
{
	DeleteFakeWeapon(client);
	char weapon_name[32];
	GetEdictClassname(weapon, weapon_name, sizeof weapon_name);
	
	if (!StrEqual(weapon_name, "weapon_knife", false))
	{
		frCweapons[client] = CreateEntityByName("prop_dynamic_override");
		if (IsValidEntity(frCweapons[client]))
		{
			SetVariantString("!activator");
			AcceptEntityInput(frCweapons[client], "SetParent", client, frCweapons[client], 0);
			SetModel(client, weapon, weapon_name);
			//Make sure the gun is not solid
			DispatchKeyValue(frCweapons[client], "solid", "0");
			//Spawn it!
			DispatchSpawn(frCweapons[client]);
			ActivateEntity(frCweapons[client]);
		}
	}
}
void DeleteFakeWeapon(int client)
{
	if (IsValidEntity(frCweapons[client]) && frCweapons[client] > MAXPLAYERS)
	{
		AcceptEntityInput(frCweapons[client], "Deactivate");
		AcceptEntityInput(frCweapons[client], "Kill" );
		//RemoveEdict(frCweapons[client]);
		frCweapons[client] = -1;
	}
}
void SetModel(int client, int weapon, char[] classname)
{
	if (StrEqual(classname, "weapon_smokegrenade", false) || StrEqual(classname, "weapon_decoy", false) || StrEqual(classname, "weapon_tagrenade", false) || StrEqual(classname, "weapon_molotov", false) || StrEqual(classname, "weapon_incgrenade", false) || StrEqual(classname, "weapon_hegrenade", false))
	{
		SetWeaponPos(client, 1);
	}
	else if (StrEqual(classname, "weapon_healthshot", false))
	{
		char modelName[128];
		GetEntPropString(weapon, Prop_Data, "m_ModelName", modelName, sizeof(modelName));
		SetEntityModel(frCweapons[client], modelName);
		SetWeaponPos(client, 2);
	}
	else
	{
		char modelName[128];
		GetEntPropString(weapon, Prop_Data, "m_ModelName", modelName, sizeof(modelName));
		SetEntityModel(frCweapons[client], modelName);
		SetWeaponPos(client, 0);
	}
}
void SetWeaponVisibility(int client, int weapon, bool enabled)
{
	if (!IsValidClient(client)) return;
	if (weapon != -1)
	{
		int worldModel = GetEntPropEnt(weapon, Prop_Send, "m_hWeaponWorldModel");
		if (worldModel != -1)
		{
			if (!enabled)
				SDKHook(worldModel, SDKHook_SetTransmit, Hook_WeaponSetTransmit);
			else
				SDKUnhook(worldModel, SDKHook_SetTransmit, Hook_WeaponSetTransmit);
		}
	}
}

void DisableFakeModel(int client)
{
	if (frChickens[client] != 0 && IsValidEdict(frChickens[client]))
	{
		AcceptEntityInput(frChickens[client], "Deactivate");
		AcceptEntityInput(frChickens[client], "Kill" );
		//RemoveEdict(frChickens[client]);
		frChickens[client] = 0;
	}
}
void ChickenDeath(int client)
{
	int rdmSound = GetRandomInt(0, 2);
	EmitSoundToAll(ChickenDeathSounds[rdmSound], client);
	float pos[3];
	frFeathersParticles[client] = CreateEntityByName("info_particle_system");
	DispatchKeyValue(frFeathersParticles[client], "effect_name", "chicken_gone_feathers");
	DispatchKeyValue(frFeathersParticles[client], "angles", "-90 0 0");
	GetClientAbsOrigin(client, pos);
	TeleportEntity(frFeathersParticles[client], pos, NULL_VECTOR, NULL_VECTOR);
	DispatchSpawn(frFeathersParticles[client]);
	ActivateEntity(frFeathersParticles[client]);
	AcceptEntityInput(frFeathersParticles[client], "Start");
	KillTimerHandle(frFeathersTimer[client]);
	frFeathersTimer[client] = CreateTimer(3.0, Timer_DestroyParticles, GetClientUserId(client));
}
/* move visible like ghost mode
void SetRotationLock(int client, bool enabled)
{
	float nullrot[3];
	float pos[3];
	if (enabled)
	{
		SetVariantString("!activator");
		AcceptEntityInput(frChickens[client], "SetParent", client, frChickens[client], 0);
		GetClientAbsOrigin(client, pos);
		TeleportEntity(frChickens[client], NULL_VECTOR, nullrot, NULL_VECTOR);
	}
	else
	{
		AcceptEntityInput(frChickens[client], "SetParent");
		GetClientAbsOrigin(client, pos);
		TeleportEntity(frChickens[client], pos, NULL_VECTOR, NULL_VECTOR);
	}
}
*/
void SetChickenSpeed(int client)
{
	float vel[3];
	float factor;
	GetEntPropVector(client, Prop_Data, "m_vecVelocity", vel);
	float velNorm = SquareRoot(vel[0]*vel[0] + vel[1]*vel[1] + vel[2]*vel[2]);
	if (frCIsWalking[client] && velNorm > frChickenWalkSpeed)
		factor = frChickenWalkSpeed;
	else if (!frCIsWalking[client] && velNorm > frChickenRunSpeed)
		factor = frChickenRunSpeed;
	
	for (int i = 0; i < sizeof(vel); i++)
	{
		if (factor > 0.0)
		{
			vel[i] /= velNorm;
			vel[i] *= factor;
		}
	}
	TeleportEntity(client, NULL_VECTOR, NULL_VECTOR, vel);
}

public void SetWeaponPos(int client, int type)
{
	//Put the gun at the chicken's side
	float rot[3];
	float pos[3];
	if (type == 0) // normal
	{
		pos[0] = -17.0;
		pos[1] = -2.0;
		pos[2] = 15.0;
	}
	else if (type == 1) // grenade
	{
		pos[0] = 0.0;
		pos[1] = -5.0;
		pos[2] = 15.0;
	}
	else if (type == 2) // healthshot
	{
		pos[0] = -7.0;
		pos[1] = -23.0;
		pos[2] = 5.0;
		rot[2] = 90.0;
	}
	
	TeleportEntity(frCweapons[client], pos, rot, NULL_VECTOR);
}
public void SlowPlayerFall(int client)
{
	float vel[3];
	GetEntPropVector(client, Prop_Data, "m_vecVelocity", vel);
	if (vel[2] < 0.0)
	{
		float oldSpeed = vel[2];
		if(vel[2] < maxFallSpeed)
			vel[2] = maxFallSpeed;
		if(oldSpeed != vel[2])
			TeleportEntity(client, NULL_VECTOR, NULL_VECTOR, vel);
	}
}
public Action Timer_ChickenAnim(Handle timer, any userid)
{
	int client = GetClientOfUserId(userid);
	if (IsValidClient(client))
	{
		int currentFlags = GetEntityFlags(client);
		
		if (!(currentFlags & FL_ONGROUND) && frFlyCounter[client] == 0)
		{
			SetVariantString(frChickensec[4]);
			AcceptEntityInput(frChickens[client], "SetAnimation");
			frCWasRunning[client] = false;
			frCWasIdle[client] = false;
			frCWasWalking[client] = false;
			++frFlyCounter[client];
		}
		else if (!(currentFlags & FL_ONGROUND))
		{
			frFlyCounter[client]++;
			if (frFlyCounter[client] == 9)
				frFlyCounter[client] = 0;
		}
		else if (currentFlags & FL_ONGROUND)
		{
			frFlyCounter[client] = 0;
			if (!frCIsMoving[client] && !frCWasIdle[client])
			{
				SetVariantString(frChickensec[2]);
				AcceptEntityInput(frChickens[client], "SetAnimation");
				frCWasRunning[client] = false;
				frCWasIdle[client] = true;
				frCWasWalking[client] = false;
			}
			else if (frCIsWalking[client] && !frCWasRunning[client] && frCIsMoving[client])
			{
				SetVariantString(frChickensec[0]);
				AcceptEntityInput(frChickens[client], "SetAnimation");
				frCWasRunning[client] = false;
				frCWasIdle[client] = false;
				frCWasWalking[client] = true;
			}
			else if (!frCIsWalking[client] && !frCWasRunning[client] && frCIsMoving[client])
			{
				SetVariantString(frChickensec[1]);
				AcceptEntityInput(frChickens[client], "SetAnimation");
				frCWasRunning[client] = true;
				frCWasIdle[client] = false;
				frCWasWalking[client] = false;
			}
		}
		frLastFlags[client] = currentFlags;
	}
}

public Action Timer_DestroyParticles(Handle timer, any userid)
{
	int client = GetClientOfUserId(userid);
	if (frFeathersTimer[client] == timer && IsValidEdict(frFeathersParticles[client]))
	{
		AcceptEntityInput(frFeathersParticles[client], "Deactivate");
		AcceptEntityInput(frFeathersParticles[client], "Kill" );
	}
	return Plugin_Handled;
}
public void SpawnChickens()
{
	RemoveChickens();
	int entitySpawnCounter = 0;
	float worldOrigin[3];
	frChickenNumber = CalcPlayers();
	if (frChickenSpawnOrigin)
	{
		while (entitySpawnCounter < frChickenNumber)
		{
			entitySpawnCounter += CreateChickenRandom(worldOrigin);
		}
	}
	else
	{
		float fOrigin[3];
		int spawn = FindEntityByClassname(MAXPLAYERS, "info_player_terrorist");
		GetEntPropVector(spawn, Prop_Send, "m_vecOrigin", fOrigin);
		while (entitySpawnCounter < (frChickenNumber / 2))
		{
			entitySpawnCounter += CreateChickenRandom(fOrigin);
		}
		spawn = FindEntityByClassname(MAXPLAYERS, "info_player_counterterrorist");
		GetEntPropVector(spawn, Prop_Send, "m_vecOrigin", fOrigin);
		while (entitySpawnCounter < frChickenNumber)
		{
			entitySpawnCounter += CreateChickenRandom(fOrigin);
		}
	}
}

public int CreateChickenRandom(float origin[3])
{
	float boxMin[3] =  { -16.0, -16.0, -16.0 };
	float boxMax[3] =  { 16.0, 16.0, 16.0 };
	int entity = CreateEntityByName("chicken");
	if (IsValidEntity(entity))
	{
		float newPos[3];
		newPos[0] = origin[0] + GetRandomFloat(-2500.0, 2500.0);
		newPos[1] = origin[1] + GetRandomFloat(-2500.0, 2500.0);
		newPos[2] = origin[2] + GetRandomFloat(-1000.0, 1000.0);
		
		float rot[3];
		rot[1] = GetRandomFloat(0.0, 360.0);
		
		TeleportEntity(entity, newPos, rot, NULL_VECTOR);
		DispatchSpawn(entity);
		ActivateEntity(entity);
		
		TR_TraceHullFilter(newPos, newPos, boxMin, boxMax, MASK_SOLID, TRDontHitSelf, entity);
		if (TR_DidHit())
		{
			//RemoveEdict(entity);
			AcceptEntityInput(entity, "Deactivate");
			AcceptEntityInput(entity, "Kill");
			return 0;
		}
		else
			return 1;
	}
	else
		return 0;
}
public bool TRDontHitSelf(int entity, int mask, any data)
{
	if (entity == data)return false;
	return true;
}
public Action Timer_RemoveChicken(Handle timer)
{
	RemoveChickens();
}
public void RemoveChickens()
{
	char className[64];
	for (int i = MaxClients; i < GetMaxEntities(); ++i)
	{
		if (IsValidEntity(i) && IsValidEdict(i) && i != ChickenC4)
		{
			GetEdictClassname(i, className, sizeof(className));
			if (StrEqual(className, "chicken") && GetEntPropEnt(i, Prop_Send, "m_hOwnerEntity") == -1)
			{
				AcceptEntityInput(i, "Deactivate");
				AcceptEntityInput(i, "Kill");
			}
		}
	}
}
void PlayRandomPanicSound(int client)
{
	if (frChickenSoundCD[client]) return;
	int rdmSound = GetRandomInt(0, sizeof(frChickenPanicSounds) - 1);
	EmitSoundToAll(frChickenPanicSounds[rdmSound], client);
	frChickenSoundCD[client] = true;
	CreateTimer(1.0, Timer_ChickenSoundCD, GetClientUserId(client));
}

void PlayRandomIdleSound(int client)
{
	if (frChickenSoundCD[client]) return;
	int rdmSound = GetRandomInt(0, sizeof(frChickenIdleSounds) - 1);
	EmitSoundToAll(frChickenIdleSounds[rdmSound], client);
	frChickenSoundCD[client] = true;
	CreateTimer(1.0, Timer_ChickenSoundCD, GetClientUserId(client));
}

void PlayChickenSound(int client)
{
	if (frChickenSoundCD[client]) return;
	EmitSoundToAll(ChickenkillSounds[0], client);
	frChickenSoundCD[client] = true;
	CreateTimer(1.0, Timer_ChickenSoundCD, GetClientUserId(client));
}
public Action Timer_ChickenSoundCD(Handle timer, any userid)
{
	int client = GetClientOfUserId(userid);
	frChickenSoundCD[client] = false;
}
void SetViewModel(int client, bool enabled)
{
	int EntEffects = GetEntProp(GetViewModelIndex(client), Prop_Send, "m_fEffects");
	if (enabled)
		EntEffects |= ~32;
	else
		EntEffects |= 32; // Set to Nodraw
	SetEntProp(GetViewModelIndex(client), Prop_Send, "m_fEffects", EntEffects);
}
int GetViewModelIndex(int client)
{
	int index = MAXPLAYERS;
	while ((index = FindEntityByClassname(index, "predicted_viewmodel")) != -1)
	{
		int owner = GetEntPropEnt(index, Prop_Send, "m_hOwner");
		
		if (owner != client)
			continue;
		
		return index;
	}
	return -1;
}
int CalcPlayers()
{
	int num = 0;
	for (int i = 0; i <= MAXPLAYERS; ++i)
	{
		if (IsValidClient(i)) ++num;
	}
	num *= 2;
	if (num < 10) num = 10;
	return num;
}