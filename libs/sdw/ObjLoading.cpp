#include <fstream>
#include <unordered_map>
#include <fstream>
#include <vector>
#include <iostream>
#include <string>
#include <glm/glm.hpp>
#include "ModelTriangle.h"
#include "TexturedMaterial.h"
#include "MatrixUtils.h"
#include "ObjLoading.h"

int getSubStringIndex(std::string s, char delimiter, int startIndex)
{
	for (int i = startIndex; i < s.length(); i++)
	{
		if (s[i] == delimiter) 
		{
			return i;
		}
	}

	return -1;
}
std::string stringRange(std::string s, int from, int to)
{
	std::string sub = "";
	for (int i = from; i < to; i++)
	{
		sub += s[i];
	}
	
	return sub;
}

glm::vec3 vertFromString(std::string s, float scale)
{
	int xTo = getSubStringIndex(s, ' ', 2);
	float x = std::stof(stringRange(s, 2, xTo));
	int yTo = getSubStringIndex(s, ' ', xTo + 1);
	float y = std::stof(stringRange(s, xTo + 1, yTo));
	float z = std::stof(stringRange(s, yTo + 1, s.length()));

	return glm::vec3(x, y, z) * scale;
}

void triFromString(Model &model, std::string s, int fromCount)
{
	int xTo = getSubStringIndex(s, ' ', 2);
	int x = std::stoi(stringRange(s, 2, xTo)) - 1;
	int yTo = getSubStringIndex(s, ' ', xTo + 1);
	int y = std::stoi(stringRange(s, xTo + 1, yTo)) - 1;
	int z = std::stoi(stringRange(s, yTo + 1, s.length())) - 1;

	model.AddTriangle(x - fromCount, y - fromCount, z - fromCount);
}

std::string getMatNameFromString(std::string s)
{
	return stringRange(s, 6, s.length());
}

Colour getColourFromString(std::string s) 
{
	int xTo = getSubStringIndex(s, ' ', 3);
	float x = std::stof(stringRange(s, 3, xTo));
	int yTo = getSubStringIndex(s, ' ', xTo + 1);
	float y = std::stof(stringRange(s, xTo + 1, yTo));
	float z = std::stof(stringRange(s, yTo + 1, s.length()));

	return Colour(x * 255,y * 255,z * 255);
}

void loadMtl(std::unordered_map<std::string, Material*> &materials, std::string path)
{
	std::string line;
	std::ifstream file(path);

	while (getline(file, line))
	{
		if (line[0] == 'n')
		{
			std::string name = getMatNameFromString(line);
			getline(file, line);
			glm::vec3 colour = colourToVector(getColourFromString(line));
			bool mirror = false;
			bool refract = false;
			getline(file, line);
			if (line[0] == 'm')
			{
				mirror = true;
				getline(file, line);
			}
			else if (line[0] == 'r')
			{
				refract = true;
				getline(file, line);
			}
			if (line[0] == 't')
			{
				std::string texpath = stringRange(line, 4, line.length());
				materials[name] = new TexturedMaterial(colour, mirror, refract, texpath);
			}
			else 
			{
				materials[name] = new Material(colour, mirror, refract);
			}
		}
	}
}

void loadObj(std::vector<Model*> &models, std::string path, std::unordered_map<std::string, Material*> &materials, float scale) 
{
	std::string line;
	std::ifstream file(path);
	int fromCount = 0;
	while (getline(file, line))
	{
		if (line[0] == 'o') 
		{
			getline(file, line);
			std::string mat = getMatNameFromString(line);
			Model* model = new Model(materials[mat]);
			while (getline(file, line) && line.length() > 0) 
			{
				if (line[0] == 'v')
				{
					glm::vec3 pos = vertFromString(line, scale);
					ModelVertex mv = ModelVertex(pos);
					mv.texcoord = glm::vec2(pos.x / 2, pos.z / 2);
					model->verts->push_back(mv);
				}
				else if (line[0] == 'f')
				{	
					triFromString(*model, line, fromCount);
				}
			}
			fromCount += model->verts->size();
			model->NormaliseNormals();
			models.push_back(model);
		}
	}

	file.close();
}