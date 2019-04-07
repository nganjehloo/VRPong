/************************************************************************************

Authors     :   Bradley Austin Davis <bdavis@saintandreas.org>
Copyright   :   Copyright Brad Davis. All Rights reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

************************************************************************************/

#include <iostream>
#include <memory>
#include <exception>
#include <algorithm>
#include <cstdio>

#include <Windows.h>

#include "OVRUTIL.h"

#define __STDC_FORMAT_MACROS 1

#define FAIL(X) throw std::runtime_error(X)

///////////////////////////////////////////////////////////////////////////////
//
// GLEW gives cross platform access to OpenGL 3.x+ functionality.  
//

#include <GL/glew.h>

bool checkFramebufferStatus(GLenum target = GL_FRAMEBUFFER) {
	GLuint status = glCheckFramebufferStatus(target);
	switch (status) {
	case GL_FRAMEBUFFER_COMPLETE:
		return true;
		break;

	case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
		std::cerr << "framebuffer incomplete attachment" << std::endl;
		break;

	case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
		std::cerr << "framebuffer missing attachment" << std::endl;
		break;

	case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
		std::cerr << "framebuffer incomplete draw buffer" << std::endl;
		break;

	case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
		std::cerr << "framebuffer incomplete read buffer" << std::endl;
		break;

	case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
		std::cerr << "framebuffer incomplete multisample" << std::endl;
		break;

	case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
		std::cerr << "framebuffer incomplete layer targets" << std::endl;
		break;

	case GL_FRAMEBUFFER_UNSUPPORTED:
		std::cerr << "framebuffer unsupported internal format or image" << std::endl;
		break;

	default:
		std::cerr << "other framebuffer error" << std::endl;
		break;
	}

	return false;
}

bool checkGlError() {
	GLenum error = glGetError();
	if (!error) {
		return false;
	}
	else {
		switch (error) {
		case GL_INVALID_ENUM:
			std::cerr << ": An unacceptable value is specified for an enumerated argument.The offending command is ignored and has no other side effect than to set the error flag.";
			break;
		case GL_INVALID_VALUE:
			std::cerr << ": A numeric argument is out of range.The offending command is ignored and has no other side effect than to set the error flag";
			break;
		case GL_INVALID_OPERATION:
			std::cerr << ": The specified operation is not allowed in the current state.The offending command is ignored and has no other side effect than to set the error flag..";
			break;
		case GL_INVALID_FRAMEBUFFER_OPERATION:
			std::cerr << ": The framebuffer object is not complete.The offending command is ignored and has no other side effect than to set the error flag.";
			break;
		case GL_OUT_OF_MEMORY:
			std::cerr << ": There is not enough memory left to execute the command.The state of the GL is undefined, except for the state of the error flags, after this error is recorded.";
			break;
		case GL_STACK_UNDERFLOW:
			std::cerr << ": An attempt has been made to perform an operation that would cause an internal stack to underflow.";
			break;
		case GL_STACK_OVERFLOW:
			std::cerr << ": An attempt has been made to perform an operation that would cause an internal stack to overflow.";
			break;
		}
		return true;
	}
}

void glDebugCallbackHandler(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *msg, GLvoid* data) {
	OutputDebugStringA(msg);
	std::cout << "debug call: " << msg << std::endl;
}

//////////////////////////////////////////////////////////////////////
//
// GLFW provides cross platform window creation
//

#include <GLFW/glfw3.h>

namespace glfw 
{
	inline GLFWwindow * createWindow(const uvec2 & size, const ivec2 & position = ivec2(INT_MIN)) {
		GLFWwindow * window = glfwCreateWindow(size.x, size.y, "glfw", nullptr, nullptr);
		if (!window) {
			FAIL("Unable to create rendering window");
		}
		if ((position.x > INT_MIN) && (position.y > INT_MIN)) {
			glfwSetWindowPos(window, position.x, position.y);
		}
		return window;
	}
}

// A class to encapsulate using GLFW to handle input and render a scene
class GlfwApp 
{
protected:
	uvec2 windowSize;
	ivec2 windowPosition;
	GLFWwindow * window{ nullptr };
	unsigned int frame{ 0 };

public:
	GlfwApp() {
		// Initialize the GLFW system for creating and positioning windows
		if (!glfwInit()) {
			FAIL("Failed to initialize GLFW");
		}
		glfwSetErrorCallback(ErrorCallback);
	}

