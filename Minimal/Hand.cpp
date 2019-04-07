#include "Hand.h"
Hand::Hand(ovrSession _session, long long frame, bool isleft) : Model(HAND_PATH)
{
	if (isleft) {
		handiness = 0;
	}
	else {
		handiness = 1;
	}

	displayMidpointSeconds = ovr_GetPredictedDisplayTime(_session, frame);
	trackState = ovr_GetTrackingState(_session, displayMidpointSeconds, ovrTrue);
	HandPose = trackState.HandPoses[handiness].ThePose;

	bool HandHigh = false;
	if (HandPose.Position.y > 1.0f) {
		HandHigh = true;
	}
	for (GLuint i = 0; i < this->meshes.size(); i++) {
		meshes[i].toWorld = glm::translate(glm::scale(glm::mat4(1.0f), glm::vec3(0.05f, 0.05f, 0.05f)), glm::vec3(0.0f, -1.0f, 30.0f));
	}
	toWorld = glm::translate(glm::scale(ovr::toGlm(HandPose), glm::vec3(0.05f, 0.05f, 0.05f)), glm::vec3(0.0f, -1.0f, 30.0f));
}

Hand::Hand(bool isleap) : Model(HAND_PATH)
{
	isLeap = isleap;
	//Controller and frame setup
}

Hand::~Hand()
{
}
void Hand::calcAABB() {
	min = vec3(10e10f, 10e10f, 10e10f);
	max = vec3(-10e10f, -10e10f, -10e10f);
	for (GLuint i = 0; i < this->meshes.size(); i++) {
		for (unsigned int j = 0; j < meshes[i].vertices.size(); ++j)
		{
			vec3 v = toWorld * vec4(meshes[i].vertices[j].Position, 1.0f);
			min = glm::min(v, min);
			max = glm::max(v, max);
		}
	}
	//cout << "min: " << min.x << min.y << min.z << endl;
	//cout << "max: " << max.x << max.y << max.z << endl;
}
void Hand::pollOculusInput(ovrSession _session, long long frame) {
	displayMidpointSeconds = ovr_GetPredictedDisplayTime(_session, frame);
	trackState = ovr_GetTrackingState(_session, displayMidpointSeconds, ovrTrue);
	HandPose = trackState.HandPoses[handiness].ThePose;
}

void Hand::pollLeapInput(Leap::Controller & controller, Player & player) {
	if (controller.frame(0).hands().count() > 0) {
		if (controller.frame(0).isValid()) {
			Leap::HandList hands = controller.frame(0).hands();
			int i = 0;
			auto hl = hands.begin();
			if ((*hl).isValid()) {
				//cout << (*hl).palmPosition().toString() << endl;
				//cout << i << endl;
				Leap::Vector pos = (*hl).palmPosition();
				player.hand->HandPose.Position.x = -pos.x / 100.0f;
				player.hand->HandPose.Position.y = pos.y / 100.0f - 1.0f;
				player.hand->HandPose.Position.z = -pos.z / 100.0f - 2.3f;

				player.hand->HandPose.Orientation.x = (*hl).palmNormal().x;
				player.hand->HandPose.Orientation.y = -(*hl).palmNormal().y;
				player.hand->HandPose.Orientation.z = (*hl).palmNormal().z;
				player.hand->HandPose.Orientation.w = 0;
			}
		}
	}
}
bool Hand::update() {
	//cout << "starting update" << endl;
	//transform hands
	if (!isLeap) {
		//cout << "deg" << endl;

		for (GLuint i = 0; i < this->meshes.size(); i++) {
			meshes[i].toWorld = glm::scale(ovr::toGlm(HandPose), glm::vec3(0.05f, 0.05f, 0.05f));
		}
		toWorld = glm::scale(ovr::toGlm(HandPose), glm::vec3(0.05f, 0.05f, 0.05f));
		calcAABB();
		return false;
	}
	else {
		//cout << "update" << endl;
		for (GLuint i = 0; i < this->meshes.size(); i++) {
			meshes[i].toWorld = glm::translate(glm::scale(ovr::toGlm(HandPose), glm::vec3(0.05f, 0.05f, 0.05f)), glm::vec3(0.0f, -1.0f, -3.0f));
		}
		toWorld = glm::translate(glm::scale(ovr::toGlm(HandPose), glm::vec3(0.05f, 0.05f, 0.05f)), glm::vec3(0.0f, -1.0f, -3.0f));
		calcAABB();
		return false;
	}
}
//game logic
void Hand::Draw(Shader shader) {
	Model::Draw(shader);
}