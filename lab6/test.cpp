#include <iostream>
#include <fstream>
#include <iomanip> // 包含 setw 和 setfill
#include <string>
#include <unordered_map>
#include <vector>
#include <dirent.h>
#include <sys/stat.h>
#include <openssl/sha.h>
#include <cstring> // 包含 strcmp
#include <unistd.h>

using namespace std;

std::string fileSHA1(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Unable to open file: " << filename << std::endl;
        return "";
    }

    SHA_CTX context;
    if (!SHA1_Init(&context)) {
        std::cerr << "Failed to initialize SHA" << std::endl;
        return "";
    }

    char buf[8192];
    while (file.good()) {
        file.read(buf, sizeof(buf));
        SHA1_Update(&context, buf, file.gcount());
    }

    unsigned char result[SHA_DIGEST_LENGTH];
    SHA1_Final(result, &context);

    std::ostringstream sout;
    sout << std::hex << std::setfill('0');
    for (long long c : result) {
        sout << std::setw(2) << c;
    }
    return sout.str();
}

void traverse(const std::string& directory, std::unordered_map<std::string, std::string>& hashMap) {
    DIR* dir;
    struct dirent* entry;
    struct stat info;

    dir = opendir(directory.c_str());
    if (!dir) {
        std::cerr << "Could not open directory: " << directory << std::endl;
        return;
    }

    while ((entry = readdir(dir)) != nullptr) {
        std::string path = directory + "/" + entry->d_name;
        if (entry->d_type == DT_DIR) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;
            traverse(path, hashMap);
        } else {
            std::string hash = fileSHA1(path);
            if (hashMap.find(hash) == hashMap.end()) {
                hashMap[hash] = path;
            } else {
                // Found a duplicate
                std::string originalPath = hashMap[hash];
                unlink(path.c_str());
                link(originalPath.c_str(), path.c_str());
            }
        }
    }
    closedir(dir);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <directory>" << std::endl;
        return 1;
    }

    std::string directory = argv[1];
    std::unordered_map<std::string, std::string> hashMap;
    traverse(directory, hashMap);

    return 0;
}