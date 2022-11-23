#pragma once
#include <glm/glm.hpp>
#include <vector>

struct Node
{
    glm::vec3 location{};
    glm::vec3 colour{};
    Node* leftChild{};
    Node* rightChild{};
    Node();
    Node(glm::vec3 &location, glm::vec3 &colour);
    ~Node();
    glm::vec3 Search(glm::vec3 &location, float closestDistance, int depth=0);
    void Insert(glm::vec3 &location, glm::vec3 &colour, int depth=0);
};

class KdTree
{
    public:
        KdTree();
        KdTree(std::vector<glm::vec3> &location, std::vector<glm::vec3> &data);
        glm::vec3 Search(glm::vec3 &location);
        ~KdTree();
    private:
        Node* root{};
};
