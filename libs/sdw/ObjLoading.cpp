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

glm::vec2 texcoordFromString(std::string s)
{
	int xTo = getSubStringIndex(s, ' ', 3);
	float x = std::stof(stringRange(s, 2, xTo));
	float y = std::stof(stringRange(s, xTo + 1, s.length()));

	return glm::vec2(x, y);
}

void triFromStringOld(Model &model, std::string s, int fromCount)
{
	int xTo = getSubStringIndex(s, ' ', 2);
	int x = std::stoi(stringRange(s, 2, xTo)) - 1;
	int yTo = getSubStringIndex(s, ' ', xTo + 1);
	int y = std::stoi(stringRange(s, xTo + 1, yTo)) - 1;
	int z = std::stoi(stringRange(s, yTo + 1, s.length())) - 1;

	model.AddTriangle(x - fromCount, y - fromCount, z - fromCount);
}

void triFromString(Model &model, std::vector<glm::vec2> &texcoords, std::string s, int fromCount)
{
	int xTo = getSubStringIndex(s, '/', 2);
	int x = std::stoi(stringRange(s, 2, xTo)) - 1;
	int tXTo = getSubStringIndex(s, ' ', xTo);
	int tX = std::stoi(stringRange(s, xTo + 1, tXTo)) - 1;

	int yTo = getSubStringIndex(s, '/', tXTo);
	int y= std::stoi(stringRange(s, tXTo + 1, yTo)) - 1;
	int tYTo = getSubStringIndex(s, ' ', yTo);
	int tY = std::stoi(stringRange(s, yTo + 1, tYTo)) - 1;

	int zTo = getSubStringIndex(s, '/', tYTo);
	int z = std::stoi(stringRange(s, tYTo + 1, zTo)) - 1;
	int tZ = std::stoi(stringRange(s, zTo + 1, s.length())) - 1;

	// std::cout << x << " " << y << " " << z << std::endl;

	model.AddTriangle(x - fromCount, y - fromCount, z - fromCount);
	model.GetVertex(x - fromCount).texcoord = texcoords[tX];
	model.GetVertex(y - fromCount).texcoord = texcoords[tY];
	model.GetVertex(z - fromCount).texcoord = texcoords[tZ];
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
			float refractiveIndex = 0;
			getline(file, line);
			float metallic = std::stof(stringRange(line, 9, line.length()));
			getline(file, line);
			if (line[0] == 'm')
			{
				mirror = true;
				getline(file, line);
			}
			else if (line[0] == 'r')
			{
				refract = true;
				refractiveIndex = std::stof(stringRange(line, 8, line.length()));
				getline(file, line);
			}
			if (line[0] == 't')
			{
				std::string texpath = stringRange(line, 4, line.length());
				getline(file, line);
				std::string nrmpath = stringRange(line, 4, line.length());
				getline(file, line);
				float normalStrength = std::stof(stringRange(line, 4, line.length()));
				if (refract)
				{
					materials[name] = new TexturedMaterial(colour, metallic, refract, refractiveIndex, texpath, nrmpath, normalStrength);
				}
				else 
				{
					materials[name] = new TexturedMaterial(colour, metallic, mirror, texpath, nrmpath, normalStrength);
				}
				
			}
			else 
			{
				if (refract)
				{
					materials[name] = new Material(colour, metallic, refract, refractiveIndex);
				}
				else 
				{
					materials[name] = new Material(colour, metallic, mirror);
				}
			}
		}
	}
}

void loadObjOld(std::vector<Model*> &models, std::string path, std::unordered_map<std::string, Material*> &materials, float scale) 
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
					model->AddVertex(mv);
				}
				else if (line[0] == 'f')
				{	
					triFromStringOld(*model, line, fromCount);
				}
			}
			fromCount += model->VertexAmount();
			model->NormaliseNormals();
			models.push_back(model);
		}
	}

	file.close();
}

void loadObj(std::vector<Model*> &models, std::string path, std::unordered_map<std::string, Material*> &materials, float scale) 
{
	std::string line;
	std::ifstream file(path);
	std::vector<glm::vec2> texcoords = std::vector<glm::vec2>();
	int fromCount = 0;
	while (getline(file, line))
	{
		if (line[0] == 'o') 
		{
			std::cout << line << std::endl;
			getline(file, line);
			std::cout << line << std::endl;
			std::string mat = getMatNameFromString(line);
			Model* model = new Model(materials[mat]);
			while (getline(file, line) && line.length() > 1) 
			{
				if (line[0] == 'v')
				{
					if (line[1] == ' ')
					{
						glm::vec3 pos = vertFromString(line, scale);
						ModelVertex mv = ModelVertex(pos);
						model->AddVertex(mv);
					}
					else if (line[1] == 't')
					{
						glm::vec2 pos = texcoordFromString(line);
						texcoords.push_back(pos);
					}
				}
				else if (line[0] == 'f')
				{	
					triFromString(*model, texcoords, line, fromCount);
				}
				else
				{
					// std::cout << "Unknonwn line: " << line << std::endl;
					// std::cout << line[1] << std::endl;
				}
			}
			fromCount += model->VertexAmount();
			model->NormaliseNormals();
			models.push_back(model);
		}
	}

	file.close();
}