	virtual ~GlfwApp() {
		if (nullptr != window) {
			glfwDestroyWindow(window);
		}
		glfwTerminate();
	}

	virtual int run() {
		preCreate();

		window = createRenderingTarget(windowSize, windowPosition);

		if (!window) {
			std::cout << "Unable to create OpenGL window" << std::endl;
			return -1;
		}

		postCreate();

		initGl();

		while (!glfwWindowShouldClose(window)) {
			++frame;
			glfwPollEvents();
			update();
			draw();
			finishFrame();
		}

		shutdownGl();

		return 0;
	}


protected:
	virtual GLFWwindow * createRenderingTarget(uvec2 & size, ivec2 & pos) = 0;

	virtual void draw() = 0;

	void preCreate() {
		glfwWindowHint(GLFW_DEPTH_BITS, 16);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
	}


	void postCreate() {
		glfwSetWindowUserPointer(window, this);
		glfwSetKeyCallback(window, KeyCallback);
		glfwSetMouseButtonCallback(window, MouseButtonCallback);
		glfwMakeContextCurrent(window);

		// Initialize the OpenGL bindings
		// For some reason we have to set this experminetal flag to properly
		// init GLEW if we use a core context.
		glewExperimental = GL_TRUE;
		if (0 != glewInit()) {
			FAIL("Failed to initialize GLEW");
		}
		glGetError();

		if (GLEW_KHR_debug) {
			GLint v;
			glGetIntegerv(GL_CONTEXT_FLAGS, &v);
			if (v & GL_CONTEXT_FLAG_DEBUG_BIT) {
				//glDebugMessageCallback(glDebugCallbackHandler, this);
			}
		}
	}

	virtual void initGl() {
	}

	virtual void shutdownGl() {
	}

	virtual void finishFrame() {
		glfwSwapBuffers(window);
	}

	virtual void destroyWindow() {
		glfwSetKeyCallback(window, nullptr);
		glfwSetMouseButtonCallback(window, nullptr);
		glfwDestroyWindow(window);
	}

	virtual void onKey(int key, int scancode, int action, int mods) {
		if (GLFW_PRESS != action) {
			return;
		}

		switch (key) {
		case GLFW_KEY_ESCAPE:
			glfwSetWindowShouldClose(window, 1);
			return;
		}
	}

	virtual void update() {}

	virtual void onMouseButton(int button, int action, int mods) {}

protected:
	virtual void viewport(const ivec2 & pos, const uvec2 & size) {
		glViewport(pos.x, pos.y, size.x, size.y);
	}

private:

	static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
		GlfwApp * instance = (GlfwApp *)glfwGetWindowUserPointer(window);
		instance->onKey(key, scancode, action, mods);
	}

	static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
		GlfwApp * instance = (GlfwApp *)glfwGetWindowUserPointer(window);
		instance->onMouseButton(button, action, mods);
	}

	static void ErrorCallback(int error, const char* description) {
		FAIL(description);
	}
};



class RiftManagerApp 
{
protected:
	ovrSession _session;
	ovrHmdDesc _hmdDesc;
	ovrGraphicsLuid _luid;

public:
	RiftManagerApp() {
		if (!OVR_SUCCESS(ovr_Create(&_session, &_luid))) {
			FAIL("Unable to create HMD session");
		}

		_hmdDesc = ovr_GetHmdDesc(_session);
	}

	~RiftManagerApp() {
		ovr_Destroy(_session);
		_session = nullptr;
	}
};

class RiftApp : public GlfwApp, public RiftManagerApp 
{
public:
	ovrPosef headPose;
private:
	GLuint _fbo{ 0 };
	GLuint _depthBuffer{ 0 };
	ovrTextureSwapChain _eyeTexture;

	GLuint _mirrorFbo{ 0 };
	ovrMirrorTexture _mirrorTexture;

	ovrEyeRenderDesc _eyeRenderDescs[2];

	mat4 _eyeProjections[2];

	ovrLayerEyeFov _sceneLayer;
	ovrViewScaleDesc _viewScaleDesc;

	uvec2 _renderTargetSize;
	uvec2 _mirrorSize;

public:

