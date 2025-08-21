#include "TimeSeries.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept> //the idea of this header from chatgpt, for helping manage runtime errors, like in this project, invalid string to double(change of datatype) conversions and could ensure it can recover and provide proper error message when something unexcepted happens
#include <vector>
#include <cmath>

TimeSeries::TimeSeries() //TimeSeries class constructor
{
    size = 0;
    year = new int[100];    // Fixed-size allocation for years.
    data = new double[100]; // Fixed-size allocation for data values.
    seriesNext = nullptr;   // Initialize linked-list pointer.
}

TimeSeries::~TimeSeries()   //TimeSeries class destructor
{
    delete[] year;
    delete[] data;
}

// Constructor for TreeNode
TreeNode::TreeNode(double val, double minR, double maxR, int i)
         : value(val),          // Set the node's value
            minRange(minR),     // Set the minimum range of values for the node's group
            maxRange(maxR),     // Set the maximum range of values for the node's group
            index(i),           // Set the index (for debugging or tracking purposes)
            left(nullptr),      // Initialize the left child pointer to nullptr (no left child yet)
            right(nullptr),     // Initialize the right child pointer to nullptr (no right child yet)
            countryCount(0) {}  // Start with zero countries stored in this node's array

// Constructor for Tree
Tree::Tree() : root(nullptr) {} // Initializes a Tree with its root pointer set to nullptr.

// Destructor for Tree
Tree::~Tree()
{
    deleteTree(root);   // Recursively delete all of the nodes from the root
    root = nullptr;     // Set the root pointer to nullptr after deleting
}

void Tree::deleteTree(TreeNode* node)
{
    if(!node)
    {
        return; //if the tree is empty, then there's nothing to delete
    }

    deleteTree(node->left);     // Recursively delete the left subtree
    deleteTree(node->right);    // Recursively delete the right subtree
    delete node;                // Delete the current node
}

//Starting of the DELETE function
//Tree Deletion Helper Functions(from Project 3) used this from chatgpt
bool removeCountryFromArray(TreeNode* node, const std::string &countryName)
{
    bool foundAny = false; //flag to check if any country were removed, add this for hash
    int i = 0;  // Initialize the index as zero for iterating through the countries array

    while(i < node->countryCount) //loop over countries in the node
    {
        if(node->countries[i] == countryName) //if current country matches with the given country name
        {
            for(int j = i; j < node->countryCount - 1; j++) //shift all country names one position to the left
            {
                node->countries[j] = node->countries[j + 1];
            }
            node->countryCount--; // Decrement the count of countries since one has been removed.
            foundAny = true; // Mark the flag when found and removed happens.

            // Do not increment 'i' here because after shifting, a new element now occupies index i. 
        }
        else
        {
            i++; //if no matches at the index
        }
    }
    return foundAny; //return true if >= 1 country removed, otherwise, return false
}

TreeNode* deleteCountryHelper(TreeNode* root, const std::string &countryName, bool &found)
{
    if (!root)
    {
        return nullptr; //if current node is null, exit the function
    }

    bool removedHere = removeCountryFromArray(root, countryName); //when >=1 country removed, use the previous helper function to chect this

    if(removedHere) // If the country was removed from this node, update the 'found' flag.
    {
        found = true;
    }

    root->left  = deleteCountryHelper(root->left, countryName, found); // Recursively process the left subtree and update the left pointer.
    root->right = deleteCountryHelper(root->right, countryName, found); // Recursively process the right subtree and update the right pointer.

    if(!root->left && !root->right && root->countryCount == 0)
    {
        delete root; //aviod memory leak
        return nullptr;
    }
    return root; //return the current node
}

