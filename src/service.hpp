#ifndef __MY_SERVICE__
#define __MY_SERVICE__

#include "data.hpp"
#include "httplib.h"

extern cloud::DataManager *_data;
namespace cloud
{
    class Service
    {
        // 搭建http服务器，并进行业务处理
    private:
        int _server_port;
        std::string _server_ip;
        std::string _download_prefix;
        httplib::Server _server;

    private:
        // 上传请求处理函数
        static void Upload(const httplib::Request &req, httplib::Response &rsp)
        {
            // post /upload 文件数据在正文中(正文并不全是文件数据)
            auto ret = req.has_file("file"); // 判断有没有上传的文件区域
            if (ret == false)
            {
                rsp.status = 400;
                return;
            }
            const auto &file = req.get_file_value("file");
            // file.filename // 文件名称
            // file.content // 文件数据
            std::string back_dir = Config::GetInstance()->GetBackDir();
            std::string realpath = back_dir + FileUtil(file.filename).FileName();
            FileUtil fu(realpath);
            fu.SetContent(file.content); // 将数据写入文件中
            BackupInfo info;
            info.NewBackupInfo(realpath); // 组织备份的文件信息
            _data->Insert(info); // 向数据管理模块添加备份文件信息

            return ;
        }
        static std::string TimetoStr(time_t t)
        {
            std::string tmp = std::ctime(&t);
            return tmp;
        }
        // 展示页面获取请求
        static void ListShow(const httplib::Request &req, httplib::Response &rsp)
        {
            // 1. 获取所有文件备份信息
            std::vector<BackupInfo> array;
            _data->GetAll(&array);
            // 2. 根据所有备份信息，组织HTML文件数据
            std::stringstream ss;
            ss << "<html><head><title>Download</title></head>";
            ss << "<body><h1>Download</h1><table>";
            for (auto &a : array)
            {
                ss << "<tr>";
                std::string filename = FileUtil(a.real_path).FileName();
                ss << "<td><a href='" << a.url << "'>" << filename << "</a></td>";
                ss << "<td align='right'>" << TimetoStr(a.mtime) << "</td>";
                ss << "<td align='right'>" << a.fsize / 1024 << "k</td>";
                ss << "</tr>";
            }
            ss << "</table></body></html>";
            rsp.body = ss.str();
            rsp.set_header("Content-Type", "text/html");
            rsp.status = 200;
            return;
        }

        static std::string GetETag(const BackupInfo &info)
        {
            // ETag : filename - fsize - mtime
            FileUtil fu(info.real_path);
            std::string etag = fu.FileName();
            etag += "-";
            etag += std::to_string(info.fsize);
            etag += "-";
            etag += std::to_string(info.mtime);
            return etag;
        }
        // 文件下载处理函数
        static void Download(const httplib::Request &req, httplib::Response &rsp)
        {
            // 1. 获取客户端请求的资源路径 path req.path
            // 2. 根据资源路径，获取文件备份信息
            BackupInfo info;
            _data->GetOneByURL(req.path, &info);
            // 3. 判断文件是否被压缩，如果被压缩先解压缩
            if (info.pack_flag == true)
            {
                FileUtil fu(info.pack_path);
                fu.UnCompress(info.real_path);
                // 4. 删除压缩包，修改备份信息
                fu.Remove();
                info.pack_flag = false;
                _data->Update(info);
            }

            // 5. 读取文件数据，放入rsp.body中
            FileUtil fu(info.real_path);

            //  -- 进行断点续传
            bool retrans = false;
            std::string old_etag;
            if (req.has_header("If-Range"))
            {
                old_etag = req.get_header_value("If-Range");
                // 有If-Range字段且，这个字段与请求文件的最新etag一致，则符合断点续传
                if (old_etag == GetETag(info))
                {
                    retrans = true;
                }
            }
            // 如果没有If-Range字段则是正常下载，或者如果有这个字段，但是它的值与当前文件的etag不一致，则也必须返回全部数据

            if (retrans == false)
            {
                // 6. 设置响应头部字段 ETag Accept-Ranges: bytes
                fu.GetContent(&rsp.body);
                rsp.set_header("Accept-Ranges", "bytes");
                rsp.set_header("ETag", GetETag(info));  
                rsp.set_header("Content-Type", "application/octet-stream");
                rsp.status = 200;
            }
            else
            {
                // httplib内部实现了对于区间请求也就是断电续传请求的处理
                // 只需要我们用户将文件所有数据读取到rsp.body中，它内部会自动根据请求区间从body中取出指定区间数据进行响应
                // std::string range = req.get_header_val("Range"); bytes = start-end
                fu.GetContent(&rsp.body);
                rsp.set_header("Accept-Ranges", "bytes");
                rsp.set_header("ETag", GetETag(info));  
                // rsp.set_header("Content-Range", "bytes start-end/fsize");
                rsp.status = 206; // 区间请求响应的是206
            }
        }

    public:
        Service() 
        {
            Config *config = Config::GetInstance();
            _server_port = config->GetServerPort();
            _server_ip = config->GetServerIP();
            _download_prefix = config->GetDownloadPrefix();
        }
        bool RunModule()
        {
            _server.Post("/upload", Upload);
            _server.Get("/listshow", ListShow);
            _server.Get("/", ListShow);
            std::string download_url = _download_prefix + "(.*)";
            _server.Get(download_url, Download);
            _server.listen("0.0.0.0", _server_port);
            return true;
        }
    };
}

#endif
