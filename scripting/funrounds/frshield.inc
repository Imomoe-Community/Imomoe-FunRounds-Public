public Action Timer_EnergyShieldRecharge(Handle timer, any userid)
{
	int client = GetClientOfUserId(userid);
	if (!bEnergyShield || !IsValidClient(client) || !IsPlayerAlive(client)) return Plugin_Stop;
	int clientshield = GetEntProp(client, Prop_Send, "m_ArmorValue");
	if (clientshield <= 0) return Plugin_Stop;
	if (!StrEqual(Health, "random"))
	{
		int BaseHealth = StringToInt(Health);
		int clientHealth = GetHealth(client);
		if (clientHealth < BaseHealth)
		{
			clientshield -= BaseHealth - clientHealth;
			SetHealth(client, BaseHealth);
		}
	}
	SetArmorValue(client, clientshield + 1);
	return Plugin_Continue;
}
