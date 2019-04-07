#include "Player.h"

Player::Player(int playernum, Hand * phand)
{
	playerNum = playernum;
	hand = phand;
	head = new Head();
}

void Player::Draw(Shader shader, int playernum) {
	if (playernum != playerNum) {
		head->Draw(shader);
	}
	hand->Draw(shader);
}

void Player::update(ovrSession _session, long long frame) {
	hand->update();
	if (hand->isLeap) {
		head->HeadPose = hand->HandPose;
		head->update(true);
	}
	else {
		head->update(false);
	}
}
Player::~Player()
{
}
