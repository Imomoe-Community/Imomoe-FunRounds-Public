//Round Kv
KeyValues Kv;

//Timers
Handle frSpeedChange = INVALID_HANDLE;
Handle frC4Bounce[MAXENTITIES + 1] = INVALID_HANDLE;
Handle frHideHud = INVALID_HANDLE;
Handle frThirdperson = INVALID_HANDLE;
Handle frHealthLeak = INVALID_HANDLE;
Handle frChickenEating[MAXPLAYERS + 1] = INVALID_HANDLE;
Handle frTimerMoney = INVALID_HANDLE;
Handle frBossEvolve = INVALID_HANDLE;
Handle frCheckMoving = INVALID_HANDLE;

//Rounds
int INTRoundNumber;
int INTLastRoundNumber;
int frTKs[MAXPLAYERS + 1] = 0;
char KeyValuePath[PLATFORM_MAX_PATH];
char RoundNumber[3];
char RoundName[200];
char RoundInfo[200];
char ThirdPerson[3];
char Weapon[128];
char WeaponFilter[5];
char Health[10];
char Armor[10];
char GiveC4[3];
char TempFall[3];
char HeadShot[3];
char Gravity[64];
char PSpeed[10];
char InfAmmo[3];
char InfNade[20];
char DodgeBall[3];
char NoRecoil[3];
char NoKnife[3];
char RechargeTime[10];
char NoScope[3];
char HostTime[10];
char AutoBhop[3];
char Fov[10];
char DefuseMode[10];
char Vampire[3];
char FreeBuy[3];
char BackWards[3];
char SpeedChange[10];
char PColor[15];
char RecoilView[7];
char Friction[3];
char BombTime[7] = "60";
char FFF[3];
char Chicken[3];
char DamageMode[70];
char RandomSpawn[3];
char TeammateKill[3];
char RespawnPlayer[3];
char PMoney[10];
char TimerMoney[10] = "0;0";
char HideHud[10];
char FriendlyFire[10];
char FFA[3];
char Ghost[3];
char Chrono[3];
char C4Number[5];
char SuicideBomber[3];
char ExpGrenade[10];
char FastWeaponSwitch[3];
char BladeMail[10];
char ExplodeBullet[3];
char ThrowKnife[3];
char BossMode[3];
char AutoFire[3];
char WallHang[3];
char OneShot[3];
char ArmRace[3];
char DropWeapon[3];
char Aimbot[3];
char Wallhack[3];
char SelfDMG[10];
char ChickenEater[3];
char KnockBack[3];
char TeslaBullet[3];
char DefuseTime[10] = "5.0";
char PlantTime[10] = "3.0";
char ExoJump[3];
char MeleeRange[3];
char EnergyShield[3];
char GrappleHook[10];
char AutoStrafe[3];
char BossEvolve[3];
char AutoRespawn[10];
char BattleRoyale[10];

char LastRoundName[200] = "FunRounds!";
bool RoundNumbers[200] = false;
int RoundCounts = 0;

void BuildRounds()
{
	Kv = new KeyValues("Rounds");
	BuildPath(Path_SM, KeyValuePath, sizeof(KeyValuePath), "data/funrounds/rounds.txt");
	Kv.ImportFromFile(KeyValuePath);
	Kv.Rewind();
}

