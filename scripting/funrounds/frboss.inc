#define BEAMMODEL "materials/sprites/purplelaser1.vmt"
static bool frIsBossPlayer[MAXPLAYERS + 1];
bool frIsPlayerCanHook[MAXPLAYERS + 1];
bool frIsHooked[MAXPLAYERS + 1];
int frHookEnt[MAXPLAYERS + 1] =  { INVALID_ENT_REFERENCE, ... };
int frHookProp[MAXPLAYERS + 1] =  { INVALID_ENT_REFERENCE, ... };
float frHookLength[MAXPLAYERS + 1];
float frHookPos[MAXPLAYERS + 1][3];
static char HookSound[] = "funrounds/boss/hook.mp3";
static bool IsBossPlayerAlive = false;
static bool RandomDamageStatus;
static int LatestBossPlayer = -1;

void FrBossStart()
{
	PreCacheSoundAndDownload(HookSound);
	PrecacheModel(BEAMMODEL);
}

void FrBossEnd()
{
	RandomDamageStatus = false;
	IsBossPlayerAlive = false;
	LatestBossPlayer = -1;
	for (int client = 1; client <= MaxClients; ++client)
	{
		frIsBossPlayer[client] = false;
		frIsHooked[client] = false;
	}
}

bool CheckBoss(int client)
{
	return frIsBossPlayer[client];
}
void SetBossStatus(int client, bool status)
{
	if (IsValidClient(client))
	{
		frIsBossPlayer[client] = status;
		if (status && IsPlayerAlive(client))
		{
			SetEntData(client, FindSendPropInfo("CCSPlayer", "m_bCanMoveDuringFreezePeriod"), 1);
			int random = Math_GetRandomInt(1, 100);
			if (random <= 20)
			{
				PrintToChat(client, "[SM] 您的BOSS技能为：抓钩");
				SetGrappleHook(client, status);
			}
			else if (20 < random <= 40)
			{
				frPlayerDamageMultiplier[client] = 5.0;
				PrintToChat(client, "[SM] 您的BOSS技能为：五倍伤害");
			}
			else if (40 < random <= 60)
			{
				SetSpeed(client, 2.0);
				PrintToChat(client, "[SM] 您的BOSS技能为：两倍速");
			}
			else if (60 < random <= 80)
			{
				PlayerGhostStatus(client, status);
				ThrowKnifeStatus(client, status);
				SetAutoStrafe(client, status);
				SetEntityGravity(client, 0.3);
				PrintToChat(client, "[SM] 您的BOSS技能为：低重力幽灵加飞刀");
			}
			else
			{
				SetExplodeClient(client, status);
				PrintToChat(client, "[SM] 您的BOSS技能为：子弹爆炸");
			}
		}
		else
		{
			frIsPlayerCanHook[client] = status;
			SetGrappleHook(client, status);
			if (IsValidClient(client))
			{
				SetSpeed(client, 1.0);
				SetEntityGravity(client, 1.0);
			}
			SetAutoStrafe(client, status);
			PlayerGhostStatus(client, status);
			ThrowKnifeStatus(client, status);
			SetExplodeClient(client);
			frPlayerDamageMultiplier[client] = 1.0;
		}
	}
}

void SetGrappleHook(int client, bool status)
{
	frIsPlayerCanHook[client] = status;
	GrappleHookOff(client);
	if ((!StrEqual(GrappleHook, "0") || !StrEqual(BossEvolve, "0")) && status) PrintToChat(client, "[SM] 您可以按E键使用抓钩");
}

void Knockback(int client, float value)
{
	if (!(GetEntityFlags(client) & FL_DUCKING) && !(GetClientButtons(client) & IN_SPEED))
	{
		float Velocity[3], EyeAngle[3], Vector[3];
		GetEntPropVector(client, Prop_Data, "m_vecVelocity", Velocity);
		GetClientEyeAngles(client, EyeAngle);
		GetAngleVectors(EyeAngle, Vector, NULL_VECTOR, NULL_VECTOR);
		ScaleVector(Vector, -value);
		ScaleVector(Velocity, 0.3);
		AddVectors(Velocity, Vector, Velocity);
		TeleportEntity(client, NULL_VECTOR, NULL_VECTOR, Velocity);
	}
}


