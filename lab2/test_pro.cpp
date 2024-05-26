#include <iostream>  
#include <vector>  
#include <set>  
#include <cstdlib>  
#include <ctime>  
#include <algorithm>  

// 生成指定范围内的随机整数，不包括最大值  
int getRandomInt(int min, int max) {
    return min + rand() % (max - min);
}

// 生成指定数量的文件ID列表，并确保包含至少一个指定的文件ID（例如requiredFileId），  
// 且按从小到大排列。同时，每行至少包含minNumIds个元素，其中除了requiredFileId外，  
// 至少还有minCommonIds - 1个共同的文件ID。  
std::vector<int> commonIds0(int minCommonIds, int requiredFileId, int numFiles) {
    std::set<int> commonIds; // 用于存储共同的文件ID 
    // 生成minCommonIds - 1个共同的文件ID  
    for (int i = 0; i < minCommonIds - 1; ++i) {
        int fileId;
        do {
            fileId = getRandomInt(1, numFiles);
        } while (commonIds.find(fileId) != commonIds.end());
        commonIds.insert(fileId);
    }
    std::vector<int> re(commonIds.begin(),commonIds.end());

    return re;
}

std::vector<int> generateLine(int numFiles, int requiredFileId, int minNumIds, int minCommonIds,std::vector<int>li) {
    std::vector<int> line = li;

    // 随机决定每行生成多少个元素（至少为minNumIds）  
    int numIdsToAdd = getRandomInt(minNumIds - line.size(), 300 - line.size());

    // 生成剩余的文件ID，确保它们不重复且不是共同的文件ID  
    for (int i = 0; i < numIdsToAdd; ++i) {
        int fileId;
        do {
            fileId = getRandomInt(1, numFiles);
        } while (std::find(line.begin(), line.end(), fileId) != line.end());
        line.push_back(fileId);
    }

    // 对文件ID进行排序  
    std::sort(line.begin(), line.end());

    return line;
}

// 生成指定规模的测试数据  
void generateTestData(int numLines, int numFiles, int requiredFileId, int minNumIds, int minCommonIds) {
    srand(time(nullptr)); // 初始化随机数种子  

    std::vector<int>li = commonIds0(minCommonIds, requiredFileId, numFiles);

    for (int i = 0; i < numLines; ++i) {
        std::vector<int> line = generateLine(numFiles, requiredFileId, minNumIds, minCommonIds,li);

        // 输出当前行的文件ID  
        for (size_t j = 0; j < line.size(); ++j) {
            std::cout << line[j];
            if (j < line.size() - 1) {
                std::cout << " ";
            }
        }
        std::cout << std::endl;
    }
}

int main() {
    int numFiles = 1000; // 文件ID总数  
    int numLines = 1100; // 要生成的行数  
    int requiredFileId = 55; // 至少包含的文件ID  
    int minNumIds = 100; // 每行至少包含的元素个数  
    int minCommonIds = 50; // 除了requiredFileId外，每行至少包含的共同元素个数  

    generateTestData(numLines, numFiles, requiredFileId, minNumIds, minCommonIds);

    return 0;
}