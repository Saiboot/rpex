#include "cmds.h"

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

	void cmdlist::Issue_command(CNetMsg_Cl_Say *pMsg, CPlayer *pPlayer, CGameContext *pGameServer)
	{
		char *msg = (char *)malloc(sizeof(char) * strlen(pMsg->m_pMessage));
		strcpy(msg, pMsg->m_pMessage);

		int ClientID = pPlayer->GetCID();

		//register
		if (!strncmp(msg, "/register", 9))
		{
			if (pPlayer->m_pAccount)
			{
				pGameServer->SendChatTarget(ClientID, "You are already registerd and logged in!");
				return;
			}

			char name[512];
			char pw[512];
			if (sscanf(msg, "/register %s %s", name, pw) != 2)
			{
				pGameServer->SendChatTarget(ClientID, "Please stick to the given structure:");
				pGameServer->SendChatTarget(ClientID, "/register <username> <password>");
				return;
			}

			CAccsys::New(name, pw, pPlayer);
			return;
		}

		//login
		else if (!strncmp(msg, "/login", 6))
		{
			if (pPlayer->m_pAccount)
			{
				pGameServer->SendChatTarget(ClientID, "You're already logged in.");
				return;
			}

			char name[512];
			char pw[512];

			if (sscanf(msg, "/login %s %s", name, pw) != 2)
			{
				pGameServer->SendChatTarget(ClientID, "Please stick to the given structure:");
				pGameServer->SendChatTarget(ClientID, "/login <username> <password>");
				return;
			}

			char File[64];
			str_format(File, sizeof(File), "accounts/%s.3FFAcc", name);

			if (!CAccsys::DoesExist(File))
			{
				pGameServer->SendChatTarget(ClientID, "This account does not exist!");
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
		else if (!strcmp(msg, "/logout"))
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
		else if (!strcmp(msg, "/update"))
		{
			if (!IsAvailable(pPlayer, pGameServer)) return;

			// update database
			CAccsys::Save(pPlayer->m_pAccount);
			pGameServer->SendChatTarget(ClientID, "Account updated succesfully.");
			return;
		}

		//info
		else if (!strcmp(msg, "/info"))
		{
			pGameServer->SendChatTarget(ClientID, "   =Info=");
			pGameServer->SendChatTarget(ClientID, "mod: ...");
			pGameServer->SendChatTarget(ClientID, "by: Amol");
			pGameServer->SendChatTarget(ClientID, "   =----=");
			return;
		}

		//cmdlist
		else if (!strcmp(msg, "/cmdlist"))
		{
			pGameServer->SendChatTarget(ClientID, "issue: /login  -to gain access to play.");
			pGameServer->SendChatTarget(ClientID, "/info  -for further information");
			return;
		}



		//invalid (command)
		else
		{
			pGameServer->SendChatTarget(ClientID, "--->");
			pGameServer->SendChatTarget(ClientID, "invalid command.");
			pGameServer->SendChatTarget(ClientID, "Say /cmdlist for list of available commands.");
			return;
		}

		/*

		if (!strncmp(thcmd, "/upgr", 5))	// upgrade menu
		{
			char *weap;
			int amount;

			sscanf(thcmd, "%s %s %d", nullptr, weap, &amount);

			pGameServer->SendChatTarget(ClientID, weap);

			if (!strcmp(weap, "hp"))
			{
				pGameServer->SendChatTarget(ClientID, "Upgraded Health!");
			}
			if (!strcmp(weap, "def"))
			{
				pGameServer->SendChatTarget(ClientID, "Upgraded Defense");
			}
			if (!strcmp(weap, "handle"))
			{

			}
			if (!strcmp(weap, "hammer"))
			{

			}
			if (!strcmp(weap, "gun"))
			{

			}
			if (!strcmp(weap, "shotgun"))
			{

			}
			if (!strcmp(weap, "grenade"))
			{

			}
			if (!strcmp(weap, "rifle"))
			{

			}
		}
		else { pGameServer->SendChatTarget(ClientID, "no such command :("); }
		*/
	}