void GrappleHookAction(int client)
{
	if (IsValidClient(client) && IsPlayerAlive(client))
	{
		Grapple(client);
		frIsHooked[client] = true;
		SetEntityGravity(client, 0.001);
		CreateHookBeam(client);
		
		SetVariantString("!activator");
		AcceptEntityInput(EntRefToEntIndex(frHookEnt[client]), "SetParent", client);
		float pos[3];
		GetClientEyePosition(client, pos);
		EmitAmbientSound(HookSound, pos);
	}
}
void GrappleHookOff(int client)
{
	frIsHooked[client] = false;
	KillBeam(client);
	if (IsValidClient(client))
		SetEntityGravity(client, 1.0);
}
void Grapple(int client)
{

	if (frIsHooked[client])
		return;
	
	if (!IsPlayerAlive(client)) 
	{
		GrappleHookOff(client);
		return;
	}
	
	float origin[3];
	float vel[3];

	GetClientAbsOrigin(client, origin);

	float distance = GetVectorDistance(frHookPos[client], origin);
	float laggedspeed = GetEntPropFloat(client, Prop_Data, "m_flLaggedMovementValue");
	if ( distance > 60 )
	{
		float inverseTime = 1000 / distance;
		vel[0] = ((frHookPos[client][0] - origin[0]) * inverseTime) / laggedspeed;
		vel[1] = ((frHookPos[client][1] - origin[1]) * inverseTime) / laggedspeed;
		vel[2] = ((frHookPos[client][2] - origin[2]) * inverseTime) / laggedspeed;
	}

	TeleportEntity(client, NULL_VECTOR, NULL_VECTOR, vel);
}

void KillBeam(int client)
{
	int beam = EntRefToEntIndex(frHookEnt[client]);
	int prop = EntRefToEntIndex(frHookProp[client]);
	if(beam != INVALID_ENT_REFERENCE)
		AcceptEntityInput(beam, "Kill");
	if(prop != INVALID_ENT_REFERENCE)
		AcceptEntityInput(prop, "Kill");
}

stock void CreateHookBeam(int client)
{
	float startPos[3];
	GetClientAbsOrigin(client, startPos);
	startPos[2] += 40.0;
	
	float eyeAngles[3];
	GetClientEyeAngles(client, eyeAngles);

	Handle trace = TR_TraceRayFilterEx(startPos, eyeAngles, MASK_ALL, RayType_Infinite, TraceFilterNotSelf, client);
	if(TR_DidHit(trace))
		TR_GetEndPosition(frHookPos[client], trace);
	trace.Close();
	
	frHookLength[client] = GetVectorDistance(startPos, frHookPos[client]);
	//BEAM
	int beam = CreateEntityByName("env_beam");

	char color[16];
	
	ConVar cvar = FindConVar("mp_teammates_are_enemies");
	if(!GetConVarBool(cvar))
	{
		if(GetClientTeam(client) == CS_TEAM_T)
			Format(color, sizeof(color), "255 0 0 255");
		else
			Format(color, sizeof(color), "0 0 255 255");
	}
	else
		Format(color, sizeof(color), "255 255 255 255");
		
	SetEntityModel(beam, BEAMMODEL); // This is where you would put the texture, ie "sprites/laser.vmt" or whatever.
	DispatchKeyValue(beam, "rendercolor", color);
	DispatchKeyValue(beam, "renderamt", "200");
	DispatchKeyValue(beam, "decalname", "Bigshot"); 
	DispatchKeyValue(beam, "life", "0"); 
	DispatchKeyValue(beam, "TouchType", "0");
	DispatchSpawn(beam);
	TeleportEntity(beam, startPos, NULL_VECTOR, NULL_VECTOR);
	
	//PROP
	int prop = CreateEntityByName("info_target");
	DispatchSpawn(prop);
	TeleportEntity(prop, frHookPos[client], NULL_VECTOR, NULL_VECTOR);
	//PROP END
	
	SetEntPropEnt(beam, Prop_Send, "m_hAttachEntity", beam);
	SetEntPropEnt(beam, Prop_Send, "m_hAttachEntity", prop, 1);
	SetEntProp(beam, Prop_Send, "m_nNumBeamEnts", 2);
	SetEntProp(beam, Prop_Send, "m_nBeamType", 2);
	
	SetEntPropFloat(beam, Prop_Data, "m_fWidth", 5.0); 
	SetEntPropFloat(beam, Prop_Data, "m_fEndWidth", 5.0); 
	ActivateEntity(beam);
	AcceptEntityInput(beam, "TurnOn");

	frHookProp[client] = EntIndexToEntRef(prop);
	frHookEnt[client] = EntIndexToEntRef(beam);
}

