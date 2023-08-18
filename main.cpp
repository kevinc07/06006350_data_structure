//
//  main.cpp
//  HW2
//
//  Created by kevin on 2023/8/18.
//
#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <string>

using namespace std;

struct Student {
    char sid[10];
    char sname[10];
    unsigned char score[6];
    float average;
};

struct HashEntry {
    int hvalue;            // 線性探測後的雜湊位址
    int originalHvalue;   // 原始的雜湊值
    Student student;
    bool occupied = false; // Check if the hash entry is occupied
};


// Function to check if a given number is prime
bool isPrime(int n) {
    if (n <= 1) return false;           // If number is 1 or negative, it's not prime
    if (n == 2) return true;            // 2 is prime
    if (n % 2 == 0) return false;       // Any even number (except 2) is not prime
    for (int i = 3; i <= sqrt(n); i += 2) {  // Only check odd factors up to square root of n
        if (n % i == 0) return false;   // If n is divisible by any odd number, it's not prime
    }
    return true;                        // Otherwise, n is prime
}

// Function to find the next prime number greater than a given number
int nextPrime(int n) {
    while (!isPrime(n)) {               // Keep checking until a prime number is found
        n++;
    }
    return n;                           // Return the next prime number
}

// Hash function: generates a hash value from the given key
int hashFunc(const char* key, int tableSize) {
    long long result = 1;
    for (int i = 0; i < 10 && key[i] != '\0'; i++) { // Multiply ASCII values of characters in the key
        result *= key[i];
    }
    return result % tableSize;          // Modulus by table size to fit into the hash table
}


void task0(const string& txtFilename) {
    ifstream txtFile(txtFilename);
    if (!txtFile.is_open()) {
        cout << "Error opening file." << endl;
        return;
    }

    // Create a binary filename from the text filename
    string binFilename = txtFilename.substr(0, txtFilename.size() - 3) + "bin";
    ofstream binFile(binFilename, ios::binary);

    if (!binFile.is_open()) {
        cout << "Error creating binary file." << endl;
        return;
    }

    // Read data from the text file and write it in binary format
    string sid, sname, temp;
    float average;
    vector<unsigned char> scores(6);
    while (txtFile >> sid >> sname) {   // Read each student's data
        for (int i = 0; i < 6; i++) {
            txtFile >> temp;
            scores[i] = stoi(temp);     // Convert score from string to integer
        }
        txtFile >> average;

        // Fill the student struct with the read data
        Student student;
        memset(student.sid, 0, sizeof(student.sid));      // Initialize with zeros
        memset(student.sname, 0, sizeof(student.sname));  // Initialize with zeros
        memcpy(student.sid, sid.c_str(), sid.size() + 1); // Copy SID from string to char array
        memcpy(student.sname, sname.c_str(), sname.size() + 1); // Copy SName from string to char array
        for (int i = 0; i < 6; i++) {
            student.score[i] = scores[i];  // Assign scores to student
        }
        student.average = average;

        // Write the student struct to the binary file
        binFile.write(reinterpret_cast<char*>(&student), sizeof(student));
    }

    // Close the files
    txtFile.close();
    binFile.close();
}

void computeAverageComparisons(const vector<HashEntry>& hashTable) {
    int tableSize = hashTable.size();

    // For existing values
    int totalComparisonsExisting = 0;
    int existingEntriesCount = 0;

    // For non-existing values
    int totalComparisonsNonExisting = 0;
    int trialsForNonExisting = tableSize; // Assume we will search for a non-existing value from each slot

    for (int i = 0; i < tableSize; i++) {
        const HashEntry& entry = hashTable[i];

        if (entry.occupied) {
            int distance = (i - entry.originalHvalue + tableSize) % tableSize;
            totalComparisonsExisting += (distance + 1); // +1 because comparison includes the actual position too
            existingEntriesCount++;
        }
    }

    for (int i = 0; i < tableSize; i++) {
        if (!hashTable[i].occupied) {
            continue;  // Skip if the slot is already empty.
        }

        int nextPos = (i + 1) % tableSize;
        int comparisonsForThisSlot = 1;

        while (hashTable[nextPos].occupied) {
            comparisonsForThisSlot++;
            nextPos = (nextPos + 1) % tableSize;
        }

        totalComparisonsNonExisting += comparisonsForThisSlot;
    }

    double averageExisting = static_cast<double>(totalComparisonsExisting) / existingEntriesCount;
    double averageNonExisting = static_cast<double>(totalComparisonsNonExisting) / trialsForNonExisting;
    
    cout << "unsuccessful search: " << averageNonExisting << " comparisions on average" << endl;
    cout << "successful search: " << averageExisting << " comparisions on average" << endl;
}


