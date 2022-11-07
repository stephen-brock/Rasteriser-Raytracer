#include <fstream>
#include <unordered_map>
#include <fstream>
#include <vector>
#include <iostream>
#include <string>
#include <glm/glm.hpp>
#include "ModelTriangle.h"
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

glm::vec3 vec3FromString(std::string s, float scale)
{
	int xTo = getSubStringIndex(s, ' ', 2);
	float x = std::stof(stringRange(s, 2, xTo));
	int yTo = getSubStringIndex(s, ' ', xTo + 1);
	float y = std::stof(stringRange(s, xTo + 1, yTo));
	float z = std::stof(stringRange(s, yTo + 1, s.length()));

	return glm::vec3(x, y, z) * scale;
}

ModelTriangle triFromString(std::string s, std::vector<glm::vec3> &verts, Colour col)
{
	int xTo = getSubStringIndex(s, ' ', 2);
	int x = std::stoi(stringRange(s, 2, xTo)) - 1;
	int yTo = getSubStringIndex(s, ' ', xTo + 1);
	int y = std::stoi(stringRange(s, xTo + 1, yTo)) - 1;
	int z = std::stoi(stringRange(s, yTo + 1, s.length())) - 1;

	return ModelTriangle(verts[x], verts[y], verts[z], col);
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

std::unordered_map<std::string, Colour> loadMtl(std::string path)
{
	std::unordered_map<std::string, Colour> materials;

	std::string line;
	std::ifstream file(path);

	while (getline(file, line))
	{
		if (line[0] == 'n')
		{
			std::string name = getMatNameFromString(line);
			getline(file, line);
			Colour col = getColourFromString(line);
			materials[name] = col;
		}
	}

	return materials;
}

std::vector<ModelTriangle> loadObj(std::string path, std::unordered_map<std::string, Colour> &materials, float scale) 
{
	std::string line;
	std::ifstream file(path);
	std::vector<ModelTriangle> triangles;
	std::vector<glm::vec3> verts;
	Colour currentColour = Colour(-1,-1,-1);
	while (getline(file, line))
	{
		if (line[0] == 'o') 
		{
			getline(file, line);
			std::string mat = getMatNameFromString(line);
			currentColour = materials[mat];
			while (getline(file, line) && line.length() > 0) 
			{
				if (line[0] == 'v')
				{
					verts.push_back(vec3FromString(line, scale));
				}
				else if (line[0] == 'f')
				{
					triangles.push_back(triFromString(line, verts, currentColour));
				}
			}
		}
	}

	file.close();

	return triangles;
}