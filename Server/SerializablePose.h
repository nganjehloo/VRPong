#ifndef SERIALIZABLE_POSE_H
#define SERIALIZABLE_POSE_H

#include <LibOVR/OVR_CAPI.h>
#include <rpc/server.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>

// shared defines for RPC parameters
#define OCULUS 0
#define LEAP 1
#define HEAD 0
#define HAND 1
#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8080

// serializable pose object
struct s_Pose
{
	float pos_x, pos_y, pos_z;
	float rot_x, rot_y, rot_z, rot_w;

	MSGPACK_DEFINE_MAP(pos_x, pos_y, pos_z,
		rot_x, rot_y, rot_z, rot_w);
};

struct s_Mat
{
	std::vector<float> values;
	MSGPACK_DEFINE_MAP(values);
};

// serializes an ovr pose for the server
s_Pose serializePose(ovrPosef poseIn)
{
	s_Pose retPose;

	retPose.pos_x = poseIn.Position.x;
	retPose.pos_y = poseIn.Position.y;
	retPose.pos_z = poseIn.Position.z;

	retPose.rot_x = poseIn.Orientation.x;
	retPose.rot_y = poseIn.Orientation.y;
	retPose.rot_z = poseIn.Orientation.z;
	retPose.rot_w = poseIn.Orientation.w;

	return retPose;
}

// deserializes a server pose for ovr
ovrPosef deserializePose(s_Pose poseIn)
{
	ovrVector3f pos;
	pos.x = poseIn.pos_x;
	pos.y = poseIn.pos_y;
	pos.z = poseIn.pos_z;

	ovrQuatf rot;
	rot.x = poseIn.rot_x;
	rot.y = poseIn.rot_y;
	rot.z = poseIn.rot_z;
	rot.w = poseIn.rot_w;

	ovrPosef retPose;
	retPose.Position = pos;
	retPose.Orientation = rot;
	return retPose;
}

// serialize a glm matrix for the server
s_Mat serializeMat(glm::mat4 mat)
{
	float convertedMat[16] = { 0.0f };

	const float *pSource = (const float*)glm::value_ptr(mat);
	for (int i = 0; i < 16; ++i)
		convertedMat[i] = pSource[i];

	std::vector<float> m(convertedMat, convertedMat + sizeof(convertedMat) / sizeof(convertedMat[0]));

	s_Mat ret;
	ret.values = m;
	return ret;
}

// deserialize a server matrix for glm
glm::mat4 deserializeMat(s_Mat mat)
{
	glm::mat4 ret = glm::make_mat4(mat.values.data());
	return ret;
}

#endif
