#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <string>
#include <cstring>
#include <iomanip>

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
int nextPrime(double n) {
    int startingPoint = static_cast<int>(ceil(n));  // Round up to the nearest integer
    while (!isPrime(startingPoint)) {
        startingPoint++;
    }
    return startingPoint;  // Return the next prime number
}

// Hash function: generates a hash value from the given key
int hashFunc(const char* key, int tableSize) {
    long long result = 1;
    for (int i = 0; i < 10 && key[i] != '\0'; i++) { // Multiply ASCII values of characters in the key
        result *= key[i];
    }
    return result % tableSize;          // Modulus by table size to fit into the hash table
}


bool task0(const string& txtFilename, const string& binFilename) {
    ifstream txtFile(txtFilename);
    if (!txtFile.is_open()) {
        cout << "### " << binFilename << " does not exist! ### " << endl;
        cout << "### " << txtFilename << " does not exist! ### " << endl;
        return false ;
    }

    // Create a binary filename from the text filename
    string binFilename_new = txtFilename.substr(0, txtFilename.size() - 3) + "bin";
    ofstream binFile(binFilename_new, ios::binary);

    if (!binFile.is_open()) {
        cout << "Error creating binary file." << endl;
        return false ;
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
    
    return true;
}

void computeAverageComparisons(const vector<HashEntry>& hashTable, int studentCount, int tableSize) {

    // For existing values
    int totalComparisonsExisting = 0;
    int existingEntriesCount = 0;

    // For non-existing values
    int totalComparisonsNonExisting = 0;


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

    double averageExisting = static_cast<double>(totalComparisonsExisting) / studentCount;
    double averageNonExisting = static_cast<double>(totalComparisonsNonExisting) / tableSize;
    
    cout << "unsuccessful search: " << averageNonExisting << " comparisions on average" << endl;
    cout << "successful search: " << averageExisting << " comparisions on average" << endl;
}


void task1(const string& inputNumber) {
    // 根據輸入的編號生成文件名
    string filename = "input" + inputNumber + ".txt";
    string binFilename = "input" + inputNumber + ".bin";

    // 嘗試打開二進制文件
    ifstream binFile(binFilename, ios::binary);

    // 如果二進制文件不存在或無法打開
    if (!binFile) {
        // 轉換文本文件到二進制文件
        if (!task0(filename, binFilename)) {
            return; // If task0 fails, then exit task1
        }
        binFile.open(binFilename, ios::binary);  // 重新打开新创建的二进制文件
        if (!binFile) {                          // 检查文件是否成功打开
            cerr << "Error opening binary file after creation." << endl;
            return;
        }
    }

    // 跳到二進制文件的末尾以獲取其大小
    binFile.seekg(0, ios::end);
    // 計算學生的數量（文件大小 / Student結構的大小）
    long long studentCount = static_cast<long long>(binFile.tellg()) / sizeof(Student);
    // 將文件指標移回文件的開始
    binFile.seekg(0, ios::beg);

    // 計算雜湊表的大小（學生數量的1.2倍的下一個質數）
    int tableSize = nextPrime(static_cast<int>(1.2 * studentCount));

    // 初始化一個大小為tableSize的空雜湊表
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
    string outputFilename = "linear" + inputNumber+".txt";
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
    computeAverageComparisons(hashTable, studentCount, tableSize);
}

int stepFunc(const char* key, int maxStep) {
    long long result = 1;
    for (int i = 0; i < 10 && key[i] != '\0'; i++) {
        result *= key[i];
    }
    return maxStep - (result % maxStep);
}

void task2(const string& inputNumber) {
    string filename = "input" + inputNumber + ".txt";
    string binFilename = "input" + inputNumber + ".bin";
    ifstream binFile(binFilename, ios::binary);
    
    if (!binFile) {
        cerr << "Error opening binary file." << endl;
        return;
    }

    binFile.seekg(0, ios::end);
    long long studentCount = static_cast<long long>(binFile.tellg()) / sizeof(Student);
    binFile.seekg(0, ios::beg);
    //cout << "studentCount : " << studentCount << endl;
    int tableSize = nextPrime(static_cast<int>(1.2 * studentCount));
    int maxStep = nextPrime(studentCount / 3.0);
    //cout << "maxStep : " << maxStep << endl;
    vector<HashEntry> hashTable(tableSize);
    
    Student student;
    int totalComparisons = 0; // ?算搜索?存值的平均比?次?
    
    while (binFile.read(reinterpret_cast<char*>(&student), sizeof(student))) {
        int originalHvalue = hashFunc(student.sid, tableSize);
        int step = stepFunc(student.sid, maxStep);
        //cout << "Step : " << step << endl;

        int hvalue = originalHvalue;
        int comparisons = 0; // For this specific student

        // Add a guard against infinite loops by adding a counter
        int counter = 0;
        while (hashTable[hvalue].occupied && counter < tableSize) {
            hvalue = (hvalue + step) % tableSize;
            comparisons++;
            counter++;
        }

        if (counter == tableSize) {
            cerr << "Couldn't find an empty slot in the hash table. Aborting." << endl;
            break; // Exit the loop
        }

        totalComparisons += comparisons + 1;  // +1 for the initial position
        hashTable[hvalue].originalHvalue = originalHvalue;
        hashTable[hvalue].hvalue = hvalue;
        hashTable[hvalue].student = student;
        hashTable[hvalue].occupied = true;
    }


    double averageExisting = static_cast<double>(totalComparisons) / studentCount;
    cout << "Average comparisons for existing values: " << fixed << setprecision(3) << averageExisting << endl;

    string outputFilename = "double" + inputNumber+".txt";
    ofstream outputFile(outputFilename);
    if (!outputFile) {
        cerr << "Error opening output file." << endl;
        return;
    }
    
    outputFile << " --- Hash Table Y --- (double hashing)" << endl;

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

    binFile.close();
    outputFile.close();
}

void task3(const string& inputNumber) {
    string filename = "input" + inputNumber + ".txt";
    string binFilename = "input" + inputNumber + ".bin";
    ifstream binFile(binFilename, ios::binary);
    
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
    for (long long i = 0; i < studentCount; i++) {
        // 讀取學生資料
        binFile.read(reinterpret_cast<char*>(&student), sizeof(Student));

        // 進行雜湊計算
        int hValue = hashFunc(student.sid, tableSize);
        int originalHValue = hValue;  // 儲存原始的雜湊值
        int probeCount = 1;           // 平方探測

        // 使用平方探測來尋找適當的雜湊位址
        while (hashTable[hValue].occupied) {
            hValue = (originalHValue + probeCount * probeCount) % tableSize;
            probeCount++;
        }

        // 將學生資料儲存到雜湊表中
        hashTable[hValue].hvalue = hValue;
        hashTable[hValue].originalHvalue = originalHValue;
        hashTable[hValue].student = student;
        hashTable[hValue].occupied = true;
    }

    binFile.close();

    // 創建輸出文件名並嘗試打開它
    string outputFilename = "quadratic" + inputNumber+".txt";
    ofstream outputFile(outputFilename);
    if (!outputFile) {
        cerr << "Error opening output file." << endl;
        return;
    }
    
    // 寫雜湊表的標題到輸出文件中
    outputFile << " --- Hash Table Z --- (quadratic probing)" << endl;

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
    outputFile.close();
    
    // 1. 計算搜尋現存值的平均比較次數
    double searchExistingAverage = 0.0;
    for (const HashEntry &entry : hashTable) {
        if (entry.occupied) {
            int probeCount = 1;
            int hValue = hashFunc(entry.student.sid, tableSize);
            int originalHValue = hValue;

            while (hashTable[hValue].occupied && (hashTable[hValue].student.sid != entry.student.sid)) {
                hValue = (originalHValue + probeCount * probeCount) % tableSize;
                probeCount++;
            }
            searchExistingAverage += probeCount;
        }
    }
    // 2. 計算搜尋不存在值的平均比較次數
    double searchNonExistingAverage = 0.0;

    // 模擬從每個位置開始的搜索
    for (int i = 0; i < tableSize; i++) {
        int probeCount = 0;
        int nextPos = i;

        // 模擬從這個位置開始搜尋一個「不存在的值」
        while (hashTable[nextPos].occupied && probeCount < tableSize) {
            probeCount++;
            nextPos = (i + probeCount * probeCount) % tableSize;
        }

        // 如果 probeCount 還小於 tableSize，那麼這意味著我們找到了一個空槽位
        if (probeCount < tableSize) {
            searchNonExistingAverage += (probeCount);
        } else {
            // 否則，如果 probeCount 達到 tableSize，這意味著哈希表已滿，我們已經探測了整個表
            searchNonExistingAverage += probeCount;
        }
    }

    searchNonExistingAverage /= tableSize;  // 除以雜湊表的大小得到平均值
    cout << "Average comparisons for non-existing values: " << searchNonExistingAverage << endl;


    searchExistingAverage /= studentCount;
    cout << "Average comparisons for existing values: " << searchExistingAverage << endl;
    }

int main() {
    // 請求用戶輸入文件名
    string filename1;
    string filename2;
    string filename3;
    int choice;

    while (true) {
        cout << "*** Hash Table Builder **" << endl;
        cout << "* 0. QUIT              *" << endl;
        cout << "* 1. Linear probing    *" << endl;
        cout << "* 2. Double hashing    *" << endl;
        cout << "* 3. Quadratic probing *" << endl;
        cout << "*************************************" << endl;
        cout << "Input a choice (0, 1, 2, 3): ";
        cin >> choice;

        switch (choice) {
            case 0:
                cout << "Exiting the program." << endl;
                return 0;
            case 1:
                cout << "Input a file number ([0] Quit): ";
                cin >> filename1;
                if (filename1 == "0") {
                    cout << "Exiting the program." << endl;
                    return 0;
                }
                task1(filename1);
                break;
            case 2:
                if (filename1.empty()) {
                    cout << "### Choose 1 first. ###" << endl;
                    continue;
                }
                task2(filename1);
                break;
            case 3:
                if (filename1.empty()) {
                    cout << "### Choose 1 first. ###" << endl;
                    continue;
                }
                task3(filename1);
                break;
            default:
                cout << "Invalid choice. Please select a valid option (0, 1, 2, 3)." << endl;
                continue;
        }
    }

    return 0;
}