//ConVars
ConVar cvPluginEnable;
ConVar cvMaxRounds;
ConVar cvEnableBuyC4;
ConVar cvFallDMG;
ConVar cvFrChicken;
ConVar cvFrBreathSound;
ConVar cvSuicideBomber;
ConVar cvChickenEater;
ConVar cvKnockBack;

//global bools
bool bC4Bounce[MAXENTITIES + 1] = false;
bool bGhost;
bool bChrono;
bool bHideHud;
bool bInfNade;
bool bDodgeBall;
bool bSpeedChange;
bool bFallDMG;
bool bSuicide;
bool bChickenEater;
bool bKnockback;
bool bBuyC4;
bool bWallHang;
bool bOneShot;
bool bNoScope;
bool bFWS;
bool bVampire;
bool bEnergyShield;
bool bThrowKnife;
bool bAutoFire;
bool bAimbot = false;
bool bBossEvolve;
bool frRoundEnd;
bool frGodMode[MAXPLAYERS + 1] = false;
bool frKnockBack[MAXPLAYERS + 1] = true;
bool IsPlayerWallHanged[MAXPLAYERS + 1] = false;
bool c4Planted = false;
bool carryingC4[MAXPLAYERS + 1] = false;
bool frBuyedHealthshot[MAXPLAYERS + 1] = false;
bool frExplodeBulletCD[MAXPLAYERS + 1] = false;
bool frAteChicken[MAXPLAYERS + 1] = false;

//int BeamIndex;
int ChickenC4;
int plyCarryingC4;
int plantedC4;
int secondaryC4;
int c4weapon;
bool botweapon[MAXENTITIES + 1] = false;
float frPlantTime[MAXPLAYERS + 1] = 3.0;
float emptyVector[3];
int frAteChickenCount[MAXPLAYERS + 1] = 0;
int frArmorValue[MAXPLAYERS + 1] = 0;
//Health Percent Base Multiplier
float frPlayerSpeedMultiplier[MAXPLAYERS + 1] = 1.0; 
float frPlayerDamageMultiplier[MAXPLAYERS + 1] = 1.0;


//Dead Player Explode
int ExplosionSprite;
int SmokeSprite;


void BuildFRCvars()
{
	RegAdminCmd("sm_getround", Command_GetRound, ADMFLAG_KICK, "Get round");
	RegConsoleCmd("sm_c4", Command_BuyC4);
	RegConsoleCmd("sm_nvg", Command_NVG);
	RegConsoleCmd("sm_knock", Command_KnockBack);
	RegConsoleCmd("sm_heavyarmor", Command_HeavyArmor);
	RegConsoleCmd("sm_tablet", Command_Tablet);
	RegConsoleCmd("sm_healthshot", Command_Healthshot);
	RegConsoleCmd("sm_shield", Command_Shield);
	RegConsoleCmd("sm_bump", Command_Bumpmine);
	RegConsoleCmd("sm_chicken", Command_Chicken);
	RegConsoleCmd("sm_frmenu", Command_FRCommands);
	RegConsoleCmd("buyammo2", Command_FRCommands);
	RegConsoleCmd("fr_togglesound", Command_ToggleSound);
	RegConsoleCmd("sm_nade", Command_NadeBug);
	RegConsoleCmd("fr_suicide", Command_GoSuicide);
	RegConsoleCmd("sm_aimbot", Command_ToggleAimbot);
	
	cvPluginEnable = CreateConVar("fr_enable", "1", "Enable/disable FunRounds.", FCVAR_NOTIFY);
	cvMaxRounds = CreateConVar("fr_maxrounds", "86", "FunRounds! max rounds.");
	cvEnableBuyC4 = CreateConVar("fr_buyc4", "1", "Enable/disable Buy C4.");
	cvFallDMG = CreateConVar("fr_falldamage", "0", "Enable/disable fall damage");
	cvFrChicken = CreateConVar("fr_cmovemode", "1", "Change chicken move mode", FCVAR_NOTIFY);
	cvFrBreathSound = CreateConVar("fr_gbsound", "10.0", "ghost player breath sound timer");
	cvSuicideBomber = CreateConVar("fr_suicidebomber", "1", "Enable/disable Suicide Bomber");
	cvChickenEater = CreateConVar("fr_chickeneater", "1", "Enable/disable eat chicken");
	cvKnockBack = CreateConVar("fr_knockback", "1", "Enable/disable Shoot KnockBack");
}
public Action Command_BuyC4(int client, int args)
{
	if (cvEnableBuyC4.BoolValue && bBuyC4 && (GetClientTeam(client) == CS_TEAM_T))
	{
		if (CS_GetClientContributionScore(client) < 5 && GetMoney(client) < 10000)
		{
			ReplyToCommand(client, "您没有10000金钱或5分数 无法购买c4");
			return Plugin_Stop;
		}
		if (GetMoney(client) >= 10000)
		{
			SetMoney(client, GetMoney(client) - 10000);
			EquipPlayerWeapon(client, GivePlayerItem(client, "weapon_c4"));
		} else if (CS_GetClientContributionScore(client) >= 5)
		{
			CS_SetClientContributionScore(client, CS_GetClientContributionScore(client) - 5);
			EquipPlayerWeapon(client, GivePlayerItem(client, "weapon_c4"));
		}
		//CSGOItems_GiveWeapon(client, "weapon_c4");
	}
	return Plugin_Handled;
}

