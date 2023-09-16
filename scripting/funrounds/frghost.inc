static bool frIsGhostClient[MAXPLAYERS + 1];
bool frIsPlayerVisible[MAXPLAYERS + 1];
Handle frGhostBreathTimers[MAXPLAYERS + 1];
char frGhostBreathSound[] = "player/breathe1.wav";
//char frGhostHeartSound[PLATFORM_MAX_PATH] =  { "player/heartbeatloop.wav" };

stock bool CheckGhostPlayer(int client)
{
	return frIsGhostClient[client];
}

void FrGhostOnMapStart()
{
	PreCacheSoundAndDownload(frGhostBreathSound);
}

void EnableGhost(int team)
{
	if (team <= 0) return;
	SetCvar("mp_teamname_1", "潜伏者");
	SetCvar("mp_teamname_2", "保卫者");
	for (int i = 1; i <= MaxClients; ++i)
	{
		if ((team > 1) && IsValidClient(i) && !IsFakeClient(i) && GetClientTeam(i) == team)
		{
			PlayerGhostStatus(i, true);
		}
		else if (IsValidClient(i) && !IsFakeClient(i) && (team == 1))
		{
			{
				PlayerGhostStatus(i, true);
			}
		}
	}
}
void DisableGhost()
{
	for (int i = 1; i <= MaxClients; ++i)
	{
		if(IsValidClient(i))
		{
			PlayerGhostStatus(i, false);
			frIsPlayerVisible[i] = false;
			SetEntityRenderMode(i, RENDER_NORMAL);
			SetEntityRenderColor(i, 255, 255, 255, 0);
		}
	}
}
void PlayerGhostStatus(int client, bool status)
{
	frIsGhostClient[client] = status;
	if (IsValidClient(client) && IsPlayerAlive(client)) FrGhostVisibility(client, !status);
	if (status)
		frGhostBreathTimers[client] = CreateTimer(GetConVarFloat(cvFrBreathSound), Timer_BreathSound, GetClientUserId(client), TIMER_REPEAT | TIMER_FLAG_NO_MAPCHANGE);
}

void FrGhostVisibility(int client, bool visible)
{
	if (frIsGhostClient[client])
	{
		if(visible)
		{
			frIsPlayerVisible[client] = true;
			SetGhostVisibility(client, true);
			SetClientWeaponsVisibility(client, true);
			FadeClient(client, 255, 255, 255, 0);
		}
		else
		{
			CreateTimer(0.5, Delay_SetInVisible, GetClientUserId(client));
		}
	}
}

void SetClientWeaponsVisibility(int client, bool visible)
{
	int weapon;
	for (int i = 0; i < CS_SLOT_C4; i++)
	{
		if ((weapon = GetPlayerWeaponSlot(client, i)) != -1 && CSGOItems_IsValidWeapon(weapon))
		{
			int worldModel = GetEntPropEnt(weapon, Prop_Send, "m_hWeaponWorldModel");
			if (worldModel != -1)
			{
				if (!visible)
					SDKHook(worldModel, SDKHook_SetTransmit, Hook_WeaponSetTransmit);
				else
					SDKUnhook(worldModel, SDKHook_SetTransmit, Hook_WeaponSetTransmit);
			}
		}
	}
}
void StopBreathSound(int client)
{
	KillTimerHandle(frGhostBreathTimers[client]);
}
void SetGhostVisibility(int entity, bool visible, bool enable = true)
{
	if (frIsGhostClient[entity] && enable)
	{
		SetEntityRenderMode(entity, RENDER_TRANSCOLOR);
		//SetEntityRenderMode(entity, RENDER_NONE);
		if (visible)
		{
			SetEntityRenderColor(entity, 255, 255, 255, 127);
		}
		else
		{
			SetEntityRenderColor(entity, 0, 0, 0, 127);
		}
	}
	else
	{
		SetEntityRenderMode(entity, RENDER_NORMAL);
	}
}
public Action Delay_SetInVisible(Handle timer, any userid)
{
	int client = GetClientOfUserId(userid);
	if (!frIsPlayerVisible[client] || !IsValidClient(client)) return Plugin_Stop;
	frIsPlayerVisible[client] = false;
	SetGhostVisibility(client, false);
	SetClientWeaponsVisibility(client, false);
	FadeClient(client, 64, 0, 64, 64);
	return Plugin_Continue;
}
public Action Timer_BreathSound(Handle timer, any userid)
{
	int client = GetClientOfUserId(userid);
	if (IsValidClient(client) && IsPlayerAlive(client) && frIsGhostClient[client])
	{
		EmitSoundToAll(frGhostBreathSound, client, SNDCHAN_BODY, SNDLEVEL_NORMAL, SND_CHANGEVOL, 0.9);
		CreateTimer((GetConVarFloat(cvFrBreathSound) / 2), Timer_StopBSound, GetClientUserId(client), TIMER_FLAG_NO_MAPCHANGE);
	}
	return Plugin_Continue;
}
public Action Timer_StopBSound(Handle timer, any userid)
{
	int client = GetClientOfUserId(userid);
	EmitSoundToAll(frGhostBreathSound, client, SNDCHAN_BODY, SNDLEVEL_NORMAL, SND_STOPLOOPING);
	return Plugin_Continue;
}

bool IsEntitySolid(int entity)
{
	return (GetEntProp(entity, Prop_Send, "m_nSolidType") != 0) && ((GetEntProp(entity, Prop_Send, "m_usSolidFlags") & 4) == 0);
}
public Action FadeClient(int client, int r, int g, int b, int a)
{
	#define FFADE_STAYOUT 0x0008
	#define	FFADE_PURGE 0x0010
	Handle hFadeClient = StartMessageOne("Fade", client);
	if (GetUserMessageType() == UM_Protobuf)
	{
		int color[4];
		color[0] = r;
		color[1] = g;
		color[2] = b;
		color[3] = a;
		PbSetInt(hFadeClient, "duration", 0);
		PbSetInt(hFadeClient, "hold_time", 0);
		PbSetInt(hFadeClient, "flags", (FFADE_PURGE|FFADE_STAYOUT));
		PbSetColor(hFadeClient, "clr", color);
	}
	else
	{
		BfWriteShort(hFadeClient, 0);
		BfWriteShort(hFadeClient, 0);
		BfWriteShort(hFadeClient, (FFADE_PURGE|FFADE_STAYOUT));
		BfWriteByte(hFadeClient, r);
		BfWriteByte(hFadeClient, g);
		BfWriteByte(hFadeClient, b);
		BfWriteByte(hFadeClient, a);		
	}
	EndMessage();
	return Plugin_Continue;
}