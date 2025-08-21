#include <iostream>
#include <string>
#include "TimeSeries.h" // Node
#include "Graph.h"      // Graph

int main()
{
    Node ts;   // Project 4 data manager
    Graph g;   // Project 5 relational graph

    std::string command;
    while(std::cin >> command)
    {
        if(command=="LOAD")
        {
            std::string filename; std::cin >> filename;
            ts.load(filename);
        }
        else if(command=="LIST")
        {
            std::string country; std::cin >> country;
            ts.list(country);
        }
        else if(command=="RANGE")
        {
            std::string sc; std::cin >> sc;
            ts.range(sc);
        }
        else if(command=="BUILD")
        {
            std::string sc; std::cin >> sc;
            ts.build(sc); // prints "success" for direct BUILD
        }
        else if(command=="FIND")
        {
            double val; std::string op;
            if(std::cin >> val >> op) ts.find(val, op);
            else std::cout<<"failure"<<std::endl;
        }
        else if(command=="DELETE")
        {
            std::string c; std::cin >> c;
            ts.deleteCountryByName(c);
        }
        else if(command=="LIMITS")
        {
            std::string cond; std::cin >> cond;
            ts.limits(cond);
        }
        else if(command=="LOOKUP")
        {
            std::string code; std::cin >> code;
            ts.lookup(code);
        }
        else if(command=="REMOVE")
        {
            std::string code; std::cin >> code;
            ts.remove(code);
        }
        else if(command=="INSERT")
        {
            std::string code,fn; std::cin >> code >> fn;
            ts.insert(code,fn);
        }
        // Project 5 commands:
        else if(command=="INITIALIZE")
        {
            g.initialize(ts);
            std::cout<<"success"<<std::endl;
        }
        else if(command=="UPDATE_EDGES")
        {
            std::string sc, relation;
            double threshold;
            std::cin >> sc >> threshold >> relation;
            bool updated=g.updateEdges(sc, threshold, relation);
            if(updated) std::cout<<"success"<<std::endl;
            else std::cout<<"failure"<<std::endl;
        }
        else if(command=="ADJACENT")
        {
            std::string code; std::cin >> code;
            g.adjacent(code);
        }
        else if(command=="PATH")
        {
            std::string c1,c2; std::cin >> c1 >> c2;
            bool connected=g.path(c1,c2);
            std::cout<<(connected?"true":"false")<<std::endl;
        }
        else if(command=="RELATIONSHIPS")
        {
            std::string c1,c2; std::cin >> c1 >> c2;
            g.relationships(c1,c2);
        }
        else if(command=="EXIT")
        {
            break;
        }
    }
    return 0;
}