	RiftApp() {
		using namespace ovr;
		_viewScaleDesc.HmdSpaceToWorldScaleInMeters = 1.0f;

		memset(&_sceneLayer, 0, sizeof(ovrLayerEyeFov));
		_sceneLayer.Header.Type = ovrLayerType_EyeFov;
		_sceneLayer.Header.Flags = ovrLayerFlag_TextureOriginAtBottomLeft;

		ovr::for_each_eye([&](ovrEyeType eye) {
			ovrEyeRenderDesc& erd = _eyeRenderDescs[eye] = ovr_GetRenderDesc(_session, eye, _hmdDesc.DefaultEyeFov[eye]);
			ovrMatrix4f ovrPerspectiveProjection =
				ovrMatrix4f_Projection(erd.Fov, 0.01f, 1000.0f, ovrProjection_ClipRangeOpenGL);
			_eyeProjections[eye] = ovr::toGlm(ovrPerspectiveProjection);
			_viewScaleDesc.HmdToEyeOffset[eye] = erd.HmdToEyeOffset;

			ovrFovPort & fov = _sceneLayer.Fov[eye] = _eyeRenderDescs[eye].Fov;
			auto eyeSize = ovr_GetFovTextureSize(_session, eye, fov, 1.0f);
			_sceneLayer.Viewport[eye].Size = eyeSize;
			_sceneLayer.Viewport[eye].Pos = { (int)_renderTargetSize.x, 0 };

			_renderTargetSize.y = std::max(_renderTargetSize.y, (uint32_t)eyeSize.h);
			_renderTargetSize.x += eyeSize.w;
		});
		// Make the on screen window 1/4 the resolution of the render target
		_mirrorSize = _renderTargetSize;
		_mirrorSize /= 4;
	}

protected:
	GLFWwindow * createRenderingTarget(uvec2 & outSize, ivec2 & outPosition) override {
		return glfw::createWindow(_mirrorSize);
	}

	void initGl() override {
		GlfwApp::initGl();

		// Disable the v-sync for buffer swap
		glfwSwapInterval(0);

		ovrTextureSwapChainDesc desc = {};
		desc.Type = ovrTexture_2D;
		desc.ArraySize = 1;
		desc.Width = _renderTargetSize.x;
		desc.Height = _renderTargetSize.y;
		desc.MipLevels = 1;
		desc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;
		desc.SampleCount = 1;
		desc.StaticImage = ovrFalse;
		ovrResult result = ovr_CreateTextureSwapChainGL(_session, &desc, &_eyeTexture);
		_sceneLayer.ColorTexture[0] = _eyeTexture;
		if (!OVR_SUCCESS(result)) {
			FAIL("Failed to create swap textures");
		}

		int length = 0;
		result = ovr_GetTextureSwapChainLength(_session, _eyeTexture, &length);
		if (!OVR_SUCCESS(result) || !length) {
			FAIL("Unable to count swap chain textures");
		}
		for (int i = 0; i < length; ++i) {
			GLuint chainTexId;
			ovr_GetTextureSwapChainBufferGL(_session, _eyeTexture, i, &chainTexId);
			glBindTexture(GL_TEXTURE_2D, chainTexId);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		}
		glBindTexture(GL_TEXTURE_2D, 0);

		// Set up the framebuffer object
		glGenFramebuffers(1, &_fbo);
		glGenRenderbuffers(1, &_depthBuffer);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fbo);
		glBindRenderbuffer(GL_RENDERBUFFER, _depthBuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, _renderTargetSize.x, _renderTargetSize.y);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
		glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _depthBuffer);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

		ovrMirrorTextureDesc mirrorDesc;
		memset(&mirrorDesc, 0, sizeof(mirrorDesc));
		mirrorDesc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;
		mirrorDesc.Width = _mirrorSize.x;
		mirrorDesc.Height = _mirrorSize.y;
		if (!OVR_SUCCESS(ovr_CreateMirrorTextureGL(_session, &mirrorDesc, &_mirrorTexture))) {
			FAIL("Could not create mirror texture");
		}
		glGenFramebuffers(1, &_mirrorFbo);
	}

	void onKey(int key, int scancode, int action, int mods) override {
		if (GLFW_PRESS == action) switch (key) {
		case GLFW_KEY_R:
			ovr_RecenterTrackingOrigin(_session);
			return;
		}

		GlfwApp::onKey(key, scancode, action, mods);
	}

	void draw() final override {
		ovrPosef eyePoses[2];
		ovr_GetEyePoses(_session, frame, true, _viewScaleDesc.HmdToEyeOffset, eyePoses, &_sceneLayer.SensorSampleTime);

		int curIndex;
		ovr_GetTextureSwapChainCurrentIndex(_session, _eyeTexture, &curIndex);
		GLuint curTexId;
		ovr_GetTextureSwapChainBufferGL(_session, _eyeTexture, curIndex, &curTexId);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fbo);
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, curTexId, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		ovr::for_each_eye([&](ovrEyeType eye) {
			eyePoses[eye].Position.z += 2.5f;
			const auto& vp = _sceneLayer.Viewport[eye];
			glViewport(vp.Pos.x, vp.Pos.y, vp.Size.w, vp.Size.h);
			_sceneLayer.RenderPose[eye] = eyePoses[eye];
			headPose = eyePoses[0];
			renderScene(_eyeProjections[eye], ovr::toGlm(eyePoses[eye]), eyePoses[eye]);
		});
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		ovr_CommitTextureSwapChain(_session, _eyeTexture);
		ovrLayerHeader* headerList = &_sceneLayer.Header;
		ovr_SubmitFrame(_session, frame, &_viewScaleDesc, &headerList, 1);

		GLuint mirrorTextureId;
		ovr_GetMirrorTextureBufferGL(_session, _mirrorTexture, &mirrorTextureId);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, _mirrorFbo);
		glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mirrorTextureId, 0);
		glBlitFramebuffer(0, 0, _mirrorSize.x, _mirrorSize.y, 0, _mirrorSize.y, _mirrorSize.x, 0, GL_COLOR_BUFFER_BIT, GL_NEAREST);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	}

	virtual void renderScene(const glm::mat4 & projection, const glm::mat4 & headPose, ovrPosef & eyePose) = 0;
};