public bool TraceFilterNotSelf(int entityhit, int mask, any entity)
{
	if(entity == 0 && entityhit != entity)
		return true;
	
	return false;
}

public Action Timer_BossEvolveGiveHealth(Handle timer)
{
	int hp;
	for (int client = 1; client <= MaxClients; ++client)
	{
		if (IsValidClient(client) && IsPlayerAlive(client))
		{
			hp = GetHealth(client) + 50;
			if (CheckBoss(client))
			{
				hp += 25;
			}
			SetHealth(client, hp);
		}
	}
	EvolveBoss();
	return Plugin_Continue;
}

void EvolveBoss()
{
	int boss = -1;
	int hp[MAXPLAYERS + 1];
	for (int client = 1; client <= MaxClients; ++client)
	{
		if (IsValidClient(client) && IsPlayerAlive(client))
			hp[client] = GetHealth(client);
	}
	boss = SelectHighestHealthPlayer(hp, sizeof(hp));
	if (LatestBossPlayer != boss && boss > 0)
	{
		if (IsValidClient(boss) && IsPlayerAlive(boss))
		{
			if (IsValidClient(LatestBossPlayer)) SetBossStatus(LatestBossPlayer, false);
			SetBossStatus(boss, true);
			LatestBossPlayer = boss;
		}
	}
	char name[64];
	GetClientName(boss, name, sizeof(name));
	int bosshealth = GetHealth(boss);
	if (boss > 0)
		PrintToChatAll("[SM] 当前的BOSS玩家为：%s, 血量为：%i ", name, bosshealth);
	else
		PrintToChatAll("[SM] 当前没有任何玩家成为BOSS");
	RandomDamageStatus = bosshealth > 3000;
}

stock int SelectHighestHealthPlayer(const int[] array, const int maxlength)
{
	int hp = array[0];
	int max = 0;
	for (int i = 1; i < maxlength; ++i)
	{
		if (hp < array[i])
		{
			hp = array[i];
			max = i;
		}
	}
	return max;
}

void RandomSelectBoss(int count)
{
	ArrayList clientlist = CreateArray();
	int num = 0;
	char name[64];
	for (int client = 1; client <= MaxClients; ++client)
	{
		if (IsValidClient(client) && IsPlayerAlive(client))
		{
			clientlist.Push(client);
			++num;
		}
	}
	if (num > 0)
	{
		if (count > num)
			count = num;
		for (int i = 0; i < count; ++i)
		{
			int index = GetRandomInt(0, num - 1);
			--num;
			int select = clientlist.Get(index);
			CS_SwitchTeam(select, CS_TEAM_T);
			SetBossPlayer(select);
			GetClientName(select, name, sizeof(name));
			PrintToChatAll("[SM] BOSS玩家为：%s", name);
			clientlist.Erase(index);
		}
	}
	clientlist.Close();
}

void SetBossPlayer(int client)
{
	SetBossStatus(client, true);
	SetHealth(client, GetHealth(client) * 10);
	SetArmorValue(client, GetArmorValue(client) * 4, true, true);
}

bool IsBossAlive()
{
	return IsBossPlayerAlive;
}

void CheckBossAlive()
{
	int bossnum = 0;
	for (int client = 1; client <= MaxClients; ++client)
	{
		if (IsValidClient(client) && IsPlayerAlive(client) && CheckBoss(client))
		{
			++bossnum;
		}
	}
	IsBossPlayerAlive = bossnum > 0;
}

bool IsRandomDamageEnable()
{
	return RandomDamageStatus;
}

void BossEvolveStatus(bool status)
{
	bBossEvolve = status;
	KillTimerHandle(frBossEvolve);
	LatestBossPlayer = -1;
	if (status)
		frBossEvolve = CreateTimer(5.0, Timer_BossEvolveGiveHealth, _, TIMER_REPEAT);
}