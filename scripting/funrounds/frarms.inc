char frWeapons[57][21] =  { "weapon_ak47", "weapon_aug", "weapon_bizon", "weapon_famas", "weapon_g3sg1", "weapon_galilar", "weapon_m249", "weapon_m4a1", "weapon_mac10", "weapon_mag7", "weapon_mp7", "weapon_mp9", "weapon_negev", "weapon_nova", "weapon_p90", "weapon_sawedoff", "weapon_scar20", "weapon_sg556", "weapon_ssg08", "weapon_ump45", "weapon_xm1014", "weapon_awp", "weapon_m4a1_silencer", "weapon_mp5sd", "weapon_deagle", "weapon_elite", "weapon_fiveseven", "weapon_glock", "weapon_hkp2000", "weapon_p250", "weapon_tec9", "weapon_usp_silencer", "weapon_cz75a", "weapon_revolver", "weapon_shield", "weapon_healthshot", "weapon_breachcharge", "weapon_tablet", "weapon_c4", "weapon_taser", "weapon_hegrenade", "weapon_incgrenade", "weapon_flashbang", "weapon_molotov", "weapon_smokegrenade", "weapon_decoy", "weapon_firebomb", "weapon_diversion", "weapon_bumpmine", "weapon_tagrenade", "weapon_snowball", "weapon_knifegg", "weapon_fists", "weapon_axe", "weapon_hammer", "weapon_spanner", "chicken" };

char frBuyMenuWeapons[37][] =  { "glock", "hkp2000", "usp_silencer", "elite", "p250", "tec9", "fn57", "deagle", "galilar", "famas", "ak47", "m4a1", "m4a1_silencer", "ssg08", "aug", "sg556", "awp", "scar20", "g3sg1", "nova", "xm1014", "mag7", "m249", "negev", "mac10", "mp9", "mp7", "ump45", "p90", "bizon", "taser", "molotov", "incgrenade", "decoy", "flashbang", "hegrenade", "smokegrenade" };

float g_fLastAngles[MAXPLAYERS + 1][3];
bool frPlayerWeaponFired[MAXPLAYERS + 1] = false;
bool frExplodeBulletClient[MAXPLAYERS + 1] = false;
int CurrentRoundWeapon[3] = -1;

void EquipRandomWeapon(int client, int count = 1, bool force = false)
{
	if (force) DisarmPlayerWeapons(client);
	for (int i = 0; i < count; ++i)
	{
		int random = Math_GetRandomInt(0, 56);
		if (StrEqual(frWeapons[random], "chicken")) GivePlayerItem(client, frWeapons[random]);
		else EquipPlayerWeapon(client, GivePlayerItem(client, frWeapons[random]));
	}
}
public bool TR_DontHitSelf(int entity, int mask, any data)
{
	if (entity == data) 
		return false;
	return true;
}
void GetWeaponAttachmentPosition(int client, const char[] attachment, float pos[3])
{
	if (!attachment[0])
		return;
		
	int entity = CreateEntityByName("info_target");
	DispatchSpawn(entity);
	
	int weapon;
	
	if ((weapon = GetEntPropEnt(client, Prop_Send, "m_hActiveWeapon")) == -1)
		return;
	
	if ((weapon = GetEntPropEnt(weapon, Prop_Send, "m_hWeaponWorldModel")) == -1)
		return;
		
	SetVariantString("!activator");
	AcceptEntityInput(entity, "SetParent", weapon, entity, 0);
	
	SetVariantString(attachment); 
	AcceptEntityInput(entity, "SetParentAttachment", weapon, entity, 0);
	
	TeleportEntity(entity, NULL_VECTOR, NULL_VECTOR, NULL_VECTOR);
	GetEntPropVector(entity, Prop_Data, "m_vecAbsOrigin", pos);
	AcceptEntityInput(entity, "kill");
}
void TE_DispatchEffect(const char[] particle, const float pos[3], const float endpos[3], const float angles[3] = NULL_VECTOR)
{
	TE_Start("EffectDispatch");
	TE_WriteFloatArray("m_vStart.x", pos, 3);
	TE_WriteFloatArray("m_vOrigin.x", endpos, 3);
	TE_WriteVector("m_vAngles", angles);
	TE_WriteNum("m_nHitBox", GetParticleEffectIndex(particle));
	TE_WriteNum("m_iEffectName", GetEffectIndex("ParticleEffect"));
}
int GetParticleEffectIndex(const char[] sEffectName)
{
	static int table = INVALID_STRING_TABLE;

	if (table == INVALID_STRING_TABLE)
		table = FindStringTable("ParticleEffectNames");

	int iIndex = FindStringIndex(table, sEffectName);

	if (iIndex != INVALID_STRING_INDEX)
		return iIndex;

	return 0;
}
int GetEffectIndex(const char[] sEffectName)
{
	static int table = INVALID_STRING_TABLE;

	if (table == INVALID_STRING_TABLE)
		table = FindStringTable("EffectDispatch");

	int iIndex = FindStringIndex(table, sEffectName);

	if (iIndex != INVALID_STRING_INDEX)
		return iIndex;

	return 0;
}
public Action Timer_TeslaRechange(Handle timer, any userid)
{
	int client = GetClientOfUserId(userid);
	frPlayerWeaponFired[client] = false;
}