//Node class constructor
Node::Node() : countryCount(0)  //initialize the member variable countryCount to 0 before the constructor's body runs, means when node created, it will starts with zero countries stored in the hash table
{
    for( int i = 0; i < TABLE_SIZE; i++ )   //loop iterates every index in the hash table.
    {
        table[i] = nullptr;           //each slots set to nullptr, means they are empty
        slotOccupied[i] = false;      //means no slot has been used yet
    }
}

 //Node class destructor
Node::~Node()
{
    for(int i=0;i<TABLE_SIZE;i++)   //traverse the linked list
    {
        if(table[i])
        {
            TimeSeries* s=table[i]->seriesNext;

            while(s)        //recursively deleting the tree
            {
                TimeSeries* tmp=s;
                s=s->seriesNext;
                delete tmp;
            }

            delete table[i];
        }
    }
    if(tree.root)
    {
        tree.deleteTree(tree.root); //indicating the tree no longer exist
        tree.root=nullptr;
    }
}


// Hash functions from project4
int Node::computeW(const std::string &countryCode) ////to covert the countrycode to hash code
{
    int W=0;
    for(int i=0;i<3 && i<(int)countryCode.size();i++)   //loop through the characters of the country code
    {
        char c=countryCode[i];
        int digit=c-'A';        // Convert the letters to 0-25.
        W=W*26 + digit; 
    }
    return W;       //return the numerical value
}

int Node::primaryHash(const std::string &countryCode)   //primary Hash function
{
    int W=computeW(countryCode);    //compute the countryCode to a numerical value
    return W%TABLE_SIZE;            //return the mod num to put it in hash table
}

int Node::secondaryHash(const std::string &countryCode)   //secondary Hash function
{
    int W=computeW(countryCode);
    int h=(W/TABLE_SIZE)%TABLE_SIZE;   //primary hash value
    if(h%2==0) h+=1;
    return h;     //return the secondary hash value
}



//helper function for the LOOKUP, INSERT, REMOVE function
int Node::findCountryIndex(const std::string &countryCode, int &steps) // the int &steps make steps a pass by reference, in this way, its updated value inside this helper function will be available when calling this function
{
    int h1 = primaryHash(countryCode); //calculate the first hash value, convert the country code
    int h2 = secondaryHash(countryCode);//compute the second hash value, this value will used a step size for collision resolution

    steps = 0;

    for(int i = 0; i < TABLE_SIZE; i++)
    {
        int idx =(h1 + i * h2) % TABLE_SIZE; //compute index using double hashing

        steps++;

        if(table[idx] == nullptr && !slotOccupied[idx])
        {
            // Empty slot and not previously occupied: country not in table.
            return -1; //check if slot is empty
        }

        if(table[idx] != nullptr && table[idx]->getCountryCode() == countryCode)
        {
            return idx; //return the index when find the input country code
        }

    }
    return -1;
}

int Node::insertCountry(TimeSeries* newCountry) //insert country into the hash table //used this insert country from chatgpt in project3
{
    std::string countryCode = newCountry->getCountryCode();

    int h1 = primaryHash(countryCode);
    int h2 = secondaryHash(countryCode);


    int slotIndex = -1; //initalize a number that will never gonna be the index

    for (int i = 0; i < TABLE_SIZE; i++)
    {
        int idx = (h1 + i * h2) % TABLE_SIZE;
        if (table[idx] == nullptr)
        {
            if (slotOccupied[idx])
            { // Record first available slot.
                if (slotIndex == -1)
                slotIndex = idx;
            }
            else
            {
                // Found an empty slot, if a occupied slot has seen earlier, then use it.
                if(slotIndex != -1) //when the slot is empty
                {
                    table[slotIndex] = newCountry; //insert the slot in the hash table with the newcountry
                    slotOccupied[slotIndex] = false;
                    countryCount++;
                    return slotIndex;
                }
                else
                {
                    table[idx] = newCountry;
                    slotOccupied[idx] = false;
                    countryCount++;
                    return idx;
                }
            }
        }

        else if(table[idx]->getCountryCode() == countryCode)
        {
            return idx; //if the country is already exist
        }
    }

    if(slotIndex != -1)
    {
        table[slotIndex] = newCountry;
        slotOccupied[slotIndex] = false;
        countryCount++;
        return slotIndex;
    }

    return -1;  // Table is full.
}

