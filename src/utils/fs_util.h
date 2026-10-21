#ifndef __SYLAR_UTILS_FS_H__
#define __SYLAR_UTILS_FS_H__

#include <string>
#include <vector>
#include <cstdint>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <exception>

namespace sylar {
namespace utils {

class FSException : public std::exception {
public:
    FSException(int err, const std::string& path, const std::string& msg);
    ~FSException() noexcept override = default;

    const char* what() const noexcept override;

    int GetErrno() const { return m_errno; }
    const std::string& GetPath() const { return m_path; }
private:
    int m_errno;
    std::string m_path;
    std::string m_msg;
};

class FS {
public:
    enum class FileType {
        Unknown,
        Regular,
        Directory,
        Symlink,
        CharDev,
        BlockDev,
        Fifo,
        Socket
    };

    struct FileStat {
        FileType type{FileType::Unknown};
        uint64_t size{0};
        mode_t mode{0};
        uid_t uid{0};
        gid_t gid{0};
        time_t atime{0};
        time_t mtime{0};
        time_t ctime{0};
    };

    struct DiskInfo {
        uint64_t total_bytes{0}, free_bytes{0}, avail_bytes{0};
    };

    // ========== 文件打开 / 关闭 ==========
    static int Open(const std::string& path, int flags, mode_t mode = 0644);
    static int OpenRead(const std::string& path);
    static void Close(int fd);

    // ========== 文件创建 ==========
    static void CreateFile(const std::string& path, mode_t perm = 0644);
    static void CreateFileRecursive(const std::string& dir, const std::string& file, mode_t dir_mode = 0755, mode_t file_mode = 0644);
    static void Touch(const std::string& path, mode_t perm = 0644);

    // ========== 路径工具 ==========
    static std::string PathJoin(const std::string& a, const std::string& b);
    static std::string Dirname(const std::string& path);
    static std::string Basename(const std::string& path);
    static bool IsAbsolute(const std::string& path);

    // ========== stat / 判断 ==========
    static FileStat StatFile(const std::string& path, bool follow_symlink = true);
    static FileType GetFileType(const std::string& path, bool follow_symlink = true);
    static bool Exists(const std::string& path);
    static bool IsRegular(const std::string& path, bool follow_symlink = true);
    static bool IsDir(const std::string& path, bool follow_symlink = true);
    static bool IsSymlink(const std::string& path);
    static uint64_t FileSize(const std::string& path, bool follow_symlink = true);

    // ========== 权限 / 属主 ==========
    static void Chmod(const std::string& path, mode_t perm);
    static void Chown(const std::string& path, uid_t uid, gid_t gid);

    // ========== 目录 ==========
    static void Mkdir(const std::string& path, mode_t perm = 0755);
    static void MkdirRecursive(const std::string& path, mode_t perm = 0755);
    static std::vector<std::string> ListDir(const std::string& dir, bool skip_dot = true);

    // ========== 删除 / 移动 / 复制 ==========
    static void Remove(const std::string& path);
    static void RemoveRecursive(const std::string& path);
    static void Rename(const std::string& oldpath, const std::string& newpath);
    static void CopyFile(const std::string& src, const std::string& dst);
    static void CopyRecursive(const std::string& src, const std::string& dst);

    // ========== 软链接 ==========
    static void Symlink(const std::string& target, const std::string& linkpath);
    static std::string ReadSymlink(const std::string& linkpath);

    // ========== 文件读写 ==========
    static std::string ReadFile(const std::string& path);
    static void WriteFile(const std::string& path, const std::string& content, mode_t perm = 0644);
    static void AppendFile(const std::string& path, const std::string& content, mode_t perm = 0644);

    // ========== 磁盘信息 ==========
    static DiskInfo DiskStat(const std::string& mount_path);
};

}
}

#endif