#include "model.h"
#include "shader.h"
#include "Hand.h"
#include "Level.h"
#include "Ball.h"
#include "Player.h"
#include <stdio.h>
#include <conio.h>

#include <irrKlang\irrKlang.h>
using namespace irrklang;

// server includes
#include "rpc/client.h"
#include "rpc/rpc_error.h"
#include "SerializablePose.h"

#define VERTEX_SHADER_PATH "shader.vert"
#define FRAGMENT_SHADER_PATH "shader.frag"

#define SYNC_INTERVAL 1

glm::vec3 lightPos(0.0f, 0.2f, 0.0f);
glm::vec3 lightAmbient(0.5f, 0.5f, 0.5f);
glm::vec3 lightDiffuse(0.700000f, 0.500000f, 1.00000f);
glm::vec3 lightSpecular(1.0f, 1.0f, 1.0f);

// Defines the main application to run
class ExampleApp : public RiftApp 
{
	vector<Player> players;
	Level * level;
	Ball * ball;
	Shader * shader = NULL;
	ISound* music;
	ISound* sheild;
	ISoundEngine *SoundEngine;
	GLint shaderProgram;
	rpc::client* client;
	ovrPosef remoteHeadPose;
	ovrPosef remoteHandPose;
	bool initialized = false;
	bool ready = false;
	float deltaTime = 0.0f;
	float lastFrame = 0.0f;
	float currentFrame = 0.0f;

public:
	ExampleApp() { }

protected:
	
	void initSound() {
		irrklang::ISoundDeviceList* deviceList = createSoundDeviceList();

		printf("Devices available:\n\n");

		for (int i = 0; i<deviceList->getDeviceCount(); ++i)
			printf("%d: %s\n", i, deviceList->getDeviceDescription(i));

		printf("\nselect a device using the number (or press any key to use default):\n\n");
		int deviceNumber = _getch() - '0';

		// create device with the selected driver
		const char* deviceID = deviceList->getDeviceID(deviceNumber);
		SoundEngine = createIrrKlangDevice(irrklang::ESOD_AUTO_DETECT,
			irrklang::ESEO_DEFAULT_OPTIONS,
			deviceID);

		deviceList->drop(); // delete device list

		music = SoundEngine->play2D("Assets/sound/level.mp3", true, false, true);
		music->setVolume(0.5);
		SoundEngine->setListenerPosition(vec3df(0, 0, 3.0),
			vec3df(0, 0, 1));
	}
	
