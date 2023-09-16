#define MAXENTITIES 2048
char WeaponPrimary[25][] =
{
	"weapon_ak47", "weapon_aug", "weapon_bizon", "weapon_famas", "weapon_g3sg1", "weapon_galilar", "weapon_m249", "weapon_m4a1", "weapon_mac10", "weapon_mag7", "weapon_mp7", "weapon_mp9", "weapon_negev", "weapon_nova", "weapon_p90", "weapon_sawedoff", "weapon_scar20", "weapon_sg556", "weapon_ssg08", "weapon_ump45", "weapon_xm1014", "weapon_awp", "weapon_m4a1_silencer", "weapon_mp5sd" , ""
};

char WeaponSecondary[11][] =
{
	"weapon_deagle", "weapon_elite", "weapon_fiveseven", "weapon_glock", "weapon_hkp2000", "weapon_p250", "weapon_tec9", "weapon_usp_silencer", "weapon_cz75a", "weapon_revolver", ""
};

char OtherEquipment[22][] =
{
	"weapon_shield", "weapon_healthshot", "weapon_breachcharge", "weapon_tablet", "weapon_taser", "weapon_hegrenade", "weapon_incgrenade", "weapon_flashbang", "weapon_molotov", "weapon_smokegrenade", "weapon_decoy", "weapon_firebomb", "weapon_diversion", "weapon_bumpmine", "weapon_tagrenade", "weapon_snowball", "weapon_knifegg", "weapon_fists", "weapon_axe", "weapon_hammer", "weapon_spanner", ""
};

char SemiAutoWeapons[13][] =
{
	"weapon_nova", "weapon_mag7", "weapon_sawedoff", "weapon_ssg08", "weapon_awp", "weapon_deagle", "weapon_elite", "weapon_fiveseven", "weapon_glock", "weapon_hkp2000", "weapon_p250", "weapon_tec9", "weapon_usp_silencer"
};

int GrenadesAll[] = 
{
	15, 17, 16, 14, 18, 17
};

int TRWin[6] = { 0, 3, 8, 12, 17, 18 };
int CTWin[9] = { 4, 5, 6, 7, 10, 11, 13, 16, 19 };
const int RoundDraw = 9;

stock int Math_GetRandomInt(int min, int max)
{
	int random = GetURandomInt();
	if (random == 0)
	{
		random++;
	}
	return RoundToCeil(float(random) / (float(2147483647) / float(max - min + 1))) + min - 1;
}
stock bool RemoveWeaponBySlot(int client, int slot)
{
	int entity = GetPlayerWeaponSlot(client, slot);
	if(IsValidEdict(entity))
	{
		CSGOItems_RemoveWeapon(client, entity);
		RemovePlayerItem(client, entity); 
		AcceptEntityInput(entity, "Kill");
		RemoveEntity(entity);
		return true;
	}
	return false;
}

stock void SetActiveWeapon(int client, int weapon)
{
	SetEntPropEnt(client, Prop_Data, "m_hActiveWeapon", weapon);
	ChangeEdictState(client, FindDataMapInfo(client, "m_hActiveWeapon"));
}

stock int GetWeaponsOffset(int client)
{
	static int offset = -1;
	if (offset == -1)
	{
		offset = FindDataMapInfo(client, "m_hMyWeapons");
	}
	
	return offset;
}
stock void SetCvar(char[] scvar, char[] svalue, bool hide = true)
{
	ConVar cvar = FindConVar(scvar);
	if (hide)
	{
		int flags = cvar.Flags;
		flags &= ~FCVAR_NOTIFY;
		cvar.Flags = flags;
	}
	cvar.SetString(svalue);
	cvar.Close();
}
stock void DisarmPlayerWeapons(int client)
{
	for (int i = 0; i < 14; ++i)
	{
		int weapon = -1;
		while ((weapon = GetPlayerWeaponSlot(client, i)) != -1)
		{
			if (IsValidEntity(weapon))
			{
				RemovePlayerItem(client, weapon);
				AcceptEntityInput(weapon, "Kill");
				RemoveEntity(weapon);
			}
		}
		SetEntPropEnt(client, Prop_Send, "m_hActiveWeapon", -1);
	}
}
stock void DropPlayerWeapons(int client)
{
	for (int i = 0; i < 14; ++i)
	{
		int weapon = -1;
		while ((weapon = GetPlayerWeaponSlot(client, i)) != -1)
		{
			if (IsValidEntity(weapon) && GetEntPropEnt(weapon, Prop_Send, "m_hOwner") == client)
			{
				CS_DropWeapon(client, weapon, true);
			}
		}
	}
}
stock bool IsValidClient(int client)
{
	if (!(1 <= client <= MaxClients) || !IsClientInGame(client))
	{
		return false;
	}
	return true;
}
stock void SetModelScaleRandom(int weapon)
{
	int random = Math_GetRandomInt(1, 5);
	if (random == 5)
		SetEntPropFloat(weapon, Prop_Send, "m_flModelScale", GetRandomFloat(3.0, 15.0));
	else if (random == 4)
		SetEntPropFloat(weapon, Prop_Send, "m_flModelScale", GetRandomFloat(0.5, 0.75));
	else
		SetEntPropFloat(weapon, Prop_Send, "m_flModelScale", GetRandomFloat(0.75, 3.00));
}
stock bool TeamCheck(int client, int team)
{
	if (team < 1 || team > 3)
		return false;
	if (team == 1 || GetClientTeam(client) == team)
		return true;
	return false;
}
stock int GetMoney(int client)
{
	if (IsValidClient(client))
	{
  		int offset = FindSendPropInfo("CCSPlayer", "m_iAccount");
  		return GetEntData(client, offset);
  	}
  	return 0;
}