//learnt from chatgpt, used it for extract all occupied country entires from the hash table into a managemable form
//This function just for making clear which slots can I use for later on
int Node::getAllCountries(TimeSeries* countries[], int max)
{
    int count = 0;
    for(int i = 0; i < TABLE_SIZE; i++) //iteratese all slots in the table
    {
        if (table[i] != nullptr) 
        {
            countries[count++] = table[i]; //stores the pointer in the countries array and increment the counter

            if (count >= max) //iterate until exceed 512 countries
            {
                break;
            }
        }
    }
    return count; //return the num of country found
}

//Helper function for the FIND function (P3)
// Node::getMeanForCountry - Looks up the country in our fixed-size array and
// computes the mean for the currently built series (using currentSeriesCode).
double Node::getMeanForCountry(const std::string &countryName)
{
    for (int i = 0; i < TABLE_SIZE; i++) // loop through every slots in the hash table
    {
        if (table[i] && table[i]->getCountryName() == countryName)    // if found the country; now search its series list.
        {
            TimeSeries* s = table[i]->seriesNext;

            while(s)    //traverse the series to find the series that matches with the current seris code
            {
                if (s->getSeriesCode() == currentSeriesCode)
                {
                    double sum = 0;
                    for (int j = 0; j < s->size; j++)
                    {
                        sum+=s->data[j];    //if found, compute the sum of all data points
                    }
                    return (s->size > 0) ? (sum / s->size) : 0.0;   //return the mean value of all these data points, otherwise, return 0.0
                }
                s=s->seriesNext;    //move to the next series in the linked list
            }
            return 0.0; // If the country is found but no matching series code is found
        }
    }
    return 0.0; //when country not found in the hash table
}

// findMatching function: used this from chatgpt
//std::vector STL source from:https://stackoverflow.com/questions/10750057/how-do-i-print-out-the-contents-of-a-vector
void Node::findMatchingHelper(TreeNode* node,double threshold,const std::string &relation, std::vector<std::string> &results)
{
    if(!node)
    {
        return; // Base case: if the node is null, return immediately.
    }
    for(int i =0;i < node->countryCount; i++)           // Iterate through each country stored in this tree node.
    {
        std::string cname = node->countries[i];       // Get the current country's name.

        double meanVal = getMeanForCountry(cname);    // Compute the mean value for this country (using the currentSeriesCode).
        bool match = false;                           // Initialize a flag indicating whether the country meets the condition.

        //checking the relationship condition
        if (relation == "greater" && meanVal > threshold)  // If relation is "greater", check if the country's mean is above the threshold.
        {
            match = true;
        }
        else if (relation == "less" && meanVal<threshold) // If relation is "less", check if the country's mean is below the threshold.
        {
            match=true;
        }
        //source of cmaths STL: https://stackoverflow.com/questions/76282274/should-i-use-std-when-calling-cmath-functions
        else if (relation == "equal" && std::fabs(meanVal - threshold)<1e-3) // If relation is "equal", check if the country's mean is within a tolerance (1e-3) of the threshold.
        {
            match = true;
        }

        if (match)           // If the condition is met, add the country to the results vector if it's not already present.
        {
            bool exists = false;
            for (const auto &x: results)   // Check for duplicates in results. this part using chatgpt
            {
                if (x == cname)
                {
                    exists = true;
                    break;
                }
            }
            if (!exists) results.push_back(cname); // If not already in results, add the country name.
        }
    }

    findMatchingHelper(node->left, threshold, relation, results);
    findMatchingHelper(node->right, threshold, relation, results);
}

