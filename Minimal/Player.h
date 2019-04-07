#ifndef PLAYER_H
#define PLAYER_H

#include "Head.h"
#include "Hand.h"
class Hand;
class Player
{
public:
	Player();
	Player(int playernum, Hand * phand);
	void Draw(Shader shader, int playernum);
	void update(ovrSession, long long);
	~Player();
	int playerNum;
	Head * head;
	Hand * hand;
};
#endif

