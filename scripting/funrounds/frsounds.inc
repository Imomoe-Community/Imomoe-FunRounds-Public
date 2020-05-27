void PreCacheListSound(char[][] list, int max)
{
	for (int i = 0; i < max; ++i)
	{
		if (!StrEqual(list[i], ""))
		{
			PreCacheSoundAndDownload(list[i]);
		}
	}
}

void PreCacheSoundAndDownload(char[] sound)
{
	char Path[PLATFORM_MAX_PATH];
	PrecacheSound(sound);
	Format(Path, sizeof(Path), "sound/%s", sound);
	AddFileToDownloadsTable(Path);
}

//frSounds
KeyValues SoundsKv;
char HeadShotSounds[PLATFORM_MAX_PATH][PLATFORM_MAX_PATH], KnifeSounds[PLATFORM_MAX_PATH][PLATFORM_MAX_PATH], InfernoSounds[PLATFORM_MAX_PATH][PLATFORM_MAX_PATH], GrenadeSounds[PLATFORM_MAX_PATH][PLATFORM_MAX_PATH], KillSounds[PLATFORM_MAX_PATH][PLATFORM_MAX_PATH], SuicideSounds[PLATFORM_MAX_PATH][PLATFORM_MAX_PATH], TeamKillSounds[PLATFORM_MAX_PATH][PLATFORM_MAX_PATH], RoundStartSounds[PLATFORM_MAX_PATH][PLATFORM_MAX_PATH], CTWinSounds[PLATFORM_MAX_PATH][PLATFORM_MAX_PATH], TRWinSounds[PLATFORM_MAX_PATH][PLATFORM_MAX_PATH], RoundDrawSounds[PLATFORM_MAX_PATH][PLATFORM_MAX_PATH], ServerJoinSounds[PLATFORM_MAX_PATH][PLATFORM_MAX_PATH];
int HeadShotMaxs, KnifeMaxs, InfernoMaxs, GrenadeMaxs, KillMaxs, SuicideMaxs, TeamKillMaxs, RoundStarts, CTWins, TRWins, RoundDraws, ServerJoins;
char BlindSound[][] =  { "funrounds/sounds/blind.mp3" };
char ChickenkillSounds[][] =  { "funrounds/sounds/chickensound.mp3" };

char ExplodeSound[] = "ambient/explosions/explode_8.mp3";
bool ClientTurnOffSounds[MAXPLAYERS + 1] = false;
void LoadSounds()
{
	SoundsKv = new KeyValues("frsounds");
	char SoundListConfig[PLATFORM_MAX_PATH];
	BuildPath(Path_SM, SoundListConfig, sizeof(SoundListConfig), "data/funrounds/frsounds.txt");
	if (SoundsKv.ImportFromFile(SoundListConfig))
	{
		SoundsKv.Rewind();
		CalcSounds("headshot", HeadShotSounds, sizeof(HeadShotSounds), HeadShotMaxs);
		CalcSounds("knife", KnifeSounds, sizeof(KnifeSounds), KnifeMaxs);
		CalcSounds("inferno", InfernoSounds, sizeof(InfernoSounds), InfernoMaxs);
		CalcSounds("grenade", GrenadeSounds, sizeof(GrenadeSounds), GrenadeMaxs);
		CalcSounds("otherkill", KillSounds, sizeof(KillSounds), KillMaxs);
		CalcSounds("suicide", SuicideSounds, sizeof(SuicideSounds), SuicideMaxs);
		CalcSounds("teamkill", TeamKillSounds, sizeof(TeamKillSounds), TeamKillMaxs);
		CalcSounds("roundstart", RoundStartSounds, sizeof(RoundStartSounds), RoundStarts);
		CalcSounds("ctwin", CTWinSounds, sizeof(CTWinSounds), CTWins);
		CalcSounds("trwin", TRWinSounds, sizeof(TRWinSounds), TRWins);
		CalcSounds("rounddraw", RoundDrawSounds, sizeof(RoundDrawSounds), RoundDraws);
		CalcSounds("joinserver", ServerJoinSounds, sizeof(ServerJoinSounds), ServerJoins);
	}
	delete SoundsKv;
	PreCacheListSound(BlindSound, sizeof(BlindSound));
	PreCacheListSound(ChickenkillSounds, sizeof(ChickenkillSounds));
}