// Public function that returns a vector of country names that satisfy the specified condition
// by calling the helper function starting at the root of the binary tree. //got idea from chatgpt
//STL source from: https://en.cppreference.com/w/cpp/container/vector
std::vector<std::string> Node::findMatching(double threshold, const std::string &relation)
{
    std::vector<std::string> matches;        // Create an empty vector to hold matching country names.
    findMatchingHelper(tree.root, threshold, relation, matches); // Recursively populate the vector.
    return matches;                                  // Return the list of matching countries.
}

//Modified function for project5
// ----- Private silent buildInternal ----- 
//helper function for build
// This function builds the binary tree for the given series code without printing any output, it computes the mean values for all countries stored in the hash table, determines the global minimum and maximum means, and then builds the tree by recursively partitioning the data.
void Node::buildInternal(const std::string &seriesCode)
{
    TimeSeries* arr[512];   // Create an array, holding pointers to all occupied countries from the hash table
    int n=getAllCountries(arr,512);
    std::string cNames[512];    //array store country names

    double means[512];
    int c=0;        //to count number of data that compute a mean

    for(int i=0;i<n;i++)
    {
        double sum=0;int cnt=0;
        TimeSeries* s=arr[i]->seriesNext;   //get the pointer to the linked list of the timeseries of this country

        while(s)    //traverse of the linked list of series data of this country
        {
            if(s->getSeriesCode() == seriesCode)    //check if the series code matches  
            {
                for(int j = 0;j < s->size; j++)     //if match then iterates it's data points
                {
                    sum+=s->data[j];    //sum up the datas
                    cnt++;      //simultaneously, count the num of data 
                }
                break;
            }
            s = s->seriesNext;  //move to the next series of this linked list
        }

        double m = (cnt>0)?(sum/cnt):0.0;
        cNames[c] = arr[i]->getCountryName();
        means[c] = m;
        c++;                    //increase the amount of the proceeded country
    }

    double gMin=1e9,gMax=-1e9;  //initialize the gloval min with extremely large, gloval max with extremely small
    for(int i=0;i<c;i++)        //iterate all means to find global max and min
    {
        if(means[i] < gMin)
        {
            gMin = means[i];
        }
        if(means[i] > gMax)
        {
            gMax = means[i];
        }
    }

    currentSeriesCode = seriesCode; //update the current series code for later

    if(tree.root)
    {
        tree.deleteTree(tree.root);
        tree.root = nullptr;
    }
    if(c > 0)
    {
        tree.root = buildSubtree(cNames,means,0,c-1,gMin,gMax);
    }
}

// ----- Public build: calls buildInternal + prints success -----
void Node::build(const std::string &seriesCode)
{
    buildInternal(seriesCode);
    std::cout << "success" << std::endl;
}

//Helper function for building the tree (from P3)

//Starting of the BUILD function:
bool Node::allSameMean(double* means, int start, int end, double tolerance) // This is a helper function checking if all means in means[] are within tolerance of the first
{
    for(int i = start + 1; i <= end; i++)
    {
        double difference = means[i] - means[start]; //to compare the mean[i] to mean[start]

        if(difference < 0)
        {
            difference = -difference; //convert the negative difference to positive
        }

        if(difference > tolerance) //if exceed tolarance, it shows that they are not same, then different mean
        {
            return false;
        }
    }
    return true; //if no difference exceed tolerance
}

