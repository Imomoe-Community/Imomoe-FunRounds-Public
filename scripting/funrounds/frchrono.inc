bool frIsChronoClient[MAXPLAYERS + 1] = false;
bool frIsPlayerTelePorted[MAXPLAYERS + 1] = false;
bool frIsChronoFreeze[MAXPLAYERS + 1] = false;
bool frChronoGunStatus[MAXPLAYERS + 1] = false; // false: teleport gun , true: erase gun
float frPlayerPosition[MAXPLAYERS + 1][3];
bool frChronoFixCoolDown[MAXPLAYERS + 1] = false;
bool frCanUseChronoFix[MAXPLAYERS + 1] = false;
bool frEraseingVictim[MAXPLAYERS + 1] = false;
int frEraseing[MAXPLAYERS + 1] = -1;
//Handle frChronoCoolDownTimers[MAXPLAYERS + 1] = INVALID_HANDLE;
//Handle frEraseTimer[MAXPLAYERS + 1] = INVALID_HANDLE;
char frChronoSound[] =  { "funrounds/chrono/chrmove.mp3" };
char frChronoEnableSound[] = { "funrounds/chrono/chropen.mp3" };
//char frChronoDrop[] =  { "funrounds/chrono/chrdrop.mp3" };
char frEraseGunSound[] =  { "funrounds/chrono/chrattack.mp3" };
char frEraseKillSound[] =  { "funrounds/chrono/chrkill.mp3" };

void FrChronoOnMapStart()
{
	PreCacheSoundAndDownload(frChronoSound);
	PreCacheSoundAndDownload(frChronoEnableSound);
	PreCacheSoundAndDownload(frEraseGunSound);
	PreCacheSoundAndDownload(frEraseKillSound);
	RegConsoleCmd("sm_tpbug", Command_ChronoFix);
}

void ChronoStatus(bool status)
{
	bChrono = status;
	for (int i = 1; i <= MaxClients; ++i)
	{
		if (IsValidClient(i))
		{
			if (status)
				SetChrono(i);
			else
				RemoveChrono(i);
		}
	}
}
void SetChrono(int client)
{
	frIsChronoClient[client] = true;
	EmitSoundToClient(client, frChronoEnableSound, client, SNDCHAN_USER_BASE, SNDLEVEL_NORMAL);
}

void RemoveChrono(int client)
{
	frIsChronoClient[client] = false;
	frIsChronoFreeze[client] = false;
	frChronoGunStatus[client] = false;
	frEraseingVictim[client] = false;
	frEraseing[client] = -1;
	frPlayerPosition[client][0] = 0.0;
	frPlayerPosition[client][1] = 0.0;
	frPlayerPosition[client][2] = 0.0;
	SetTimeStop(client, false);
	CreateTimer(0.0, Timer_FreezeOff, GetClientUserId(client));
}

void FrTeleport(int client)
{
	if (frIsPlayerTelePorted[client] || frChronoGunStatus[client]) return;
	GetPlayerPosition(client);
	TeleportPlayer(client);
	SetEntityRenderMode(client, RENDER_TRANSCOLOR);
	SetEntityRenderColor(client, 255, 255, 255, 127);
	CreateTimer(ChronoCoolDownTime() ,Timer_ChronoCD, GetClientUserId(client));
}

public Action Timer_ChronoCD(Handle timer, any userid)
{
	int client = GetClientOfUserId(userid);
	frIsPlayerTelePorted[client] = false;
	SetEntityRenderMode(client, RENDER_NORMAL);
	SetEntityRenderColor(client, 255, 255, 255, 0);
}

public Action Timer_FreezeOff(Handle timer, any userid)
{
	int client = GetClientOfUserId(userid);
	if (IsValidClient(client))
	{
		frIsChronoFreeze[client] = false;
		SetEntityMoveType(client, MOVETYPE_WALK);
	}
}

//TODO
float ChronoCoolDownTime()
{
	return 2.0;
}

float ChronoFreezeTime(float[3] pos1, float[3] pos2)
{
	float distance = GetVectorDistance(pos1, pos2, false);
	float returnvalue = distance / 1000;
	if (returnvalue >= 0.0)
		return returnvalue;
	else return 0.0;
}

void GetPlayerPosition(int client)
{
	GetEntPropVector(client, Prop_Send, "m_vecOrigin", frPlayerPosition[client]);
	frCanUseChronoFix[client] = true;
	PrintToChat(client, "[SM] \x06如果不小心瞬移到空气墙中可以输入!tpbug回到原来的位置");
	PrintCenterText(client, "瞬移到空气墙中可以输入!tpbug回到原来的位置");
}

void TeleportPlayer(int client)
{
	TeleportToEyeAngle(client);
	//StartStuckDetection(client);
	EmitSoundToAll(frChronoSound, client, SNDCHAN_USER_BASE, SNDLEVEL_NORMAL);
	frIsPlayerTelePorted[client] = true;
	frIsChronoFreeze[client] = true;
	SetEntityMoveType(client, MOVETYPE_NONE);
}

