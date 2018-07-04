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


	if (mkdir("accs"))
		dbg_msg("accsys", "account folder created");

	char File[64];
	str_format(File, sizeof(File), "accounts/%s.3FFAcc", name);

	if (DoesExist(File));
	{
		m_pGameServer->SendChatTarget(ClientID, "The Account already exists!");
		return;
	}
	
	FILE *Acc = fopen(File, "w");

	fprintf(Acc, "Version: %s\n", ACC_VERSION);
	fprintf(Acc, "%s\n", name);
	fprintf(Acc, "%s\n", pw);

	fprintf(Acc, "Lvl = %d\n", 1);	// Level
	fprintf(Acc, "Exp = %d\n", 0);	// Experience
	fprintf(Acc, "SP = %d\n", 1);	// Stat points
	fprintf(Acc, "ET = %d\n", 1);	// Experience Threshold

	for (size_t i = 0; i < UPGRADES; i++)
		fprintf(Acc, "%s = 0\n", UpgradeTypes[i]);

	fclose(Acc);

	m_pGameServer->SendChatTarget(ClientID, "Your account has been created!");
	m_pGameServer->SendChatTarget(ClientID, "You may now login, syntax:");
	m_pGameServer->SendChatTarget(ClientID, "/login <username> <password>");
}

CAccount *CAccsys::Load(const char *name, const char *pw, const CPlayer *pl)
{
	short len_name = strlen(name);
	short len_pw = strlen(pw);

	int ClientID = pl->GetCID();

	if (!validInfo(len_name, len_pw, ClientID))	// validate name & password.
		return 0;
	
	char File[64];
	str_format(File, sizeof(File), "accounts/%s.3FFAcc", name);

	if (!DoesExist(File))
	{
		m_pGameServer->SendChatTarget(ClientID, "Account does not exist.");
		return 0;
	}

	dbg_msg("accsys", "account is not in use");

	FILE *Acc = fopen(File, "r");

	char Version[16];
	fscanf(Acc, "Version: %s\n", Version);
	if (strncmp(Version, ACC_VERSION, ACC_LEN))
	{
		dbg_msg("accsys", "account \"%s\" is outdated!", name);
		m_pGameServer->SendChatTarget(ClientID, "Your account is outdated, error: E0V3R510N");
		m_pGameServer->SendChatTarget(ClientID, "Please contact an Admin!");
		m_pGameServer->SendChatTarget(ClientID, "Admin: Åmol");
		fclose(Acc);
		return 0;
	}

	char len_uLogin[USERNAME_MAX_LEN];
	fgets(len_uLogin, USERNAME_MAX_LEN, Acc);
	if (strncmp(len_uLogin, name, len_name))
	{
		dbg_msg("accsys", "account \"%s\" has been corrupted!", name);
		m_pGameServer->SendChatTarget(ClientID, "Oops! Your account has been corrupted!, error: E0C088U97");
		m_pGameServer->SendChatTarget(ClientID, "Please contact an Admin!");
		m_pGameServer->SendChatTarget(ClientID, "Admin: Åmol");
		fclose(Acc);
		return 0;
	}

	char len_pLogin[PASSWORD_MAX_LEN];
	fgets(len_pLogin, USERNAME_MAX_LEN, Acc);
	if (strncmp(len_uLogin, name, len_pw))
	{
		m_pGameServer->SendChatTarget(ClientID, "You entered the wrong password!");
		fclose(Acc);
		return 0;
	}

	CAccount *Account = new CAccount();

	strncpy(Account->m_Username, name, USERNAME_MAX_LEN - 1);
	Account->m_Username[USERNAME_MAX_LEN - 1] = '\0';

	strncpy(Account->m_Password, pw, PASSWORD_MAX_LEN - 1);
	Account->m_Password[PASSWORD_MAX_LEN - 1] = '\0';

	int stats[UPGRADES];
	int stat_val;

	for (size_t i = 0; fscanf(Acc, "%s = %d\n", UpgradeTypes[i], &stat_val) == 2; i++) {
		Account->m_Stats[i] = stat_val;
	}

	fclose(Acc);

	m_pGameServer->SendChatTarget(ClientID, "You've logged in successfully!");
	return Account;
}

void CAccsys::Save(const CAccount *account)
{
	char File[64];
	str_format(File, sizeof(File), "accounts/%s.3FFAcc", account->m_Username);

	FILE *Acc = fopen(File, "w");
	if (!Acc)
		return;

	fprintf(Acc, "Version: %s\n", ACC_VERSION);
	fprintf(Acc, "%s\n", account->m_Username);
	fprintf(Acc, "%s\n", account->m_Password);

	for (size_t i = 0; i < UPGRADES; i++)
		fprintf(Acc, "%s = %d\n", UpgradeTypes[i], account->m_Stats[i]);

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

bool CAccsys::DoesExist(const char *name)
{
	FILE *Acc = fopen(name, "r");
	if (!Acc)
		return false;

	fclose(Acc);
	return true;
}