#include <glm/glm.hpp>
#include "MatrixUtils.h"

glm::vec4 posFromMatrix(glm::mat4 mat)
{
	return glm::vec4(mat[0][3], mat[1][3], mat[2][3], 1);
}


glm::mat4 xRotation(glm::mat4 mat, float angle)
{
	return glm::mat4(
		1,0,0,0,
		0,cos(angle),-sin(angle),0,
		0,sin(angle),cos(angle),0,
		0,0,0,1
	) * mat;
}

glm::mat4 yRotation(glm::mat4 mat, float angle)
{
	return glm::mat4(
		cos(angle),0,sin(angle),0,
		0,1,0,0,
		-sin(angle),0,cos(angle),0,
		0,0,0,1
	) * mat;
}

glm::mat4 zRotation(glm::mat4 mat, float angle)
{
	return glm::mat4(
		cos(angle),-sin(angle),0,0,
		sin(angle),cos(angle),0,0,
		0,0,1,0,
		0,0,0,1
	) * mat;
}

glm::mat4 move(glm::mat4 mat, glm::vec3 add)
{
	mat[0][3] += add.x;
	mat[1][3] += add.y;
	mat[2][3] += add.z;

	return mat;
}

glm::mat4 matrixTRS(glm::vec3 pos, glm::vec3 eulerAngles)
{
	glm::mat4 mat = glm::mat4(
		1,0,0,0,
		0,1,0,0,
		0,0,1,0,
		0,0,0,1
	);
	mat = xRotation(mat, eulerAngles.x);
	mat = yRotation(mat, eulerAngles.y);
	mat = zRotation(mat, eulerAngles.z);
	return move(mat, pos);
}

glm::mat4 lookAt(glm::mat4 mat, glm::vec3 origin)
{
	glm::vec4 pos = posFromMatrix(mat);
	glm::vec3 fwd = glm::normalize((glm::vec3)pos - origin);
	glm::vec3 right = glm::cross(fwd, glm::vec3(0,1,0));
	glm::vec3 up = glm::cross(fwd, -right);
	glm::mat4 newMat = glm::mat4(
		right.x, up.x, fwd.x, pos.x,
		right.y, up.y, fwd.y, pos.y,
		right.z, up.z, fwd.z, pos.z,
		0,0,0,1
		);

	return newMat;
}


Colour vectorToColour(glm::vec3 col)
{
	return Colour(
		col.x > 1 ? 255 : (col.x < 0 ? 0 : col.x * 255),
		col.y > 1 ? 255 : (col.y < 0 ? 0 : col.y * 255),
		col.z > 1 ? 255 : (col.z < 0 ? 0 : col.z * 255)
	);
}

glm::vec3 colourToVector(Colour col)
{
	return glm::vec3(col.red / 255.0, col.green / 255.0, col.blue / 255.0);
}


glm::vec3 intToVector(uint32_t colour) 
{
    //const uint32_t alphaMask = 0xFF000000;
    const uint32_t redMask = 0x00FF0000;
    const uint32_t greenMask = 0x0000FF00;
    const uint32_t blueMask = 0x000000FF;
    //int a = (colour & alphaMask) >> 24;
    int r = (colour & redMask) >> 16;
    int g = (colour & greenMask) >> 8;
    int b = (colour & blueMask);
	return glm::vec3(r / 255.0f, g / 255.0f, b / 255.0f);
}