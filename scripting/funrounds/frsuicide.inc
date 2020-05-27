bool frKilledBySuicide[MAXPLAYERS + 1];
bool frIsSuicideBomber[MAXPLAYERS + 1];
bool frIsGoingSuicide[MAXPLAYERS + 1];
bool frTriggeredSuicide[MAXPLAYERS + 1] = false;
char frSuicideSound[][] =  { "funrounds/suicide/suicide1.mp3", "funrounds/suicide/suicide2.mp3", "funrounds/suicide/suicide3.mp3" };
char frSuicideWarningSound[][] =  { "funrounds/suicide/warning1.mp3", "funrounds/suicide/warning2.mp3", "funrounds/suicide/warning3.mp3" };
char frExplodeSound[][] =  { "weapons/c4/c4_explode1.wav", "weapons/c4/c4_exp_deb2.wav" };
int frBomb[MAXPLAYERS + 1];
int killed[MAXPLAYERS + 1] = 0;
int frDiedByBomb[MAXPLAYERS + 1] =  { -1, ... };

void FrSuicideBomberStart()
{
	PreCacheListSound(frSuicideSound, sizeof(frSuicideSound));
	PreCacheListSound(frSuicideWarningSound, sizeof(frSuicideWarningSound));
	PreCacheListSound(frExplodeSound, sizeof(frExplodeSound));
}

void RemoveSuicide(int client)
{
	frKilledBySuicide[client] = false;
	frIsSuicideBomber[client] = false;
	frIsGoingSuicide[client] = false;
	frDiedByBomb[client] = -1;
	killed[client] = 0;
}
void RemoveAllSuicide()
{
	for (int client = 1; client <= MaxClients; ++client)
		RemoveSuicide(client);
}

void BomberGoSuicide(int client)
{
	PreSuicide(client);
	char name[64];
	GetClientName(client, name, sizeof(name));
	if (GetClientTeam(client) == CS_TEAM_T)
	{
		CreateTimer(5.0, Timer_DelayExplosion, client);
		PrintToChatAll("[SM] \x06%s\x01 启动了自杀式炸弹，将在五秒钟后爆炸！", name);
	}
	else if (GetClientTeam(client) == CS_TEAM_CT)
	{
		TriggerSuicide(client);
	}
}

void PreSuicide(int client)
{
	for (int i = 0; i < 14; ++i)
	{
		int weapon = -1;
		while ((weapon = GetPlayerWeaponSlot(client, i)) != -1)
		{
			if (CSGOItems_IsValidWeapon(weapon))
			{
				char classname[64];
				GetEntityClassname(weapon, classname, sizeof(classname));
				if (!StrEqual(classname, "weapon_c4") && GetEntPropEnt(weapon, Prop_Send, "m_hOwnerEntity") == client)
				{
					CS_DropWeapon(client, weapon, false);
					//RemoveEdict(weapon);
				}
				else if (StrContains(classname, "weapon_") != -1)
				{
					//CSGOItems_RemoveWeapon(client, weapon);
					RemovePlayerItem(client, weapon);
					AcceptEntityInput(weapon, "Kill");
				}
			}
		}
	}
	//EquipPlayerWeapon(client, GivePlayerItem(client, "weapon_fists"));
	int rdmSound = GetRandomInt(0, sizeof(frSuicideWarningSound) - 1);
	if (GetClientTeam(client) == CS_TEAM_T) EmitSoundToAll(frSuicideWarningSound[rdmSound], client, SNDCHAN_USER_BASE, SNDLEVEL_RAIDSIREN);
	SetSpeed(client, GetEntPropFloat(client, Prop_Data, "m_flLaggedMovementValue") * 1.6);
	frIsGoingSuicide[client] = true;
}
void CreateExplosion(int client)
{
	float vecPosition[3];
	GetClientAbsOrigin(client, vecPosition); 
	frBomb[client] = CreateEntityByName("info_particle_system");
	DispatchKeyValue(frBomb[client], "start_active", "0");
	DispatchKeyValue(frBomb[client], "effect_name", "explosion_c4_500");
	DispatchSpawn(frBomb[client]);
	TeleportEntity(frBomb[client], vecPosition, NULL_VECTOR, NULL_VECTOR);
	ActivateEntity(frBomb[client]);
	SetVariantString("!activator");
	AcceptEntityInput(frBomb[client], "SetParent", frBomb[client], frBomb[client], 0);
	//ForcePlayerSuicide(client);
	CreateTimer(0.25, Timer_Run, frBomb[client]);
}
public Action Timer_DelayExplosion(Handle timer, int client)
{
	if (frIsSuicideBomber[client] && frIsGoingSuicide[client] && IsPlayerAlive(client))
	{
		TriggerSuicide(client);
	}
}
void TriggerSuicide(int client)
{
	frKilledBySuicide[client] = true;
	frDiedByBomb[client] = client;
	CreateExplosion(client);
	int rdmSound = GetRandomInt(0, sizeof(frSuicideSound) - 1);
	EmitSoundToAll(frSuicideSound[rdmSound], client, SNDCHAN_USER_BASE, SNDLEVEL_RAIDSIREN);
	EmitAmbientSound(frExplodeSound[GetRandomInt(0, sizeof(frExplodeSound) - 1)], NULL_VECTOR, client);
	EquipPlayerWeapon(client, GivePlayerItem(client, "weapon_c4"));
	ProcessSuicide(client);
	ShowSuicideText(client);
}
public Action Timer_Run(Handle timer, any ent)
{
	if(IsValidEntity(ent))
	{
		AcceptEntityInput(ent, "Start");
		CreateTimer(7.0, Timer_Die, ent);
	}
}

