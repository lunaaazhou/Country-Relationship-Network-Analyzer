#include "Graph.h"
#include "TimeSeries.h"
#include <iostream>
#include <queue>    // For std::queue used in BFS (path function).
#include <cmath>    // For std::fabs to compare doubles.


// addRelationship: adds a new relationship to the edge if it is not already present.
bool GraphEdge::addRelationship(const Relationship &rel)
{
    for (const auto &r : relationships) //loop through all the existing elements in the edge
    {
        if (r == rel)    // If the new relationship matches an existing relationship, then return false
        {
            return false;
        }
    }
    relationships.push_back(rel); //otherwise, add the new relationship to the vector
    return true;
}

// Graph Constructor
Graph::Graph() : nodePtr(nullptr) {}

// initialize: builds graph nodes from Node's data
void Graph::initialize(Node &n)
{
    nodePtr = &n;   // Save a pointer to the Node object for later on calculating the mean
    nodes.clear();  // Clear all existing nodes in the graph.

    TimeSeries* arr[512];  // Create an array to hold pointers to all countries from the Node object.
    int count = n.getAllCountries(arr, 512); // Get all country entries (up to 512).

    for(int i=0; i<count; i++) // For each country found, create a GraphNode.
    {
        GraphNode gn;                               // Temporary GraphNode.
        gn.countryName = arr[i]->getCountryName();  // Set country name.
        gn.countryCode = arr[i]->getCountryCode();  // Set country code.
        nodes.push_back(gn);                        // Add the node to the graph.
    }
}

// updateEdges: if the tree isn't built, call nodePtr->buildInternal(...) quietly
// setCurrentSeriesCode, then findMatching, then create edges //get this function idea from chatgpt
bool Graph::updateEdges(const std::string &seriesCode, double threshold, const std::string &relation)
{
    if(!nodePtr) return false;

    // If the tree isn't built, do a silent build.
    if(!nodePtr->isTreeBuilt())
    {
        // Because Graph is a friend of Node, so call buildInternal:
        nodePtr->buildInternal(seriesCode);
    }
    // Then set the series code
    nodePtr->setCurrentSeriesCode(seriesCode);

    // get matching countries from the binary tree
    //std::vector STL source from:https://stackoverflow.com/questions/10750057/how-do-i-print-out-the-contents-of-a-vector
    std::vector<std::string> matchingCountries = nodePtr->findMatching(threshold, relation);

    if(matchingCountries.empty())
    {
        return false;
    }

    bool added=false; // Flag to track if any new relationships were added.

    // for each pair in matchingCountries, update edges
    for (size_t i = 0; i < matchingCountries.size(); i++)
    {
        for (size_t j = i+1; j < matchingCountries.size(); j++) // For each unique pair of matching countries, update or create an edge.
        {
            int idx1 = -1, idx2 = -1;
            
            for (size_t k = 0;k < nodes.size(); k++) // Find the corresponding indices of these country names in the graph's nodes vector.
            {
                if(nodes[k].countryName == matchingCountries[i]) idx1 = (int)k;
                if(nodes[k].countryName == matchingCountries[j]) idx2 = (int)k;
            }

            if (idx1 != -1 && idx2 != -1) // If both countries are found in the graph:
            {
                Relationship rel{seriesCode, threshold, relation}; // Create the relationship tuple.
                bool edgeFound=false; // Flag to indicate if an edge already exists between these two nodes.

                for (auto &edgePair : nodes[idx1].edges)  // Iterate over the edges of the first node.
                {
                    if (edgePair.first->countryCode==nodes[idx2].countryCode) // Check if the adjacent node has the same country code as the second node.
                    {
                        if (edgePair.second.addRelationship(rel)) // If found, attempt to add the relationship.
                            added=true;
                        edgeFound=true;

                        break; // Stop checking further edges.
                    }
                }

                if (!edgeFound) // If no edge exists between these two nodes, create a new edge.
                {
                    GraphEdge newEdge;
                    newEdge.addRelationship(rel); // Add the relationship to the new edge.

                    nodes[idx1].edges.push_back({ &nodes[idx2], newEdge }); // Add the edge to both nodes' adjacency lists.
                    nodes[idx2].edges.push_back({ &nodes[idx1], newEdge });

                    added=true;
                }
            }
        }
    }
    return added;     // Return true if at least one new relationship was added; false otherwise.
}

// adjacent
void Graph::adjacent(const std::string &countryCode)
{
    for (auto &gn : nodes) // Iterate over all graph nodes.
    {
        if (gn.countryCode == countryCode)  // Check if the current node's country code matches the input code
        {
            if (gn.edges.empty())
            {
                std::cout << "none" << std::endl;  // If the node has no edges, output "none".
            }
            else
            {
                for (auto &ep : gn.edges) // Otherwise, print the country names of all adjacent nodes.
                {
                    std::cout<<ep.first->countryName<<" ";
                }

                std::cout<<std::endl;
            }
            return; //exist after processing the matching nodes
        }
    }

    std::cout << "failure" << std::endl;
}

// path
bool Graph::path(const std::string &code1,const std::string &code2) //write this function using chatgpt
{
    if (code1 == code2) // If the two codes are identical, then they are connected.
    {
        return true;  
    }
    int start=-1;

    for (size_t i = 0;i < nodes.size(); i++) // Find the starting node in the graph with country code code1.
    {
        if (nodes[i].countryCode == code1)
        {
            start=(int)i;
            break;
        }
    }

    if (start==-1)     // If no starting node is found, return false.
    {
        return false;
    }

    // Prepare a visited vector for BFS.
    std::vector<bool> visited(nodes.size(),false);  //https://en.cppreference.com/w/cpp/container/vector_bool
    std::queue<int>q; //https://cplusplus.com/reference/queue/queue/

    q.push(start);
    visited[start]=true;

    // Perform BFS.
    while (!q.empty())
    {
        int cur=q.front();q.pop();

        if (nodes[cur].countryCode==code2) // If the current node's code matches code2, a path exists.
        {
            return true;
        }

        for (auto &edgePair : nodes[cur].edges) // For each adjacent node of the current node:
        {
            int neighborIdx=-1;

            for (size_t k=0;k<nodes.size();k++) // Find the index of the neighbor in the nodes vector.
            {
                if (&nodes[k]==edgePair.first)
                {
                    neighborIdx=(int)k;break;
                }
            }

            if (neighborIdx!=-1 && !visited[neighborIdx]) // If the neighbor exists and has not been visited, mark it and enqueue it.
            {
                visited[neighborIdx]=true;
                q.push(neighborIdx);
            }
        }
    }
    return false;     // If BFS completes without finding code2, no path exists.
}

// relationships
void Graph::relationships(const std::string &code1,const std::string &code2)
{
    for(auto &gn : nodes) // Iterate over graph nodes to find the node with country code code1.
    {
        if(gn.countryCode == code1)
        {
            for(auto &edgePair : gn.edges)// Iterate through the edges of this node.
            {
                if(edgePair.first->countryCode == code2) // Check if the adjacent node's code matches code2.
                {
                    for(const auto &rel : edgePair.second.relationships) // print each relationship in the edge
                    {
                        std::cout << "(" << rel.seriesCode << " " << rel.threshold << " " << rel.relation << ") ";
                    }

                    std::cout<<std::endl;

                    return;
                }
            }
            
            break;
        }
    }

    std::cout << "none" << std::endl;  // If no edge exists between the specified countries, output "none". 
}
