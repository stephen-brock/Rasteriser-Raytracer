#include "KdTree.h"

Node::Node() = default;

Node::Node(glm::vec3 &location, glm::vec3 &colour)
{
    this->location = location;
    this->colour = colour;
}

glm::vec3 Node::Search(glm::vec3 &location, float closestDistance, int depth)
{
    int axis = fmod(depth, 3);
    Node* nextNode = location[axis] > this->location[axis] ? rightChild : leftChild;
    if (nextNode == nullptr)
    {
        return colour;
    }

    float nextDst = glm::distance(location, this->location);
    if (nextDst > closestDistance)
    {
        return colour;
    }

    return nextNode->Search(location, nextDst, depth + 1);
}

Node::~Node()
{
    delete leftChild;
    delete rightChild;
}

void Node::Insert(glm::vec3 &location, glm::vec3 &colour, int depth)
{
    int axis = fmod(depth, 3);
    int value = location[axis];
    int thisValue = location[axis];
    // std::cout << depth << std::endl;
    if (value > thisValue)
    {
        if (leftChild == nullptr)
        {
            leftChild = new Node(location, colour);
        }
        else 
        {
            leftChild->Insert(location, colour, depth+1);
        }
    }
    else 
    {
        if (rightChild == nullptr)
        {
            rightChild = new Node(location, colour);
        }
        else 
        {
            rightChild->Insert(location, colour, depth+1);
        }
    }
}

KdTree::KdTree() = default;

KdTree::KdTree(std::vector<glm::vec3> &location, std::vector<glm::vec3> &data)
{
    root = new Node(location[0], data[0]);
    for (int i = 1; i < data.size(); i++)
    {
        root->Insert(location[i], data[i]);
    }
}
glm::vec3 KdTree::Search(glm::vec3 &location)
{
    return root->Search(location, 1000000);
}

KdTree::~KdTree()
{
    delete root;
}