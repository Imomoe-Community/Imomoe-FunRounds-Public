static bool frIsAimbotPlayer[MAXPLAYERS + 1];
bool frAimbotEnable[MAXPLAYERS + 1];

static float frAimbotFov = 20.0;
static float frAimbotDistance = 8000.0;
static int frAimbotRecoilMode = 1;
static bool frAimbotAutoAim = true;

static bool frAutoStrafe[MAXPLAYERS + 1];
static float frStrafeGain[MAXPLAYERS + 1];

void SetCheats(int client, bool status)
{
	ToggleAimbot(client, status);
	SetAutoStrafe(client, status);
}

void SetAimbot(int client, bool status)
{
	frIsAimbotPlayer[client] = status;
	ToggleAimbot(client, status);
}

void SetAllAimbot(bool status)
{
	for (int client = 1; client <= MaxClients; ++client)
	{
		SetAimbot(client, status);
	}
}

void SetAutoStrafe(int client, bool status = false)
{
	frAutoStrafe[client] = status;
	frStrafeGain[client] = 0.0;
	if (status) ToggleStrafe(client);
}

void ToggleStrafe(int client)
{
	if (IsValidClient(client) && !IsFakeClient(client))
	{
		if (frAutoStrafe[client])
		{
			SendCvar(client, "sv_autobunnyhopping", "1");
			SendCvar(client, "sv_accelerate_use_weapon_speed", "0");
			SendCvar(client, "sv_staminajumpcost", "0");
			SendCvar(client, "sv_staminalandcost", "0");
		}
		else
		{
			SendCvarServerString(client, "sv_autobunnyhopping");
			SendCvarServerString(client, "sv_accelerate_use_weapon_speed");
			SendCvarServerString(client, "sv_staminajumpcost");
			SendCvarServerString(client, "sv_staminalandcost");
		}
		
	}
	
}
void SetAimbotCvar(float fov = 20.0, float distance = 8000.0, int recoil = 1, bool autoaim = true)
{
	frAimbotFov = fov;
	frAimbotDistance = distance;
	frAimbotRecoilMode = recoil;
	frAimbotAutoAim = autoaim;
}

public Action Command_ToggleAimbot(int client, int args)
{
	if (IsValidClient(client))
	{
		frAimbotEnable[client] = !frAimbotEnable[client];
		PrintToChat(client, "[SM] 自动瞄准对你已%s", frAimbotEnable[client] ? "开启" : "关闭");
	}
	return Plugin_Handled;
}

stock bool IsAimbotavailable(int client)
{
	if (frIsAimbotPlayer[client] && frAimbotEnable[client] && !IsFakeClient(client))
		return true;
	return false;
}

void SendNoRecoil(int client, bool force = false)
{
	if (frAimbotRecoilMode == 1 || force)
	{
		SendCvar(client, "weapon_accuracy_nospread", "1");
		SendCvar(client, "weapon_recoil_cooldown", "0");
		SendCvar(client, "weapon_recoil_decay1_exp", "0");
		SendCvar(client, "weapon_recoil_decay2_exp", "0");
		SendCvar(client, "weapon_recoil_decay2_lin", "0");
		SendCvar(client, "weapon_recoil_scale", "0");
		SendCvar(client, "weapon_recoil_suppression_shots", "0");
		SendCvar(client, "weapon_recoil_variance", "0");
		SendCvar(client, "weapon_recoil_view_punch_extra", "0");
	}
	else
	{
		SendCvarServerString(client, "weapon_accuracy_nospread");
		SendCvarServerString(client, "weapon_recoil_cooldown");
		SendCvarServerString(client, "weapon_recoil_decay1_exp");
		SendCvarServerString(client, "weapon_recoil_decay2_exp");
		SendCvarServerString(client, "weapon_recoil_decay2_lin");
		SendCvarServerString(client, "weapon_recoil_scale");
		SendCvarServerString(client, "weapon_recoil_suppression_shots");
		SendCvarServerString(client, "weapon_recoil_variance");
		SendCvarServerString(client, "weapon_recoil_view_punch_extra");
	}
}

void ToggleAimbot(int client, bool status)
{
	if (IsValidClient(client) && !IsFakeClient(client) && frAimbotEnable[client])
	{
		frIsAimbotPlayer[client] = status;
		SendNoRecoil(client);
	}
}