public Action Timer_Die(Handle timer, any ent)
{
	if(IsValidEntity(ent))
	{
		if(IsValidEdict(ent))
		{
			AcceptEntityInput(ent, "Kill");
		}
	}
}
void ProcessSuicide(int client)
{
	SetFF(true);
	killed[client] = 0;
	int weapon = GetPlayerWeaponSlot(client, CS_SLOT_C4);
	if (!IsValidEntity(weapon)) weapon = client;
	for (int i = 1; i <= MaxClients; ++i)
	{
		if (IsValidClient(i) && IsPlayerAlive(i) && client != i)
		{
			SuicideKill(client, i, weapon);
		}
		
	}
	float damage = GetHealth(client) * 10.0;
	CreateDamage(0.0, client, client, weapon, damage, CS_DMG_HEADSHOT, weapon, true);
	DisarmPlayerWeapons(client);
	SetFF(false);
	//SDKHooks_TakeDamage(client, weapon, client, (GetHealth(client) + 0.00) * 10, CS_DMG_HEADSHOT, weapon);
}
void ShowSuicideText(int client)
{
	char name[128];
	GetClientName(client, name, sizeof(name));
	for (int i = 1; i <= MaxClients; ++i)
	{
		if (IsValidClient(i) && IsPlayerAlive(i) && frKilledBySuicide[i] && client != i)
		{
			frDiedByBomb[i] = -1;
			frKilledBySuicide[i] = false;
			--killed[client];
			if (!frIsSuicideBomber[i] && !frIsGoingSuicide[i]) RemoveSuicide(i);
		}
	}
	int team = GetClientTeam(client);
	if (killed[client] >= 1)
	{
		if (team == CS_TEAM_T)
		{
			if (!frTriggeredSuicide[client]) PrintToChatAll("[SM] \x06%s\x01 高喊“安拉胡阿克巴！”英勇的引爆了自杀式炸弹并炸飞了 \x06%i\x01 人!", name, killed[client]);
			else PrintToChatAll("[SM] \x06%s\x01 来不及喊“安拉胡阿克巴！”身上的炸弹就被引爆了并炸飞了 \x06%i\x01 人!", name, killed[client]);
		}
		if (team == CS_TEAM_CT) PrintToChatAll("[SM] \x06%s\x01 壮烈牺牲了，但是连带炸飞了 \x06%i\x01 人!", name, killed[client]);
	}
	else
	{
		if (team == CS_TEAM_T)
		{
			if (!frTriggeredSuicide[client]) PrintToChatAll("[SM] \x06%s\x01 死得真惨还没拉到垫背的。", name);
			else PrintToChatAll("[SM] \x06%s\x01 被炸死还没拉到垫背的。", name);
		}
		if (team == CS_TEAM_CT) PrintToChatAll("[SM] \x06%s\x01 壮烈牺牲了。", name);
	}
	RemoveSuicide(client);
}
void SetFF(bool status)
{
	if (status)
	{
		if (StrEqual(FriendlyFire, "0"))
		{
			SetCvar("mp_friendlyfire", "1");
			SetCvar("ff_damage_reduction_bullets", "1");
			SetCvar("ff_damage_reduction_grenade", "1");
			SetCvar("ff_damage_reduction_other", "1");
		}
	}
	else
	{
		if (StrEqual(FriendlyFire, "0"))
		{
			SetCvar("mp_friendlyfire", "0");
		}
		SetCvar("ff_damage_reduction_bullets", "0");
		SetCvar("ff_damage_reduction_grenade", "0");
		SetCvar("ff_damage_reduction_other", "0");
	}
}

