//
//  main.cpp
//  23tree
//
//  Created by kevin on 2023/8/10.
//

// g++ -std=c++11
#include <iostream>
#include <vector>
#include <string>
#include <stack> // push, pop, top
#include <sstream>
#include <fstream>
#define PTR_NUM 3
#define KEY_NUM PTR_NUM - 1
using namespace std;

//********************************************************************************************/

class GraduateInfo {
public:
    int serialNumber; // Unique serial number for each record
    std::string schoolCode;
    std::string schoolName;
    std::string departmentCode;
    std::string departmentName;
    std::string dayNight;
    std::string level;
    std::string studentCount;
    std::string teacherCount;
    std::string lastYearGraduates;
    std::string cityName;
    std::string systemType;

    GraduateInfo() {}

    GraduateInfo(int serial, const std::string &line) {
        serialNumber = serial;
        std::stringstream ss(line);
        std::string token;

        // Parsing the line based on the given format
        getline(ss, schoolCode, '\t');
        getline(ss, schoolName, '\t');
        getline(ss, departmentCode, '\t');
        getline(ss, departmentName, '\t');
        getline(ss, dayNight, '\t');
        getline(ss, level, '\t');
        getline(ss, studentCount, '\t');
        getline(ss, teacherCount, '\t');
        getline(ss, lastYearGraduates, '\t');
        getline(ss, cityName, '\t');
        getline(ss, systemType, '\t');
    }
};


typedef struct slotT // a slot in a tree node
{
    std::vector<int> rSet; // a set of record identifiers with the same key
    std::string key; // a key for comparisons
} slotType;

typedef struct nT // a tree node of a 23 tree
{
    slotType data[KEY_NUM]; // a list of records sorted by keys
    struct nT *link[PTR_NUM]; // a list of pointers
    struct nT *parent; // a pointer to the parent node
} nodeType;

typedef struct pointT // a point on the search path
{
    nodeType *pnode; // pointer to a parent node
    int pidx; // entrance index on the parent node
} pointType;

typedef struct bT // a data block received from a split
{
    slotType slot; // a pair of (record id, key)
    nodeType *link; // a pointer to a child on the right
} blockType;

nodeType *createNode(nodeType *pNode, nodeType *left, nodeType *right, slotType newS) // create a node with one record inserted
{
    nodeType *newNode = NULL;
    try
    {
        newNode = new nodeType; // create a new node
        newNode->data[0].rSet = newS.rSet; // put the record into the 1st slot
        newNode->data[1].rSet.clear();
        newNode->data[0].key = newS.key;
        newNode->data[1].key = "";
        newNode->parent = pNode; // set up a link to the parent
        newNode->link[0] = left; // set up the leftmost link
        newNode->link[1] = right; // set up the middle link
        newNode->link[2] = NULL; // clear up the rightmost link
    }
    catch (std::bad_alloc &ba) // unable to allocate space
    {
        std::cerr << "\nbad_alloc caught: " << ba.what() << std::endl;
    }
    return newNode; // pass a pointer to the new-created node
} // end createNode

void splitLeaf(pointType aLeaf, slotType newS, blockType &aBlock)
{
    std::cout << "Entering splitLeaf for key: " << newS.key << std::endl;
    slotType buf[PTR_NUM];
    int idx = 0;
    for (int i = 0; i < PTR_NUM; i++)
    {
        buf[i].rSet = (i == aLeaf.pidx) ? newS.rSet : aLeaf.pnode->data[idx].rSet;
        buf[i].key = (i == aLeaf.pidx) ? newS.key : aLeaf.pnode->data[idx++].key;
    }
    aLeaf.pnode->data[0].rSet = buf[0].rSet;
    aLeaf.pnode->data[0].key = buf[0].key;
    for (int i = 1; i < KEY_NUM; i++)
        aLeaf.pnode->data[i].rSet.clear();
    aBlock.link = createNode(aLeaf.pnode->parent, NULL, NULL, buf[2]);  // 修改這裡
    aLeaf.pnode->parent = aBlock.link;  // 新增這一行
    aBlock.slot.rSet = buf[1].rSet;
    aBlock.slot.key = buf[1].key;
    std::cout << "Exiting splitLeaf for key: " << newS.key << std::endl;
}