	void initGl() override {
		RiftApp::initGl();
		glClearColor(0.0f, 0.0f, 0.5f, 1.0f);
		glEnable(GL_DEPTH_TEST);
		ovr_RecenterTrackingOrigin(_session);
		shader = new Shader(VERTEX_SHADER_PATH, FRAGMENT_SHADER_PATH);
		level = new Level();
		ball = new Ball();
		players.push_back(Player(players.size() + 1, new Hand(_session, frame, false)));
		players.push_back(Player(players.size() + 1, new Hand(true)));
		initSound();

		// start the client and initialize the poses for this player on the server
		try
		{
			client = new rpc::client(SERVER_IP, SERVER_PORT);
			client->call("setPose", OCULUS, HEAD, serializePose(players[0].head->HeadPose));
			client->call("setPose", OCULUS, HAND, serializePose(players[0].hand->HandPose));
		}
		catch (rpc::rpc_error& e)
		{
			cerr << "Unable to set initial poses from server!" << endl;
			cerr << "Reason: " << e.what() << endl;
		}
	}

	void shutdownGl() override {
		//cubeScene.reset();
		delete client;
		exit(1);
	}

	bool intersect(int playernum) {
		vec3 center = ball->calcCenterPoint();
		vec3 min = players[playernum].hand->min;
		vec3 max = players[playernum].hand->max;
		//cout << "center: " << center.x << center.y << center.z << endl;
		return (center.x >= min.x && center.x <= max.x) &&
			(center.y >= min.y && center.y <= max.y) &&
			(center.z >= min.z && center.z <= max.z);
	}

	void update() 
	{
		currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		try
		{
			// let the server know oculus is ready
			if (!initialized)
			{
				cout << "Initializing oculus..." << endl;
				client->call("oculusReady");
				initialized = true;
			}

			// Check if all players are connected
			if (!ready)
			{
				while (!(ready = client->call("checkConnection").as<bool>()));
				cout << "Starting program!" << endl;
			}
		}
		catch (rpc::rpc_error& e)
		{
			cerr << "Unable to ready oculus!" << endl;
			cerr << "Reason: " << e.what() << endl;
		}

		bool triggerr = false;
		SoundEngine->setListenerPosition(vec3df(headPose.Position.x, headPose.Position.y, headPose.Position.z),
			vec3df(headPose.Orientation.x, headPose.Orientation.y, headPose.Orientation.z));
		if(frame%30 == 0)
			ovr_SetControllerVibration(_session, ovrControllerType_RTouch, 0.0f, 0.0f);

		ball->update(deltaTime);

		// send an updated position for the ball
		if (frame % SYNC_INTERVAL == 0)
		{
			try
			{
				client->async_call("setBallPose", serializeMat(ball->meshes[0].toWorld), 0);
				client->async_call("setBallPose", serializeMat(ball->meshes[1].toWorld), 1);
			}
			catch (rpc::rpc_error& e)
			{
				cerr << "Unable to set ball position!" << endl;
				cerr << "Reason: " << e.what() << endl;
			}
		}

		/*if (ball->outOfBounds)
		{
			try
			{
				client->async_call("setLastPlayer", ball->lastPlayer);
				ball->outOfBounds = false;
			}
			catch (rpc::rpc_error& e)
			{
				cerr << "Unable to set last player!" << endl;
				cerr << "Reason: " << e.what() << endl;
			}
		}*/
		
		// TODO: set the update rates lower and interpolate to new remote positions
		for (int i = 0; i < players.size(); ++i) {
			if (players[i].hand->isLeap) 
			{
				// get an updated position from the server every certain number of frames
				if (frame % SYNC_INTERVAL == 0)
				{
					//cout << "Getting remote pose" << endl;
					try
					{
						remoteHeadPose = deserializePose(client->call("getPose", LEAP, HEAD).as<s_Pose>());
						remoteHandPose = deserializePose(client->call("getPose", LEAP, HAND).as<s_Pose>());
					}
					catch (rpc::rpc_error& e)
					{
						cerr << "Unable to retrieve updated poses from server!" << endl;
						cerr << "Reason: " << e.what() << endl;
					}
				}

				//cout << "leap" << endl;
				//set hand and head pose here fromw/e we got from network
				// TODO: interpolate to new position for smoothness
				players[i].head->HeadPose = remoteHeadPose;
				players[i].hand->HandPose = remoteHandPose;
				players[i].update(NULL, NULL);
			}
			else 
			{
				//cout << "not leap" << endl;
				players[i].head->HeadPose = headPose;
				players[i].hand->pollOculusInput(_session, frame);
				players[i].hand->HandPose.Position.z += 2.5f;
				players[i].update(NULL, NULL);

				// send an updated position to the server every certain number of frames
				if (frame % SYNC_INTERVAL == 0)
				{
					//cout << "Updating remote pose" << endl;
					try
					{
						client->async_call("setPose", OCULUS, HAND, serializePose(players[i].hand->HandPose));
						client->async_call("setPose", OCULUS, HEAD, serializePose(headPose));
					}
					catch (rpc::rpc_error& e)
					{
						cerr << "Unable to send updated poses to server!" << endl;
						cerr << "Reason: " << e.what() << endl;
					}
				}
			}

			// get the last player to hit
			/*try
			{
				ball->lastPlayer = client->call("getLastPlayer").as<int>();
			}
			catch (rpc::rpc_error& e)
			{
				cerr << "Unable to get last player!" << endl;
				cerr << "Reason: " << e.what() << endl;
			}*/

			// check for a collision between a player's hands and the ball
			if (intersect(i) && ball->lastPlayer != players[i].playerNum)
			{	
				cout << "Hit the ball for player " << players[i].playerNum << endl;
				vec3 s = ball->calcCenterPoint();
				sheild = SoundEngine->play3D("Assets/sound/clang.wav",
					vec3df(s.x, s.y, s.z), false, false, true);
				sheild->setMinDistance(1.0f);
				ball->velocity = -ball->velocity;

				// update the last player to touch the ball
				try
				{
					ball->lastPlayer = players[i].playerNum;
					//client->async_call("setLastPlayer", players[i].playerNum);
				}
				catch (rpc::rpc_error& e)
				{
					cerr << "Unable to set last player!" << endl;
					cerr << "Reason: " << e.what() << endl;
				}

				glm::quat direc = ovr::toGlm(players[i].hand->HandPose.Orientation);
				vec3 reflect = glm::mat4_cast(direc)* vec4(0.0, 0.0, 1.0f, 1.0f);
				cout << reflect.x << reflect.y << reflect.z << endl;
				ball->velocity = reflect*-0.15f;
				if(i = 1)
					ovr_SetControllerVibration(_session, ovrControllerType_RTouch, 0.0f, 1.0f);
			}
		}
	}

