#ifndef GRAPH_H
#define GRAPH_H

#include <vector>
#include <string>
#include <cmath>
#include <queue>

// Forward-declare this Node class so that I can reference it in the Graph class.
class Node;

struct Relationship // This struct represents a relationship tuple between two countries. //used this part from chatgpt
{
    std::string seriesCode;
    double threshold;      //used to compare country data
    std::string relation; // "greater", "less", or "equal"

    bool operator==(const Relationship &other) const    //Two Relationship objects are considered equal if all three components match.
    {
        return (seriesCode == other.seriesCode && threshold == other.threshold && relation == other.relation);
    }
};

class GraphEdge // Represents an edge between two countries in the graph. //Each edge stores a collection of unique Relationship tuples that describe how the two countries are related.
{
    public:
        std::vector<Relationship> relationships;         // A vector that holds all relationship tuples for the edge.
        bool addRelationship(const Relationship &rel);   // Adds a new relationship to the edge if it does not already exist, returns true if the relationship was added, false otherwise.
};

class GraphNode // Represents a node in the graph, corresponding to a country.
{
    public:
        std::string countryName;
        std::string countryCode;
        std::vector<std::pair<GraphNode*, GraphEdge>> edges; // Edges: a vector of pairs containing a pointer to an adjacent node and the edge details.
};

class Graph
{
    private:
        Node* nodePtr; // Pointer to the Node object from Project 4, which holds country data and a binary tree.

    public:
        std::vector<GraphNode> nodes; // Vector of graph nodes (each corresponding to a country).

        Graph();                      // Constructor.
        void initialize(Node &n);     
        bool updateEdges(const std::string &seriesCode, double threshold, const std::string &relation);
        void adjacent(const std::string &countryCode);
        bool path(const std::string &code1, const std::string &code2);
        void relationships(const std::string &code1, const std::string &code2);
};

#endif