public Action Command_GetRound(int client, int args)
{
	bool force = false;
	GetCmdArg(1, RoundNumber, sizeof(RoundNumber));
	if (StrEqual(RoundNumber, "0"))
	{
		INTRoundNumber = 0;
	}
	else
	{
		INTRoundNumber = StringToInt(RoundNumber);
		force = true;
	}
	if (INTRoundNumber > cvMaxRounds.IntValue)
	{
		INTRoundNumber = cvMaxRounds.IntValue;
		if (force) force = false;
	}
	CreateRound(force);
	RemoveAllWeapon();
	PreStart();
	StartRound();
	return Plugin_Handled;
}
void ResetRounds()
{
	for (int i = 0; i <= cvMaxRounds.IntValue; ++i)
	{
		RoundNumbers[i] = false;
	}
	RoundCounts = 0;
}
void CreateRound(bool force = false)
{
	if (!cvPluginEnable.BoolValue) return;
	TurnSettingsOff();
	//Get round number
	if (RoundCounts >= cvMaxRounds.IntValue) ResetRounds();
	if (INTRoundNumber == 0)
	{
		INTRoundNumber = GetRandomInt(1, cvMaxRounds.IntValue);
	}
	if (!force)
	{
		if (INTLastRoundNumber == INTRoundNumber)
		{
			INTRoundNumber = GetRandomInt(1, cvMaxRounds.IntValue);
		}
		while (RoundNumbers[INTRoundNumber])
		{
			INTRoundNumber = GetRandomInt(1, cvMaxRounds.IntValue);
			if (INTLastRoundNumber == INTRoundNumber)
			{
				INTRoundNumber = GetRandomInt(1, cvMaxRounds.IntValue);
			}
			if (!RoundNumbers[INTRoundNumber]) continue;
		}
		++RoundCounts;
		RoundNumbers[INTRoundNumber] = true;
	}
	
	IntToString(INTRoundNumber, RoundNumber, sizeof(RoundNumber));
	BuildRounds();
	if (Kv.JumpToKey(RoundNumber))
	{
		Kv.GetString("name", RoundName, sizeof(RoundName), "暂无模式名称");
		Kv.GetString("info", RoundInfo, sizeof(RoundInfo), "暂无模式说明");
		Kv.GetString("thirdperson", ThirdPerson, sizeof(ThirdPerson), "0");
		Kv.GetString("weapon", Weapon, sizeof(Weapon), "none");
		Kv.GetString("weaponfilter", WeaponFilter, sizeof(WeaponFilter), "all");
		Kv.GetString("health", Health, sizeof(Health), "100");
		Kv.GetString("armor", Armor, sizeof(Armor), "0");
		Kv.GetString("c4", GiveC4, sizeof(GiveC4), "1");
		Kv.GetString("fall", TempFall, sizeof(TempFall), "0");
		Kv.GetString("headshot", HeadShot, sizeof(HeadShot), "0");
		Kv.GetString("gravity", Gravity, sizeof(Gravity), "1.0");
		Kv.GetString("speed", PSpeed, sizeof(PSpeed), "1.0");
		Kv.GetString("hosttime", HostTime, sizeof(HostTime), "1.0");
		Kv.GetString("infiniteammo", InfAmmo, sizeof(InfAmmo), "0");
		Kv.GetString("infinitenade", InfNade, sizeof(InfNade), "weapon_none");
		Kv.GetString("dodgeball", DodgeBall, sizeof(DodgeBall), "0");
		Kv.GetString("norecoil", NoRecoil, sizeof(NoRecoil), "0");
		Kv.GetString("noknife", NoKnife, sizeof(NoKnife), "0");
		Kv.GetString("zeus", RechargeTime, sizeof(RechargeTime), "-1");
		Kv.GetString("noscope", NoScope, sizeof(NoScope), "0");
		Kv.GetString("autobhop", AutoBhop, sizeof(AutoBhop), "0");
		Kv.GetString("friction", Friction, sizeof(Friction), "5.2");
		Kv.GetString("backwards", BackWards, sizeof(BackWards), "0");
		Kv.GetString("fff", FFF, sizeof(FFF), "0");
		Kv.GetString("fov", Fov, sizeof(Fov), "90");
		Kv.GetString("vampire", Vampire, sizeof(Vampire), "0");
		Kv.GetString("recoilview", RecoilView, sizeof(RecoilView), "0.055");
		Kv.GetString("freebuy", FreeBuy, sizeof(FreeBuy), "0");
		Kv.GetString("pcolor", PColor, sizeof(PColor), "null");
		Kv.GetString("chicken", Chicken, sizeof(Chicken), "0");
		Kv.GetString("damage", DamageMode, sizeof(DamageMode), "1");
		Kv.GetString("money", PMoney, sizeof(PMoney), "0");
		Kv.GetString("speedchange", SpeedChange, sizeof(SpeedChange), "0");
		Kv.GetString("randomspawn", RandomSpawn, sizeof(RandomSpawn), "0");
		Kv.GetString("friendlyfire", FriendlyFire, sizeof(FriendlyFire), "0");
		Kv.GetString("ffa", FFA, sizeof(FFA), "0");
		Kv.GetString("ghost", Ghost, sizeof(Ghost), "0");
		Kv.GetString("chrono", Chrono, sizeof(Chrono), "0");
		Kv.GetString("c4num", C4Number, sizeof(C4Number), "1");
		Kv.GetString("suicide", SuicideBomber, sizeof(SuicideBomber), "1");
		Kv.GetString("respawn", RespawnPlayer, sizeof(RespawnPlayer), "0");
		Kv.GetString("c4time", BombTime, sizeof(BombTime), "45");
		Kv.GetString("hidehud", HideHud, sizeof(HideHud), "none");
		Kv.GetString("nademode", ExpGrenade, sizeof(ExpGrenade), "none");
		Kv.GetString("defmode", DefuseMode ,sizeof(DefuseMode), "none");
		Kv.GetString("fws", FastWeaponSwitch, sizeof(FastWeaponSwitch), "0");
		Kv.GetString("tkallow", TeammateKill, sizeof(TeammateKill), "0");
		Kv.GetString("oneshot", OneShot, sizeof(OneShot), "0");
		Kv.GetString("blademail", BladeMail, sizeof(BladeMail), "0.0");
		Kv.GetString("drop", DropWeapon, sizeof(DropWeapon), "1");
		Kv.GetString("aimbot", Aimbot, sizeof(Aimbot), "0");
		Kv.GetString("wallhack", Wallhack, sizeof(Wallhack), "0");
		Kv.GetString("selfdmg", SelfDMG, sizeof(SelfDMG), "0.0");
		Kv.GetString("eb", ExplodeBullet, sizeof(ExplodeBullet), "-1");
		Kv.GetString("throwknife", ThrowKnife, sizeof(ThrowKnife), "0");	
		Kv.GetString("chickeneater", ChickenEater, sizeof(ChickenEater), "1");
		Kv.GetString("knockback", KnockBack, sizeof(KnockBack), "1");
		Kv.GetString("wallhang", WallHang, sizeof(WallHang), "0");
		Kv.GetString("tesla", TeslaBullet, sizeof(TeslaBullet), "0");
		Kv.GetString("deftime", DefuseTime, sizeof(DefuseTime), "5.0");
		Kv.GetString("planttime", PlantTime, sizeof(PlantTime), "3.0");
		Kv.GetString("exojump", ExoJump, sizeof(ExoJump), "0");
		Kv.GetString("meleerange", MeleeRange, sizeof(MeleeRange), "0");
		Kv.GetString("armrace", ArmRace, sizeof(ArmRace), "0");
		Kv.GetString("grapplehook", GrappleHook, sizeof(GrappleHook), "0");
		Kv.GetString("shield", EnergyShield, sizeof(EnergyShield), "0");
		Kv.GetString("autofire", AutoFire, sizeof(AutoFire), "0");
		Kv.GetString("autostrafe", AutoStrafe, sizeof(AutoStrafe), "0");
		Kv.GetString("bossevolve", BossEvolve, sizeof(BossEvolve), "0");
		Kv.GetString("timermoney", TimerMoney, sizeof(TimerMoney), "0;0");
		Kv.GetString("bossmode", BossMode, sizeof(BossMode), "0");
		Kv.GetString("autorespawn", AutoRespawn, sizeof(AutoRespawn), "0;0");
		Kv.GetString("br", BattleRoyale, sizeof(BattleRoyale), "0");
	}
	
	//print round name && round info
	PrintCenterTextAll("%s", RoundName);
	PrintToChatAll("\x3 \x4 ----------------------------------------");
	PrintToChatAll("\x3 \x4 %s", RoundName);
	PrintToChatAll("\x3 \x4 回合编号 : %s", RoundNumber);
	PrintToChatAll("\x3 \x4 %s", RoundInfo);
	PrintToChatAll("\x3 \x4 ----------------------------------------");
	PrintToConsoleAll("[妹萌え!]回合名称 : %s", RoundName);
	PrintToConsoleAll("[妹萌え!]回合编号 : %s", RoundNumber);
	//PrintToServer("Round Name: %s", RoundName);
	//PrintToServer("Round Number: %s", RoundNumber);
	SetCvar("mp_teamname_1", RoundName);
	SetCvar("mp_teamname_2", LastRoundName);
	
	//RandomSpawn
	SetCvar("mp_randomspawn", RandomSpawn);
	//FFA
	SetFFA(StringToInt(FFA) == 1);
	//RespawnPlayer
	if (StrEqual(RespawnPlayer, "1"))
	{
		CreateTimer(0.1, Timer_RespawnPlayers);
	}
	//HideHud--Radar
	if (StrEqual(HideHud, "radar"))
	{
		SetCvar("sv_disable_radar", "1");
	}
	//TP Toggle need Another Plugin
	//if (StrEqual(NoRecoil, "-1") || StringToFloat(RecoilView) > 0.055)
	//	ServerCommand("sm_allow_thirdperson 0");
	//Freebuy
	if (StrEqual(FreeBuy, "1"))
	{
		SetCvar("mp_buytime", "3600");
		SetCvar("mp_maxmoney", "65535");
		//SetCvar("mp_buy_anywhere", "1");
		//SetCvar("sv_buy_status_override", "0");
		//SetCvar("mp_weapons_allow_zeus", "0");
	}
	LastRoundName = RoundName;
	INTLastRoundNumber = INTRoundNumber;
}
public Action Timer_PreStart(Handle timer)
{
	PreStart();
}
void PreStart()
{
	//BossMode Zombie Mode
	if (!StrEqual(BossMode, "0"))
	{
		StorePlayersTeam();
	}
	//Money
	int Moneyvalue = StringToInt(PMoney);
	for (int client = 1; client <= MaxClients; ++client)
	{
		if (IsValidClient(client))
			SetMoney(client, Moneyvalue);
	}
}
void StartRound()
{
	if (!cvPluginEnable.BoolValue) return;
	int IntFov;
	IntFov = StringToInt(Fov);
	
	//Aimbot Ragebot
	if (StrEqual(Aimbot, "1") || StrEqual(Aimbot, "legit"))
	{
		bAimbot = true;
		SetAimbotCvar(20.0, 8000.0, 2, false);
	}
	if (StrEqual(Aimbot, "2") || StrEqual(Aimbot, "rage"))
	{
		bAimbot = true;
		SetAimbotCvar(360.0, 8000.0, 1, true);
	}
	//WallHack 
	if (StrEqual(Wallhack, "1"))
		ServerCommand("sm_giveesp @all 1");
	//ChickenEater
	if (!StrEqual(ChickenEater, "0"))
		bChickenEater = true;
	else
		bChickenEater = false;
	//Chrono
	if (StrEqual(Chrono, "1"))
		ChronoStatus(true);
	//FastWeaponSwitch
	if (StrEqual(FastWeaponSwitch, "1"))
		bFWS = true;
	else
		bFWS = false;
	//ThirdPerson
	if (StrEqual(ThirdPerson, "1"))
		EnableThirdPerson();
	//Weapons
	GiveWeapons();
	//InfNade
	if (StrEqual(InfNade, "weapon_decoy") || StrEqual(InfNade, "weapon_hegrenade") || StrEqual(InfNade, "weapon_flashbang") || StrEqual(InfNade, "weapon_molotov") || StrEqual(InfNade, "weapon_incgrenade") || StrEqual(InfNade, "weapon_smokegrenade") || StrEqual(InfNade, "weapon_tagrenade") || StrEqual(InfNade, "weapon_snowball"))
	{
		bInfNade = true;
	}
	//Chicken
	if (StrEqual(Chicken, "1"))
	{
		//SetCvar("sv_disable_immunity_alpha", "0");
		AllChickenStatus(true);
	}
	//
	if (StrEqual(DropWeapon, "0"))
	{
		SetCvar("mp_death_drop_gun", "0");
		SetCvar("mp_death_drop_grenade", "0");	
	}
	//FriendlyFire
	if (!StrEqual(FriendlyFire, "0"))
	{
		SetCvar("mp_friendlyfire", "1");
	}
	SetCvar("ff_damage_reduction_bullets", FriendlyFire);
	SetCvar("ff_damage_reduction_grenade", FriendlyFire);
	SetCvar("ff_damage_reduction_other", FriendlyFire);
	//Ghost
	int ghostteam = StringToInt(Ghost);
	EnableGhost(ghostteam);
	//HeadShot
	if (StrEqual(HeadShot, "1"))
		SetCvar("mp_damage_headshot_only", "1");
	//HideHud
	if (!StrEqual(HideHud, "none") && !StrEqual(HideHud, ""))
	{
		bHideHud = true;
		KillTimerHandle(frHideHud);
		frHideHud = CreateTimer(1.0, Timer_HideHud, _, TIMER_REPEAT);
	}
	//KnockBack
	if (!StrEqual(KnockBack, "0"))
		bKnockback = true;
	else
		bKnockback = false;
	//NoScope
	if (StrEqual(NoScope, "1"))
		bNoScope = true;
	//AutoBhop
	if (StrEqual(AutoBhop, "1"))
	{
		SetCvar("sv_autobunnyhopping", "1");
		SetCvar("sv_accelerate_use_weapon_speed", "0");
		SetCvar("sv_staminamax", "0");
		SetCvar("sv_staminajumpcost", "0");
		SetCvar("sv_staminalandcost", "0");
	}
	//Friction
	SetCvar("sv_friction", Friction);
	//BackWards
	if (StrEqual(BackWards, "1"))
		SetCvar("sv_accelerate", "-5.5");
	//FFF
	if (StrEqual(FFF, "1"))
		SetCvar("inferno_damage", "8192");
	//DodgeBall
	if (StrEqual(DodgeBall, "1"))
		bDodgeBall = true;
	//SelfDMG
	SetCvar("mp_weapon_self_inflict_amount", SelfDMG);
	//SpeedChange
	if (StrEqual(SpeedChange, "1"))
	{
		bSpeedChange = true;
		KillTimerHandle(frSpeedChange);
		frSpeedChange = CreateTimer(10.0, Timer_SpeedChange, _, TIMER_REPEAT);
	}
	//SuicideBomber
	if (StrEqual(SuicideBomber, "1"))
		bSuicide = true;
	else
		bSuicide = false;
	//HostTime
	if (!StrEqual(HostTime, ""))
		SetCvar("host_timescale", HostTime);
	//GiveC4
	if (StrEqual(GiveC4, "1"))
	{
		bBuyC4 = true;
		FR_GiveC4(StringToInt(C4Number));
	}
	//NoRecoil
	int recoilvalue = StringToInt(NoRecoil);
	SetRecoil(recoilvalue);
	//OneShot
	if (StrEqual(OneShot, "1"))
		bOneShot = true;
	//RechargeTime Zeus Taser
	SetCvar("mp_taser_recharge_time", RechargeTime);
	//RecoilView
	SetCvar("weapon_recoil_view_punch_extra", RecoilView);
	//TempFall
	if (!StrEqual(TempFall, "0"))
	{
		bFallDMG = true;
		SetCvar("sv_falldamage_scale", TempFall);
		SetCvar("sv_falldamage_exojump_multiplier", "0.4");
		SetCvar("sv_bounce", "0");
	}
	//ThrowKnife
	if (StrEqual(ThrowKnife, "1"))
		bThrowKnife = true;
	//Vampire
	if (StrEqual(Vampire, "1"))
		bVampire = true;
	//WallHang
	if (StrEqual(WallHang, "1"))
		bWallHang = true;
	//MeleeRange
	SetCvar("sv_knife_attack_extend_from_player_aabb", MeleeRange);
	if (!StrEqual(AutoFire, "0"))
		bAutoFire = true;
	//TimerMoney
	if (!StrEqual(TimerMoney, "0;0"))
		TimerMoneyCheck();
	//BossEvolve
	if (!StrEqual(BossEvolve, "0"))
	{
		bBossEvolve = true;
		KillTimerHandle(frBossEvolve);
		frBossEvolve = CreateTimer(5.0, Timer_BossEvolveGiveHealth, _, TIMER_REPEAT);
	}
	//AutoRespawn Check
	if (!StrEqual(AutoRespawn, "0;0"))
		RespawnCheck();
	//BattleRoyale Mai Editon
	if (!StrEqual(BattleRoyale, "0"))
		BRStart();
	for (int client = 1; client <= MaxClients; ++client)
	{
		if (IsValidClient(client))
		{
			//Gravity
			SetEntityGravity(client, StringToFloat(Gravity));
			//PColor
			SetPlayerColor(client);
			//Health
			FrSetHealth(client);
			//Armor
			FrSetArmor(client);
			//Speed
			SetSpeed(client);
			//Fov
			SetFov(client, IntFov);
			//PlantTime
			frPlantTime[client] = GetPlantTime();
			//exojump
			SetEntProp(client, Prop_Send, "m_passiveItems", StringToInt(ExoJump), 1, 1);
			//NoKnife
			if (StrEqual(NoKnife, "0"))
				//CSGOItems_GiveWeapon(x, "weapon_knife");
				GivePlayerItem(client, "weapon_knife");
			//EnergyShield
			if (!StrEqual(EnergyShield, "0"))
			{
				bEnergyShield = true;
				CreateTimer(0.5, Timer_EnergyShieldRecharge, GetClientUserId(client), TIMER_REPEAT);
			}
			//GrappleHook
			if (!StrEqual(GrappleHook, "0"))
			{
				SetGrappleHook(client);
			}
			//ThrowKnife
			if (bThrowKnife || !StrEqual(ThrowKnife, "0") || CheckBoss(client))
			{
				SetThrowKnife(client);
			}
			if (bAimbot || !StrEqual(Aimbot, "0"))
			{
				SetAimbot(client, true);
			}
			//ExplodeBullet
			if (StringToFloat(ExplodeBullet) >= 0.0)
			{
				SetExplodeClient(client, true);
			}
			if (!StrEqual(AutoStrafe, "0"))
			{
				SetAutoStrafe(client, true);
			}
		}
	}
	//Health Leak (Health > 100%)
	if (!StrEqual(Health, "random") && !bSpeedChange)
	{
		KillTimerHandle(frHealthLeak);
		frHealthLeak = CreateTimer(5.0, Timer_HealthLeak, _, TIMER_REPEAT);
	}
	//BossMode
	if (!StrEqual(BossMode, "0"))
		RandomSelectBoss(StringToInt(BossMode));
	
	delete Kv;
}

