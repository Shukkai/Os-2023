/*
Student No.: 110550110
Student Name: 林書愷
Email: kyle.lin0908.nycu.cs10@nycu.edu.tw
SE tag: xnxcxtxuxoxsx
Statement: I am fully aware that this program is not
supposed to be posted to a public server, such as a
public GitHub repository or a public web page.
*/
#include<iostream>
#include<unistd.h>
#include<vector>
#include<openssl/sha.h>
#include<sys/time.h>
#include<unordered_map>
#include<fstream>
#include<dirent.h>
#include<string>
#include<cstring>
#include<utility>
#include<algorithm>
#include<openssl/crypto.h>
using namespace std;
vector<pair<string,string>> getallfiles(string path){
    DIR* dir = opendir(path.c_str());
    vector<pair<string,string>> ans;
    if(dir == NULL){
        return ans;
    }
    else{
        for(dirent* entry = readdir(dir); entry != nullptr; entry = readdir(dir)){
            
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue;
            }
            string fullpath = path + "/" + entry->d_name;
            if(entry->d_type == DT_DIR){
                vector<pair<string,string>> tmp = getallfiles(fullpath);
                if(!tmp.empty()){
                    ans.insert(ans.end(),tmp.begin(),tmp.end());
                }
            }
            else{
                ans.push_back(make_pair(fullpath,entry->d_name));
            }
        }
    }
    closedir(dir);
    return ans;
}
string hashing(string data){
    SHA_CTX sha1;
    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA1((unsigned char*) data.c_str(), strlen(data.c_str()), hash);
    SHA1_Init(&sha1);
    SHA1_Update(&sha1, data.c_str(), data.length());
    SHA1_Final(hash, &sha1);
    OPENSSL_cleanse(&sha1, sizeof(sha1));
    string ans;
    for(int i = 0;i < SHA_DIGEST_LENGTH;i++){
        ans += hash[i];
    }
    return ans;
}    
int main(int argc, char* argv[]){
    string filepath = argv[1];
    vector<pair<string,string>> links = getallfiles(filepath);
    unordered_map<string,string> files_hash;
    fstream in;
    for(int i = 0; i< links.size();i++){
        in.open(links[i].first);
        string content,tmp;
        while(in>>tmp){
            content += tmp;
        }
        in.close();
        string hash_val = hashing(content);
        auto it = files_hash.find(hash_val);
        if(it != files_hash.end()){
            unlink(links[i].first.c_str());
            link(it->second.c_str(),links[i].first.c_str());
        }
        else{
            files_hash[hash_val] = links[i].first;
        }
    }
    return 0;
}