stock void SetFov(int client, int fov)
{
	if (IsValidClient(client) && !IsFakeClient(client))
	{
		SetEntProp(client, Prop_Send, "m_iDefaultFOV", fov);
		SetEntProp(client, Prop_Send, "m_iFOV", fov);
	}
}
stock void SetMoney(int client, int money)
{
	if (IsValidClient(client))
	{
		SetEntData(client, FindSendPropInfo("CCSPlayer", "m_iAccount"), money);
	}
}
stock bool HasWeapon(int client, const char[] classname)
{
	int index;
	int weapon;
	char sName[64];
	
	while((weapon = GetNextWeapon(client, index)) != -1)
	{
		GetEdictClassname(weapon, sName, sizeof(sName));
		if (StrEqual(sName, classname))
			return true;
	}
	return false;
}

stock int GetNextWeapon(int client, int &weaponIndex)
{
	static int weaponsOffset = -1;
	if (weaponsOffset == -1)
		weaponsOffset = FindDataMapInfo(client, "m_hMyWeapons");
	
	int offset = weaponsOffset + (weaponIndex * 4);
	
	int weapon;
	while (weaponIndex < 48) 
	{
		weaponIndex++;
		
		weapon = GetEntDataEnt2(client, offset);
		
		if (IsValidEdict(weapon)) 
			return weapon;
		
		offset += 4;
	}
	
	return -1;
}
stock void SetHealth(int client, int hp, bool checkhealth = true)
{
	if (IsValidClient(client) && IsPlayerAlive(client))
	{
		if (hp < 1) hp = 1;
		//SetEntityHealth(client, hp);
		SetEntProp(client, Prop_Data, "m_iHealth", hp);
		SetEntProp(client, Prop_Data, "m_iMaxHealth", 1048576);  //mb not work
		if (checkhealth) CheckHealthPercent(client);
	}
}

stock void SetPlayerArmor(int client, int value = -1, bool helmet)
{
	if (value >= 0) SetEntProp(client, Prop_Send, "m_ArmorValue", value);
	if (helmet) SetEntProp(client, Prop_Send, "m_bHasHelmet", 1);
}
stock void DisarmArmor(int client)
{
	SetEntProp(client, Prop_Send, "m_bHasHeavyArmor", 0);
	SetEntProp(client, Prop_Send, "m_bHasHelmet", 0);
	SetEntProp(client, Prop_Send, "m_ArmorValue", 0);
	SetEntProp(client, Prop_Send, "m_bWearingSuit", 0);
}
stock int GetHealth(int client)
{
	return GetEntProp(client, Prop_Data, "m_iHealth");
}
stock bool ClientCanSeeTarget(int client, int target, float distance = 0.0, float height = 50.0)
{
	float clientPosition[3]; float targetPosition[3];
	
	GetEntPropVector(client, Prop_Send, "m_vecOrigin", clientPosition);
	clientPosition[2] += height;
	
	GetClientEyePosition(target, targetPosition);
	
	if (distance == 0.0 || GetVectorDistance(clientPosition, targetPosition, false) < distance)
	{
		Handle hTrace = TR_TraceRayFilterEx(clientPosition, targetPosition, MASK_SOLID_BRUSHONLY, RayType_EndPoint, Base_TraceFilter);
		
		if (TR_DidHit(hTrace))
		{
			delete hTrace;
			return false;
		}
		
		delete hTrace;
		return true;
	}
	
	return false;
}
bool Base_TraceFilter(int entity, int contentsMask, int iData)
{
	return entity == iData;
}

stock bool PlayerIsEnemy(int client, int target)
{
	ConVar cvar = FindConVar("mp_teammates_are_enemies");
	if (cvar.BoolValue) return true;
	if (GetClientTeam(client) != GetClientTeam(target)) return true;
	return false;
}