// A helper function that recursively builds a subtree from arrays of countryNames and means. //use some idea from chatgpt in project3
TreeNode* Node::buildSubtree(std::string* countryNames, double* means, int start, int end, double globalMin, double globalMax)
{

    //if there's only one countries or they all having same mean
    if(start == end || allSameMean(means, start, end)) 
    {
        TreeNode* leaf = new TreeNode(means[start], globalMin, globalMax, 0); // Create a leaf node with the mean value of the first element, and set the range to the global values.

        int total = end - start + 1; //determine the total number of countries in this subset
        leaf->countryCount = total;

        for(int i = 0; i < total; i++)
        {
            leaf->countries[i] = countryNames[start + i]; //put the country name from the input array into the leaf's fixed size array
        }
        return leaf;
    }

    // otherwise, find the midpoint to do partition
    double midPoint = globalMin + (globalMax - globalMin) / 2.0;

    // partition the subset into left and right
    std::string leftCountries[512], rightCountries[512];

    //fized size for the left and right leaf
    double leftMeans[512], rightMeans[512];

    int leftCount = 0, rightCount = 0;


    for(int i = start; i <= end; i++) //iterates the subset and distrubute the country to left and right
    {
        if(means[i] < midPoint) //less than midpoint, put to left
        {
            leftCountries[leftCount] = countryNames[i];
            leftMeans[leftCount] = means[i];
            leftCount++;
        }
        else //greater than, right
        {
            rightCountries[rightCount] = countryNames[i];
            rightMeans[rightCount] = means[i];
            rightCount++;
        }
    }

    //for the left tree: to find the min and max mean
    double leftMin = 1e9, leftMax = -1e9; //start with initializing the left min biggest and max the smallest for later replacing

    for(int i = 0; i < leftCount; i++) 
    {
        if(leftMeans[i] < leftMin)
        {
            leftMin = leftMeans[i]; //replace the left min with the new left min
        }

        if(leftMeans[i] > leftMax)
        {
            leftMax = leftMeans[i]; //replace the left max with the new left max
        }
    }

    //for the right tree: to find the min and max mean
    double rightMin = 1e9, rightMax = -1e9; //start with initializing the right min with a big num and right max with a small num

    for(int i = 0; i < rightCount; i++)
    {
        if(rightMeans[i] < rightMin)
        {
            rightMin = rightMeans[i];
        }

        if(rightMeans[i] > rightMax)
        {
            rightMax = rightMeans[i];
        }
    }

    TreeNode* leftChild = nullptr; // Recursively build left subtree, (if there's country in the left
    if(leftCount > 0)
    {
        leftChild = buildSubtree(leftCountries, leftMeans, 0, leftCount - 1, leftMin, leftMax);
    }

    TreeNode* rightChild = nullptr; //recursively build right subtree, if there's country in the right

    if(rightCount > 0)
    {
        rightChild = buildSubtree(rightCountries, rightMeans, 0, rightCount - 1, rightMin, rightMax);
    }

    TreeNode* parent = new TreeNode(globalMax, globalMin, globalMax, 0); // Create the parent node to stores all countries in this given range
    
    int total = end - start + 1;
    parent->countryCount = total;
    
    for(int i = 0; i < total; i++)
    {
        parent->countries[i] = countryNames[start + i]; // Copy all country names from the original array (for this range) into the parent's array.
    }

    parent->left = leftChild; //attach left tree to the partent
    parent->right = rightChild; //attach right tree to the partent

    return parent; //return the parent, which is the tree of this range

}


