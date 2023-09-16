#define CT_KNIFE_MODEL "models/weapons/w_knife_default_ct_dropped.mdl"
#define T_KNIFE_MODEL "models/weapons/w_knife_default_t_dropped.mdl"

#define TKMODELSCALE 1.0
#define TKELAST 0.2
#define TKGRAVITY 0.85
#define TKVELOCITY 3500.0
#define TKLIFE 2.5

bool frThrowKnifePlayer[MAXPLAYERS + 1];

int fkBeam = -1;
int frKnifeModel = -1;

static float tkSpinValue[] =  { 4000.0, 0.0, 0.0 };
static float ThrowKnifeDamage[2] =  { 80.0, 250.0 };


void frThrowKnifeStart()
{
	fkBeam = PrecacheModel("effects/blueblacklargebeam.vmt");
}

void ThrowKnifeStatus(int client, bool status = false)
{
	frThrowKnifePlayer[client] = status;
}

void ThrowingKnife(int client)
{
	int knifeslot = GetPlayerWeaponSlot(client, CS_SLOT_KNIFE);
	int tk = CreateEntityByName("smokegrenade_projectile");
	if (knifeslot != -1 && DispatchSpawn(tk))
	{
		int team = GetClientTeam(client);
		SetEntPropEnt(tk, Prop_Send, "m_hOwnerEntity", client);
		SetEntPropEnt(tk, Prop_Send, "m_hThrower", client);
		SetEntProp(tk, Prop_Send, "m_iTeamNum", team);
		DispatchKeyValue(tk, "classname", "weapon_knife_projectile");
		
		char tkmodel[PLATFORM_MAX_PATH];
		GetEntPropString(knifeslot, Prop_Data, "m_ModelName", tkmodel, sizeof(tkmodel));
		
		if (ReplaceString(tkmodel, sizeof(tkmodel), ".mdl", "_dropped.mdl", true) != 1)
			tkmodel[0] = '\0';
		
		if (!FileExists(tkmodel, true))
			Format(tkmodel, sizeof(tkmodel), "%s", team == CS_TEAM_T ? T_KNIFE_MODEL : CT_KNIFE_MODEL);
		
		if (frKnifeModel == -1)
			frKnifeModel = PrecacheModel(tkmodel);
		
		SetEntProp(tk, Prop_Send, "m_nModelIndex", frKnifeModel);
		SetEntPropFloat(tk, Prop_Send, "m_flModelScale", TKMODELSCALE);
		SetEntPropFloat(tk, Prop_Send, "m_flElasticity", TKELAST);
		SetEntPropFloat(tk, Prop_Data, "m_flGravity", TKGRAVITY);
		
		float pos[3], ang[3], tkpos[3], vec[3], tkvec[3];
		GetClientEyePosition(client, pos);
		GetClientEyeAngles(client, ang);
		
		GetAngleVectors(ang, tkpos, NULL_VECTOR, NULL_VECTOR);
		ScaleVector(tkpos, 50.0);
		AddVectors(tkpos, pos, tkpos);
		
		GetEntPropVector(client, Prop_Data, "m_vecVelocity", vec);
		GetAngleVectors(ang, tkvec, NULL_VECTOR, NULL_VECTOR);
		ScaleVector(tkvec, TKVELOCITY);
		AddVectors(tkvec, vec, tkvec);
		
		SetEntPropVector(tk, Prop_Data, "m_vecAngVelocity", tkSpinValue);
		SetEntProp(tk, Prop_Data, "m_nNextThinkTick", -1);
		char buffer[25];
		Format(buffer, sizeof(buffer), "!self,Kill,,%0.1f,-1", TKLIFE);
		DispatchKeyValue(tk, "OnUser1", buffer);
		AcceptEntityInput(tk, "FireUser1");
		
		int color[4] =  { 255, ... };
		TE_SetupBeamFollow(tk, fkBeam,	0, 0.5, 1.0, 0.1, 0, color);
		TE_SendToAll();
		TeleportEntity(tk, tkpos, ang, tkvec);
		SDKHookEx(tk, SDKHook_Touch, ThrowKnifeHit);
	}
}

public Action ThrowKnifeHit(int knife, int victim)
{
	if (IsValidClient(victim) && IsPlayerAlive(victim))
	{
		SetVariantString("csblood");
		AcceptEntityInput(knife, "DispatchEffect");
		AcceptEntityInput(knife, "Kill");
		SDKUnhook(knife, SDKHook_Touch, ThrowKnifeHit);
		int attacker = GetEntPropEnt(knife, Prop_Send, "m_hThrower");
		int inflictor = GetPlayerWeaponSlot(attacker, CS_SLOT_KNIFE);
		if (inflictor == -1)
			inflictor = attacker;
			
		float pos[3], vec[3], victimpos[3];
		GetEntPropVector(knife, Prop_Data, "m_vecOrigin", pos);
		GetEntPropVector(knife, Prop_Data, "m_vecVelocity", vec);
		GetClientEyePosition(victim, victimpos);
		if (GetVectorLength(vec) != 0.0)
		{
			float distance = GetVectorDistance(pos, victimpos);
			bool hs = (distance <= 18.0);
			int damagetype = DMG_SLASH | DMG_NEVERGIB;
			if (hs)
				damagetype |= CS_DMG_HEADSHOT;
			SDKHooks_TakeDamage(victim, inflictor, attacker, hs ? ThrowKnifeDamage[1] : ThrowKnifeDamage[0], damagetype, knife, vec, pos);
			int color[] = {255, 0, 0, 255};
			float dir[3];

			TE_SetupBloodSprite(pos, dir, color, 1, PrecacheDecal("sprites/blood.vmt"), PrecacheDecal("sprites/blood.vmt"));
			TE_SendToAll();
			
			int ragdoll = GetEntPropEnt(victim, Prop_Send, "m_hRagdoll");
			if (ragdoll != -1)
			{
				ScaleVector(vec, 50.0);
				vec[2] = FloatAbs(vec[2]);
				SetEntPropVector(ragdoll, Prop_Send, "m_vecForce", vec);
				SetEntPropVector(ragdoll, Prop_Send, "m_vecRagdollVelocity", vec);
			}
		}
		else return Plugin_Continue;
	}
	else
	{
		char classname[64];
		GetEntityClassname(victim, classname, sizeof(classname));
		if (StrContains(classname, "_projectile") > 0)
		{
			float pos[3], dir[3];
			GetEntPropVector(knife, Prop_Data, "m_vecOrigin", pos);
			TE_SetupArmorRicochet(pos, dir);
			TE_SendToAll(0.0);

			DispatchKeyValue(knife, "OnUser1", "!self,Kill,,1.0,-1");
			AcceptEntityInput(knife, "FireUser1");
		
			CreateTimer(1.0, Timer_RemoveTK, knife);
		}
	}
	return Plugin_Continue;
}

Action Timer_RemoveTK(Handle timer, int knife)
{
	if (IsValidEntity(knife))
	{
		SDKUnhook(knife, SDKHook_Touch, ThrowKnifeHit);
		AcceptEntityInput(knife, "Kill");
	}
	return Plugin_Continue;
}