void FireTeslaBullet(int client, float pos[3])
{
	float muzzle_pos[3], camera_pos[3];
	GetWeaponAttachmentPosition(client, "muzzle_flash", muzzle_pos);
	GetWeaponAttachmentPosition(client, "camera_buymenu", camera_pos);
	
	//Create an offset for first person
	float pov_pos[3];
	pov_pos[0] = muzzle_pos[0] - camera_pos[0];
	pov_pos[1] = muzzle_pos[1] - camera_pos[1];
	pov_pos[2] = muzzle_pos[2] - camera_pos[2] + 0.1;
	ScaleVector(pov_pos, 0.4);
	SubtractVectors(muzzle_pos, pov_pos, pov_pos);
	
	//Move the beam a bit forward so it isn't too close for first person
	float distance = GetVectorDistance(pov_pos, pos);
	float percentage = 0.2 / (distance / 100);
	pov_pos[0] = pov_pos[0] + ((pos[0] - pov_pos[0]) * percentage);
	pov_pos[1] = pov_pos[1] + ((pos[1] - pov_pos[1]) * percentage);
	pov_pos[2] = pov_pos[2] + ((pos[2] - pov_pos[2]) * percentage);
	
	//Display the particle to first person 
	TE_DispatchEffect("weapon_tracers_taser", pov_pos, pos, g_fLastAngles[client]);
	TE_SendToClient(client);
	
	//Display the particle to everyone else under the normal position
	TE_DispatchEffect("weapon_tracers_taser", muzzle_pos, pos, g_fLastAngles[client]);
	
	int[] clients = new int[MaxClients];
	int client_count;
	for (int i = 1; i <= MaxClients; ++i)
	{
		if (!IsClientInGame(i) || i == client)
			continue;
		clients[client_count++] = i;
	}
	TE_Send(clients, client_count);
	
	//Move the impact glow a bit out so it doesn't clip the wall
	pos[0] = pos[0] + ((pov_pos[0] - pos[0]) * percentage);
	pos[1] = pos[1] + ((pov_pos[1] - pos[1]) * percentage);
	pos[2] = pos[2] + ((pov_pos[2] - pos[2]) * percentage);
		
	TE_DispatchEffect("weapon_taser_glow_impact", pos, pos);
	TE_SendToAll();
}
void CreateGrenadeExplosion(int entity, int client)
{
	float pos[3];
	GetEntPropVector(entity, Prop_Data, "m_vecOrigin", pos);
	float modelscale = GetEntPropFloat(entity, Prop_Send, "m_flModelScale");
	AcceptEntityInput(entity, "Kill"); //Kill it only if the attacker that did damage is a valid client, and after we save the position.
	int grenade = CreateEntityByName("hegrenade_projectile");
	if (IsValidEntity(grenade))
	{
		SetVariantString("OnUser1 !self,InitializeSpawnFromWorld,,0.0,1");
		AcceptEntityInput(grenade, "AddOutput");
		AcceptEntityInput(grenade, "FireUser1");
		DispatchSpawn(grenade);
		SetEntPropEnt(grenade, Prop_Data, "m_hThrower", client);
		SetEntProp(grenade, Prop_Data, "m_iTeamNum", GetClientTeam(client));
		TeleportEntity(grenade, pos, NULL_VECTOR, NULL_VECTOR);
		SetEntPropFloat(entity, Prop_Send, "m_flModelScale", modelscale);
	}
}

public Action Timer_ExplosiveRechange(Handle timer, any userid)
{
	int client = GetClientOfUserId(userid);
	frExplodeBulletCD[client] = false;
}

void CS_CreateExplosion(int client, int damage, int radius, float pos[3], const char[] classname)
{
	int entity;
	if((entity = CreateEntityByName("env_explosion")) != -1)
	{
		DispatchKeyValue(entity, "spawnflags", "552");
		DispatchKeyValue(entity, "rendermode", "5");
		DispatchKeyValue(entity, "classname", classname);
		SetEntProp(entity, Prop_Data, "m_iMagnitude", damage);
		SetEntProp(entity, Prop_Data, "m_iRadiusOverride", radius);
		SetEntProp(entity, Prop_Data, "m_iTeamNum", GetClientTeam(client));
		SetEntPropEnt(entity, Prop_Data, "m_hOwnerEntity", client);

		DispatchSpawn(entity);
		TeleportEntity(entity, pos, NULL_VECTOR, NULL_VECTOR);
		EmitAmbientSound("weapons/hegrenade/explode4.wav", pos, entity);
		RequestFrame(TriggerExplosion, entity);
	}
}

