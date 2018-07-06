#ifndef ACCOUNT_SYSTEM_H
#define ACCOUNT_SYSTEM_H

#include <game/server/gamecontext.h>
#include <game/server/player.h>

#include <stdlib.h>
#include <string>

#define USERNAME_MAX_LEN 24
#define PASSWORD_MAX_LEN 24
#define ACC_FILE_FORMAT "3FFAcc"

class CGameContext;
class CPlayer;

enum ECHAR_UPGRADES {
	LIFE,
	ARMOR,
	HANDLE,
	HAMMER, 
	GUN, 
	SHOTGUN, 
	GRENADE, 
	RIFLE,

	UPGRADES
};

char static *UpgradeTypes[] = { "life", "armor", "handle", "hammer", "gun", "shotgun", "grenade", "rifle" };

struct CAccount
{
	CAccount() 
	{
		m_Username = (char *)malloc(24 * sizeof(char));
		m_Password = (char *)malloc(24 * sizeof(char));
	}

	~CAccount()
	{
		free(m_Username);
		free(m_Password);
	}

	char *m_Username;
	char *m_Password;

	int m_Lvl;
	int m_SP;

	int m_Exp;
	int m_ExpThreshold;

	int m_Stats[UPGRADES];
};

#define ACC_VERSION "1.0.0"
#define ACC_LEN strlen(ACC_VERSION)

class CAccsys
{
public:
	static void SetGameContext(CGameContext *GameContext) { m_pGameServer = GameContext; }
	
	static void New(const char *username, const char *pw, const CPlayer *pl);
	static int GetNextAccInfoInt(FILE *pFile);
	static CAccount *Load(const char *name, const char *pw, const CPlayer *pl);
	static void Save(const CAccount *acc);
	//static void NewPassword(CAccount *acc, char *pw, CPlayer *pl);

	static bool IsLoggedIn(const char *name);
	static bool DoesExist(const char *filePath);

private:
	static bool validInfo(const short len_name, const short len_pw, const int ClientID);

protected:
	static ECHAR_UPGRADES GetUPGFromName(const char *name);
	static CGameContext *m_pGameServer;

};

#endif
