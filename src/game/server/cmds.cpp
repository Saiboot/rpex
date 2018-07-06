#include "cmds.h"
#include "accsys\acc.h"

#include <stdio.h>

void toLowc(char *str)
{
	for (int i = 0; str[i]; i++) {
		str[i] = tolower(str[i]);
	}
}

bool cmdlist::IsAvailable(CPlayer *pPlayer, CGameContext *pGameServer)
{
	int ClientID = pPlayer->GetCID();
	if (!pPlayer->m_pAccount)
	{
		pGameServer->SendChatTarget(ClientID, "You are not logged in.");
		return false;
	}
	if (!pPlayer->GetCharacter() || !pPlayer->GetCharacter()->IsAlive())
	{
		pGameServer->SendChatTarget(ClientID, "You are dead.");
		return false;
	}
	return true;
}

void cmdlist::Issue_command(const char *pMsg, CPlayer *pPlayer, CGameContext *pGameServer)
{
	int ClientID = pPlayer->GetCID();

	//register
	if (!strncmp(pMsg, "/register", 9))
	{
		if (pPlayer->m_pAccount)
		{
			pGameServer->SendChatTarget(ClientID, "You are already registerd and logged in!");
			return;
		}
			
		char name[512];
		char pw[512];
		if (sscanf(pMsg, "/register %s %s", name, pw) != 2)
		{
			pGameServer->SendChatTarget(ClientID, "Please stick to the given structure:");
			pGameServer->SendChatTarget(ClientID, "/register <username> <password>");
			return;
		}

		CAccsys::New(name, pw, pPlayer);
			
		return;
	}

	//login
	else if (!strncmp(pMsg, "/login", 6))
	{
		if (pPlayer->m_pAccount)
		{
			pGameServer->SendChatTarget(ClientID, "You're already logged in.");
			return;
		}

		char name[512];
		char pw[512];

		if (sscanf(pMsg, "/login %s %s", name, pw) != 2)
		{
			pGameServer->SendChatTarget(ClientID, "Please stick to the given structure:");
			pGameServer->SendChatTarget(ClientID, "/login <username> <password>");
			return;
		}

		char File[64];
		str_format(File, sizeof(File), "Accounts/%s.%s", name, ACC_FILE_FORMAT);

		if (!CAccsys::DoesExist(File))
		{
			pGameServer->SendChatTarget(ClientID, "Account does not exist!");
			pGameServer->SendChatTarget(ClientID, File);
			return;
		}
		else
		{
			pPlayer->m_pAccount = CAccsys::Load(name, pw, pPlayer);
			pPlayer->SetTeam(TEAM_RED);
			return;
		}
	}

	//logout
	else if (!strcmp(pMsg, "/logout"))
	{
		if (!IsAvailable(pPlayer, pGameServer)) return;

		// update database
		CAccsys::Save(pPlayer->m_pAccount);
		delete pPlayer->m_pAccount;
		pPlayer->m_pAccount = 0;
		pPlayer->SetTeam(TEAM_SPECTATORS);

		pGameServer->SendChatTarget(ClientID, "You're now logged out.");
		return;
	}

	//update (manually)
	else if (!strcmp(pMsg, "/update"))
	{
		if (!IsAvailable(pPlayer, pGameServer)) return;

		// update database
		CAccsys::Save(pPlayer->m_pAccount);
		pGameServer->SendChatTarget(ClientID, "Account updated succesfully.");
		return;
	}

	//info
	else if (!strcmp(pMsg, "/info"))
	{
		pGameServer->SendChatTarget(ClientID, "   =Info=");
		pGameServer->SendChatTarget(ClientID, "mod: aWorld");
		pGameServer->SendChatTarget(ClientID, "by: Amol");
		pGameServer->SendChatTarget(ClientID, "   =----=");
		return;
	}

	//cmdlist
	else if (!strcmp(pMsg, "/cmdlist"))
	{
		pGameServer->SendChatTarget(ClientID, "issue: /login  -to gain access to play.");
		pGameServer->SendChatTarget(ClientID, "/info  -for further information");
		return;
	}

	//upgr
	else if (!strncmp(pMsg, "/upgr", 5))
	{
		if (!IsAvailable(pPlayer, pGameServer)) return;

		char statb[512];
		if (!strcmp(pMsg, "/upgr"))
		{
			pGameServer->SendChatTarget(ClientID, "Available upgrades:");
			pGameServer->SendChatTarget(ClientID, "life, armor & handle (rapid fire)");
			pGameServer->SendChatTarget(ClientID, "Weapons: hammer, gun, shotgun, grenade and rifle");
			return;
		}
		else if (sscanf(pMsg, "/upgr %s", statb) != 1)
		{
			pGameServer->SendChatTarget(ClientID, "Please stick to the given structure:");
			pGameServer->SendChatTarget(ClientID, "/upgr <upgr-type>");
			pGameServer->SendChatTarget(ClientID, "/upgr  -for available upgrades.");
			return;
		}

		if (!pPlayer->m_pAccount->m_SP)
		{
			pGameServer->SendChatTarget(ClientID, "Not enough stat points to upgrade.");
			return;
		}

		toLowc(statb);

		for (size_t i = 0; i < UPGRADES; i++)
			if (!strcmp(statb, UpgradeTypes[i])) 
			{
				pPlayer->m_pAccount->m_Stats[i]++;
				pPlayer->m_pAccount->m_SP--;

				char msg[64];
				str_format(msg, sizeof(msg), "+1 %s!", UpgradeTypes[i]);
				pGameServer->SendChatTarget(ClientID, msg);
				return;
			}

		pGameServer->SendChatTarget(ClientID, "Please stick to the given structure:");
		pGameServer->SendChatTarget(ClientID, "/upgr <upgr-type>");
		pGameServer->SendChatTarget(ClientID, "/upgr  -for available upgrades.");

		return;
	}

	//fetch
	else if (!strcmp(pMsg, "/fetch"))
	{
		if (!IsAvailable(pPlayer, pGameServer)) return;

		char msg[64];
		str_format(msg, sizeof(msg), "Username: %s", pPlayer->m_pAccount->m_Username);
		pGameServer->SendChatTarget(ClientID, msg);
		str_format(msg, sizeof(msg), "Password: %s", pPlayer->m_pAccount->m_Password);
		pGameServer->SendChatTarget(ClientID, msg);
		str_format(msg, sizeof(msg), "Lvl: %d", pPlayer->m_pAccount->m_Lvl);
		pGameServer->SendChatTarget(ClientID, msg);
		str_format(msg, sizeof(msg), "SP: %d", pPlayer->m_pAccount->m_SP);
		pGameServer->SendChatTarget(ClientID, msg);
		str_format(msg, sizeof(msg), "Exp: [%d/%d]", pPlayer->m_pAccount->m_Exp, pPlayer->m_pAccount->m_ExpThreshold);
		pGameServer->SendChatTarget(ClientID, msg);

		pGameServer->SendChatTarget(ClientID, "Stats:");
		for (size_t i = 0; i < UPGRADES; i++)
		{
			str_format(msg, sizeof(msg), "%s: %d", UpgradeTypes[i], pPlayer->m_pAccount->m_Stats[i]);
			pGameServer->SendChatTarget(ClientID, msg);
		}

		pGameServer->SendChatTarget(ClientID, "Fetch complete.");
		return;
	}

	//invalid (command)
	else
	{
		pGameServer->SendChatTarget(ClientID, "--->");
		pGameServer->SendChatTarget(ClientID, "invalid command.");
		pGameServer->SendChatTarget(ClientID, "/cmdlist   -for list of available commands.");
		return;
	}


}
