#include "Level.h"
#include <iostream>


Level::Level() : Model(LEVEL_PATH)
{

}

void Level::Draw(Shader shader)
{

	Model::Draw(shader);

}
Level::~Level()
{
}
