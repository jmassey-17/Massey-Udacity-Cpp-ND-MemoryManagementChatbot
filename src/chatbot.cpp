#include <iostream>
#include <random>
#include <algorithm>
#include <ctime>

#include "chatlogic.h"
#include "graphnode.h"
#include "graphedge.h"
#include "chatbot.h"
//JM
#include <memory>

// constructor WITHOUT memory allocation
ChatBot::ChatBot()
{
    // invalidate data handles
    _image = nullptr;
    _chatLogic = nullptr;
    _rootNode = nullptr;
}

// constructor WITH memory allocation
ChatBot::ChatBot(std::string filename)
{
    std::cout << "ChatBot Constructor started" << std::endl;
    
    // invalidate data handles
    _chatLogic = nullptr;
    _rootNode = nullptr;

    // load image into heap memory
    _image = new wxBitmap(filename, wxBITMAP_TYPE_PNG);
   
  	// message for task 5
   std::cout << "ChatBot Constructor Finished" << std::endl;
}

ChatBot::~ChatBot()
{
    std::cout << "ChatBot Destructor started" << std::endl;

    // deallocate heap memory
    if(_image != nullptr) // Attention: wxWidgets used NULL and not nullptr
    {
        delete _image;
        _image = nullptr;
    }
  // message for task 5
  std::cout << "ChatBot Destructor finished" << std::endl;
}

//// STUDENT CODE
////
// copy
ChatBot::ChatBot(const ChatBot& source) {
  std::cout << "ChatBot Copy started" << std::endl;
	// invalidate data handles
    _chatLogic = source._chatLogic;
    _rootNode = source._rootNode;

    // load image into heap memory
    if (source._image != nullptr)
    {
        _image = new wxBitmap(*source._image);
    }
    else
    {
        _image = nullptr;
    }
  std::cout << "ChatBot copy finished" << std::endl;
}
// copy assign
// adapted from RuleOfFive lesson in course material
ChatBot& ChatBot::operator=(const ChatBot& source) 
    {
        std::cout << "ChatBot copy assign started" << std::endl;
      // guard against self assignment 
        if (this == &source)
            return *this;
      // Delete existing image if present
    if (_image != nullptr)
    {
        delete _image;
    }

    // Deep copy for _image
    if (source._image != nullptr)
    {
        _image = new wxBitmap(*source._image);
    }
    else
    {
        _image = nullptr;
    }
	// shallow copy - check this
    _chatLogic = source._chatLogic;
    _rootNode = source._rootNode;
      std::cout << "ChatBot copy assign finished" << std::endl;
    }
// move
// adapted from RuleOfFive lesson in course material
ChatBot::ChatBot(ChatBot&& source) 
{
    std::cout << "ChatBot move started" << std::endl;
  // move from source
    _chatLogic = source._chatLogic;
    _rootNode = source._rootNode;
    _image = source._image;

    // nullify source
    source._chatLogic = nullptr;
    source._rootNode = nullptr;
    source._image = nullptr;
  std::cout << "ChatBot move finished" << std::endl;
}
// move assign
// adapted from RuleOfFive lesson in course material
ChatBot& ChatBot::operator=(ChatBot&& source) 
{
    std::cout << "ChatBot move assign started" << std::endl;
  // guard against self assignment 
    if (this == &source)
        return *this;
  std::cout << "ChatBot move assign - check for self assignment" << std::endl;
	// Deallocate existing resources
    if (_image != nullptr)
        delete _image;
  std::cout << "ChatBot move assign - delete image" << std::endl;

    // Transfer ownership from source to this object
  std::cout << "ChatBot move assign - trasnfer ownership" << std::endl;
    _image = source._image;
    _currentNode = source._currentNode;
    _rootNode = source._rootNode;
	std::cout << "ChatBot move assign - trasnfer ownership finished" << std::endl;
    // Nullify the source object after move
  std::cout << "ChatBot move assign - nullify after move" << std::endl;
    source._image = nullptr;
    source._currentNode = nullptr;
    source._rootNode = nullptr;
  std::cout << "ChatBot move assign - nullify after move finished" << std::endl;
	std::cout << "ChatBot move assign finished" << std::endl;
    return *this;
  
}
////
//// EOF STUDENT CODE

void ChatBot::ReceiveMessageFromUser(std::string message)
{
    // loop over all edges and keywords and compute Levenshtein distance to query
    typedef std::pair<GraphEdge *, int> EdgeDist;
    std::vector<EdgeDist> levDists; // format is <ptr,levDist>

    for (size_t i = 0; i < _currentNode->GetNumberOfChildEdges(); ++i)
    {
        GraphEdge *edge = _currentNode->GetChildEdgeAtIndex(i);
        for (auto keyword : edge->GetKeywords())
        {
            EdgeDist ed{edge, ComputeLevenshteinDistance(keyword, message)};
            levDists.push_back(ed);
        }
    }

    // select best fitting edge to proceed along
    GraphNode *newNode;
    if (levDists.size() > 0)
    {
        // sort in ascending order of Levenshtein distance (best fit is at the top)
        std::sort(levDists.begin(), levDists.end(), [](const EdgeDist &a, const EdgeDist &b) { return a.second < b.second; });
        newNode = levDists.at(0).first->GetChildNode(); // after sorting the best edge is at first position
    }
    else
    {
        // go back to root node
        newNode = _rootNode;
    }

    // tell current node to move chatbot to new node
    _currentNode->MoveChatbotToNewNode(newNode);
}

void ChatBot::SetCurrentNode(GraphNode *node)
{
    // update pointer to current node
    _currentNode = node;

    // select a random node answer (if several answers should exist)
    std::vector<std::string> answers = _currentNode->GetAnswers();
    std::mt19937 generator(int(std::time(0)));
    std::uniform_int_distribution<int> dis(0, answers.size() - 1);
    std::string answer = answers.at(dis(generator));

    // send selected node answer to user - task 5
  	_chatLogic->SetChatbotHandle(this);
    _chatLogic->SendMessageToUser(answer);
}

int ChatBot::ComputeLevenshteinDistance(std::string s1, std::string s2)
{
    // convert both strings to upper-case before comparing
    std::transform(s1.begin(), s1.end(), s1.begin(), ::toupper);
    std::transform(s2.begin(), s2.end(), s2.begin(), ::toupper);

    // compute Levenshtein distance measure between both strings
    const size_t m(s1.size());
    const size_t n(s2.size());

    if (m == 0)
        return n;
    if (n == 0)
        return m;

    size_t *costs = new size_t[n + 1];

    for (size_t k = 0; k <= n; k++)
        costs[k] = k;

    size_t i = 0;
    for (std::string::const_iterator it1 = s1.begin(); it1 != s1.end(); ++it1, ++i)
    {
        costs[0] = i + 1;
        size_t corner = i;

        size_t j = 0;
        for (std::string::const_iterator it2 = s2.begin(); it2 != s2.end(); ++it2, ++j)
        {
            size_t upper = costs[j + 1];
            if (*it1 == *it2)
            {
                costs[j + 1] = corner;
            }
            else
            {
                size_t t(upper < corner ? upper : corner);
                costs[j + 1] = (costs[j] < t ? costs[j] : t) + 1;
            }

            corner = upper;
        }
    }

    int result = costs[n];
    delete[] costs;

    return result;
}