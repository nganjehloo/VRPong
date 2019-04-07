#pragma once
#include "Model.h"
#include "Shader.h"
#define BALL_PATH "Assets/dekunut/Deku_Nut.obj"

class Ball : public Model
{
public:
	Ball();
	void Draw(Shader shader);
	void update(float deltaTime);
	glm::vec3 calcCenterPoint();
	~Ball();
	glm::vec3 velocity;
	glm::mat4 toWorld;
	int lastPlayer;
	bool released;
	bool outOfBounds;
};

