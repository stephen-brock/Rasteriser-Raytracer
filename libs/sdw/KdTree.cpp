#include "KdTree.h"

Node::Node() = default;

Node::Node(glm::vec3 &location, glm::vec3 &colour)
{
    this->location = location;
    this->colour = colour;
}

glm::vec3 Node::Search(glm::vec3 &location, float &closestDistance, int depth)
{
    int axis = depth % 3;
    Node* nextNode = location[axis] > this->location[axis] ? rightChild : leftChild;
    glm::vec3 dir = this->location - location;
    glm::vec3 col = colour;
    closestDistance = glm::dot(dir, dir);
    
    if (nextNode != nullptr)
    {
        float childDistance = -1;
        glm::vec3 childCol = nextNode->Search(location, childDistance, depth + 1);

        if (childDistance < closestDistance)
        {
            closestDistance = childDistance;
            col = childCol;
        }
    }

    Node* otherNode = location[axis] > this->location[axis] ? leftChild : rightChild;
    if (otherNode != nullptr)
    {
        glm::vec3 otherDir = location - this->location;
        //does hypersphere cross the hyperplane
        if (otherDir[axis] * otherDir[axis] <= closestDistance)
        {
            float childDistance = -1;
            glm::vec3 childCol = otherNode->Search(location, childDistance, depth + 1);

            if (childDistance < closestDistance)
            {
                closestDistance = childDistance;
                col = childCol;
            }
        }
    }

    return col;
}

void Node::SearchKNeighbours(glm::vec3 &location, std::vector<glm::vec3> &colours, std::vector<float> &sqrDistances, int depth)
{
    int axis = depth % 3;
    Node* nextNode = location[axis] > this->location[axis] ? rightChild : leftChild;
    glm::vec3 dir = this->location - location;
    glm::vec3 col = colour;
    float thisDistance = glm::dot(dir, dir);
    setMinimumDistance(colours, sqrDistances, col, thisDistance);
    
    if (nextNode != nullptr)
    {
        nextNode->SearchKNeighbours(location, colours, sqrDistances, depth + 1);
    }

    Node* otherNode = location[axis] > this->location[axis] ? leftChild : rightChild;
    if (otherNode != nullptr)
    {
        glm::vec3 otherDir = this->location - otherNode->location;
        float otherDst = glm::dot(otherDir, otherDir);
        //does hypersphere cross the hyperplane
        if (otherDir[axis] * otherDir[axis] <= sqrDistances[sqrDistances.size() - 1])
        {
            otherNode->SearchKNeighbours(location, colours, sqrDistances, depth + 1);
        }
    }
}

void setMinimumDistance(std::vector<glm::vec3> &colours, std::vector<float> &sqrDistances, glm::vec3 col, float sqrDst)
{
    bool replaced = false;
    for (int i = 0; i < sqrDistances.size() && !replaced; i++)
    {
        if (sqrDst < sqrDistances[i])
        {
            replaced = true;
            colours.insert(colours.begin() + i, col);
            sqrDistances.insert(sqrDistances.begin() + i, sqrDst);
        }
    }

    if (replaced)
    {
        colours.pop_back();
        sqrDistances.pop_back();
    }
    
}

Node::~Node()
{
    delete leftChild;
    delete rightChild;
}

void Node::Insert(glm::vec3 &location, glm::vec3 &colour, int depth)
{
    int axis = depth % 3;
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
glm::vec3 KdTree::Search(glm::vec3 &location, float &sqrDistance)
{
    return root->Search(location, sqrDistance);
}

std::vector<glm::vec3> KdTree::SearchKNeighbours(glm::vec3 &location, std::vector<float> &sqrDistances, int k)
{
    std::vector<glm::vec3> colours = std::vector<glm::vec3>();
    for (int i = 0; i < k; i++)
    {
        colours.push_back(glm::vec3(0,0,0));
        sqrDistances.push_back(1000000);
    }
    
    root->SearchKNeighbours(location, colours, sqrDistances, k);
    return colours;
}

KdTree::~KdTree()
{
    delete root;
}