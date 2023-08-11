//
//  main.cpp
//  AVLtree
//
//  Created by kevin on 2023/8/11.
//
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <algorithm>

struct SchoolInfo {
    int serialNumber;
    std::string schoolName;
    std::string departmentName;
    std::string dayNight;
    std::string level;
    int studentCount;
};

class AVLNode {
public:
    std::string departmentName;
    std::vector<SchoolInfo> data;
    AVLNode* left;
    AVLNode* right;
    int height;

    AVLNode(const SchoolInfo& info) : departmentName(info.departmentName), left(nullptr), right(nullptr), height(1) {
        data.push_back(info);
    }
};

class AVLTree {
private:
    AVLNode* root;

    int getHeight(AVLNode* node) {
        return node ? node->height : 0;
    }

    int getBalance(AVLNode* node) {
        return node ? getHeight(node->left) - getHeight(node->right) : 0;
    }

    AVLNode* rightRotate(AVLNode* y) {
        AVLNode* x = y->left;
        AVLNode* T3 = x->right;

        x->right = y;
        y->left = T3;

        y->height = std::max(getHeight(y->left), getHeight(y->right)) + 1;
        x->height = std::max(getHeight(x->left), getHeight(x->right)) + 1;

        return x;
    }

    AVLNode* leftRotate(AVLNode* x) {
        AVLNode* y = x->right;
        AVLNode* T2 = y->left;

        y->left = x;
        x->right = T2;

        x->height = std::max(getHeight(x->left), getHeight(x->right)) + 1;
        y->height = std::max(getHeight(y->left), getHeight(y->right)) + 1;

        return y;
    }

    AVLNode* insert(AVLNode* node, const SchoolInfo& info) {
        if (!node) return new AVLNode(info);

        if (info.departmentName < node->departmentName) {
            node->left = insert(node->left, info);
        } else if (info.departmentName > node->departmentName) {
            node->right = insert(node->right, info);
        } else {
            node->data.push_back(info);
            return node;
        }

        node->height = 1 + std::max(getHeight(node->left), getHeight(node->right));

        int balance = getBalance(node);

        if (balance > 1 && info.departmentName < node->left->departmentName) {
            return rightRotate(node);
        }

        if (balance < -1 && info.departmentName > node->right->departmentName) {
            return leftRotate(node);
        }

        if (balance > 1 && info.departmentName > node->left->departmentName) {
            node->left = leftRotate(node->left);
            return rightRotate(node);
        }

        if (balance < -1 && info.departmentName < node->right->departmentName) {
            node->right = rightRotate(node->right);
            return leftRotate(node);
        }

        return node;
    }

public:
    AVLTree() : root(nullptr) {}

    void insert(const SchoolInfo& info) {
        root = insert(root, info);
    }

    int getHeight() {
        return getHeight(root);
    }

    std::vector<SchoolInfo> getRootData() {
        return root ? root->data : std::vector<SchoolInfo>();
    }
};

std::vector<SchoolInfo> readData() {
    std::vector<SchoolInfo> data;
    std::string line;
    int serialNumber = 0;

    std::cout << "請輸入資料檔的路徑：" << std::endl;
    std::string filePath;
    std::cin >> filePath;

    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "無法打開檔案：" << filePath << std::endl;
        return data;
    }

    for (int i = 0; i < 3; ++i) {
        std::getline(file, line);
    }

    while (std::getline(file, line)) {
        std::stringstream ss(line);
        SchoolInfo info;
        std::string temp;

        ++serialNumber;
        info.serialNumber = serialNumber;

        std::getline(ss, temp, '\t');
        std::getline(ss, info.schoolName, '\t');
        std::getline(ss, temp, '\t');
        std::getline(ss, info.departmentName, '\t');
        std::getline(ss, info.dayNight, '\t');
        std::getline(ss, info.level, '\t');
        std::getline(ss, temp, '\t');
        info.studentCount = std::stoi(temp);

        data.push_back(info);
    }

    file.close();
    return data;
}

int main() {
    std::vector<SchoolInfo> data = readData();
    AVLTree tree;

    for (const auto& info : data) {
        tree.insert(info);
    }

    std::vector<SchoolInfo> rootData = tree.getRootData();
    std::sort(rootData.begin(), rootData.end(), [](const SchoolInfo& a, const SchoolInfo& b) {
        return a.serialNumber < b.serialNumber;
    });

    std::cout << "Tree height = " << tree.getHeight() << std::endl;
    int counter = 1;
    for (const auto& info : rootData) {
        std::cout << counter++ << ": [" << info.serialNumber << "] " << info.schoolName << ", " << info.departmentName << ", " << info.dayNight << " " << info.level << ", " << info.studentCount << std::endl;
    }

    return 0;
}





