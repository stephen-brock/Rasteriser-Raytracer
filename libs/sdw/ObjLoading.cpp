#include <fstream>
#include <unordered_map>
#include <fstream>
#include <vector>
#include <iostream>
#include <string>
#include <glm/glm.hpp>
#include "ModelTriangle.h"
#include "ObjLoading.h"
#include "ModelVertex.h"

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

std::array<int, 3> triFromString(std::string s)
{
	int xTo = getSubStringIndex(s, ' ', 2);
	int x = std::stoi(stringRange(s, 2, xTo)) - 1;
	int yTo = getSubStringIndex(s, ' ', xTo + 1);
	int y = std::stoi(stringRange(s, xTo + 1, yTo)) - 1;
	int z = std::stoi(stringRange(s, yTo + 1, s.length())) - 1;
	//ModelTriangle tri = ModelTriangle(verts[x]->position, verts[y]->position, verts[z]->position, col);
	//glm::vec3 normal = glm::normalize(glm::cross(tri.vertices[1] - tri.vertices[0], tri.vertices[2] - tri.vertices[0]));
	//tri.normal = normal;
	return {x,y,z};
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
	std::vector<std::array<int, 3> > vertexIndicies;
	std::vector<ModelVertex*> verts;
	std::vector<ModelTriangle> triangles;
	Colour currentColour = Colour(128,128,128);
	while (getline(file, line))
	{
		if (line[0] == 'o') 
		{
			int fromCount = verts.size();
			getline(file, line);
			std::string mat = getMatNameFromString(line);
			currentColour = materials[mat];
			while (getline(file, line) && line.length() > 0) 
			{
				if (line[0] == 'v')
				{
					verts.push_back(new ModelVertex(vec3FromString(line, scale)));
				}
				else if (line[0] == 'f')
				{
					std::array<int, 3> newTri = triFromString(line);
					vertexIndicies.push_back(newTri);
					ModelVertex* v0 = verts[newTri[0]];
					ModelVertex* v1 = verts[newTri[1]];
					ModelVertex* v2 = verts[newTri[2]];
					glm::vec3 normal = glm::normalize(glm::cross(v1->position - v0->position, v2->position - v0->position));
					v0->AddNormal(normal);
					v1->AddNormal(normal);
					v2->AddNormal(normal);
				}
			}

			for (int i = fromCount; i < verts.size(); i++)
			{
				verts[i]->Normalize();
			}

			for (int i = 0; i < vertexIndicies.size(); i++)
			{
				std::array<int,3> tri = vertexIndicies[i];
				ModelTriangle newTri(*verts[tri[0]], *verts[tri[1]], *verts[tri[2]], currentColour);
				triangles.push_back(newTri);
			}
			

			vertexIndicies.clear();
		}
	}
	
	for (int i = 0; i < verts.size(); i++)
	{
		delete verts[i];
	}
	
	file.close();

	return triangles;
}