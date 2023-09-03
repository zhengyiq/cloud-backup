#ifndef __MY_UTIL__
#define __MY_UTIL__

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sys/stat.h>
#include <experimental/filesystem>
#include <jsoncpp/json/json.h>
#include <memory>
#include <bundle.h>

namespace cloud
{
    namespace fs = std::experimental::filesystem;
    class FileUtil
    {
    private:
        std::string _filename;

    public:
        FileUtil(const std::string &filename) :_filename(filename)
        {}
        
        // 获取文件大小
        int64_t FileSize()
        {
            struct stat st;
            if (stat(_filename.c_str(), &st) < 0)
            {
                std::cout << "get file size failed!\n";
                return -1;
            }
            return st.st_size;
        }
        
        // 获取文件最后一次修改时间
        time_t LastModTime()
        {
            struct stat st;
            if (stat(_filename.c_str(), &st) < 0)
            {
                std::cout << "get file LastModTime failed!\n";
                return -1;
            }
            return st.st_mtime;
        }         
        
        // 获取文件最后一次访问时间
        time_t LastAccTime()
        {
            struct stat st;
            if (stat(_filename.c_str(), &st) < 0)
            {
                std::cout << "get file LastAccTime failed!\n";
                return -1;
            }
            return st.st_atime;
        }          
        
        // 获取文件路径中的文件名称 /abc/test.txt
        std::string FileName()
        {
            size_t pos = _filename.find_last_of("/\\");
            if (pos == std::string::npos)
            {
                return _filename;
            }
            return _filename.substr(pos + 1);
        }
        
        // 向文件写入数据
        bool SetContent(const std::string &body)
        {
            std::ofstream ofs;
            ofs.open(_filename, std::ios::binary);
            if (ofs.is_open() == false)
            {
                std::cout << "write open file failed!\n";
            }
            ofs.write(&body[0], body.size());
            if (ofs.good() == false)
            {
                std::cout << "write file content failed!\n";
                ofs.close();
                return false;
            }
            ofs.close();
            return true;
        }

        bool GetContent(std::string *body)
        {
            size_t fsize = this->FileSize();
            return GetPosLen(body, 0, fsize);
        }
        
        // 获取文件指定位置 指定长度
        bool GetPosLen(std::string *body, size_t pos, size_t len)
        {
            size_t fsize = this->FileSize();
            if (pos + len > fsize)
            {
                std::cout << "get file len is error\n";
                return false;
            }

            std::ifstream ifs;
            ifs.open(_filename, std::ios::binary);
            if (ifs.is_open() == false)
            {
                std::cout << "read open file failed\n";
                return false;
            }
            ifs.seekg(pos, std::ios::beg);
            body->resize(len);
            ifs.read(&(*body)[0], len);
            if (ifs.good() == false)
            {
                std::cout << "get file content failed\n";
                ifs.close();
                return false;
            }
            ifs.close();
            return true;
        }
        
        // 判断文件是否存在
        bool Exists()
        {
            return fs::exists(_filename);
        }
        
        bool CreateDirectory()
        {
            if (this->Exists()) return true;
            return fs::create_directories(_filename);
        }
        
        bool ScanDirectory(std::vector<std::string> *array)
        {
            for (auto &p : fs::directory_iterator(_filename))
            {
                if (fs::is_directory(p) == true) continue;
                
                // relative_path 带有路径的文件名
                array->push_back(fs::path(p).relative_path().string());
            }

            return true;
        }
        
        // 文件压缩
        bool Compress(const std::string &packname)
        {
            // 1. 获取源文件数据
            std::string body;
            if (this->GetContent(&body) == false)
            {
                std::cout << "compress get file content failed!\n";
                return false;
            }

            // 2. 对数据进行压缩
            std::string packed = bundle::pack(bundle::LZIP, body);

            // 3. 将压缩的数据存储在压缩包文件中
            FileUtil fu(packname);
            if (fu.SetContent(packed) == false)
            {
                std::cout << "compress write packed data failed!\n";
                return false;
            }

            return true;
        }
        
        bool UnCompress(const std::string &filename)
        {
            // 将当前压缩包数据读取出来
            std::string body;
            if (this->GetContent(&body) == false)
            {
                std::cout << "uncompress get file content failed!\n";
                return false;
            }
            // 对压缩的数据进行解压缩
            std::string unpacked = bundle::unpack(body);

            // 将解压缩的数据写入到新文件中
            FileUtil fu(filename);
            if (fu.SetContent(unpacked) == false)
            {
                std::cout << "uncompress write packed data failed!\n";
                return false;
            }

            return true;
        }

        bool Remove()
        {
            if (this->Exists() == false)
            {
                return true;
            }
            remove(_filename.c_str());
            return true;
        }
    };

    class JsonUtil
    {
        public:
            static bool Serialize(const Json::Value &root, std::string *str)
            {
                Json::StreamWriterBuilder swb;
                std::unique_ptr<Json::StreamWriter> sw(swb.newStreamWriter());
                std::stringstream ss;
                if (sw->write(root, &ss) != 0)
                {
                    std::cout << "json write failed!\n";
                    return false;
                }
                *str = ss.str();
                return true;
            }

            static bool UnSerialize(const std::string &str, Json::Value *root)
            {
                Json::CharReaderBuilder crb;
                std::unique_ptr<Json::CharReader> cr(crb.newCharReader());
                std::string err;
                bool ret = cr->parse(str.c_str(), str.c_str() + str.size(), root, &err);
                if (ret == false)
                {
                    std::cout << "parse error: " << err << std::endl;
                    return false;
                }
                return true;
            }
    };
}
#endif