// A helper function that recursively searches the binary tree for countries whose actual mean satisfies the given condition.
//for FIND function (P3)
void Node::findHelper(TreeNode* node, double targetMean, const std::string &operation, std::string* results, int &count) //idea of this function from chatgpt
{
    if(!node)
    {
        return; //if there's no node then exit the function
    }

    for(int i = 0; i < node->countryCount; i++) //iterate through all countries stored in the current tree node
    {
        std::string countryName = node->countries[i]; //get current country's name form the node
        double countryMean = getMeanForCountry(countryName); //get the mean value for the current country using this helper function

        bool match = false; //create this bool match to check if the function's mean satisfy the condition

        if(operation == "less") //check the operation type and compare the country's mean to the target mean value
        {
            if(countryMean < targetMean) //matches
            {
                match = true;
            }
        }

        else if(operation == "greater") //check the operation type and compare the country's mean to the target mean value
        {
            if(countryMean > targetMean) //matches
            {
                match = true;
            }
        }

        else if(operation == "equal") //check the operation type and compare the country's mean to the target mean value
        {
            double difference = countryMean - targetMean;
            if(difference < 0)
            {
                difference = -difference; //conver the differnece between  current country's mean and the target mean to positive
            }
            if(difference < 1e-3) //if the difference less than the tolerance
            {
                match = true;  //matches
            }
        }

        if(match)//country's mean matches with the input mean: match is true, then add it to the result
        {
            bool alreadyInside = false; //used this idea from chatgpt, shows that the country already in the result to avoid duplicate
            for(int j = 0; j < count; j++)
            {
                if(results[j] == countryName)
                {
                    alreadyInside = true;
                    break; //if found, exit the loop
                }
            }
            if(!alreadyInside && count < 512) //if country not in the result yet, then add it in if there's still space
            {
                results[count++] = countryName;
            }
        }
    }

    findHelper(node->left, targetMean, operation, results, count); //recursively search the left subtree
    findHelper(node->right, targetMean, operation, results, count); //recursively search the right subtree

}



// LOAD: Read the file and insert each country using hashing.
void Node::load(std::string filename)
{
    std::ifstream file(filename);
    if(!file)
    {
        std::cout << "failure" << std::endl; //file cannot be opened
        return;
    }

    std::string line;

    while(std::getline(file, line)) //read the file line by line
    {
        std::stringstream ss(line);
        std::string countryName, countryCode, seriesName, seriesCode;

        std::getline(ss, countryName, ',');
        std::getline(ss, countryCode, ',');
        std::getline(ss, seriesName, ',');
        std::getline(ss, seriesCode, ',');


        int steps;
        int idx = findCountryIndex(countryCode, steps); //return the index of the country to where they were find, and give the updated steps value in the find country index function to the new int steps variable here

        if(idx == -1)
        {
            TimeSeries* newCountry = new TimeSeries();
            newCountry->setCountryName(countryName);
            newCountry->setCountryCode(countryCode);
            newCountry->seriesNext = nullptr;

            int insertedIndex = insertCountry(newCountry); //used insert country helper function to determine where to insert in the hash table.

            if(insertedIndex == -1)
            {
                continue;  //loop will move to the next line
            }

            idx = insertedIndex;
        }

        TimeSeries* newSeries = new TimeSeries();

        newSeries->setSeriesName(seriesName);
        newSeries->setSeriesCode(seriesCode);
        newSeries->size = 0;
        newSeries->seriesNext = nullptr;

        TimeSeries* head = table[idx]->seriesNext;

        if(head == nullptr)
        {
            table[idx]->seriesNext = newSeries;
        }

        else
        {
            while(head->seriesNext)
            {
                head = head->seriesNext;
            }

            head->seriesNext = newSeries;
        }

        int yearValue = 1960;
        std::string value;


        while(std::getline(ss, value, ','))
        {
            try
            {
                double num = std::stod(value);
                if(num >= 0)
                {
                    newSeries->year[newSeries->size] = yearValue;
                    newSeries->data[newSeries->size] = num;
                    newSeries->size++;
                }
            }
            catch(std::invalid_argument&)
            {
                // ignore invalid data.
            }

            yearValue++;
        }
    }

    file.close();
    std::cout << "success" << std::endl;

}


// LIST: Given a country name, search through the hash table and print its details.
void Node::list(std::string country)
{
    for(int i = 0; i < TABLE_SIZE; i++) //iterates through the hash table
    {
        if(table[i] != nullptr)
        {
            if(table[i]->getCountryName() == country) //if found matching country
            {
                std::cout << table[i]->getCountryName() << " " << table[i]->getCountryCode();
                TimeSeries* series = table[i]->seriesNext;
                
                while(series)
                {
                    std::cout << " " << series->getSeriesName();
                    series = series->seriesNext;
                }

                std::cout << std::endl;
                return;
            }
        }
    }
}

