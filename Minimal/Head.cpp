#include "Head.h"
#include "OVRUTIL.h"

Head::Head() : Model(HEAD_PATH)
{
	for (GLuint i = 0; i < this->meshes.size(); i++) {
		meshes[i].toWorld = glm::translate(glm::scale(ovr::toGlm(HeadPose), glm::vec3(0.2f, 0.2f, 0.2f)), glm::vec3(0.0f, -1.0f, -3.0f));
	}
	toWorld = glm::translate(glm::scale(ovr::toGlm(HeadPose), glm::vec3(0.2f, 0.2f, 0.2f)), glm::vec3(0.0f, -1.0f, -3.0f));
}

bool Head::update(bool isLeap) 
{
	//cout << "starting update" << endl;
	//transform hands
	if (!isLeap) {
		//get headPose for oculus
		//cout << "update oc head" << endl;
		for (GLuint i = 0; i < this->meshes.size(); i++) {
			meshes[i].toWorld = glm::scale(glm::rotate(ovr::toGlm(HeadPose), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f)) ,glm::vec3(0.2f, 0.2f, 0.2f));
		}
		toWorld = glm::scale(glm::rotate(ovr::toGlm(HeadPose), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f)), glm::vec3(0.2f, 0.2f, 0.2f));
		return false;
	}
	else {
		//cout << "update lp head" << endl;
		for (GLuint i = 0; i < this->meshes.size(); i++) {
			meshes[i].toWorld = glm::scale(ovr::toGlm(HeadPose), glm::vec3(0.2f, 0.2f, 0.2f));
		}
		toWorld = glm::scale(ovr::toGlm(HeadPose), glm::vec3(0.2f, 0.2f, 0.2f));
		return false;
	}
}

void Head::Draw(Shader shader)
{
	Model::Draw(shader);
}

Head::~Head()
{
}
