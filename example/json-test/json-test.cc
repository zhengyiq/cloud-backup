#include <iostream>
#include <sstream>
#include <string>
#include <memory>
#include "jsoncpp/json/json.h"

int main()
{
    const char *name = "小明";
    int age = 18;
    float score[] = {77.5, 88, 93.6};

    Json::Value root;
    root["姓名"] = name;
    root["年龄"] = age;
    root["成绩"].append(score[0]);
    root["成绩"].append(score[1]);
    root["成绩"].append(score[2]);

    Json::StreamWriterBuilder swb;
    std::unique_ptr<Json::StreamWriter> sw(swb.newStreamWriter());
    std::stringstream ss;
    sw->write(root, &ss);

    std::cout << ss.str() << std::endl;

    std::string str = R"({"姓名":"小黑","年龄":19,"成绩":[58.5,66,35.5]})"; // R保证后面的字符串中的字符都是原始字符串
    Json::Value root_P;
    Json::CharReaderBuilder crb;
    std::unique_ptr<Json::CharReader> cr(crb.newCharReader());
    std::string err;
    bool ret = cr->parse(str.c_str(), str.c_str() + str.size(), &root_P, &err);
    if (ret == false)
    {
        std::cout << "parse error: " << err <<std::endl;
        return -1;
    }
    std::cout << root_P["姓名"].asString() << std::endl;
    std::cout << root_P["年龄"].asInt() << std::endl;
    int sz = root_P["成绩"].size();
    for(int i = 0; i < sz; i++)
    {
        std::cout << root_P["成绩"][i] << std::endl;
    }

    return 0;
}