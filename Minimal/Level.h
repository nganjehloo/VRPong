#pragma once
#include "Model.h"
#include "Shader.h"
#define LEVEL_PATH "Assets/clickclock/untitled.obj"
class Level : protected Model
{
public:
	Level();
	void Draw(Shader shader);
	~Level();
};