void CalcSounds(char[] keytype, char[][] list, int maxsize, int &maxnum)
{
	if (SoundsKv.JumpToKey(keytype))
	{
		if (SoundsKv.GotoFirstSubKey())
		{
			char buffer[255];
			int num;
			do
			{
				SoundsKv.GetSectionName(buffer, sizeof(buffer));
				num = StringToInt(buffer);
				if (num >= 0)
				{
					SoundsKv.GetString("sound", list[num], maxsize);
				}
			}
			while (SoundsKv.GotoNextKey());
			{
				SoundsKv.GoBack();
			}
			maxnum = num;
			//PrintToServer("keytype: %s , maxnum: %i", keytype, maxnum);
		}
	}
	SoundsKv.Rewind();
	PreCacheListSound(list, maxnum + 1);
}

void PlayKillSounds(int client, int attacker, char[] weapon, bool headshot)
{
	if (StrEqual(weapon, "chicken"))
	{
		EmitSoundToAll(ChickenkillSounds[0], client, GetRandomInt(10, 120));
		//PlaySounds(ChickenkillSounds, 1, GetRandomInt(10, 120));
	}
	else
	{
		if (attacker == client || !IsValidClient(attacker))
		{
			PlaySounds(SuicideSounds, SuicideMaxs);
		}
		else
		{
			if (IsValidClient(attacker) && IsValidClient(client))
			{
				if (!PlayerIsEnemy(client, attacker))
				{
					PlaySounds(TeamKillSounds, TeamKillMaxs);
				}
				else if (headshot)
				{
					PlaySounds(HeadShotSounds, HeadShotMaxs);
				}
				else if (StrEqual(weapon, "knife") || StrEqual(weapon, "bayonet") || StrEqual(weapon, "fists"))
				{
					PlaySounds(KnifeSounds, KnifeMaxs);
				}
				else if (StrEqual(weapon, "inferno"))
				{
					PlaySounds(InfernoSounds, InfernoMaxs);
				}
				else if (StrEqual(weapon, "hegrenade") || StrEqual(weapon, "flashbang") || StrEqual(weapon, "smokegrenade") || StrEqual(weapon, "decoy") || StrEqual(weapon, "snowball") || StrEqual(weapon, "diversion") || StrEqual(weapon, "tagrenade") || StrEqual(weapon, "firebomb") || StrEqual(weapon, "incgrenade") || StrEqual(weapon, "molotov"))
				{
					PlaySounds(GrenadeSounds, GrenadeMaxs);
				}
				else
				{
					PlaySounds(KillSounds, KillMaxs);
				}
			}
		}
	}
}

void RoundStartSound()
{
	PlaySounds(RoundStartSounds, RoundStarts);
}

void RoundEndSound(int team)
{
	if (team == CS_TEAM_T)
	{
		PlaySounds(TRWinSounds, TRWins);
	}
	else if (team == CS_TEAM_CT)
	{
		PlaySounds(CTWinSounds, CTWins);
	}
	else if (team == 4)
	{
		PlaySounds(RoundDrawSounds, RoundDraws);
	}
}	

void PlayJoinSound(int client)
{
	int randomcount = GetRandomInt(1, ServerJoins);
	EmitSoundCustom(client, ServerJoinSounds[randomcount], _, GetRandomInt(10, 120));
}
void PlayBlindSound(int client)
{
	EmitSoundToAll(BlindSound[0], client, GetRandomInt(10, 120));
}

void PlaySounds(char[][] list, int maxnum)
{
	int randomcount = GetRandomInt(1, maxnum);
	int channel = GetRandomInt(10, 120);
	for (int clients = 0; clients <= MaxClients; ++clients)
	{
		if (IsValidClient(clients) && !IsFakeClient(clients) && !ClientTurnOffSounds[clients])
		{
			EmitSoundCustom(clients, list[randomcount], _, channel);
		}
	}
}

stock void EmitSoundCustom(int client, const char[] sample, int entity = SOUND_FROM_PLAYER, int channel = SNDCHAN_USER_BASE, int level = SNDLEVEL_NORMAL, int flags = SND_CHANGEVOL, float volume = 0.7, int pitch = SNDPITCH_NORMAL, int speakerentity = -1, const float origin[3] = NULL_VECTOR, const float dir[3] = NULL_VECTOR, bool updatePos = true, float soundtime = 0.0)
{
	int clientarray[1];
	clientarray[0] = client;
	EmitSound(clientarray[0], 1, sample, entity, channel, level, flags, volume, pitch, speakerentity, origin, dir, updatePos, soundtime);
}