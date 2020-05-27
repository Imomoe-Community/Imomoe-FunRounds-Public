static int frPlayerTeam[MAXPLAYERS + 1] = 0;
static bool TeamStored = false;

void SetPlayerColor(int client)
{
	if (!IsValidClient(client) || !IsPlayerAlive(client) || bGhost) return;
	int R, G, B, A;
	if (StrEqual(PColor, "black"))
	{
		R = 0;
		G = 0;
		B = 0;
		A = 0;
	}
	else if (StrEqual(PColor, "pink"))
	{
		R = 255;
		G = 0;
		B = 255;
		A = 0;
	}
	else if (StrEqual(PColor, "green"))
	{
		R = 0;
		G = 255;
		B = 0;
		A = 0;
	}
	else if (StrEqual(PColor, "red"))
	{
		R = 255;
		G = 0;
		B = 0;
		A = 0;
	}
	else if (StrEqual(PColor, "blue"))
	{
		R = 0;
		G = 0;
		B = 255;
		A = 0;
	}
	else if (StrEqual(PColor, "null"))
	{
		R = 255;
		G = 255;
		B = 255;
		A = 0;
	}
	else if (StrEqual(PColor, "transcolor"))
	{
		R = 255;
		G = 255;
		B = 255;
		A = 127;
		SetEntityRenderMode(client, RENDER_TRANSCOLOR);
	}
	SetEntityRenderColor(client, R, G, B, A);
}

public Action Timer_HideHud(Handle timer)
{
	if (bHideHud)
	{
		for (int i = 1; i <= MaxClients; ++i)
		{
			if (IsValidClient(i) && IsPlayerAlive(i))
			{
				//#define HIDE_RADAR (1<<12)
				#define HIDE_CROSSHAIR (1<<4)
				#define HIDE_ALL (1<<2)
				//if (StrEqual(HideHud, "radar"))
				//	SetEntProp(i, Prop_Send, "m_iHideHUD", HIDE_RADAR);
				if (StrEqual(HideHud, "crosshair"))
					SetEntProp(i, Prop_Send, "m_iHideHUD", HIDE_CROSSHAIR);
				if (StrEqual(HideHud, "all"))
					SetEntProp(i, Prop_Send, "m_iHideHUD", HIDE_ALL);
			}
		}
	}
}

void StorePlayersTeam()
{
	if (TeamStored) return;
	SetCvar("mp_limitteams", "0");
	for (int client = 1; client <= MaxClients; ++client)
	{
		if (IsValidClient(client))
		{
			frPlayerTeam[client] = GetClientTeam(client);
			if (frPlayerTeam[client] != CS_TEAM_SPECTATOR) CS_SwitchTeam(client, CS_TEAM_CT);
		}
	}
	TeamStored = true;
}


void RestorePlayersTeam()
{
	if (!TeamStored) return;
	SetCvar("mp_limitteams", "2");
	for (int client = 1; client <= MaxClients; ++client)
	{
		if (IsValidClient(client) && frPlayerTeam[client] != CS_TEAM_SPECTATOR)
		{
			CS_SwitchTeam(client, frPlayerTeam[client]);
			frPlayerTeam[client] = 0;
		}
	}
	TeamStored = false;
}

void RespawnCheck()
{
	char buffers[3][6] =  { "0", "0", "0" };
	int sum = ExplodeString(AutoRespawn, ";", buffers, sizeof(buffers), sizeof(buffers[]));
	if (sum < 2)
		return;
	if (StrEqual(buffers[0], "0") || StrEqual(buffers[1], "0"))
		return;
	if (StrEqual(buffers[0], "all"))
		buffers[0] = "1";
	if (StrEqual(buffers[0], "t"))
		buffers[0] = "2";
	if (StrEqual(buffers[0], "ct"))
		buffers[0] = "3";
	int team = StringToInt(buffers[0]); 
	float time = StringToFloat(buffers[1]);
	int wave = StringToInt(buffers[2]);
	if (0 < team < 4 && time >= 0.0)
	{
		if (wave == 1)
			SetCvar("mp_use_respawn_waves", "1");
		if (team == CS_TEAM_T || team == 1)
		{
			if (wave == 1)
				SetCvar("mp_respawnwavetime_t", buffers[1]);
			SetCvar("mp_respawn_on_death_t", buffers[1]);
		}
		if (team == CS_TEAM_CT || team == 1)
		{
			if (wave == 1)
				SetCvar("mp_respawnwavetime_ct", buffers[1]);
			SetCvar("mp_respawn_on_death_ct", buffers[1]);
			
		}
	}
}

void DisableRespawn()
{
	SetCvar("mp_respawn_on_death_ct", "0");
	SetCvar("mp_respawn_on_death_t", "0");
	SetCvar("mp_respawnwavetime_ct", "0");
	SetCvar("mp_respawnwavetime_t", "0");
	SetCvar("mp_use_respawn_waves", "0");
}

void FrSetHealth(int client)
{
	int health;
	if (StrEqual(Health, "random"))
		health = Math_GetRandomInt(1, 1000);
	else
		health = StringToInt(Health);
	SetHealth(client, health);
}

void FrSetArmor(int client)
{
	int ap;
	if (StrEqual(Armor, "heavy"))
	{
		SetArmorValue(client, 200, true, true);
	}
	else if (StrEqual(Armor, "random"))
	{
		ap = Math_GetRandomInt(0, 1000);
		SetArmorValue(client, ap, Math_GetRandomInt(0, 1) == 1, Math_GetRandomInt(0, 1) == 1);
	}
	else
	{
		ap = StringToInt(Armor);
		SetArmorValue(client, ap, ap >= 1);
	}
}

void BRStart()
{
	KillTimerHandle(frCheckMoving);
	frCheckMoving = CreateTimer(3.0, Timer_CheckMoving, _, TIMER_REPEAT);
}

public Action Timer_CheckMoving(Handle timer)
{
	float vec[3];
	int buttons;
	float damage;
	for (int client = 1; client <= MaxClients; ++client)
	{
		if (IsValidClient(client) && IsPlayerAlive(client))
		{
			buttons = GetClientButtons(client);
			GetEntPropVector(client, Prop_Data, "m_vecVelocity", vec);
			if (IsPlayerNotMoving(buttons) && vec[0] < 55.0 && vec[1] < 55.0)
			{
				damage = GetHealth(client) * 0.15 + 0.0;
				if (damage < 10.0) damage = 10.0;
				CreateDamage(0.0, client, client, client, damage, DMG_POISON, client, true);
			}
		}
	}
}

void SetFFA(bool status)
{
	if (status)
	{
		SetCvar("mp_teammates_are_enemies", "1");
	}
	else
	{
		SetCvar("mp_teammates_are_enemies", "0");
	}
	NameStatus(status);
}

void NameStatus(bool status)
{
	if (status)
	{
		SetCvar("mp_playerid", "2");
		SetCvar("sv_teamid_overhead", "0");
	}
	else
	{
		SetCvar("mp_playerid", "0");
		SetCvar("sv_teamid_overhead", "1");
	}
}