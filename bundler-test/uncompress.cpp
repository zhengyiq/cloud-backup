#include <iostream>
#include <fstream>
#include <string>
#include "bundle.h"
int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        printf("argv[1]是压缩包名称\n");
        printf("argv[2]是解压后的文件名称\n");
        return -1;
    }
    std::string ifilename = argv[1]; // 压缩包名
    std::string ofilename = argv[2]; // 解压缩后文件名
    std::ifstream ifs;
    ifs.open(ifilename, std::ios::binary);
    ifs.seekg(0, std::ios::end);
    size_t fsize = ifs.tellg();
    ifs.seekg(0, std::ios::beg);
    std::string body;
    body.resize(fsize);
    ifs.read(&body[0], fsize);
    ifs.close();
    std::string unpacked = bundle::unpack(body); // 对压缩包数据解压缩
    std::ofstream ofs;
    ofs.open(ofilename, std::ios::binary);
    ofs.write(&unpacked[0], unpacked.size());
    ofs.close();
    return 0;
}