void SuicideKill(int client, int victim, int weapon = 0)
{
	float vec[3];
	GetClientAbsOrigin(client, vec);
	float victim_vec[3];
	GetClientAbsOrigin(victim, victim_vec);
	float distance = GetVectorDistance(victim_vec, vec, false);
	if (distance <= 400.0)
	{
		if (frIsSuicideBomber[victim] && frIsGoingSuicide[victim])
		{
			frTriggeredSuicide[victim] = true;
			RequestFrame(TriggerSuicide, victim);
		}
		else
		{
			float damage = (GetHealth(victim) + 0.00) * 10;
			if (StrEqual(HeadShot, "1")) SDKHooks_TakeDamage(victim, weapon, client, damage, CS_DMG_HEADSHOT, weapon);
			else SDKHooks_TakeDamage(victim, weapon, client, damage, DMG_IGNITE, weapon);
			frDiedByBomb[victim] = client; //attacker : client
			frKilledBySuicide[victim] = true;
			++killed[client];
		}
	}
	else
	{
		distance -= 400.0;
		int damage = RoundToFloor(500.0 - distance * 1.25);
		if (damage > 0)
		{
			int hp = GetHealth(victim);
			if (hp - damage <= 0)
			{
				if (frIsSuicideBomber[victim] && frIsGoingSuicide[victim])
				{
					frTriggeredSuicide[victim] = true;
					RequestFrame(TriggerSuicide, victim);
				}
				frDiedByBomb[victim] = client; //attacker : client
				frKilledBySuicide[victim] = true;
				++killed[client];
			}
			if (StrEqual(HeadShot, "1")) SDKHooks_TakeDamage(victim, weapon, client, damage + 0.0, CS_DMG_HEADSHOT, weapon);
			else SDKHooks_TakeDamage(victim, weapon, client, damage + 0.0, DMG_IGNITE, weapon);
		}
	}
}
public Action Command_GoSuicide(int client, int args)
{
	if (!cvSuicideBomber.BoolValue || !bSuicide)
		return Plugin_Stop;
	if (IsValidClient(client) && IsPlayerAlive(client))
	{
		if (frIsSuicideBomber[client] && !frIsGoingSuicide[client])
		{
			RequestFrame(BomberGoSuicide, client);
		}
		if (!frIsSuicideBomber[client] && GetClientTeam(client) == CS_TEAM_T)
		{
			if (CS_GetClientContributionScore(client) < 5 && GetMoney(client) < 10000)
			{
				PrintToChat(client, "您身上没有c4也无法购买c4，无法自爆");
				return Plugin_Stop;
			}
			if (GetMoney(client) >= 10000)
			{
				SetMoney(client, GetMoney(client) - 10000);
				EquipPlayerWeapon(client, GivePlayerItem(client, "weapon_c4"));
				RequestFrame(BomberGoSuicide, client);
			} else if (CS_GetClientContributionScore(client) >= 5)
			{
				CS_SetClientContributionScore(client, CS_GetClientContributionScore(client) - 5);
				EquipPlayerWeapon(client, GivePlayerItem(client, "weapon_c4"));
				RequestFrame(BomberGoSuicide, client);
			}
		}
	}
	return Plugin_Handled;
}