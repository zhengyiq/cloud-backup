#ifndef __MY_HOT__
#define __MY_HOT__

#include "data.hpp"
#include <unistd.h>
extern cloud::DataManager *_data;
namespace cloud
{
    class HotManager
    {
    private:
        std::string _back_dir;    // 备份文件路径
        std::string _pack_dir;    // 压缩文件路径
        std::string _pack_suffix; // 压缩包后缀名
        int _hot_time;            // 热点判断时间
    private:
        // 非热点文件返回真；热点文件返回假
        bool HotJudge(const std::string &filename)
        {
            FileUtil fu(filename);
            time_t last_atime = fu.LastAccTime();
            time_t cur_time = time(NULL);
            if (cur_time - last_atime > _hot_time)
            {
                return true;
            }
            return false;
        }
    public:
        HotManager()
        {
            Config *config = Config::GetInstance();
            _back_dir = config->GetBackDir();
            _pack_dir = config->GetPackDir();
            _pack_suffix = config->GetPackFileSuffix();
            _hot_time = config->GetHotTime();
            FileUtil back_dir(_back_dir);
            FileUtil pack_dir(_pack_dir);
            back_dir.CreateDirectory();
            pack_dir.CreateDirectory();
        }
        bool RunModule()
        {
            while (1)
            {
                // 1. 遍历备份目录，获取所有文件路径名称
                FileUtil fu(_back_dir);
                std::vector<std::string> array;
                fu.ScanDirectory(&array);

                // 2. 逐个文件获取最后一次访问时间与当前系统时间进行判断比较
                for(auto &a : array)
                {
                    if (HotJudge(a) == false)
                        continue; // 热点文件则不需要处理
                    
                    // 3. 获取文件的备份信息
                    BackupInfo bi;
                    if (_data->GetOneByRealPath(a, &bi) == false)
                    {
                        // 现在有一个文件存在，但没有备份信息
                        bi.NewBackupInfo(a); // 设置一个新的备份信息出来
                    }
                    // 4. 对非热点文件进行压缩处理，删除源文件
                    FileUtil tmp(a);
                    tmp.Compress(bi.pack_path);
                    // 5. 修改数据管理模块对应的文件信息
                    tmp.Remove();
                    bi.pack_flag = true;
                    _data->Update(bi);
                }
                usleep(1000); // 避免空目录循环遍历，消耗CPU资源过高
            }
            return true;
        }
    };
}

#endif