void frTriggerDetect(int client, int &buttons)
{
	int AutoFireMode = StringToInt(AutoFire);
	if ((buttons & IN_RELOAD) || (buttons & IN_USE)) return;
	int target = GetClientAimTarget(client, true);
	int ActiveWeapon = CSGOItems_GetActiveWeapon(client);
	int slot = CSGOItems_GetActiveWeaponSlot(client);
	if (IsFakeClient(client))
	{
		char classname[64];
		CSGOItems_GetWeaponClassNameByWeapon(ActiveWeapon, classname, sizeof(classname));
		if (StrEqual(classname, "weapon_breachcharge") && (buttons & IN_ATTACK))
		{
			if (Math_GetRandomInt(0, 2) == 0)
			{
				buttons &= ~IN_ATTACK;
				buttons |= IN_ATTACK2;
			}
		}
	}
	if (IsValidClient(target) && PlayerIsEnemy(client, target) && ClientCanSeeTarget(client, target))
	{
		if ((slot == CS_SLOT_SECONDARY || slot == CS_SLOT_PRIMARY))
		{
			if (IsSemiAutoWeapon(ActiveWeapon))
			{
				return;
			}
		}
		if (IsScopeWeapon(ActiveWeapon))
		{
			if (GetEntProp(client, Prop_Data, "m_iFOV") == GetEntProp(client, Prop_Data, "m_iDefaultFOV"))
			{
				buttons |= IN_ATTACK2;
				//buttons |= IN_ZOOM;
			}
			else
				buttons |= IN_ATTACK;
		}
		if (!IsSemiAutoWeapon(ActiveWeapon) || !IsScopeWeapon(ActiveWeapon))
			buttons |= IN_ATTACK;
		if (AutoFireMode == 2 && GetEntityFlags(client) & FL_ONGROUND)
		{
			buttons |= IN_DUCK;
		}
		if (frThrowKnifePlayer[client] && ActiveWeapon == CS_SLOT_KNIFE && IsFakeClient(client))
			buttons |= IN_ATTACK;
	}
}

stock int GetClosestClient(int client)
{
	float clientOrigin[3], targetOrigin[3];
	
	GetClientAbsOrigin(client, clientOrigin);
	
	int target = -1;
	
	float closestDistance = -1.0;
	float targetDistance;
	
	for (int i = 1; i <= MaxClients; i++)
	{
		if (IsValidClient(i))
		{
			if (client == i || !PlayerIsEnemy(client, i) || !IsPlayerAlive(i))
			{
				continue;
			}
			
			GetClientAbsOrigin(i, targetOrigin);
			targetDistance = GetVectorDistance(clientOrigin, targetOrigin);

			if (targetDistance > closestDistance && closestDistance > -1.0)
			{
				continue;
			}

			if (!ClientCanSeeTarget(client, i))
			{
				continue;
			}

			if (GetEntPropFloat(i, Prop_Send, "m_fImmuneToGunGameDamageTime") > 0.0)
			{
				continue;
			}

			if (frAimbotDistance != 0.0 && targetDistance > frAimbotDistance)
			{
				continue;
			}
			
			if (frAimbotFov != 0.0 && !IsTargetInSightRange(client, i, frAimbotFov, frAimbotDistance))
			{
				continue;
			}
			
			closestDistance = targetDistance;
			target = i;
		}
	}
	
	return target;
}

stock bool IsTargetInSightRange(int client, int target, float angle = 90.0, float distance = 0.0, bool heightcheck = true, bool negativeangle = false)
{
	if (angle > 360.0)
		angle = 360.0;
	
	if (angle < 0.0)
		return false;
	
	float clientpos[3];
	float targetpos[3];
	float anglevector[3];
	float targetvector[3];
	float resultangle;
	float resultdistance;
	
	GetClientEyeAngles(client, anglevector);
	anglevector[0] = anglevector[2] = 0.0;
	GetAngleVectors(anglevector, anglevector, NULL_VECTOR, NULL_VECTOR);
	NormalizeVector(anglevector, anglevector);
	if (negativeangle)
		NegateVector(anglevector);
	
	GetClientAbsOrigin(client, clientpos);
	GetClientAbsOrigin(target, targetpos);
	
	if (heightcheck && distance > 0)
		resultdistance = GetVectorDistance(clientpos, targetpos);
	
	clientpos[2] = targetpos[2] = 0.0;
	MakeVectorFromPoints(clientpos, targetpos, targetvector);
	NormalizeVector(targetvector, targetvector);
	
	resultangle = RadToDeg(ArcCosine(GetVectorDotProduct(targetvector, anglevector)));
	
	if (resultangle <= angle / 2)
	{
		if (distance > 0)
		{
			if (!heightcheck)
				resultdistance = GetVectorDistance(clientpos, targetpos);
			
			if (distance >= resultdistance)
				return true;
			else return false;
		}
		else return true;
	}
	
	return false;
}