void TeleportToEyeAngle(int client)
{
	float eyepos[3], eyeang[3];
	GetClientEyePosition(client, eyepos);
	GetClientEyeAngles(client, eyeang);
	Handle trace = TR_TraceRayFilterEx(eyepos, eyeang, MASK_SOLID, RayType_Infinite, TRDontHitPlayers);
	if (TR_DidHit(trace))
	{
		if (TR_GetEntityIndex(trace) == 0)
		{
			float endpos[3];
			TR_GetEndPosition(endpos, trace);
			endpos[2] += 30.0;
			CreateTimer(ChronoFreezeTime(eyepos, endpos), Timer_FreezeOff, GetClientUserId(client));
			TeleportEntity(client, endpos, NULL_VECTOR, NULL_VECTOR);
		}
		else CreateTimer(0.1, Timer_FreezeOff, GetClientUserId(client));
	}
}
public bool TRDontHitPlayers(int entity, int mask, any data)
{
	return !((entity > 0) && (entity <= MaxClients));
}

public Action Command_ChronoFix(int client, int args)
{
	if (cvPluginEnable.BoolValue && StrEqual(Chrono, "1"))
	{
		if (frChronoFixCoolDown[client])
		{
			PrintToChat(client, "[SM] 正在冷却中无法使用");
			return Plugin_Stop;
		}
		if (!frCanUseChronoFix[client])
		{
			PrintToChat(client, "[SM] 您还没有进行传送，无法使用");
			return Plugin_Stop;
		}
		if (IsValidClient(client) && IsPlayerAlive(client) && frIsChronoClient[client])
		{
			frChronoFixCoolDown[client] = true;
			frCanUseChronoFix[client] = false;
			TeleportEntity(client, frPlayerPosition[client], NULL_VECTOR, NULL_VECTOR);
			EmitSoundToAll(frChronoSound, client, SNDCHAN_USER_BASE, SNDLEVEL_NORMAL);
			CreateTimer(10.0, Timer_ChronoFixCoolDown, GetClientUserId(client));
			frPlayerPosition[client] =  view_as<float>( { 0.0, 0.0, 0.0 } );
			PrintToChat(client, "[SM] 已经成功回到原来的位置，10秒后才能再次使用");
		}
	}
	return Plugin_Handled;
}
public Action Timer_ChronoFixCoolDown(Handle timer, any userid)
{
	int client = GetClientOfUserId(userid);
	frChronoFixCoolDown[client] = false;
}

void SetTimeStop(int client, bool status)
{
	if (status)
	{
		SetEntProp(client, Prop_Send, "m_bSimulatedEveryTick", 0);
		SetEntProp(client, Prop_Send, "m_bAnimatedEveryTick", 0);
		SetEntProp(client, Prop_Send, "m_bClientSideAnimation", 0);
		SetEntProp(client, Prop_Send, "m_bClientSideFrameReset", 1);
		SetEntityMoveType(client, MOVETYPE_NONE);
		SetEntityRenderMode(client, RENDER_TRANSCOLOR);
		SetEntityRenderColor(client, 255, 255, 255, 127);
	}
	else
	{
		SetEntProp(client, Prop_Send, "m_bSimulatedEveryTick", 1);
		SetEntProp(client, Prop_Send, "m_bAnimatedEveryTick", 1);
		SetEntProp(client, Prop_Send, "m_bClientSideAnimation", 1);
		SetEntProp(client, Prop_Send, "m_bClientSideFrameReset", 0);
		SetEntityMoveType(client, MOVETYPE_WALK);
		SetEntityRenderMode(client, RENDER_NORMAL);
		SetEntityRenderColor(client, 255, 255, 255, 0);
	}
	
}

void EraseingPlayer(int client, int victim)
{
	frEraseingVictim[victim] = true;
	frEraseing[client] = victim;
	EmitSoundToAll(frEraseGunSound, client, SNDCHAN_USER_BASE);
	EmitSoundToAll(frEraseGunSound, victim, SNDCHAN_USER_BASE);
	SetTimeStop(victim, true);
	DataPack data = new DataPack();
	data.Reset(true);
	CreateDataTimer(6.0, Timer_Erase, data);
	data.WriteCell(victim);
	data.WriteCell(client);
	data.WriteFloat(GetHealth(client) * 10 + 0.0);
	data.WriteCell(CSGOItems_GetActiveWeapon(client));
}

void RemoveEraseing(int client)
{
	int victim = frEraseing[client];
	frEraseing[client] = -1;
	frEraseingVictim[victim] = false;
	SetTimeStop(victim, false);
}

public Action Timer_Erase(Handle timer, DataPack data)
{
	data.Reset();
	int client = data.ReadCell();
	int attacker = data.ReadCell();
	float damage = data.ReadFloat();
	int weapon = data.ReadCell();
	if (IsValidClient(client) && IsPlayerAlive(client) && IsValidClient(attacker) && IsPlayerAlive(attacker))
	{
		SDKHooks_TakeDamage(client, 0, attacker, damage, DMG_DIRECT, weapon);
		frEraseing[attacker] = -1;
	}
}

void EraseRagdoll(int ragdoll)
{
	AcceptEntityInput(ragdoll, "Kill");
	RemoveEdict(ragdoll);
}