void TurnSettingsOff()
{
	for (int client = 1; client <= MaxClients; ++client)
	{
		if (IsValidClient(client) && IsPlayerAlive(client))
		{
			SetEntityGravity(client, 1.0);
			ClientCommand(client, "firstperson");
		}
	}
	SetAllAimbot(false);
}

void TurnOffAllSettings()
{
	if (!cvPluginEnable.BoolValue) return;
	TurnOffRounds();
	//Aimbot Ragebot
	bAimbot = false;
	SetAimbotCvar(20.0, 8000.0, 2, false);
	//Wallhack
	ServerCommand("sm_giveesp @all 0");
	//Chicken
	AllChickenStatus(false);
	CreateTimer(0.1, Timer_RemoveChicken);
	//ChickenEater
	bChickenEater = true;
	//Chrono
	ChronoStatus(false);
	//DropWeapon
	SetCvar("mp_death_drop_gun", "1");
	SetCvar("mp_death_drop_grenade", "1");	
	//FreeBuy
	SetCvar("mp_buytime", "0");
	SetCvar("mp_maxmoney", "0");
	//SetCvar("mp_buy_anywhere", "0");
	//SetCvar("sv_buy_status_override", "3");
	//SetCvar("mp_weapons_allow_zeus", "0");
	//BackWards
	SetCvar("sv_accelerate", "5.5");
	//GiveC4
	bBuyC4 = false;
	//DefMode
	SetCvar("mp_c4_cannot_be_defused", "0");
	//DodgeBall
	bDodgeBall = false;
	//FastWeaponSwitch
	bFWS = false;
	//FFA
	SetCvar("mp_teammates_are_enemies", "0");
	//FFF
	SetCvar("inferno_damage", "40");
	//FriendlyFire
	SetCvar("mp_friendlyfire", "0");
	SetCvar("ff_damage_reduction_bullets", "0");
	SetCvar("ff_damage_reduction_grenade", "0");
	SetCvar("ff_damage_reduction_other", "0");
	//Ghost
	DisableGhost();
	//HostTime
	SetCvar("host_timescale", "1.0");
	//NoRecoil
	SetRecoil(0);
	//NoScope
	bNoScope = false;
	//InfAmmo
	SetCvar("sv_infinite_ammo", "0");
	//InfNade
	bInfNade = false;
	//OneShot
	bOneShot = false;
	//RandomSpawn
	SetCvar("mp_randomspawn", "0");
	//RechargeTime Zeus Taser
	SetCvar("mp_taser_recharge_time", "-1");
	//RecoilView
	SetCvar("weapon_recoil_view_punch_extra", "0.055");
	//ServerCommand("sm_allow_thirdperson 1");
	//SelfDMG
	SetCvar("mp_weapon_self_inflict_amount", "0");
	//SuicideBomber
	bSuicide = false;
	//TempFall
	if(!cvFallDMG.BoolValue && bFallDMG)
	{
		bFallDMG = false;
		SetCvar("sv_bounce", "100");
		SetCvar("sv_falldamage_scale", "0");
		SetCvar("sv_falldamage_exojump_multiplier", "0");
	}
	//Thirdperson
	KillTimerHandle(frThirdperson);
	//ThrowKnife
	bThrowKnife = false;
	//Friction
	SetCvar("sv_friction", "5.2");
	//HeadShot
	SetCvar("mp_damage_headshot_only", "0");
	//HideHud
	KillTimerHandle(frHideHud);
	bHideHud = false;
	SetCvar("sv_disable_radar", "0");
	//SpeedChange
	bSpeedChange = false;
	KillTimerHandle(frSpeedChange);
	//DefMode
	c4Planted = false;
	c4weapon = -1;
	plantedC4 = -1;
	secondaryC4 = -1;
	plyCarryingC4 = -1;
	//Vampire
	bVampire = false;
	//WallHang
	bWallHang = false;
	//EnergyShield
	bEnergyShield = false;
	//AutoFire
	bAutoFire = false;
	//AutoBhop
	SetCvar("sv_autobunnyhopping", "0");
	SetCvar("sv_accelerate_use_weapon_speed", "1");
	SetCvar("sv_staminamax", "80");
	SetCvar("sv_staminajumpcost", "0.080");
	SetCvar("sv_staminalandcost", "0.050");
	//Health Leak
	KillTimerHandle(frHealthLeak);
	//TimerMoney
	KillTimerHandle(frTimerMoney);
	//BossEvolve
	bBossEvolve = false;
	KillTimerHandle(frBossEvolve);
	
	//BossMode, Zombie Mode
	RestorePlayersTeam();
	DisableRespawn();
	
	//BattleRoyale
	KillTimerHandle(frCheckMoving);
	//Weapon
	ClearRoundWeapon();
	for (int client = 1; client <= MaxClients; ++client)
	{
		carryingC4[client] = false;
		frBuyedHealthshot[client] = false;
		frAteChicken[client] = false;
		frAteChickenCount[client] = 0;
		if (IsValidClient(client) && IsPlayerAlive(client))
		{
			ClientCommand(client, "firstperson");
			SetEntityGravity(client, 1.0);
			SetFov(client, 90);
			SetMoney(client, 0);
			SetSpeed(client);
			SetEntityRenderMode(client, RENDER_NORMAL);
			SetEntityRenderColor(client, 255, 255, 255, 0);
		}
		frTKs[client] = 0;
		frPlayerDamageMultiplier[client] = 1.0;
		frPlayerSpeedMultiplier[client] = 1.0;
		frArmorValue[client] = 0;
		//ExplodeBullet
		SetExplodeClient(client);
		ClearChickenTimer(client);
		RemoveBoss(client);
		RemoveThrowKnife(client);
		SetCheats(client, false);
	}
	for (int entitys = 0; entitys <= MAXENTITIES; ++entitys)
	{
		KillTimerHandle(frC4Bounce[entitys]);
		botweapon[entitys] = false;
	}
}
void TurnOffRounds()
{
	ThirdPerson =  "0";
	Weapon = "none";
	WeaponFilter = "all";
	Health = "100";
	Armor = "0";
	GiveC4 = "1";
	TempFall = "0";
	HeadShot = "0";
	Gravity = "1.0";
	PSpeed = "1.0";
	HostTime = "1.0";
	InfAmmo = "0";
	InfNade = "weapon_none";
	DodgeBall = "0";
	NoRecoil = "0";
	NoKnife = "0";
	RechargeTime = "-1";
	NoScope = "0";
	AutoBhop = "0";
	Friction = "5.2";
	BackWards = "0";
	FFF = "0";
	Fov = "90";
	Vampire = "0";
	RecoilView = "0.055";
	FreeBuy = "0";
	PColor = "null";
	Chicken = "0";
	DamageMode = "1";
	PMoney = "0";
	SpeedChange = "0";
	RandomSpawn = "0";
	FriendlyFire = "0";
	FFA = "0";
	Ghost = "0";
	Chrono = "0";
	C4Number = "1";
	SuicideBomber = "1";
	RespawnPlayer = "0";
	BombTime = "60.0";
	HideHud = "none";
	ExpGrenade = "none";
	DefuseMode = "none";
	FastWeaponSwitch = "0";
	TeammateKill = "0";
	OneShot = "0";
	BladeMail = "0.0";
	DropWeapon = "1";
	Aimbot = "0";
	Wallhack = "0";
	SelfDMG = "0.0";
	ExplodeBullet = "-1";
	ThrowKnife = "0";	
	ChickenEater = "1";
	KnockBack = "1";
	WallHang = "0";
	TimerMoney = "0;0";
	BossMode = "0";
	AutoFire = "0";
	ArmRace = "0";
	TeslaBullet = "0";
	DefuseTime = "5.0";
	PlantTime = "3.0";
	ExoJump = "0";
	EnergyShield = "0";
	MeleeRange = "0";
	GrappleHook = "0";
	AutoStrafe = "0";
	BossEvolve = "0";
	AutoRespawn = "0;0";
	BattleRoyale = "0";
}
public Action Timer_RespawnPlayers(Handle timer)
{
	for (int client = 1; client <= MaxClients; ++client)
	{
		if (IsValidClient(client) && GetClientTeam(client) > 1)
			CS_RespawnPlayer(client);
	}
}