nodeType *createRoot(nodeType *left, nodeType *right, slotType oneSlot)
{
    nodeType *newRoot = createNode(left, right, NULL, oneSlot);
    left->parent = newRoot;
    right->parent = newRoot;
    return newRoot;
}

void insertNonleaf(const blockType oneB, pointType goal)
{
    for (int i = KEY_NUM; i > goal.pidx; i--)
    {
        goal.pnode->data[i] = goal.pnode->data[i-1];
        goal.pnode->link[i+1] = goal.pnode->link[i];
    }
    goal.pnode->data[goal.pidx] = oneB.slot;
    goal.pnode->link[goal.pidx+1] = oneB.link;
}

void splitNonleaf(pointType goal, blockType &oneB)
{
    std::cout << "Entering splitNonleaf for key: " << oneB.slot.key << std::endl;
    
    slotType buf[PTR_NUM];
    nodeType *ptr[PTR_NUM + 1];
    int idx = 0;
    
    for (int i = 0; i < PTR_NUM; i++)
    {
        buf[i].rSet = (i == goal.pidx) ? oneB.slot.rSet : goal.pnode->data[idx].rSet;
        buf[i].key = (i == goal.pidx) ? oneB.slot.key : goal.pnode->data[idx++].key;
    }
    std::cout << "Finished initializing buf array." << std::endl;

    ptr[0] = goal.pnode->link[0];
    for (int i = 1; i <= PTR_NUM; i++)
        ptr[i] = (i-1 == goal.pidx) ? oneB.link : goal.pnode->link[i-1];
    std::cout << "Finished initializing ptr array." << std::endl;

    goal.pnode->data[0] = buf[0];
    goal.pnode->link[0] = ptr[0];
    for (int i = 1; i < PTR_NUM; i++)
    {
        std::cout << "Inside loop, iteration: " << i << std::endl;
        goal.pnode->data[i].rSet.clear();
        std::cout << "Cleared rSet for iteration: " << i << std::endl;
        goal.pnode->link[i] = ptr[i];
        std::cout << "Updated link for iteration: " << i << std::endl;
    }
    std::cout << "Finished updating goal.pnode data and links." << std::endl;

    oneB.slot = buf[1];
    oneB.link = createNode(goal.pnode, ptr[2], ptr[3], buf[2]);

    // Checking for nullptr before assigning parent to avoid EXC_BAD_ACCESS
    if (ptr[2] != nullptr) {
        ptr[2]->parent = oneB.link;
    }
    if (ptr[3] != nullptr) {
        ptr[3]->parent = oneB.link;
    }
    
    std::cout << "Finished updating oneB slot and link." << std::endl;

    std::cout << "Exiting splitNonleaf for key: " << oneB.slot.key << std::endl;
}



void free23tree(nodeType *root)
{
    if (root != NULL)
    {
        free23tree(root->link[0]);
        for (int i = 0; i < KEY_NUM; i++)
        {
            if (!root->data[i].rSet.size())
                break;
            free23tree(root->link[i+1]);
        }
        delete root;
    }
}

void searchPath(nodeType *cur, const string &name, stack<pointType> &path)
{
    pointType oneP;
    int pos;
    while (cur != NULL)
    {
        oneP.pnode = cur;
        for (pos = 0; pos < KEY_NUM; pos++)
        {
            if (!cur->data[pos].rSet.size() || name.compare(cur->data[pos].key) < 0)
                break;
            else if (!name.compare(cur->data[pos].key))
            {
                oneP.pidx = pos;
                path.push(oneP);
                return;
            }
        }
        oneP.pidx = pos;
        path.push(oneP);
        cur = cur->link[pos];
    }
}

void insertLeaf(slotType newS, pointType aLeaf)
{
    for (int i = KEY_NUM - 1; i >= aLeaf.pidx; i--)
    {
        if (i > aLeaf.pidx)
        {
            aLeaf.pnode->data[i] = aLeaf.pnode->data[i-1];
        }
        else if (i == aLeaf.pidx)
        {
            aLeaf.pnode->data[i] = newS;
        }
        else
            break;
    }
}