void TriggerExplosion(int entity)
{
	AcceptEntityInput(entity, "explode");
	AcceptEntityInput(entity, "Kill");
}
void GiveNade(int client)
{
	if (bInfNade)
		EquipPlayerWeapon(client, GivePlayerItem(client, InfNade));
}

void SetExplodeClient(int client, bool status = false)
{
	frExplodeBulletClient[client] = status;
}

void TimerMoneyCheck()
{
	char buffers[2][6] =  { "0", "0" };
	int sum = ExplodeString(TimerMoney, ";", buffers, sizeof(buffers), sizeof(buffers[]));
	if (sum < 2)
		return;
	if ((StrEqual(buffers[0], "0")) || StrEqual(buffers[1], "0"))
		return;
	float time = StringToFloat(buffers[0]);
	int money = StringToInt(buffers[1]);
	KillTimerHandle(frTimerMoney);
	frTimerMoney = CreateTimer(time, Timer_GiveMoney, money, TIMER_REPEAT);
}

public Action Timer_GiveMoney(Handle timer, int money)
{
	for (int client = 1; client <= MaxClients; ++client)
	{
		if (IsValidClient(client) && IsPlayerAlive(client))
		{
			SetMoney(client, GetMoney(client) + money);
		}
	}
}

void FR_GiveC4(int c4number)
{
	int tclient = 0;
	ArrayList clientlist = CreateArray();
	for (int i = 1; i <= MaxClients; ++i)
	{
		if (IsValidClient(i) && IsPlayerAlive(i) && GetClientTeam(i) == CS_TEAM_T)
		{
			clientlist.Push(i);
			++tclient;
		}
	}
	if (tclient > 0)
	{
		if (c4number > tclient)
			c4number = tclient;
		for (int i = 0; i < c4number; ++i)
		{
			int index = GetRandomInt(0, tclient - 1);
			--tclient;
			int select = clientlist.Get(index);
			clientlist.Erase(index);
			CSGOItems_GiveWeapon(select, "weapon_c4");
		}
	}
	clientlist.Close();
}

void FR_C4Bounce(int c4)
{
	KillTimerHandle(frC4Bounce[c4]);
	if (bC4Bounce[c4])
	{
		KillTimerHandle(frC4Bounce[c4]);
		frC4Bounce[c4] = CreateTimer(1.0, Timer_Bounce, c4, TIMER_REPEAT);
	}
}

public Action Timer_Bounce(Handle timer, int c4)
{
	if ((c4 != -1) && bC4Bounce[c4] && IsValidEntity(c4))
	{
		float bounce[3];
		bounce[0] = GetRandomFloat(-360.0, 360.0);
		bounce[1] = GetRandomFloat(-360.0, 360.0);
		bounce[2] = GetRandomFloat(300.0, 360.0);
		TeleportEntity(c4, NULL_VECTOR, NULL_VECTOR, bounce);
		//int Red[] =  { 255, 0, 0, 200 };
		//TE_SetupBeamFollow(c4, BeamIndex, 0, 0.8, 1.0, 1.0, 1, Red);
		//TE_SendToAll();
	}
}

void Weapon_Random(const int array, const int index, const int team)
{
	for (int client = 1; client <= MaxClients; ++client)
	{
		if (IsValidClient(client) && IsPlayerAlive(client) && TeamCheck(client, team))
		{
			GiveRoundWeapon(client, array, index);
			//CSGOItems_GiveWeapon(client, WeaponArray[index]);
		}
	}
}

void GiveRoundWeapon(const int client, const int arrayNum, const int index)
{
	if (arrayNum < 0 || arrayNum > 2)
		return;
	if (index < 0)
		return;
	switch(arrayNum)
	{
		case 0:
		{
			CSGOItems_GiveWeapon(client, WeaponPrimary[index], _, _, 1);
		}
		case 1:
		{
			CSGOItems_GiveWeapon(client, WeaponSecondary[index]);
		}
		case 2:
		{
			CSGOItems_GiveWeapon(client, OtherEquipment[index]);
		}
	}
}

void SetRespawnArms(int client)
{
	for (int i = 0; i < 2; ++i)
	{
		if (CurrentRoundWeapon[i] != -1)
		{
			GiveRoundWeapon(client, i, CurrentRoundWeapon[i]);
		}
		FrSetArmor(client);
	}
	if (StrEqual(NoKnife, "0") && !StrEqual(Weapon, "none") && GameRules_GetProp("m_bFreezePeriod") != 1)
		GivePlayerItem(client, "weapon_knife");
}

void ClearRoundWeapon()
{
	for (int i = 0; i < sizeof(CurrentRoundWeapon); ++i)
	{
		CurrentRoundWeapon[i] = -1;
	}
}