// RANGE: Compute the min and max mean (for a given series code) across all countries.
void Node::range(const std::string &seriesCode)
{
    TimeSeries* countries[512];
    int n = getAllCountries(countries, 512);

    if(n == 0)
    {
        std::cout << "failure" << std::endl;
        return;
    }

    double minMean = 1e9, maxMean = -1e9;
    bool found = false;
    for (int i = 0; i < n; i++)
    {
        TimeSeries* cur = countries[i];
        TimeSeries* s = cur->seriesNext;
        double sum = 0.0;
        int countNum = 0;
        while(s)
        {
            if(s->getSeriesCode() == seriesCode)
            {
                for(int j = 0; j < s->size; j++)
                {
                    sum += s->data[j];
                    countNum++;
                }
                break;
            }
            
            s = s->seriesNext;
        }

        if(countNum > 0)
        {
            double mean = sum / countNum;
            if(mean < minMean)
            {
                minMean = mean;
            }
            if(mean > maxMean)
            {
                maxMean = mean;
            }
            found = true;
        }
    }

    if(!found)
    {
        std::cout << "failure" << std::endl;
    }
    else
    {
        std::cout << minMean << " " << maxMean << std::endl;
    }
}

// FIND: Using the binary tree, find all countries whose mean (for the current series) meets the given condition.
void Node::find (double meanVal,const std::string &op)
{
    if (!tree.root)  //if the tree hasn't been built before, return failue then exit
    {
        std::cout << "failure" << std::endl;return;
    } 

    std::string results[512];   //array to store name of the matching countries
    int c = 0;    //initialize the count of the num of matching countries to zero
    findHelper (tree.root,meanVal,op,results,c); //use the given mean and operation to recursively search the tree starting from the root
    
    
    if (c==0)    //if no matching country found
    {
        std::cout << std::endl; 
    }

    else
    {
        for (int i = 0;i < c; i++) //otherwise, if found, print all the matching country names
        {
            std::cout << results[i];
            if(i<c-1) std::cout << " ";
        }

        std::cout<<std::endl;
    }
}

// DELETE by country name (for backward compatibility).
void Node::deleteCountryByName(const std::string &cName)
{
    for (int i = 0; i<TABLE_SIZE; i++)
    {
        if (table[i] && table[i]->getCountryName()==cName) //if slot occupied and the country name matches
        {
            std::string code=table[i]->getCountryCode();//get that country code from hash table
            remove(code);   //call the remove funciton with this country code
            return; //exist the function after removing country
        }
    }

    std::cout << "failure" << std::endl;    //if no matching country found
}

// LIMITS: Traverse the binary tree to print countries at the lowest or highest leaf.
void Node::limits(const std::string &condition)
{
    if(!tree.root) //binary tree hasn't build before
    {
        std::cout << "failure" << std::endl;
        return;
    }

    TreeNode* current = tree.root; //start the traversal from the root of the tree

    if(condition == "lowest") // For "lowest", follow the left child pointers to reach the leftmost leaf.
    {
        while(current->left != nullptr)
        {
            current = current->left;
        }
    }

    else if(condition == "highest") // For "highest", follow the right child pointers to reach the rightmost leaf.
    {
        while(current->right != nullptr)
            current = current->right;
    }

    else
    {
        std::cout << "failure" << std::endl; //any invalid conditions
        return;
    }

    for(int i = 0; i < current->countryCount; i++)
    {
        std::cout << current->countries[i];
        if(i < current->countryCount - 1)
        {
            std::cout << " ";            
        }
    }

    std::cout << std::endl;
}


// Project4 functions

void Node::lookup (const std::string &cCode)
{
    int steps;
    int idx = findCountryIndex(cCode,steps);
    if (idx == -1) std::cout<< "failure" <<std::endl;
    else std::cout << "index " << idx << " searches " << steps << std::endl;
}

