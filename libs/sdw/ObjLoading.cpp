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

ModelVertex vertFromString(std::string s, float scale)
{
	int xTo = getSubStringIndex(s, ' ', 2);
	float x = std::stof(stringRange(s, 2, xTo));
	int yTo = getSubStringIndex(s, ' ', xTo + 1);
	float y = std::stof(stringRange(s, xTo + 1, yTo));
	float z = std::stof(stringRange(s, yTo + 1, s.length()));

	ModelVertex vert = ModelVertex(glm::vec3(x, y, z) * scale);
	return vert;
}

ModelTriangle triFromString(std::string s, std::vector<ModelVertex> &verts, Colour col)
{
	int xTo = getSubStringIndex(s, ' ', 2);
	int x = std::stoi(stringRange(s, 2, xTo)) - 1;
	int yTo = getSubStringIndex(s, ' ', xTo + 1);
	int y = std::stoi(stringRange(s, xTo + 1, yTo)) - 1;
	int z = std::stoi(stringRange(s, yTo + 1, s.length())) - 1;

	glm::vec3 v0 = verts[x].pos;
	glm::vec3 v1 = verts[y].pos;
	glm::vec3 v2 = verts[z].pos;
	glm::vec3 normal = glm::normalize(glm::cross(v1 - v0, v2 - v0));
	
	verts[x].normal += normal;
	verts[y].normal += normal;
	verts[z].normal += normal;
	ModelTriangle tri = ModelTriangle(x, y, z, col);
	return tri;
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

void loadMtl(std::unordered_map<std::string, Colour> &materials, std::string path)
{
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
}

void loadObj(std::vector<ModelTriangle> &triangles, std::string path, std::unordered_map<std::string, Colour> &materials, std::vector<ModelVertex> &verts, float scale) 
{
	std::string line;
	std::ifstream file(path);
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
					ModelVertex newVertex = vertFromString(line, scale);
					verts.push_back(newVertex);
				}
				else if (line[0] == 'f')
				{
					triangles.push_back(triFromString(line, verts, currentColour));
				}
			}
		}
	}

	file.close();

	for (int i = 0; i < verts.size(); i++)
	{
		verts[i].normal = glm::normalize(verts[i].normal);
	}
}