void task1(const string& filename) {
    // 從給定的文件名中生成二進制文件名（例如: students.txt -> students.bin）
    string binFilename = filename.substr(0, filename.find_last_of('.')) + ".bin";
    // 嘗試打開二進制文件
    ifstream binFile(binFilename, ios::binary);
    
    // 如果二進制文件不存在或無法打開
    if (!binFile) {
        // 轉換文本文件到二進制文件
        task0(filename);
        // 再次嘗試打開二進制文件
        binFile.open(binFilename, ios::binary);
    }
    // 如果二進制文件仍然不存在或無法打開
    if (!binFile) {
        cerr << "Error opening binary file." << endl;
        return;
    }

    // 跳到二進制文件的末尾以獲取其大小
    binFile.seekg(0, ios::end);
    // 計算學生的數量（文件大小 / Student結構的大小）
    long long studentCount = static_cast<long long>(binFile.tellg()) / sizeof(Student);
    // 將文件指標移回文件的開始
    binFile.seekg(0, ios::beg);

    // 計算雜湊表的大小（學生數量的1.2倍的下一個質數）
    int tableSize = nextPrime(static_cast<int>(1.2 * studentCount));

    // 初始化一個大小為tableSize的空雜湊雜湊表
    vector<HashEntry> hashTable(tableSize);
    
    Student student;
    // 從二進制文件中讀取每一個學生並將其放入雜湊表中
    while (binFile.read(reinterpret_cast<char*>(&student), sizeof(student))) {
        // 使用學生的SID計算原始的雜湊值
        int originalHvalue = hashFunc(student.sid, tableSize);
        int hvalue = originalHvalue; // 設定hvalue為原始的雜湊值
        
        // 如果此位置已被占用，則使用線性探測法尋找下一個可用的位置
        while (hashTable[hvalue].occupied) {
            hvalue = (hvalue + 1) % tableSize;
        }
        
        // 將學生放入雜湊表的適當位置
        hashTable[hvalue].originalHvalue = originalHvalue;
        hashTable[hvalue].hvalue = hvalue;
        hashTable[hvalue].student = student;
        hashTable[hvalue].occupied = true;
    }
    
    // 創建輸出文件名並嘗試打開它
    string outputFilename = filename.substr(0, filename.find_last_of('.')) + "linear.txt";
    ofstream outputFile(outputFilename);
    if (!outputFile) {
        cerr << "Error opening output file." << endl;
        return;
    }
    
    // 寫雜湊表的標題到輸出文件中
    outputFile << " --- Hash Table X --- (linear probing)" << endl;

    // 將雜湊表的內容寫入輸出文件
    for (int i = 0; i < tableSize; i++) {
        const HashEntry& entry = hashTable[i];
        outputFile << "[" << setw(3) << i << "]";
        
        if (entry.occupied) {
            outputFile  << setw(12) << entry.originalHvalue << ","
                        << setw(12) << entry.student.sid << ","
                        << setw(12) << entry.student.sname << ","
                        << setw(12) << entry.student.average;
        }

        outputFile << endl;
    }

    // 關閉文件
    binFile.close();
    outputFile.close();
    // 計算並打印成功和不成功搜索的平均比較次數
    computeAverageComparisons(hashTable);
}



int main() {
    // 請求用戶輸入文件名
    string filename1;
    string filename2;
    //cout << "Enter the filename (either .txt or .bin): ";
    //cin >> filename1;

    //task1(filename1);
    cout << "Enter the filename (either .txt or .bin): ";
    cin >> filename2;

    task2(filename2);
    return 0;
}
