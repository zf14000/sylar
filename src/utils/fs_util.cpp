#include "fs_util.h"
#include <cstring>
#include <cerrno>
#include <cstdlib>
#include <dirent.h>
#include <sys/time.h>
#include <sys/vfs.h>
#include <algorithm>
#include <sstream>

namespace sylar {
namespace utils {

FSException::FSException(int err, const std::string& path, const std::string& msg)
    : m_errno(err), m_path(path), m_msg(msg) {
    std::ostringstream oss;
    oss << m_msg << " | path=" << m_path << " errno=" << m_errno
        << " (" << ::strerror(m_errno) << ")";
    m_msg = oss.str();
}

const char* FSException::what() const noexcept {
    return m_msg.c_str();
}

// ========== 文件打开 / 关闭 ==========
int FS::Open(const std::string& path, int flags, mode_t mode) {
    int fd = ::open(path.c_str(), flags, mode);
    if (fd < 0) {
        throw FSException(errno, path, "open failed");
    }
    return fd;
}

int FS::OpenRead(const std::string& path) {
    return Open(path, O_RDONLY);
}

void FS::Close(int fd) {
    if (::close(fd) != 0) {
        throw FSException(errno, std::to_string(fd), "close fd failed");
    }
}

// ========== 文件创建 ==========
void FS::CreateFile(const std::string& path, mode_t perm) {
    // O_CREAT | O_EXCL：原子创建，存在直接抛异常
    int fd = ::open(path.c_str(), O_CREAT | O_EXCL | O_WRONLY, perm);
    if (fd < 0) {
        throw FSException(errno, path, "CreateFile open(O_CREAT|O_EXCL) failed");
    }
    ::close(fd);
}

void FS::CreateFileRecursive(const std::string& dir, const std::string& file, mode_t dir_mode, mode_t file_mode) {
    // 先拼接完整路径
    std::string full_path = PathJoin(dir, file);
    // 递归创建父目录
    MkdirRecursive(dir, dir_mode);
    // 独占创建文件（和 CreateFile 语义保持一致：存在抛异常）
    CreateFile(full_path, file_mode);
    int fd = ::open(full_path.c_str(), O_CREAT | O_EXCL | O_WRONLY, file_mode);
    if (fd < 0) {
        throw FSException(errno, full_path, "CreateFileRecursive open(O_CREAT|O_EXCL) failed");
    }
    ::close(fd);
}

void FS::Touch(const std::string& path, mode_t perm) {
    struct stat st;
    if (::stat(path.c_str(), &st) == 0) {
        struct timeval tv[2];
        ::gettimeofday(&tv[0], nullptr);
        tv[1] = tv[0];
        if (::utimes(path.c_str(), tv) != 0) {
            throw FSException(errno, path, "Touch utimes failed");
        }
        return;
    }
    int fd = ::open(path.c_str(), O_CREAT | O_WRONLY, perm);
    if (fd < 0) {
        throw FSException(errno, path, "Touch open O_CREAT failed");
    }
    ::close(fd);
}

// ========== 路径工具 ==========
std::string FS::PathJoin(const std::string& a, const std::string& b) {
    if (a.empty()) return b;
    if (b.empty()) return a;
    bool a_end = (a.back() == '/');
    bool b_start = (b.front() == '/');
    if (a_end && b_start) {
        return a + b.substr(1);
    } else if (!a_end && !b_start) {
        return a + "/" + b;
    } else {
        return a + b;
    }
}

std::string FS::Dirname(const std::string& path) {
    if (path.empty()) return ".";
    size_t pos = path.rfind('/');
    if (pos == std::string::npos) return ".";
    if (pos == 0) return "/";
    return path.substr(0, pos);
}

std::string FS::Basename(const std::string& path) {
    if (path.empty()) return "";
    size_t pos = path.rfind('/');
    if (pos == std::string::npos) return path;
    return path.substr(pos + 1);
}

bool FS::IsAbsolute(const std::string& path) {
    return !path.empty() && path[0] == '/';
}

// ========== stat / 判断 ==========
static FS::FileType StatToFileType(const struct stat& st) {
    if (S_ISREG(st.st_mode))   return FS::FileType::Regular;
    if (S_ISDIR(st.st_mode))   return FS::FileType::Directory;
    if (S_ISLNK(st.st_mode))   return FS::FileType::Symlink;
    if (S_ISCHR(st.st_mode))   return FS::FileType::CharDev;
    if (S_ISBLK(st.st_mode))   return FS::FileType::BlockDev;
    if (S_ISFIFO(st.st_mode))  return FS::FileType::Fifo;
    if (S_ISSOCK(st.st_mode))  return FS::FileType::Socket;
    return FS::FileType::Unknown;
}

FS::FileStat FS::StatFile(const std::string& path, bool follow_symlink) {
    struct stat st;
    int ret = follow_symlink ? ::stat(path.c_str(), &st) : ::lstat(path.c_str(), &st);
    if (ret != 0) {
        throw FSException(errno, path, "StatFile failed");
    }
    FileStat fs;
    fs.type    = StatToFileType(st);
    fs.size    = static_cast<uint64_t>(st.st_size);
    fs.mode    = st.st_mode;
    fs.uid     = st.st_uid;
    fs.gid     = st.st_gid;
    fs.atime   = st.st_atim.tv_sec;
    fs.mtime   = st.st_mtim.tv_sec;
    fs.ctime   = st.st_ctim.tv_sec;
    return fs;
}

FS::FileType FS::GetFileType(const std::string& path, bool follow_symlink) {
    return StatFile(path, follow_symlink).type;
}

bool FS::Exists(const std::string& path) {
    struct stat st;
    return ::stat(path.c_str(), &st) == 0;
}

bool FS::IsRegular(const std::string& path, bool follow_symlink) {
    return GetFileType(path, follow_symlink) == FileType::Regular;
}

bool FS::IsDir(const std::string& path, bool follow_symlink) {
    return GetFileType(path, follow_symlink) == FileType::Directory;
}

bool FS::IsSymlink(const std::string& path) {
    struct stat st;
    if (::lstat(path.c_str(), &st) != 0) {
        return false;
    }
    return S_ISLNK(st.st_mode);
}

uint64_t FS::FileSize(const std::string& path, bool follow_symlink) {
    return StatFile(path, follow_symlink).size;
}

// ========== 权限 / 属主 ==========
void FS::Chmod(const std::string& path, mode_t perm) {
    if (::chmod(path.c_str(), perm) != 0) {
        throw FSException(errno, path, "Chmod failed");
    }
}

void FS::Chown(const std::string& path, uid_t uid, gid_t gid) {
    if (::chown(path.c_str(), uid, gid) != 0) {
        throw FSException(errno, path, "Chown failed");
    }
}

// ========== 目录 ==========
void FS::Mkdir(const std::string& path, mode_t perm) {
    if (::mkdir(path.c_str(), perm) != 0) {
        throw FSException(errno, path, "Mkdir failed");
    }
}

void FS::MkdirRecursive(const std::string& path, mode_t perm) {
    if (path.empty()) {
        throw FSException(EINVAL, path, "MkdirRecursive empty path");
    }
    struct stat st0;
    if (::stat(path.c_str(), &st0) == 0) {
        if (!S_ISDIR(st0.st_mode)) {
            throw FSException(EEXIST, path, "MkdirRecursive exists and not directory");
        }
        return;
    }
    size_t pos = 0;
    std::string cur;
    if (path[0] == '/') {
        cur = "/";
        pos = 1;
    }
    while (pos < path.size()) {
        size_t next = path.find('/', pos);
        if (next == std::string::npos) next = path.size();
        cur += path.substr(pos, next - pos);
        struct stat st;
        if (::stat(cur.c_str(), &st) == 0) {
            if (!S_ISDIR(st.st_mode)) {
                throw FSException(EEXIST, cur, "MkdirRecursive component not a directory");
            }
        } else {
            if (::mkdir(cur.c_str(), perm) != 0) {
                throw FSException(errno, cur, "MkdirRecursive mkdir failed");
            }
        }
        if (next == path.size()) break;
        cur += '/';
        pos = next + 1;
    }
}

std::vector<std::string> FS::ListDir(const std::string& dir, bool skip_dot) {
    std::vector<std::string> res;
    DIR* dp = ::opendir(dir.c_str());
    if (!dp) {
        throw FSException(errno, dir, "ListDir opendir failed");
    }
    struct dirent* ent;
    while ((ent = ::readdir(dp)) != nullptr) {
        std::string name = ent->d_name;
        if (skip_dot && (name == "." || name == "..")) {
            continue;
        }
        res.push_back(name);
    }
    ::closedir(dp);
    return res;
}

// ========== 删除 / 移动 / 复制 ==========
void FS::Remove(const std::string& path) {
    FileType tp = GetFileType(path, false);
    int ret;
    if (tp == FileType::Directory) {
        ret = ::rmdir(path.c_str());
    } else {
        ret = ::unlink(path.c_str());
    }
    if (ret != 0) {
        throw FSException(errno, path, "Remove failed");
    }
}

void FS::RemoveRecursive(const std::string& path) {
    FileType tp = GetFileType(path, false);
    if (tp != FileType::Directory) {
        Remove(path);
        return;
    }
    auto items = ListDir(path, true);
    for (auto& name : items) {
        std::string sub = PathJoin(path, name);
        RemoveRecursive(sub);
    }
    Remove(path);
}

void FS::Rename(const std::string& oldpath, const std::string& newpath) {
    if (::rename(oldpath.c_str(), newpath.c_str()) != 0) {
        throw FSException(errno, oldpath, "Rename failed");
    }
}

void FS::CopyFile(const std::string& src, const std::string& dst) {
    int fd_in = OpenRead(src);
    int fd_out = Open(dst, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    char buf[4096];
    ssize_t n;
    while ((n = ::read(fd_in, buf, sizeof(buf))) > 0) {
        ssize_t w = ::write(fd_out, buf, static_cast<size_t>(n));
        if (w < 0) {
            ::close(fd_in);
            ::close(fd_out);
            throw FSException(errno, dst, "CopyFile write failed");
        }
    }
    if (n < 0) {
        ::close(fd_in);
        ::close(fd_out);
        throw FSException(errno, src, "CopyFile read failed");
    }
    ::close(fd_in);
    ::close(fd_out);
}

void FS::CopyRecursive(const std::string& src, const std::string& dst) {
    FileStat sst = StatFile(src, false);
    if (sst.type == FileType::Directory) {
        Mkdir(dst, sst.mode & 0777);
        auto items = ListDir(src, true);
        for (auto& name : items) {
            std::string s = PathJoin(src, name);
            std::string d = PathJoin(dst, name);
            CopyRecursive(s, d);
        }
    } else if (sst.type == FileType::Regular) {
        CopyFile(src, dst);
    } else if (sst.type == FileType::Symlink) {
        std::string target = ReadSymlink(src);
        Symlink(target, dst);
    } else {
        // 跳过设备、fifo、socket
    }
}

// ========== 软链接 ==========
void FS::Symlink(const std::string& target, const std::string& linkpath) {
    if (::symlink(target.c_str(), linkpath.c_str()) != 0) {
        throw FSException(errno, linkpath, "Symlink failed");
    }
}

std::string FS::ReadSymlink(const std::string& linkpath) {
    char buf[PATH_MAX];
    ssize_t r = ::readlink(linkpath.c_str(), buf, sizeof(buf)-1);
    if (r < 0) {
        throw FSException(errno, linkpath, "ReadSymlink failed");
    }
    buf[r] = '\0';
    return std::string(buf);
}

// ========== 文件读写 ==========
std::string FS::ReadFile(const std::string& path) {
    int fd = OpenRead(path);
    std::string out;
    char buf[4096];
    ssize_t n;
    while ((n = ::read(fd, buf, sizeof(buf))) > 0) {
        out.append(buf, static_cast<size_t>(n));
    }
    if (n < 0) {
        ::close(fd);
        throw FSException(errno, path, "ReadFile read failed");
    }
    ::close(fd);
    return out;
}

void FS::WriteFile(const std::string& path, const std::string& content, mode_t perm) {
    int fd = Open(path, O_WRONLY | O_CREAT | O_TRUNC, perm);
    size_t off = 0;
    while (off < content.size()) {
        ssize_t w = ::write(fd, content.data() + off, content.size() - off);
        if (w <= 0) {
            ::close(fd);
            throw FSException(errno, path, "WriteFile write failed");
        }
        off += static_cast<size_t>(w);
    }
    ::close(fd);
}

void FS::AppendFile(const std::string& path, const std::string& content, mode_t perm) {
    int fd = Open(path, O_WRONLY | O_CREAT | O_APPEND, perm);
    size_t off = 0;
    while (off < content.size()) {
        ssize_t w = ::write(fd, content.data() + off, content.size() - off);
        if (w <= 0) {
            ::close(fd);
            throw FSException(errno, path, "AppendFile write failed");
        }
        off += static_cast<size_t>(w);
    }
    ::close(fd);
}

// ========== 磁盘信息 ==========
FS::DiskInfo FS::DiskStat(const std::string& mount_path) {
    struct statfs sfs;
    if (::statfs(mount_path.c_str(), &sfs) != 0) {
        throw FSException(errno, mount_path, "DiskStat statfs failed");
    }
    DiskInfo di;
    di.total_bytes = static_cast<uint64_t>(sfs.f_blocks) * sfs.f_bsize;
    di.free_bytes  = static_cast<uint64_t>(sfs.f_bfree)  * sfs.f_bsize;
    di.avail_bytes = static_cast<uint64_t>(sfs.f_bavail) * sfs.f_bsize;
    return di;
}

}
}