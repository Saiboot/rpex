#include <stdio.h>
#include <string.h>
#include <direct.h>

#include "acc.h"
#include "../gamecontext.h"

CGameContext *CAccsys::m_pGameServer = 0;

bool CAccsys::validInfo(const short len_name, const short len_pw, const int ClientID)
{
	if (len_name <= 4)
	{
		m_pGameServer->SendChatTarget(ClientID, "The username is too short.");
		return false;
	}
	if (len_name >= USERNAME_MAX_LEN)
	{
		m_pGameServer->SendChatTarget(ClientID, "The username is too long.");
		return false;
	}

	if (len_pw <= 4)
	{
		m_pGameServer->SendChatTarget(ClientID, "The password is too short.");
		return false;
	}
	if (len_pw >= PASSWORD_MAX_LEN)
	{
		m_pGameServer->SendChatTarget(ClientID, "The password is too long.");
		return false;
	}

	return true;
}

void CAccsys::New(const char *name, const char *pw, const CPlayer *pl)
{
	int ClientID = pl->GetCID();

	if (!validInfo(strlen(name), strlen(pw), ClientID))	// validate name & password.
		return;

	// TODO: Move into validInfo function
	char characters[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789.-";
	if (strspn(name, characters) != strlen(name))
	{
		m_pGameServer->SendChatTarget(ClientID, "You entered an invalid character!");
		return;
	}


	if (mkdir("Accounts"))
		dbg_msg("accsys", "account folder created");

	char File[64];
	str_format(File, sizeof(File), "Accounts/%s.%s", name, ACC_FILE_FORMAT);

	if (DoesExist(File))
	{
		m_pGameServer->SendChatTarget(ClientID, "The Account already exists!");
		return;
	}
	
	FILE *Acc = fopen(File, "w");

	fprintf(Acc, "Version: %s\n", ACC_VERSION);
	fprintf(Acc, "%s\n", name);
	fprintf(Acc, "%s\n", pw);

	fprintf(Acc, "%d\n", 1);	// Level
	fprintf(Acc, "%d\n", 0);	// Experience
	fprintf(Acc, "%d\n", 1);	// Stat points
	fprintf(Acc, "%d\n", 1);	// Experience Threshold

	for (size_t i = 0; i < UPGRADES; i++)
		fprintf(Acc, "%d\n", 0);

	fclose(Acc);

	m_pGameServer->SendChatTarget(ClientID, "Your account has been created!");
	m_pGameServer->SendChatTarget(ClientID, "You may now login, syntax:");
	m_pGameServer->SendChatTarget(ClientID, "/login <username> <password>");
}

int CAccsys::GetNextAccInfoInt(FILE *pFile)
{
	int INT;
	fscanf(pFile, "%d", &INT);
	return INT;
}

CAccount *CAccsys::Load(const char *name, const char *pw, const CPlayer *pl)
{
	short len_name = strlen(name);
	short len_pw = strlen(pw);

	int ClientID = pl->GetCID();

	if (!validInfo(len_name, len_pw, ClientID))	// validate name & password.
		return 0;
	
	char File[64];
	str_format(File, sizeof(File), "Accounts/%s.%s", name, ACC_FILE_FORMAT);

	if (!DoesExist(File))
	{
		m_pGameServer->SendChatTarget(ClientID, "Account does not exist.");
		return 0;
	}

	dbg_msg("accsys", File);

	FILE *Acc = fopen(File, "r");

	char Version[16];
	fscanf(Acc, "Version: %s\n", Version);
	if (strncmp(Version, ACC_VERSION, ACC_LEN))
	{
		dbg_msg("accsys", "account \"%s\" is outdated!", name);
		m_pGameServer->SendChatTarget(ClientID, "Your account is outdated, error: E0V3R510N");
		m_pGameServer->SendChatTarget(ClientID, "Please contact an Admin!");
		m_pGameServer->SendChatTarget(ClientID, "Admin: Amol");
		fclose(Acc);
		return 0;
	}

	char usLogin[USERNAME_MAX_LEN];
	fgets(usLogin, USERNAME_MAX_LEN, Acc);
	if (strncmp(usLogin, name, len_name))
	{
		dbg_msg("accsys", "account \"%s\" has been corrupted!", name);
		m_pGameServer->SendChatTarget(ClientID, "Oops! Your account has been corrupted!, error: E0C088U97");
		m_pGameServer->SendChatTarget(ClientID, "Please contact an Admin!");
		m_pGameServer->SendChatTarget(ClientID, "Admin: Amol");
		fclose(Acc);
		return 0;
	}

	char pwLogin[PASSWORD_MAX_LEN];
	fgets(pwLogin, PASSWORD_MAX_LEN, Acc);
	if (strncmp(pwLogin, pw, len_pw))
	{
		m_pGameServer->SendChatTarget(ClientID, "You entered the wrong password!");
		fclose(Acc);
		return 0;
	}

	CAccount *pAccount = new CAccount();

	strncpy(pAccount->m_Username, name, USERNAME_MAX_LEN - 1);
	pAccount->m_Username[USERNAME_MAX_LEN - 1] = '\0';

	strncpy(pAccount->m_Password, pw, PASSWORD_MAX_LEN - 1);
	pAccount->m_Password[PASSWORD_MAX_LEN - 1] = '\0';

	pAccount->m_Lvl = GetNextAccInfoInt(Acc);
	pAccount->m_Exp = GetNextAccInfoInt(Acc);
	pAccount->m_SP = GetNextAccInfoInt(Acc);
	pAccount->m_ExpThreshold = GetNextAccInfoInt(Acc);

	for (size_t i = 0; i < UPGRADES; i++)
		pAccount->m_Stats[i] = GetNextAccInfoInt(Acc);

	fclose(Acc);

	m_pGameServer->SendChatTarget(ClientID, "You've logged in successfully!");
	return pAccount;
}

void CAccsys::Save(const CAccount *account)
{
	char File[64];
	str_format(File, sizeof(File), "Accounts/%s.%s", account->m_Username, ACC_FILE_FORMAT);

	FILE *Acc = fopen(File, "w");
	if (!Acc)
		return;

	fprintf(Acc, "Version: %s\n", ACC_VERSION);
	fprintf(Acc, "%s\n", account->m_Username);
	fprintf(Acc, "%s\n", account->m_Password);

	fprintf(Acc, "%d\n", account->m_Lvl);
	fprintf(Acc, "%d\n", account->m_Exp);
	fprintf(Acc, "%d\n", account->m_ExpThreshold);
	fprintf(Acc, "%d\n", account->m_SP);

	for (size_t i = 0; i < UPGRADES; i++)
		fprintf(Acc, "%d\n", account->m_Stats[i]);

	fclose(Acc);
}

bool CAccsys::IsLoggedIn(const char *name)
{
	short len_name = strlen(name);

	for (size_t i = 0; i < MAX_CLIENTS; i++)
	{
		if(m_pGameServer->m_apPlayers[i] && m_pGameServer->m_apPlayers[i]->m_pAccount)
			if (!strncmp(m_pGameServer->m_apPlayers[i]->m_pAccount->m_Username, name, len_name))
				return true;
	}
	return false;
}

bool CAccsys::DoesExist(const char *filePath)
{
	FILE *pFile;
	if (pFile = fopen(filePath, "r"))
	{
		fclose(pFile);
		return true;
	}
	return false;
}