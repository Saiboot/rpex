#ifndef CHAT_COMMANDS_H
#define CHAT_COMMANDS_H

#include <stdlib.h>
#include <ctype.h>
#include <string>

#include <engine/server.h>
#include <game/version.h>
#include "gamecontext.h"
#include "player.h"

class CPlayer;
class CGameContext;

void toLowc(char *str);

namespace cmdlist {

	bool IsAvailable(CPlayer *pPlayer, CGameContext *pGameServer);
	void Issue_command(CNetMsg_Cl_Say *pMsg, CPlayer *pPlayer, CGameContext *pGameServer);

}

#endif