void SetClientEyePos(int client)
{
	int target = GetClosestClient(client);
	if (target > 0)
	{
		float targetPos[3]; float targetAngles[3]; float clientPos[3]; float finalPos[3];
		GetClientEyePosition(client, clientPos);
		GetClientEyePosition(target, targetPos);
		GetClientEyeAngles(target, targetAngles);
		
		float vecFinal[3];
		AddInFrontOf(targetPos, targetAngles, 7.0, vecFinal);
		MakeVectorFromPoints(clientPos, vecFinal, finalPos);
		
		GetVectorAngles(finalPos, finalPos);
		
		//Recoil Control System
		if (frAimbotRecoilMode == 2)
		{
			float vecPunchAngle[3];
			GetEntPropVector(client, Prop_Send, "m_aimPunchAngle", vecPunchAngle);
			//GetEntPropVector(client, Prop_Send, "m_vecPunchAngle", vecPunchAngle);
			
			float recoilscale = StringToFloat(GetCvarString("weapon_recoil_scale"));
			
			finalPos[0] -= vecPunchAngle[0] * recoilscale;
			finalPos[1] -= vecPunchAngle[1] * recoilscale;
		}
		
		TeleportEntity(client, NULL_VECTOR, finalPos, NULL_VECTOR);
	}
}

void SetAimbotViewPunch(int client, int weapon)
{
	SetEntPropFloat(weapon, Prop_Send, "m_fAccuracyPenalty", 0.0);
		
	if (frAimbotRecoilMode == 1)
	{
		SetEntPropVector(client, Prop_Send, "m_aimPunchAngle", NULL_VECTOR);
		SetEntPropVector(client, Prop_Send, "m_aimPunchAngleVel", NULL_VECTOR);
		SetEntPropVector(client, Prop_Send, "m_viewPunchAngle", NULL_VECTOR);
	}
}

void AutoAimCheck(int client, int &buttons)
{
	if ((buttons & IN_RELOAD) || (buttons & IN_USE)) return;
	int ActiveWeapon = CSGOItems_GetActiveWeapon(client);
	if (CSGOItems_IsValidWeapon(ActiveWeapon))
	{
		int ammo = GetEntProp(ActiveWeapon, Prop_Send, "m_iClip1");
		if (ammo > 0 && (frAimbotAutoAim || (buttons & IN_ATTACK)))
			SetClientEyePos(client);
	}
}

stock void AddInFrontOf(float vecOrigin[3], float vecAngle[3], float units, float outPut[3])
{
	float vecView[3];
	GetViewVector(vecAngle, vecView);
	
	outPut[0] = vecView[0] * units + vecOrigin[0];
	outPut[1] = vecView[1] * units + vecOrigin[1];
	outPut[2] = vecView[2] * units + vecOrigin[2];
}

stock void GetViewVector(float vecAngle[3], float outPut[3])
{
	outPut[0] = Cosine(vecAngle[1] / (180 / FLOAT_PI));
	outPut[1] = Sine(vecAngle[1] / (180 / FLOAT_PI));
	outPut[2] = -Sine(vecAngle[0] / (180 / FLOAT_PI));
}

void ApplyAutoStrafe(int client, int &buttons, float vel[3], float angles[3])
{
	if (frAutoStrafe[client])
	{
		// If they are currently on the ground, or on a ladder, disable the autostrafe
		if (GetEntityFlags(client) & FL_ONGROUND || GetEntityMoveType(client) & MOVETYPE_LADDER)
			return;
		
		// If they are currently pushing buttons, disable the autostrafe
		if (buttons & IN_MOVELEFT || buttons & IN_MOVERIGHT || buttons & IN_FORWARD || buttons & IN_BACK)
		{
			return;
		}
		float velocity[3];
		GetEntPropVector(client, Prop_Data, "m_vecVelocity", velocity);
		
		float flyVel = RadToDeg(ArcTangent2(velocity[1], velocity[0]));
		
		float diffAngle = NormalizeAngle(angles[1] - flyVel);
		
		float speed = 450.0;
		int random = GetRandomInt(0, 1);
		
		if (random == 1)
			speed = -speed;
		
		vel[1] = -speed;
		
		if (diffAngle > 0.0)
			vel[1] = -speed;
		
		// Check whether the player has tried to move their mouse more than the strafer
		float lastGain = frStrafeGain[client];
		float angleGain = RadToDeg(ArcTangent(vel[1] / vel[0]));
		
		// This check tells you when the mouse player movement is higher than the autostrafer one, and decide to put it or not
		if (!((lastGain < 0.0 && angleGain < 0.0) || (lastGain > 0.0 && angleGain > 0.0))) 
			angles[1] -= diffAngle;
		frStrafeGain[client] = angleGain;
	}
}

float NormalizeAngle(float angle)
{
	float temp = angle;
	
	while (temp <= -180.0)
	{
		temp += 360.0;
	}
	
	while (temp > 180.0)
	{
		temp -= 360.0;
	}
	
	return temp;
}

stock bool IsAutoStrafePlayer(int client)
{
	return frAutoStrafe[client];
}