stock bool IsSemiAutoWeapon(int weapon)
{
	char classname[64];
	CSGOItems_GetWeaponClassNameByWeapon(weapon, classname, sizeof(classname));
	for (int list = 0; list < sizeof(SemiAutoWeapons); ++list)
	{
		if (StrEqual(classname, SemiAutoWeapons[list]))
		{
			return true;
		}
	}
	return false;
}

stock bool IsScopeWeapon(int weapon)
{
	char classname[64];
	CSGOItems_GetWeaponClassNameByWeapon(weapon, classname, sizeof(classname));
	if (StrEqual(classname, "weapon_awp") || StrEqual(classname, "weapon_ssg08") || StrEqual(classname, "weapon_sg556") || StrEqual(classname, "weapon_aug") || StrEqual(classname, "weapon_g3sg1") || StrEqual(classname, "weapon_scar20"))
		return true;
	return false;
}

stock bool IsCTReason(int reason)
{
	for(int i = 0; i < sizeof(CTWin) ; ++i)
	{
		if (CTWin[i] == reason) return true;
	}
	return false;
}

stock bool IsTRReason(int reason)
{
	for(int i = 0; i < sizeof(TRWin); ++i)
	{
		if (TRWin[i] == reason) return true;
	}

	return false;
}

stock bool IsDrawReason(int reason)
{
	if (RoundDraw == reason) return true;
	return false;
}

stock int GetWinner(int reason)
{
	if (IsTRReason(reason))
		return 2;
	if (IsCTReason(reason))
		return 3;
	if (IsDrawReason(reason))
		return 4;
	return 0;
}

stock void SendCvar(int client, char[] scvar, char[] svalue)
{
	ConVar cvar = FindConVar(scvar);
	cvar.ReplicateToClient(client, svalue);
	cvar.Close();
}

stock void SendCvarDefault(int client, char[] scvar)
{
	ConVar cvar = FindConVar(scvar);
	char buffer[64];
	cvar.GetDefault(buffer, sizeof(buffer));
	cvar.ReplicateToClient(client, buffer);
}

stock void SendCvarServerString(int client, char[] scvar)
{
	ConVar cvar = FindConVar(scvar);
	char buffer[64];
	cvar.GetString(buffer, sizeof(buffer));
	cvar.ReplicateToClient(client, buffer);
}

stock char[] GetCvarString(char[] scvar)
{
	ConVar cvar = FindConVar(scvar);
	char buffer[64];
	cvar.GetString(buffer, sizeof(buffer));
	return buffer;
}

stock bool KillTimerHandle(Handle &timer)
{
	if (timer != INVALID_HANDLE)
	{
		KillTimer(timer);
		timer = INVALID_HANDLE;
	}
	return true;
}

stock bool IsPlayerHasHelmet(int client)
{
	if (GetEntProp(client, Prop_Send, "m_bHasHelmet") == 1)
		return true;
	return false;
}

stock bool IsPlayerNotMoving(int buttons)
{
	return !IsMoveButtonsPressed(buttons);
}
stock bool IsPlayerWalking(int buttons, int flags)
{
	return IsMoveButtonsPressed(buttons) && (buttons & IN_SPEED || flags & FL_DUCKING);
}

stock bool IsMoveButtonsPressed(int buttons)
{
	return buttons & IN_FORWARD || buttons & IN_BACK || buttons & IN_MOVELEFT || buttons & IN_MOVERIGHT;
}

stock bool IsPlayerAttacking(int buttons)
{
	return buttons & IN_ATTACK || buttons & IN_ATTACK2 || buttons & IN_ATTACK3 || buttons & IN_RELOAD || buttons & IN_USE;
}

stock int ReplacePlayerWeapon(const int client, const int slot, const char[] classname)
{
	int weapon = GetPlayerWeaponSlot(client, slot);

	if (IsValidEntity(weapon))
	{
		if (GetEntPropEnt(weapon, Prop_Send, "m_hOwnerEntity") != client)
			SetEntPropEnt(weapon, Prop_Send, "m_hOwnerEntity", client);

		CS_DropWeapon(client, weapon, false, true);
	}

	weapon = GivePlayerItem(client, classname);

	if (IsValidEntity(weapon))
		EquipPlayerWeapon(client, weapon);

	return weapon;
}

stock char[] GetActiveWeaponClassname(int client)
{
	char classname[64];
	int weapon = CSGOItems_GetActiveWeapon(client);//GetEntPropEnt(client, Prop_Data, "m_hActiveWeapon");
	if (IsValidEntity(weapon))
		GetEntityClassname(weapon, classname, sizeof(classname));
	return classname;
}