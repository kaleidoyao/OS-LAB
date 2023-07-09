#include <iostream>
#include <cstring>
#include <vector>

#define DIR_FILE 0x10
#define NORMAL_FILE 0x20
#define ROOT_ENTRY_SIZE 32

using namespace std;

extern "C" {
    void string_print(const char *);
}

// 全局变量区（转为32位）
int BytsPerSec;   // 每个扇区的字节数
int SecPerClus;   // 每个簇的扇区数
int BytsPerClus;  // 每个簇的字节数
int RsvdSecCnt;   // Boot record占用的扇区数
int NumFATs;      // FAT的数量，一般为2
int RootEntCnt;   // 根目录文件数的最大值
int FATSz;        // FAT表的扇区数
int FATBase;      // FAT首字节的偏移数
int RootBase;     // 根目录首字节的偏移数
int DataBase;     // 数据区首字节的偏移数

int getFAT(FILE *fat12, int n) {
    int pos = FATBase + n * 3 / 2;
    uint16_t bytes;
    fseek(fat12, pos, SEEK_SET);
    fread(&bytes, 1, 2, fat12);
    
    if(n % 2 == 0) {
        return bytes & 0x0fff;  // 取后12位
    }
    else {
        return bytes >> 4;      // 取前12位
    }
}

vector<string> split(string str, char op) {
    vector<string> result;
    string tmp = "";
    for(int i=0; i<str.size(); i++) {
        if(str[i] == op) {
            if(tmp.size() > 0) {
                result.push_back(tmp);
                tmp = "";
            }
        } 
        else {
            tmp += str[i];
        }
    }
    if(tmp.size() > 0) {
        result.push_back(tmp);
    }
    return result;
}

#pragma pack(1) // 指定按1字节对齐
class FileNode {
private:
    string name;
    string path;
    bool isFile;
    char *content = new char[10000];  // 如果是文件，存放文件中的内容
    FileNode *father;
    vector<FileNode *> children;
    int dirCnt = 0;
    int fileCnt = 0;
public:
    FileNode(string name, string path, bool isFile, FileNode *father);
    void setFather(FileNode *father) { this->father = father; }
    void addChildren(FileNode *fileNode);
    void fetchChildren(FILE *fat12, int fstClus);
    void getFileContent(FILE *fat12, int fstClus);
    string getName() { return name; }
    string getPath() { return path; }
    bool getIsFile() { return isFile; }
    char *getContent() { return content; }
    FileNode *getFather() { return father; }
    vector<FileNode *> getChildren() { return children; }
    int getDirCnt() { return dirCnt; }
    int getFileCnt() { return fileCnt; }
};

// 引导扇区
class BPB {
private:
    uint16_t BPB_BytsPerSec;  // 每个扇区的字节数
    uint8_t  BPB_SecPerClus;  // 每个簇的扇区数
    uint16_t BPB_RsvdSecCnt;  // Boot record占用的扇区数
    uint8_t  BPB_NumFATs;     // FAT的数量，一般为2
    uint16_t BPB_RootEntCnt;  // 根目录文件数的最大值
    uint16_t BPB_TotSec16;    // 扇区总数
    uint8_t  BPB_Media;       // 媒体描述符
    uint16_t BPB_FATSz16;     // 一个FAT表的扇区数
    uint16_t BPB_SecPerTrk;   // 每个磁道的扇区数
    uint16_t BPB_NumHeads;    // 磁头数
    uint32_t BPB_HiddSec;     // 隐藏扇区数
    uint32_t BPB_TotSec32;    // 如果BPB_FATSz16为0，该值为扇区总数
public:
    BPB(FILE *fat12);
};

// 根目录区，用以读取目录数据
class RootEntry {
private:
    char     DIR_Name[11];  // 文件名8字节，扩展名3字节
    uint8_t  DIR_Attr;      // 文件属性
    uint8_t  reserve[10];   // 保留位
    uint16_t DIR_WrtTime;   // 最后一次写入时间
    uint16_t DIR_WrtDate;   // 最后一次写入日期
    uint16_t DIR_FstClus;   // 文件开始的簇号
    uint32_t DIR_FileSize;  // 文件大小
public:
    RootEntry() {};
    RootEntry(FILE *fat12, FileNode *root);
    void fetchChildren(FILE *fat12, int base, FileNode *root);
    bool isInvalidName();
    string getDirName();
    string getFileName();
};