void insert23tree(nodeType *&root, const string &newKey, int newRid)
{
    std::cout << "Inserting: " << newKey << std::endl;
    slotType newSlot;
    newSlot.rSet.push_back(newRid);
    newSlot.key = newKey;
    if (root == NULL)
        root = createNode(NULL, NULL, NULL, newSlot);
    else
    {
        stack<pointType> aPath;
        pointType curP;
        blockType blockUp;
        searchPath(root, newKey, aPath);
        if (!aPath.empty())
        {
            curP = aPath.top();
            if (curP.pnode->data[curP.pidx].rSet.size() && !newKey.compare(curP.pnode->data[curP.pidx].key))
                curP.pnode->data[curP.pidx].rSet.push_back(newRid);
            else if (!curP.pnode->data[KEY_NUM - 1].rSet.size())
                insertLeaf(newSlot, curP);
            else
            {
                splitLeaf(curP, newSlot, blockUp);
                if (curP.pnode->parent == NULL)
                    root = createRoot(curP.pnode, blockUp.link, blockUp.slot);
                else
                {
                    do
                    {
                        std::cout << "Inside do-while loop for: " << newKey << std::endl;
                        aPath.pop();

                        if (aPath.empty())
                        {
                            root = createRoot(curP.pnode, blockUp.link, blockUp.slot);
                            break;
                        }

                        // Check if aPath is not empty before accessing its top
                        if (!aPath.empty())
                        {
                            curP = aPath.top();
                        }
                        else
                        {
                            std::cerr << "Error: aPath stack is empty!" << std::endl;
                            break;
                        }

                        if (!curP.pnode->data[KEY_NUM - 1].rSet.size())
                        {
                            insertNonleaf(blockUp, curP);
                            break;
                        }
                        else
                        {
                            splitNonleaf(curP, blockUp);
                        }
                    } while (true);
                    std::cout << "Finished inserting: " << newKey << std::endl;
                }
            }
        }
    }
}



int getTreeHeight(nodeType* root) {
    if (root == NULL) {
        return 0;
    }

    int max_height = 0;
    for (int i = 0; i < PTR_NUM; i++) {
        if (root->link[i] != NULL) {
            int height = getTreeHeight(root->link[i]);
            if (height > max_height) {
                max_height = height;
            }
        }
    }

    return max_height + 1; // 加1是因為要加上當前節點的高度
}

class SchoolTree {
private:
    nodeType *root;

public:
    SchoolTree() : root(NULL) {}

    // Insert a new record into the 2-3 tree
    void insert(const GraduateInfo &info) {
        insert23tree(root, info.schoolName, info.serialNumber);
    }

    // Display the height of the tree
    int height() {
        return getTreeHeight(root);
    }

    // Display the data in the root node
    void displayRootData(const std::vector<GraduateInfo> &dataList) {
        if (root == NULL) {
            std::cout << "The tree is empty." << std::endl;
            return;
        }

        for (int i = 0; i < KEY_NUM; i++) {
            if (root->data[i].rSet.size() == 0) {
                break;
            }

            for (int rid : root->data[i].rSet) {
                const GraduateInfo &info = dataList[rid - 1]; // Adjusting for 0-based indexing
                std::cout << info.serialNumber << "\t"
                          << info.schoolName << "\t"
                          << info.departmentName << "\t"
                          << info.dayNight << "\t"
                          << info.level << "\t"
                          << info.studentCount << std::endl;
            }
        }
    }
};

// Next, we'll implement the main function to read the file, build the tree, and display the results.
#include <locale>

int main() {
    std::vector<GraduateInfo> dataList;
    SchoolTree tree;
    std::string filename = "/Users/kevin/Desktop/c++_xcode/23tree/23tree/input104.txt"; // Adjust to your path
    std::ifstream infile(filename);
    
    if (!infile.is_open()) {
        std::cerr << "Failed to open the file." << std::endl;
        return 1;
    }

    std::string line;
    int serial = 1; // Assuming serial numbers start from 1

    // Read the data file line by line
    while (getline(infile, line)) {
        GraduateInfo info(serial, line);
        dataList.push_back(info);
        tree.insert(info);
        serial++;
    }

    infile.close();

    // Display the height of the 2-3 tree
    std::cout << "Height of the tree: " << tree.height() << std::endl;

    // Display the data in the root of the 2-3 tree
    tree.displayRootData(dataList);

    return 0;
}
