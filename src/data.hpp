#ifndef __MY_DATA__
#define __MY_DATA__

#include <unordered_map>
#include <pthread.h>
#include "util.hpp"
#include "config.hpp"

namespace cloud
{
    typedef struct BackupInfo
    {
        bool pack_flag;        // 是否压缩标志
        size_t fsize;          // 文件大小
        time_t atime;          // 最后一次访问时间
        time_t mtime;          // 最后一次修改时间
        std::string real_path; // 文件实际存储路径
        std::string pack_path; // 压缩包存储路径名称
        std::string url;
 
        bool NewBackupInfo(const std::string realpath)
        {
            FileUtil fu(realpath);
            if (fu.Exists() == false)
            {
                std::cout << "new backupinfo: file not exists!\n";
                return false;
            }

            Config *config = Config::GetInstance();
            std::string packdir = config->GetPackDir();
            std::string packsuffix = config->GetPackFileSuffix();
            std::string download_prefix = config->GetDownloadPrefix();
            this->pack_flag = false;
            this->fsize = fu.FileSize();
            this->mtime = fu.LastModTime();
            this->atime = fu.LastAccTime();
            this->real_path = realpath;
            // ./back_dir/a.txt -> ./pack_dir/a.txt.lz
            this->pack_path = packdir + fu.FileName() + packsuffix;
            // ./back_dir/a.txt -> ./download/a.txt
            this->url = download_prefix + fu.FileName();
            return true;
        }
    } BackupInfo;

    class DataManager
    {
    private:
        std::string _backup_file;                           // 持久化存储文件
        std::unordered_map<std::string, BackupInfo> _table; // 内存中以hash表存储
        pthread_rwlock_t _rwlock;                           // 读写锁 -- 读共享，写互斥。
    public:
        DataManager() 
        {
            _backup_file = Config::GetInstance()->GetBackupFile();
            pthread_rwlock_init(&_rwlock, nullptr);
            InitLoad();
        }
        ~DataManager()
        {
            pthread_rwlock_destroy(&_rwlock);
        }
        // 新增
        bool Insert(const BackupInfo &info)
        {
            pthread_rwlock_wrlock(&_rwlock);
            _table[info.url] = info;
            pthread_rwlock_unlock(&_rwlock);
            Storage();
            return true;
        }
        // 修改
        bool Update(const BackupInfo &info)
        {
            pthread_rwlock_wrlock(&_rwlock);
            _table[info.url] = info;
            pthread_rwlock_unlock(&_rwlock);
            Storage();
            return true;
        }
        bool GetOneByURL(const std::string &url, BackupInfo *info)
        {
            pthread_rwlock_wrlock(&_rwlock);
            // url是key值，可以直接通过find进行查找
            auto it = _table.find(url);
            if (it == _table.end())
            {
                pthread_rwlock_unlock(&_rwlock);
                return false;
            }
            *info = it->second;
            pthread_rwlock_unlock(&_rwlock);
            return true;
        }
        bool GetOneByRealPath(const std::string &realpath, BackupInfo *info)
        {
            pthread_rwlock_wrlock(&_rwlock);
            auto it = _table.begin();
            for (;it != _table.end(); ++it)
            {
                if (it->second.real_path == realpath)
                {
                    *info = it->second;
                    pthread_rwlock_unlock(&_rwlock);
                    return true;
                }
            }
            pthread_rwlock_unlock(&_rwlock);
            return false;
        }
        bool GetAll(std::vector<BackupInfo> *arr)
        {
            pthread_rwlock_wrlock(&_rwlock);
            auto it = _table.begin();
            for (;it != _table.end(); ++it)
            {
                arr->push_back(it->second);
            }
            pthread_rwlock_unlock(&_rwlock);
            return true;
        }
        // 每次数据新增或者修改都需要重新持久化存储，避免数据丢失
        bool Storage()
        {
            // 1. 获取所有数据
            std::vector<BackupInfo> array;
            this->GetAll(&array);
            // 2. 添加到Json::Value
            Json::Value root;
            for (int i = 0; i < array.size(); i++)
            {
                Json::Value item;
                item["pack_flag"] = array[i].pack_flag;
                item["fsize"] = (Json::Int64)array[i].fsize;
                item["atime"] = (Json::Int64)array[i].atime;
                item["mtime"] = (Json::Int64)array[i].mtime;
                item["real_path"] = array[i].real_path;
                item["pack_path"] = array[i].pack_path;
                item["url"] = array[i].url;
                root.append(item);
            }
            // 3. 对Json::Value序列化
            std::string body;
            JsonUtil::Serialize(root, &body);
            // 4. 写文件
            FileUtil fu(_backup_file);
            fu.SetContent(body);

            return true;
        }
        // 初始化加载，在每次系统重启都需要加载以前的数据
        bool InitLoad()
        {
            // 1. 将数据文件中的数据读取出来
            FileUtil fu(_backup_file);
            if (fu.Exists() == false) 
                return true;
            std::string body;
            fu.GetContent(&body);
            // 2. 反序列化
            Json::Value root;
            JsonUtil::UnSerialize(body, &root);
            // 3. 将反序列化得出的Json::Value中的数据添加到table中
            for (int i = 0; i < root.size(); ++i)
            {
                BackupInfo info;
                info.pack_flag = root[i]["pack_flag"].asBool();
                info.fsize = root[i]["fsize"].asInt64();
                info.atime = root[i]["atime"].asInt64();
                info.mtime = root[i]["mtime"].asInt64();
                info.real_path = root[i]["real_path"].asString();
                info.pack_path = root[i]["pack_path"].asString();
                info.url = root[i]["url"].asString();
                Insert(info);
            }

            return true;
        }
    };
}

#endif