public Action Command_NVG(int client, int args)
{
	if (IsValidClient(client) && IsPlayerAlive(client))
	{
		if(GetEntProp(client, Prop_Send, "m_bNightVisionOn") == 0)
		{
			SetEntProp(client, Prop_Send, "m_bNightVisionOn", 1);
		}
		else
		{
			SetEntProp(client, Prop_Send, "m_bNightVisionOn", 0);
		}
	}
	return Plugin_Handled;
}

public Action Command_KnockBack(int client, int args)
{
	if (IsValidClient(client))
	{
		if (bKnockback && cvKnockBack.BoolValue && cvPluginEnable.BoolValue)
		{
			frKnockBack[client] = !frKnockBack[client];
			PrintToChat(client, "[SM] 开枪击退已%s", frKnockBack[client] ? "开启" : "关闭");
		}
		else
			PrintToChat(client, "[SM] 开枪击退现在不可用");
	}
	return Plugin_Handled;
}
public Action Command_HeavyArmor(int client, int args)
{
	if (IsValidClient(client) && IsPlayerAlive(client) && cvPluginEnable.BoolValue)
	{
		if (frBuyedHealthshot[client])
		{
			PrintToChat(client, "[SM] 您已经有医疗针，不可以购买重甲");
			return Plugin_Stop;
		}
		char name[128];
		GetClientName(client, name, sizeof(name));
		if (GetMoney(client) >= 16000 && GetEntProp(client, Prop_Send, "m_bHasHeavyArmor") == 0)
		{
			SetMoney(client, GetMoney(client) - 16000);
			SetEntProp(client, Prop_Send, "m_bHasHelmet", 1);
			SetEntProp(client, Prop_Send, "m_bHasHeavyArmor", 1);
			SetEntProp(client, Prop_Send, "m_bWearingSuit", 1);
			SetEntProp(client, Prop_Data, "m_ArmorValue", 200);
			PrintToChatAll("[SM] \x3 \x4%s 购买了重甲！", name);
			return Plugin_Handled;
		} else if (GetMoney(client) >= 6000 && GetEntProp(client, Prop_Send, "m_bHasHeavyArmor") == 1 && GetEntProp(client, Prop_Data, "m_ArmorValue") < 200)
		{
			SetEntProp(client, Prop_Send, "m_bHasHelmet", 1);
			SetEntProp(client, Prop_Send, "m_bHasHeavyArmor", 1);
			SetEntProp(client, Prop_Send, "m_bWearingSuit", 1);
			SetEntProp(client, Prop_Data, "m_ArmorValue", 200);
		}
	}
	return Plugin_Handled;
}
public Action Command_Shield(int client, int args)
{
	if (IsValidClient(client) && IsPlayerAlive(client) && cvPluginEnable.BoolValue)
	{
		if (CS_GetClientContributionScore(client) < 2 && GetMoney(client) < 1000)
		{
			PrintToChat(client, "[SM] 您没有1000金钱或2分数 无法购买盾牌");
			return Plugin_Stop;
		}
		if (GetMoney(client) >= 1000)
		{
			SetMoney(client, GetMoney(client) - 1000);
			EquipPlayerWeapon(client, GivePlayerItem(client, "weapon_shield"));
		} else if (CS_GetClientContributionScore(client) >= 2)
		{
			CS_SetClientContributionScore(client, CS_GetClientContributionScore(client) - 2);
			EquipPlayerWeapon(client, GivePlayerItem(client, "weapon_shield"));
		}
	}
	return Plugin_Handled;
}
public Action Command_Healthshot(int client, int args)
{
	if (IsValidClient(client) && IsPlayerAlive(client) && cvPluginEnable.BoolValue)
	{
		if (GetEntProp(client, Prop_Send, "m_bHasHeavyArmor") == 1)
		{
			PrintToChat(client, "[SM] 您已经有重甲，不可以购买医疗针");
			return Plugin_Stop;
		}
		if (CS_GetClientContributionScore(client) < 5 && GetMoney(client) < 4000)
		{
			PrintToChat(client, "[SM] 您没有4000金钱或5分数 无法购买医疗针");
			return Plugin_Stop;
		}
		if (GetMoney(client) >= 4000)
		{
			SetMoney(client, GetMoney(client) - 4000);
			EquipPlayerWeapon(client, GivePlayerItem(client, "weapon_healthshot"));
			frBuyedHealthshot[client] = true;
			
		} else if (CS_GetClientContributionScore(client) >= 5)
		{
			CS_SetClientContributionScore(client, CS_GetClientContributionScore(client) - 5);
			EquipPlayerWeapon(client, GivePlayerItem(client, "weapon_healthshot"));
			frBuyedHealthshot[client] = true;
		}
	}
	return Plugin_Handled;
}
public Action Command_Bumpmine(int client, int args)
{
	if (IsValidClient(client) && IsPlayerAlive(client) && cvPluginEnable.BoolValue)
	{
		if (CS_GetClientContributionScore(client) < 2 && GetMoney(client) < 500)
		{
			PrintToChat(client, "[SM] 您没有500金钱或2分数 无法购买");
			return Plugin_Stop;
		}
		if (GetMoney(client) >= 4000)
		{
			SetMoney(client, GetMoney(client) - 500);
			EquipPlayerWeapon(client, GivePlayerItem(client, "weapon_bumpmine"));
		} else if (CS_GetClientContributionScore(client) >= 2)
		{
			CS_SetClientContributionScore(client, CS_GetClientContributionScore(client) - 2);
			EquipPlayerWeapon(client, GivePlayerItem(client, "weapon_bumpmine"));
		}
	}
	return Plugin_Handled;
}
public Action Command_Chicken(int client, int args)
{
	if (IsValidClient(client) && IsPlayerAlive(client))
	{
		if (CS_GetClientContributionScore(client) < 1 && GetMoney(client) < 1)
		{
			PrintToChat(client, "[SM] 您没有1金钱或1分数 无法购买小鸡");
			return Plugin_Stop;
		}
		if (GetMoney(client) >= 1)
		{
			SetMoney(client, GetMoney(client) - 1);
			GivePlayerItem(client, "chicken");
		} else if (CS_GetClientContributionScore(client) >= 2)
		{
			CS_SetClientContributionScore(client, CS_GetClientContributionScore(client) - 1);
			GivePlayerItem(client, "chicken");
		}
	}
	return Plugin_Handled;
}
public Action Command_Tablet(int client, int args)
{
	if (IsValidClient(client) && IsPlayerAlive(client))
	{
		EquipPlayerWeapon(client, GivePlayerItem(client, "weapon_tablet"));
	}
	return Plugin_Handled;
}
public Action Command_ToggleSound(int client, int args)
{
	if (IsValidClient(client))
	{
		ClientTurnOffSounds[client] = !ClientTurnOffSounds[client];
		PrintToChat(client, "[SM] 音效系统%s", ClientTurnOffSounds[client] ? "关闭" : "开启");
	}
}

public int FrMenu(Menu menu, MenuAction action, int param1, int param2) 
{
	switch (action)
	{
		case MenuAction_Select:
		{
			char MenuString[32];
			menu.GetItem(param2, MenuString, sizeof(MenuString));
			ClientCommand(param1, "%s", MenuString);
		}
		case MenuAction_End:
		{
			menu.Close();
		}
	}
}