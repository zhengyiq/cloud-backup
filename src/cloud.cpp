#include "util.hpp"
#include "config.hpp"
#include "data.hpp"
#include "hot.hpp"
void FileUtilTest(const std::string &filename)
{
    // 测试获取文件相关属性
    // cloud::FileUtil fu(filename);
    // std::cout << fu.FileSize() << std::endl;
    // std::cout << fu.LastModTime() << std::endl;
    // std::cout << fu.LastAccTime() << std::endl;
    // std::cout << fu.FileName() << std::endl;


    // 测试文件压缩解压缩
    // cloud::FileUtil fu(filename);
    // std::string body;
    // fu.GetContent(&body);
    // cloud::FileUtil nfu("./hello.txt");
    // nfu.SetContent(body);
    // std::string packname = filename + ".lz";
    // cloud::FileUtil fu(filename);
    // fu.Compress(packname);
    // cloud::FileUtil pfu(packname);
    // pfu.UnCompress("hello.txt");

    // 测试创建目录显示目录中的内容
    // cloud::FileUtil fu(filename);
    // fu.CreateDirectory();
    // std::vector<std::string> array;
    // fu.ScanDirectory(&array);
    // for(auto &a : array)
    // {
    //     std::cout << a << std::endl;
    // }

    return;
}

void JsonUtilTest()
{
    const char *name = "小明";
    int age = 19;
    float score[] = {85, 88.5, 99};
    Json::Value root;
    root["姓名"] = name;
    root["年龄"] = age;
    root["成绩"].append(score[0]);
    root["成绩"].append(score[1]);
    root["成绩"].append(score[2]);

    std::string json_str;
    cloud::JsonUtil::Serialize(root, &json_str);
    std::cout << json_str << std::endl;
    
    Json::Value val;
    cloud::JsonUtil::UnSerialize(json_str, &val);
    std::cout << val["姓名"].asString() << std::endl;
    std::cout << val["年龄"].asInt() << std::endl;
    for (int i = 0; i < val["成绩"].size(); i++)
    {
        std::cout << val["成绩"][i].asFloat() << std::endl;
    }
}

void ConfigTest()
{
    cloud::Config *config = cloud::Config::GetInstance();
    std::cout << config->GetHotTime() << std::endl;
    std::cout << config->GetServerPort() << std::endl;
    std::cout << config->GetServerIP() << std::endl;
    std::cout << config->GetDownloadPrefix() << std::endl;
    std::cout << config->GetPackFileSuffix() << std::endl;
    std::cout << config->GetPackDir() << std::endl;
    std::cout << config->GetBackDir() << std::endl;
    std::cout << config->GetBackupFile() << std::endl;
}

void DataTest(const std::string &filename)
{
    cloud::DataManager data;
    std::vector<cloud::BackupInfo> array;
    data.GetAll(&array);
    for(auto &e : array)
    {
        std::cout << e.pack_flag << std::endl;
        std::cout << e.fsize << std::endl;
        std::cout << e.mtime << std::endl;
        std::cout << e.atime << std::endl;
        std::cout << e.real_path << std::endl;
        std::cout << e.pack_path << std::endl;
        std::cout << e.url << std::endl;
    }


    // cloud::BackupInfo info;
    // info.NewBackupInfo(filename);
    // cloud::DataManager data;
    // data.Insert(info);
    // cloud::BackupInfo tmp;

    // std::cout << "--------------------update and get one----------------------" << std::endl;
    // data.GetOneByURL("/download/bundlecpp.tar", &tmp);
    // std::cout << tmp.pack_flag << std::endl;
    // std::cout << tmp.fsize << std::endl;
    // std::cout << tmp.mtime << std::endl;
    // std::cout << tmp.atime << std::endl;
    // std::cout << tmp.real_path << std::endl;
    // std::cout << tmp.pack_path << std::endl;
    // std::cout << tmp.url << std::endl;

    // std::cout << "--------------------update and get all----------------------" << std::endl;
    // info.pack_flag = true;
    // data.Update(info);
    // std::vector<cloud::BackupInfo> array;
    // data.GetAll(&array);
    // for(auto &e : array)
    // {
    //     std::cout << e.pack_flag << std::endl;
    //     std::cout << e.fsize << std::endl;
    //     std::cout << e.mtime << std::endl;
    //     std::cout << e.atime << std::endl;
    //     std::cout << e.real_path << std::endl;
    //     std::cout << e.pack_path << std::endl;
    //     std::cout << e.url << std::endl;
    // }

    // std::cout << "-------------------------realpath---------------------------" << std::endl;
    // data.GetOneByRealpath(info.real_path, &tmp);
    // std::cout << tmp.pack_flag << std::endl;
    // std::cout << tmp.fsize << std::endl;
    // std::cout << tmp.mtime << std::endl;
    // std::cout << tmp.atime << std::endl;
    // std::cout << tmp.real_path << std::endl;
    // std::cout << tmp.pack_path << std::endl;
    // std::cout << tmp.url << std::endl;

    // data.Storage();
}
cloud::DataManager *_data;
void HotTest()
{
    _data = new cloud::DataManager();
    cloud::HotManager hot;
    hot.RunModule();
}

int main(int argc, char *argv[])
{
    // FileUtilTest(argv[1]);
    // JsonUtilTest();
    // ConfigTest();
    // DataTest(argv[1]);
    HotTest();
    return 0;
}