float GetPlantTime()
{
	float time = 3.0;
	if (StrEqual(PlantTime, "random"))
	{
		int random = Math_GetRandomInt(0, 100);
		if (70 >= random)
			time = GetRandomFloat(2.0, 5.0);
		else if (70 < random <= 85)
			time = GetRandomFloat(0.0, 3.0);
		else
			time = GetRandomFloat(3.0, 30.0);
	}
	else
		time = StringToFloat(PlantTime);
	return time;
}
void SetPlantTime(int client, int c4)
{
	SetEntPropFloat(c4, Prop_Send, "m_fArmedTime", GetGameTime() + frPlantTime[client]);
	if (frPlantTime[client] != 3.0) PrintToChat(client, "[SM] 下包时间为: %f 秒", frPlantTime[client]);
}
void SetDefuseTime(int client, int c4)
{
	float deftime = StringToFloat(DefuseTime);
	if (!GetEntProp(client, Prop_Send, "m_bHasDefuser"))
		deftime *= 2;
	if (c4 != -1)
	SetEntPropFloat(c4, Prop_Send, "m_flDefuseLength", deftime);
	SetEntPropFloat(c4, Prop_Send, "m_flDefuseCountDown", GetGameTime() + deftime);
	SetEntProp(client, Prop_Send, "m_iProgressBarDuration", RoundFloat(deftime));
}
public Action Timer_Plant(Handle timer, any userid)
{
	int client = GetClientOfUserId(userid);
	if (!IsPlayerAlive(client)) return;
	int c4 = GetEntPropEnt(client, Prop_Send, "m_hActiveWeapon");
	char weapon[64];
	GetEntityClassname(c4, weapon, sizeof(weapon));
	if (StrEqual(weapon, "weapon_c4"))
		SetPlantTime(client, c4);
}
public Action Timer_Defuse(Handle timer, any userid)
{
	int client = GetClientOfUserId(userid);
	if (!IsPlayerAlive(client)) return;
	int c4 = FindEntityByClassname(MaxClients + 1, "planted_c4");
	SetDefuseTime(client, c4);
}
void SetBombTimer(int c4)
{
	float time = StringToFloat(BombTime);
	SetEntPropFloat(c4, Prop_Send, "m_flC4Blow", GetGameTime() + time);
}