	void renderScene(const glm::mat4 & projection, const glm::mat4 & headPose, ovrPosef & eyePose) override 
	{
		shader->Use();
		
		glUniform3fv(glGetUniformLocation(shader->Program, "light.position"), 1, &lightPos[0]);
		glUniform3fv(glGetUniformLocation(shader->Program, "light.ambient"), 1, &lightAmbient[0]);
		glUniform3fv(glGetUniformLocation(shader->Program, "light.diffuse"), 1, &lightDiffuse[0]);
		glUniform3fv(glGetUniformLocation(shader->Program, "light.specular"), 1, &lightSpecular[0]);

		glUniformMatrix4fv(glGetUniformLocation(shader->Program, "CameraMatrix"), 1, GL_FALSE, &(glm::inverse(headPose))[0][0]);
		glUniformMatrix4fv(glGetUniformLocation(shader->Program, "ProjectionMatrix"), 1, GL_FALSE, &projection[0][0]);
		
		glUniform3fv(glGetUniformLocation(shader->Program, "viewPos"), 1, &(ovr::toGlm(eyePose.Position)[0]));
		GLfloat dehom = eyePose.Orientation.w;
		glm::vec3 viewDir = vec3(eyePose.Orientation.x/dehom, eyePose.Orientation.y / dehom, eyePose.Orientation.z / dehom);
		glUniform3fv(glGetUniformLocation(shader->Program, "viewDir"), 1, &viewDir[0]);
		
		ball->Draw(*shader);
		level->Draw(*shader);
		for (int i = 0; i < players.size(); ++i) {
			players[i].Draw(*shader, 1);
		}
	}
};

// Execute our example class
int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	int result = -1;

	try {
		AllocConsole();
		freopen("conin$", "r", stdin);
		freopen("conout$", "w", stdout);
		freopen("conout$", "w", stderr);
		if (!OVR_SUCCESS(ovr_Initialize(nullptr))) {
			FAIL("Failed to initialize the Oculus SDK");
		}
		result = ExampleApp().run();
	}
	catch (std::exception & error) {
		OutputDebugStringA(error.what());
		std::cerr << error.what() << std::endl;
	}
	ovr_Shutdown();
	return result;
}