// REMOVE: remove a country by given countryCode from both hash table and tree, and then free its memory.
void Node::remove (const std::string &cCode)
{
    int steps;
    int idx=findCountryIndex(cCode,steps);

    if(idx == -1) //country wasn't found in the hash table
    {
        std::cout << "failure" << std::endl; //do nothing if there's no country
    }
    else
    {
        std::string cName = table[idx]->getCountryName();   //gets the country name
        if(tree.root)   //check if the binary tree exist, check tree.root is not nullptr
        {
            bool found=false;
            tree.root=deleteCountryHelper(tree.root,cName,found);   //remove the country from the tree
        }
        TimeSeries* s=table[idx]->seriesNext;

        while(s)    //iterates though the timeseries
        {
            TimeSeries* tmp = s;
            s=s->seriesNext;
            delete tmp; // free the memory for that timeseries
        }

        delete table[idx];  
        table[idx] = nullptr;   //show the slot is empty now
        slotOccupied[idx]=true; //this flag tells the hash table that this slot was once used, ensuring that during future lookups the probing doesn’t stop prematurely
        countryCount--;         //count of countries -1

        std::cout << "success" << std::endl;
    }
}

// INSERT: Given a country code and filename, find the country in the file and insert it into the hash table.
void Node::insert(const std::string &cCode, const std::string &filename)
{
    int steps;  //to count number of hash probes
    int idx=findCountryIndex(cCode,steps);  //return the index of the country to where they were find, and give the updated steps value in the find country index function to the new int steps variable here
    
    if(idx != -1)
    {
        std::cout << "failure" << std::endl;return;
    }

    std::ifstream file(filename);   //open the input file
    if(!file)
    { 
        std::cout << "failure" << std::endl;return;
    }

    std::string line;
    bool foundLine = false;   //having a boolean flag here to false, make it to true after finding the matches countryCode.

    while(std::getline(file,line))  //loop through the file
    {
        std::stringstream ss(line); //for parsing
        std::string cn,cc,sn,sc;

        std::getline(ss,cn,',');
        std::getline(ss,cc,',');
        std::getline(ss,sn,',');
        std::getline(ss,sc,',');

        if(cc==cCode)       //find matches countryCode
        {
            foundLine = true; //mark this flag as true
            TimeSeries* newCountry = new TimeSeries();

            newCountry->setCountryName(cn); //set the series name
            newCountry->setCountryCode(cc); //set the series code
            newCountry->seriesNext = nullptr;

            TimeSeries* newS = new TimeSeries();
            newS->setSeriesName(sn);
            newS->setSeriesCode(sc);

            newS->size = 0;
            newS->seriesNext = nullptr;
            newCountry->seriesNext = newS;


            int yVal = 1960;
            std::string val;

            while(std::getline(ss,val,','))
            {
                try //used this try and catch from chatgpt
                {
                    double num = std::stod(val);  //using this std::stod to convert this std::string value to a double data type

                    if(num>=0)
                    {
                        newS->year[newS->size] = yVal;
                        newS->data[newS->size] = num;
                        newS->size++;
                    }
                }
                catch(...)  // used this std::invalid_argument& from chatgpt, for when the std::stod receiveds argument that it cannot convert or process properly, by catching it with '&', it can aviod copying the exception object and can optionally inspect its detail//use this try and catch for: if std::stod fails, for example, when the value is invalud number, the excepting is cought and the data will be ignored. 
                {
                    //ignore valid data
                }
                yVal++;
            }

            int ins = insertCountry(newCountry);  //means hash table is full, no available slots.

            if(ins == -1)
            {
                std::cout << "failure" << std::endl;
            }
            else
            {
                std::cout << "success" << std::endl;
            }

            break;
        }
    }
    
    file.close();

    if(!foundLine)
    {
        std::cout << "failure" << std::endl; //means no matching found
    }
}