FileNode::FileNode(string name, string path, bool isFile, FileNode *father) {
    this->name = name;
    this->path = path;
    this->isFile = isFile;
    this->father = father;
}
void FileNode::addChildren(FileNode *fileNode) {
    this->children.push_back(fileNode);
    if(fileNode->isFile) {
        fileCnt++;
    } else {
        dirCnt++;
    }
}
void FileNode::fetchChildren(FILE *fat12, int fstClus) {
    RootEntry *rootEntry = new RootEntry;
    int curClus = fstClus;
    int FAT = 0;
    while(FAT < 0xFF8) {  // 尚未达到文件的最后一个簇
        int pos = DataBase + (curClus - 2) * BytsPerClus;
        for(int i=0; i<BytsPerClus/ROOT_ENTRY_SIZE; i++) {  // 每个簇中有多少个目录项
            rootEntry->fetchChildren(fat12, pos + i*ROOT_ENTRY_SIZE, this);
        }

        FAT = getFAT(fat12, fstClus);
        if(FAT == 0xFF7) {
            string_print("这是一个坏簇!\n");
            break;
        }
        curClus = FAT;
    }
}
void FileNode::getFileContent(FILE *fat12, int fstClus) {
    int curClus = fstClus;
    int loopCnt = 0;
    int FAT = 0;
    while(FAT < 0xFF8) {  // 尚未达到文件的最后一个簇
        int pos = DataBase + (curClus - 2) * BytsPerClus;
        fseek(fat12, pos, SEEK_SET);
        fread(this->content + loopCnt * BytsPerClus, 1, BytsPerClus, fat12);

        FAT = getFAT(fat12, curClus);
        if(FAT == 0xFF7) {
            string_print("这是一个坏簇!\n");
            break;
        }
        curClus = FAT;
        loopCnt++;
    }
}
FileNode *findFileInFatrDir(FileNode *root, string name) {
    for(FileNode *child : root->getChildren()) {
        if(child->getName() == name) {
            return child;
        }
    }
    return nullptr;
}
FileNode *findFileByPath(FileNode *root, vector<string> paths) {
    if(paths.size() == 0) return root;
    FileNode *retNode = root;
    for(string path : paths) {
        if(path == ".") {
            continue;
        }
        else if(path == "..") {
            retNode = retNode->getFather();
        }
        else {
            retNode = findFileInFatrDir(retNode, path);
            if(retNode == nullptr) {
                return nullptr;
            }
        }
    }
    return retNode;
}
FileNode *findFileByName(FileNode *root, string name) {
    if(name == "") return root;
    for(FileNode *child : root->getChildren()) {
        if(child->getName() == name) {
            return child;
        }
        if(!child->getIsFile()) {
            FileNode *retNode = findFileByName(child, name);
            if(retNode != nullptr) {
                return retNode;
            }
        }
    }
    return nullptr;
}


BPB::BPB(FILE *fat12) {
    fseek(fat12, 11, SEEK_SET);  // 设置文件指针到偏移11个字节处 SEEK_SET: 文件的开头
    fread(this, 1, 25, fat12);   // 读取BPB长度为25字节

    BytsPerSec = this->BPB_BytsPerSec;
    SecPerClus = this->BPB_SecPerClus;
    RsvdSecCnt = this->BPB_RsvdSecCnt;
    NumFATs = this->BPB_NumFATs;
    RootEntCnt = this->BPB_RootEntCnt;
    if(this->BPB_FATSz16 == 0) {
        FATSz = this->BPB_TotSec32;
    }
    else {
        FATSz = this->BPB_FATSz16;
    }

    BytsPerClus = SecPerClus * BytsPerSec;
    FATBase = RsvdSecCnt * BytsPerSec;
    RootBase = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec;
    DataBase = (RsvdSecCnt + NumFATs * FATSz + (RootEntCnt * ROOT_ENTRY_SIZE + BytsPerSec - 1) / BytsPerSec) * BytsPerSec;
}


RootEntry::RootEntry(FILE *fat12, FileNode *root) {
    int curBase = RootBase;
    for(int i=0; i<RootEntCnt; i++) {
        this->fetchChildren(fat12, curBase, root);
        curBase += ROOT_ENTRY_SIZE;
    }
}
void RootEntry::fetchChildren(FILE *fat12, int base, FileNode *root) {
    fseek(fat12, base, SEEK_SET);
    fread(this, 1, ROOT_ENTRY_SIZE, fat12);

    if(!isInvalidName()) {
        if(this->DIR_Attr == NORMAL_FILE) {
            string name = this->getFileName();
            FileNode *newNode = new FileNode(name, root->getPath() + name + "/", true, root);                
            newNode->getFileContent(fat12, this->DIR_FstClus);
            root->addChildren(newNode);
        }
        else if(this->DIR_Attr == DIR_FILE) {
            string name = this->getDirName();
            FileNode *newNode = new FileNode(name, root->getPath() + name + "/", false, root);
            newNode->fetchChildren(fat12, this->DIR_FstClus);
            root->addChildren(newNode);
        }
    }
}
bool RootEntry::isInvalidName() {
    for(int i=0; this->DIR_Name[i]!='\0' && i<11; i++) {
        if((this->DIR_Name[i] >= 'a' && this->DIR_Name[i] <= 'z')
        || (this->DIR_Name[i] >= 'A' && this->DIR_Name[i] <= 'Z')
        || (this->DIR_Name[i] >= '0' && this->DIR_Name[i] <= '9')
        || this->DIR_Name[i] == ' ') {
            continue;
        }
        else {
            return true;
        }
    }
    return false;
}
string RootEntry::getDirName() {
    string ret = "";
    for(int i=0; this->DIR_Name[i]!=' ' && i<8; i++) {  // 8位文件名
        ret += this->DIR_Name[i];
    }
    return ret;
}
string RootEntry::getFileName() {
    string ret = getDirName() + ".";
    for(int i=8; this->DIR_Name[i]!=' ' && i<11; i++) { // 3位扩展名
        ret += this->DIR_Name[i];
    }
    return ret;
}