void SetRecoil(int value)
{
	switch(value)
	{
		case 0:
		{
			SetCvar("weapon_accuracy_forcespread", "0");
			SetCvar("weapon_accuracy_nospread", "0");
			SetCvar("weapon_accuracy_reset_on_deploy", "0");
			SetCvar("weapon_accuracy_shotgun_spread_patterns", "1");
			SetCvar("weapon_recoil_cooldown", "0.55");
			SetCvar("weapon_recoil_decay1_exp", "3.5");
			SetCvar("weapon_recoil_decay2_exp", "8");
			SetCvar("weapon_recoil_decay2_lin", "18");
			SetCvar("weapon_recoil_decay_coefficient", "2.0");
			SetCvar("weapon_recoil_scale", "2.0");
			SetCvar("weapon_recoil_scale_motion_controller", "1.0");
			SetCvar("weapon_recoil_suppression_factor", "0.75");
			SetCvar("weapon_recoil_suppression_shots", "4");
			SetCvar("weapon_recoil_variance", "0.55");
			SetCvar("weapon_recoil_vel_decay", "4.5");
		}
		case 1:
		{
			SetCvar("weapon_accuracy_nospread", "1");
			SetCvar("weapon_accuracy_reset_on_deploy", "1");
			SetCvar("weapon_accuracy_shotgun_spread_patterns", "0");
			SetCvar("weapon_recoil_cooldown", "0");
			SetCvar("weapon_recoil_decay1_exp", "0");
			SetCvar("weapon_recoil_decay2_exp", "0");
			SetCvar("weapon_recoil_decay2_lin", "0");
			SetCvar("weapon_recoil_decay_coefficient", "0");
			SetCvar("weapon_recoil_scale", "0");
			SetCvar("weapon_recoil_scale_motion_controller", "0");
			SetCvar("weapon_recoil_suppression_factor", "0");
			SetCvar("weapon_recoil_suppression_shots", "0");
			SetCvar("weapon_recoil_variance", "0");
			SetCvar("weapon_recoil_vel_decay", "99999");
		}
		case -1:
		{
			SetCvar("weapon_accuracy_forcespread", "0");
			SetCvar("weapon_accuracy_nospread", "0");
			SetCvar("weapon_accuracy_shotgun_spread_patterns", "0");
			SetCvar("weapon_recoil_cooldown", "99999");
			SetCvar("weapon_recoil_decay_coefficient", "99999");
			SetCvar("weapon_recoil_scale", "99999");
			SetCvar("weapon_recoil_scale_motion_controller", "99999");
			SetCvar("weapon_recoil_suppression_factor", "99999");
			SetCvar("weapon_recoil_suppression_shots", "1");
			SetCvar("weapon_recoil_variance", "1");
		}
	}
}