void printLs(FileNode *root) {
    string str = root->getPath() + ":\n";
    if(!root->getIsFile() && root->getPath()!="/") {
        str += "\033[31m.  ..  \033[0m";
    }
    vector<FileNode *> subDirs;
    for(FileNode *child : root->getChildren()) {
        if(child->getIsFile()) {
            str += child->getName();
        }
        else {
            str += "\033[31m" + child->getName() + "\033[0m";
            subDirs.push_back(child);
        }
        str += "  ";
    }
    str += "\n";
    string_print(str.c_str());
    for(FileNode *subDir : subDirs) {
        printLs(subDir);
    }
}
void printLsl(FileNode *root) {
    string str = root->getPath() + " " + to_string(root->getDirCnt()) + " " + to_string(root->getFileCnt()) + ":\n";
    if(!root->getIsFile() && root->getPath()!="/") {
        str += "\033[31m.\n..\n\033[0m";
    }
    vector<FileNode *> subDirs;
    for(FileNode *child : root->getChildren()) {
        if(child->getIsFile()) {
            string content = child->getContent();
            str += child->getName() + " " + to_string(content.length());
        }
        else {
            str += "\033[31m" + child->getName() + "\033[0m";
            str += " " + to_string(child->getDirCnt()) + " " + to_string(child->getFileCnt());
            subDirs.push_back(child);
        }
        str += "\n";
    }
    str += "\n";
    string_print(str.c_str());
    for(FileNode *subDir : subDirs) {
        printLsl(subDir);
    }
}
void checkLs(vector<string> command, FileNode *root) {
    bool hasOptionL = false;
    string arg = "";
    
    for(string tmp : command) {
        if(tmp[0] == '-') {
            if(tmp.length() == 1) {
                string str = "ls: 指令缺失选项!\n";
                string_print(str.c_str());
                return;
            }
            for(char ch : tmp.substr(1)) {
                if(ch == 'l') hasOptionL = true;
                else {
                    string str = "ls: 指令不支持选项 " + tmp + " !\n";
                    string_print(str.c_str());
                    return;
                }
            }
        }
        else {
            if(arg == "") {
                arg = tmp;
            }
            else {
                string_print("ls: 指令不支持多个路径!\n");
                return;
            }
        }
    }

    vector<string> paths = split(arg, '/');
    FileNode *node = findFileByPath(root, paths);
    if(node == nullptr) {
        string str = "ls: 指令找不到路径 " + arg + " !\n";
        string_print(str.c_str());
    }
    else if(node->getIsFile()) {
        string str = "ls: 指令不支持文件!\n";
        string_print(str.c_str());
    }
    else {
        if(hasOptionL) {
            printLsl(node);
        }
        else {
            printLs(node);
        }
    }
}


void checkCat(vector<string> command, FileNode *root) {
    string arg = "";
    for(string tmp : command) {
        if(tmp[0] == '-') {
            string_print("cat: 指令不支持选项!\n");
            return;
        } 
        else {
            if(arg == "") {
                arg = tmp;
            }
            else {
                string_print("cat: 指令不支持多个路径!\n");
                return;
            }
        }
    }

    vector<string> paths = split(arg, '/');
    if(paths.size() == 0) {
        string_print("cat: 指令不支持根目录!\n");
        return;
    }
    FileNode *fileFoundByPath = findFileByPath(root, paths);
    FileNode *fileFoundByName = findFileByName(root, paths[paths.size() - 1]);

    if(fileFoundByName == nullptr) {
        string_print("cat: 文件不存在!\n");
        return;
    }
    if(fileFoundByPath == nullptr) {
        string_print("cat: 路径错误!\n");
        return;
    }
    if(!fileFoundByPath->getIsFile()) {
        string_print("cat: 指令不支持目录!\n");
        return;
    }
    
    string_print(fileFoundByPath->getContent());
}


int main() {
    FILE *fat12 = fopen("a.img", "rb"); // 打开文件系统
    BPB *bpb = new BPB(fat12);

    FileNode *root = new FileNode("", "/", false, nullptr);
    root->setFather(root);
    RootEntry *rootEntry = new RootEntry(fat12, root);

    while(true) {
        string_print("> ");
        string cmd, keyWord;
        getline(cin, cmd);
        vector<string> command = split(cmd, ' ');
        keyWord = command[0];
        command.erase(command.begin());

        if (keyWord == "ls") {
            if(command.size() == 0) {
                printLs(root);
            }
            else {
                checkLs(command, root);
            }
        }
        else if (keyWord == "cat") {
            checkCat(command, root);
        }
        else if (keyWord == "exit") {
            if(command.size() != 0) {
                string_print("exit: 指令不支持选项和路径!\n");
            }
            else {
                string_print("Goodbye!\n");
                fclose(fat12);
                break;
            }
        }
        else {
            string_print("错误的指令: ");
            string_print(cmd.c_str());
            string_print("\n");
        }
    }
    
    return 0;
}