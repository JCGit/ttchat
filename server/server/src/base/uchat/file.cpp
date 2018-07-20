/* This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. */
#include "uchat/file.hpp"
#include <stdio.h>
#include <unistd.h>
#include <ulimit.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#if defined __arm__ || defined __aarch64__
#   include <linux/limits.h>
#endif
#include <iostream>
#include "uchat/errno.hpp"
#include "uchat/logger.hpp"
#include "uchat/loghelper.cpp.hpp"
namespace uchat
{
using BoostScopedReadLock = ::uchat::ScopedReadLock;
using BoostScopedWriteLock = ::uchat::ScopedWriteLock;
template<typename T>
using SharedPtr = std::shared_ptr<T>;
constexpr char const* kN = kFileIoLogName;
ssize_t AvailableByte(int const fd) noexcept
{
    // FIONREAD 得到缓冲区里有多少字节要被读取
    long bytes = 0;
    if (::ioctl(fd, FIONREAD, &bytes) == -1) {
        int const e = errno;
        Error0("ioctl FIONREAD fail " << e << " " << ::strerror(e))
        if (e) {
            return -e;
        } else {
            return -1;
        }
    }
    return bytes;
}
long GetMaxOpenFiles()
{
    long m = 0;
    return ::ulimit(4, m);
}
int64_t GetFileSize(std::string const& filename) noexcept
{
    struct stat s;
    s.st_size = 0;
    int const ret = ::stat(filename.c_str(), &s);
    if (-1 == ret) {
        int const e = errno;
        if (e) {
            return -e;
        } else {
            return -1;
        }
    }
    return s.st_size;
}
int64_t GetFileSize(int const fd) noexcept
{
    struct stat s;
    s.st_size = 0;
    int const ret = ::fstat(fd, &s);
    if (-1 == ret) {
        int const e = errno;
        if (e) {
            return -e;
        } else {
            return -1;
        }
    }
    return s.st_size;
}
mode_t GetFileMode(std::string const& filename)
{
    struct stat s;
    s.st_mode = 0;
    int const e = ::stat(filename.c_str(), &s);
    Assert(e >= 0, "stat() fail: " << errno)
    return s.st_mode;
}
int IsExists(std::string const& file) noexcept
{
    ssize_t const sz = GetFileSize(file);
    if (sz >= 0) {
        return 1;
    } else if (-ENOENT == sz) {
        return 0;
    } else {
        return sz;
    }
}
int IsDirectory(std::string const& filename) noexcept
{
    struct stat filestat;
    filestat.st_mode = 0;
    int const ret = ::stat(filename.c_str(), &filestat);
    if (ret < 0) {
        int const e = errno;
        if (!e) {
            return -GetErrno(Errno::StatError);
        } else {
            return -GetErrno(e);
        }
    }
    return S_ISDIR(filestat.st_mode);
}
int IsPipe(int const fd) noexcept
{
    if (fd < 0) {
        return -EINVAL;
    }
    struct stat filestat;
    filestat.st_mode = 0;
    int const ret = ::fstat(fd, &filestat);
    if (ret < 0) {
        int const e = errno;
        if (!e) {
            return -GetErrno(Errno::StatError);
        } else {
            return -GetErrno(e);
        }
    }
    return S_ISFIFO(filestat.st_mode);
}
std::string GetCurrentWorkDirectory()
{
    constexpr size_t n = PATH_MAX + sizeof(void*);
    char buf[n];
    Assert(::getcwd(buf, PATH_MAX), "getcwd() fail: " << errno)
    buf[PATH_MAX] = 0;
    return buf;
}
void CreateSymlink(
    std::string const& directory,
    std::string const& filename,
    std::string const& symname)
{
    bool const cd = !directory.empty();
    std::string oldPwd;
    // get oldPwd when need
    if (cd) {
        try {
            oldPwd = GetCurrentWorkDirectory();
        } catch(std::exception const& e) {
            Assert(false, "GetCurrentWorkDirectory() fail: " << e.what())
        }
        int const e = ::chdir(directory.c_str());
        Assert(e >= 0, "chdir() fail")
    }
    int const ret = ::symlink(filename.c_str(), symname.c_str());
    int const e = errno;
    if (cd) {
        // go back to previous wd
        int const ignore = ::chdir(oldPwd.c_str());
        (void)(ignore);
    }
    Assert(ret >= 0, "symlink() fail: " << e)
}
int RemoveFiles(std::string const& pathname) noexcept
{
    int e = ::remove(pathname.c_str());
    if (e < 0) {
        e = errno;
        return e ? -e : -EPERM;
    }
    return 0;
}
int RemoveFile(std::string const& pathname) noexcept
{
    int e = ::unlink(pathname.c_str());
    if (e < 0) {
        e = errno;
        return e ? -e : -EPERM;
    }
    return 0;
}
int RemoveFileBySystem(std::string const& pathname) noexcept
{
#   ifndef _WIN32
    std::string const s("rm -f " + pathname);
    int e = ::system(s.c_str());
    if (e) {
        e = errno;
        return e ? -e : -EPERM;
    }
    return 0;
#   else
    return ::system(("del " + pathname).c_str());
#   endif
}
int MkDirs(char const* path, mode_t const mode) noexcept
{
    if (!path || ::strlen(path) <= 0) {
        return  -EINVAL;
    }
    {
        int64_t l = ::strlen(path);
        std::vector<char> buf0(l + 1 + sizeof(long));
        char* const buf = buf0.data();
        ::memcpy(buf, path, l + 1);
        if ('/' != buf[l - 1]) {
            buf[l] = '/';
            buf[l + 1] = '\0';
        }
        ++l;
        /* Mkdirs */
        for (int64_t i = 0; i < l; ++i) {
            if ('/' == buf[i] && (i > 0)) {
                buf[i] = '\0';
                if (!IsExists(buf)) {
                    if (::mkdir(buf, mode)) {
                        int ret = errno;
                        if (!ret) {
                            ret = EPERM;
                        }
                        return -ret;
                    }
                }
                buf[i] = '/';
            }
        }
    }
    return 0;// OK
}
int64_t Write2Stream(
    void const* const buf,
    uint32_t const shouldWrite,
    FILE* const stream,
    uint32_t const perWriteBytes) noexcept
{
    if ((!buf) || (!stream)) {
        return -EINVAL;
    }
    int en;
    {
        // Write
        uint8_t const* const buffer = reinterpret_cast<uint8_t const*>(buf);
        size_t w, tow;
        int64_t wt = 0;
        do {
            if ((shouldWrite - wt) >= perWriteBytes) {
                tow = perWriteBytes;
                w = ::fwrite(buffer + wt, 1, tow, stream);
            } else if ((shouldWrite - wt) > 0) {
                tow = shouldWrite - wt;
                w = fwrite(buffer + wt, 1, tow, stream);
            } else {
                break;
            }
            if (w != tow) {
                en = errno;
                goto write_fail;
            }
            wt += w;
        } while((wt < static_cast<int64_t>(shouldWrite)) && (w > 0));
        return wt;
    }
write_fail:
    if (0 != en) {
        return -en;
    } else {
        return -1;
    }
}
FileOpenConfig::FileOpenConfig(): flags(0) {}
FileOpenConfig::FileOpenConfig(PosixFileAccessMode const& access): flags(
    static_cast<int>(access)) {}
FileOpenConfig::FileOpenConfig(
    SimplifiedFileOpenFlag const& openFlag,
    PosixFileModes const* const fileMode): flags(0)
{
    switch (openFlag) {
    case SimplifiedFileOpenFlag::Append:
        this->flags |= static_cast<int>(PosixFileOpenFlag::Append);
        break;
    case SimplifiedFileOpenFlag::Truncate:
        this->flags |= static_cast<int>(PosixFileOpenFlag::Trunc);
        break;
    case SimplifiedFileOpenFlag::Directory:
        this->flags |= static_cast<int>(PosixFileOpenFlag::Directory);
        break;
    case SimplifiedFileOpenFlag::Create: {
        Assert(fileMode, "SimplifiedFileOpenFlag::Create but fileMode is nil")
        this->flags = static_cast<int>(PosixFileCreationFlag::Creat) |
            static_cast<int>(PosixFileAccessMode::WriteOnly) |
            static_cast<int>(PosixFileCreationFlag::Trunc);
        this->mode.reset(new PosixFileModes(*fileMode));
    } break;
    default: break;
    }
}
FileOpenConfig::FileOpenConfig(
    PosixFileAccessMode const& access,
    SimplifiedFileOpenFlag const& openFlag,
    PosixFileModes const* const fileMode): flags(static_cast<int>(access))
{
    switch (openFlag) {
    case SimplifiedFileOpenFlag::Append:
        this->flags |= static_cast<int>(PosixFileOpenFlag::Append);
        break;
    case SimplifiedFileOpenFlag::Truncate:
        this->flags |= static_cast<int>(PosixFileOpenFlag::Trunc);
        break;
    case SimplifiedFileOpenFlag::Directory:
        this->flags |= static_cast<int>(PosixFileOpenFlag::Directory);
        break;
    case SimplifiedFileOpenFlag::Create: {
        Assert(fileMode, "SimplifiedFileOpenFlag::Create but fileMode is nil")
        this->flags = static_cast<int>(PosixFileCreationFlag::Creat) |
            static_cast<int>(PosixFileAccessMode::WriteOnly) |
            static_cast<int>(PosixFileCreationFlag::Trunc);
        this->mode.reset(new PosixFileModes(*fileMode));
    } break;
    default: break;
    }
}
File::File(std::string const& filename): filename(filename), openConfig() {}
File::~File() { this->close(); }
void File::setFilename(std::string const& filename) noexcept
{
    BoostScopedWriteLock writeLock(this->filenameRwlock);
    // Close old when need
    if (this->filename != filename) {
        this->close();
    }
    // Update filename after close
    this->filename = filename;
}
std::string File::getFilename() const noexcept
{
    BoostScopedReadLock fileReadLock(this->filenameRwlock);
    return this->filename;
}
int File::isDirectory() const noexcept
{
    BoostScopedReadLock readLock(this->fdRwlock);
    struct stat filestat;
    filestat.st_mode = 0;
    int ret;
    if (this->fd >= 0) {
        ret = ::fstat(this->fd, &filestat);
    } else {
        ret = ::stat(this->filename.c_str(), &filestat);
    }
    if (ret < 0) {
        int const e = errno;
        if (!e) {
            return -GetErrno(Errno::StatError);
        } else {
            return -GetErrno(e);
        }
    }
    return S_ISDIR(filestat.st_mode);
}
bool File::isOpen() const noexcept
{
    BoostScopedReadLock readLock(this->fdRwlock);
    return (this->fd >= 0);
}
ssize_t File::size() const noexcept
{
    BoostScopedReadLock readLock(this->fdRwlock);
    return this->__size();
}
ssize_t File::__size() const noexcept
{
    struct stat filestat;
    filestat.st_size = 0;
    int ret;
    if (this->fd >= 0) {
        ret = ::fstat(this->fd, &filestat);
    } else {
        ret = ::stat(this->filename.c_str(), &filestat);
    }
    if (ret < 0) {
        int const e = errno;
        if (!e) {
            return -GetErrno(Errno::StatError);
        } else {
            return -GetErrno(e);
        }
    }
    if (filestat.st_size >= 0) {
        return filestat.st_size;
    } else {
        return -GetErrno(Errno::StatError);
    }
}
ssize_t File::ioPostion() const noexcept
{
    BoostScopedReadLock readLock(this->fdRwlock);
    if (this->fd < 0) {
        return -GetErrno(Errno::NotOpen);
    }
    {
        /* exclusive io */
        std::lock_guard<std::mutex> lock(this->ioMutex);
        off_t const pos = ::lseek(this->fd, 0, SEEK_CUR);
        if (-1 == static_cast<ssize_t>(pos)) {
            int const e = errno;
            if (!e) {
                return -GetErrno(Errno::SeekError);
            } else {
                return -GetErrno(e);
            }
        }
        return pos;
    }
}
ssize_t File::ioRestPostion() const noexcept
{
    BoostScopedReadLock readLock(this->fdRwlock);
    if (this->fd < 0) {
        return -GetErrno(Errno::NotOpen);
    }
    {
        /* exclusive io */
        std::lock_guard<std::mutex> lock(this->ioMutex);
        off_t const pos = ::lseek(this->fd, 0, SEEK_CUR);
        if (-1 == static_cast<ssize_t>(pos)) {
            int const e = errno;
            if (!e) {
                return -GetErrno(Errno::SeekError);
            } else {
                return -GetErrno(e);
            }
        }
        ssize_t const total = this->__size();
        if (total < 0) {
            return total;
        }
        return total - pos;
    }
}
int File::jump2Begin() noexcept
{
    BoostScopedReadLock readLock(this->fdRwlock);
    if (this->fd < 0) {
        return -GetErrno(Errno::NotOpen);
    }
    {
        // Exclusive io
        std::lock_guard<std::mutex> lock(this->ioMutex);
        off_t const pos = ::lseek(this->fd, 0, SEEK_SET);
        if (-1 == static_cast<ssize_t>(pos)) {
            int const e = errno;
            if (!e) {
                return -GetErrno(Errno::SeekError);
            } else {
                return -GetErrno(e);
            }
        }
        return 0;
    }
}
int File::jump2Offset(long const offset) noexcept
{
    BoostScopedReadLock readLock(this->fdRwlock);
    if (this->fd < 0) {
        return -GetErrno(Errno::NotOpen);
    }
    {
        // Exclusive io
        std::lock_guard<std::mutex> lock(this->ioMutex);
        off_t const pos = ::lseek(this->fd, offset, SEEK_CUR);
        if (-1 == static_cast<ssize_t>(pos)) {
            int const e = errno;
            if (!e) {
                return -GetErrno(Errno::SeekError);
            } else {
                return -GetErrno(e);
            }
        }
        return 0;
    }
}
int File::rjump2Offset(long const offset) noexcept
{
    BoostScopedReadLock readLock(this->fdRwlock);
    if (this->fd < 0) {
        return -GetErrno(Errno::NotOpen);
    }
    {
        // Exclusive io
        std::lock_guard<std::mutex> lock(this->ioMutex);
        off_t const pos = ::lseek(this->fd, offset, SEEK_END);
        if (-1 == static_cast<ssize_t>(pos)) {
            int const e = errno;
            if (!e) {
                return -GetErrno(Errno::SeekError);
            } else {
                return -GetErrno(e);
            }
        }
        return 0;
    }
}
int File::open(FileOpenConfig const& openConfig) noexcept
{
    BoostScopedWriteLock writeLock(this->fdRwlock);
    if (this->__isOpen()) {
        return -GetErrno(Errno::AlreadyOpen);
    }
    if (this->filename.length() <= 0) {
        return -GetErrno(Errno::InvalidFilename);
    }
    int fd;
    int const openFlag = openConfig.getFlags();
    auto const fileMode = openConfig.getMode();
    if (!fileMode) {
        fd = File::open(this->filename, openFlag);
    } else {
        fd = File::open(this->filename, openFlag, mode_t(int(*fileMode)));
    }
    if (fd < 0) {
        return fd;
    }
    {
        std::lock_guard<std::mutex> lock(this->ioMutex);
        this->fd = fd;
    }
    this->openConfig = openConfig;
    return GetErrno(Errno::Ok);
}
int File::open(std::string const& filename, int const flag) noexcept
{
    // Check create flag
    if (flag & static_cast<int>(PosixFileOpenFlag::Creat)) {
        return -GetErrno(Errno::InvalidParam);
    }
    int const ret = ::open(filename.c_str(), flag);
    if (ret >= 0) {
        return ret;
    }
    int const e = abs(errno);
    if (!e) {
        return -GetErrno(Errno::OpenError);
    } else {
        return -GetErrno(e);
    }
}
int File::open(std::string const& filename, int const flag, mode_t const mode)
noexcept
{
    int const ret = ::open(filename.c_str(), flag, mode);
    if (ret >= 0) {
        return ret;
    }
    int const e = abs(errno);
    if (!e) {
        return -GetErrno(Errno::OpenError);
    } else {
        return -GetErrno(e);
    }
}
void File::close() noexcept
{
    BoostScopedWriteLock writeLock(this->fdRwlock);
    if (this->fd >= 0) {
        std::lock_guard<std::mutex> lock(this->ioMutex);
        ::close(this->fd);
        this->fd = -1;
    }
}
std::tuple<int, SharedPtr<std::vector<uint8_t> > > File::read(
    ssize_t const size, bool checkavail) noexcept
{
    using RType = std::tuple<int, SharedPtr<std::vector<uint8_t> > >;
    /* chk busy */
    if (this->ioBusy) {
        Warning("busy")
        return RType{ -GetErrno(Errno::Busy), nullptr };
    }
    IoBusyGuard iobusy(this->ioBusy);/* make io busy */
    (void)(iobusy);
    int ret, initfd;
    SharedPtr<std::vector<uint8_t> > buffer = nullptr;
    std::string initfilename;
    {
        // get primary
        BoostScopedReadLock readLock(this->fdRwlock);
        initfd = this->fd;
        initfilename = this->filename;
    }
    /* check fd i.e. chk open */
    if (initfd < 0) {
        ret = -GetErrno(Errno::NotOpen);
        goto end;
    }
    {
    size_t maxRead;
    if (checkavail || size < 0) {
        ssize_t const fileSize = this->__size();
        if (fileSize < 0) {
            ret = fileSize;
            goto end;
        }
        if ((size < 0) || (size > fileSize)) {
            maxRead = fileSize;
        } else {
            maxRead = size;
        }
    } else {
        maxRead = size;
    }
    if (maxRead > kFileIoUpperBound) {
        ret = -GetErrno(Errno::InvalidParam);
        goto end;
    }
    try {
        buffer.reset(new std::vector<uint8_t>());
        buffer->reserve(maxRead);
    } catch (std::exception const& e) {
        Error0("alloc buffer fail: " << e.what())
        ret = -GetErrno(Errno::NoMem);
        goto end;
    }
    ssize_t didread;
    int fd;
    std::string filename;
    {
        /* get init incase changed */
        BoostScopedReadLock readLock(this->fdRwlock);
        fd = this->fd;
        filename = this->filename;
    }
    // Check fd .. changed
    if ((fd != initfd) || (filename != initfilename)) {
        Error0("file changed")
        ret = -GetErrno(Errno::FileChanged);
        buffer = nullptr;
        goto end;
    }
    buffer->resize(maxRead);
    {
        /* lock to read! (exclusive io) */
        std::lock_guard<std::mutex> lock(this->ioMutex);
        didread = ::read(fd, buffer->data(), maxRead);
    }
    // Chk if fail
    if (didread < 0) {
        int const e = errno;
        Error0("read fail: " << e)
        if (!e) {
            ret = -GetErrno(Errno::ReadError);
        } else {
            ret = -GetErrno(e);
        }
        buffer = nullptr;
        goto end;
    }
    // Success
    buffer->resize(didread);
    buffer->shrink_to_fit();
    ret = 0;
    }
end:
    return RType{ ret, buffer };
}
std::tuple<int32_t, uint64_t> File::traverse(
    std::function<bool(uint8_t const* const data, uint32_t const size)>
        didRead,
    uint64_t const eachRead0,
    int64_t const limit) noexcept
{
    using RT = std::tuple<int, uint64_t>;
    // Save total traversed bytes
    uint64_t total = 0;
    // Chk busy
    if (this->ioBusy) {
        Error0("busy")
        return RT{ -GetErrno(Errno::Busy), total };
    }
    // Make io busy
    IoBusyGuard iobusy(this->ioBusy);
    int ret;
    int primaryFd;
    std::string primaryFilename;
    {
        // Get primary fd and filename, incase changed
        BoostScopedReadLock readLock(this->fdRwlock);
        primaryFd = this->fd;
        primaryFilename = this->filename;
    }
    // Check fd i.e. chk open
    if (primaryFd < 0) {
        Error0("file not open")
        ret = -GetErrno(Errno::NotOpen);
        goto end;
    }
    {
        int64_t const fileSize = this->__size();
        if (fileSize < 0) {
            ret = int32_t(fileSize);
            goto end;
        }
        uint64_t maxRead;
        if ((limit < 0) || (limit > fileSize)) {
            maxRead = fileSize;
        } else {
            maxRead = limit;
        }
        uint64_t eachRead;
        if (eachRead0 <= 0) {
            eachRead = kFileIoUpperBound;
        } else if (eachRead0 > kMaxFileRdMem) {
            eachRead = kMaxFileRdMem;
        } else {
            eachRead = eachRead0;
        }
        if (eachRead > maxRead) {
            eachRead = maxRead;
        }
        uint8_t* buffer = nullptr;
        int64_t offset, paOffset, length;
        int fd;
        std::string filename;
        do {
            {
                // Get primary fd and filename again, incase changed
                BoostScopedReadLock readLock(this->fdRwlock);
                fd = this->fd;
                filename = this->filename;
            }
            // Check fd .. changed
            if ((fd != primaryFd) || (filename != primaryFilename)) {
                std::cerr << __func__ << ": file changed, " << __FILE__ << "+"
                    << __LINE__ << ".\n";
                ret = -GetErrno(Errno::FileChanged);
                goto end;
            }
            offset = total;
            // Offset for mmap() must be page aligned
            paOffset = offset & ~(::sysconf(_SC_PAGE_SIZE) - 1);
            if (eachRead <= (maxRead - total)) {
                length = eachRead;
            } else {
                length = maxRead - total;
            }
            {
                // Lock io to read! (exclusive io) */
                std::lock_guard<std::mutex> lock(this->ioMutex);
                buffer = reinterpret_cast<uint8_t*>(::mmap(
                    nullptr,
                    length + offset - paOffset,
                    PROT_READ,
                    MAP_PRIVATE,
                    fd,
                    paOffset));
            }
            // Chk if mmap fail
            if (MAP_FAILED == buffer) {
                int const e = errno;
                std::cerr << __func__ << ": mmap fail: " << e << ", "
                    << __FILE__ << "+" << __LINE__ << ".\n";
                if (!e) {
                    ret = -GetErrno(Errno::ReadError);
                } else {
                    ret = -GetErrno(e);
                }
                goto end;
            }
            // Success and accumulate total
            total += length;
            // Success and callback
            if (didRead(buffer, length)) {
                ::munmap(buffer, length + offset - paOffset);
                std::cout << __func__ << ": cancelled, " << __FILE__ << "+"
                    << __LINE__ << ".\n";
                ret = GetErrno(Errno::Cancelled);
                goto end;
            }
            ::munmap(buffer, length + offset - paOffset);
            // Check end
            if (0 == length) {
                ret = 0;
                break;
            }
        } while (total < maxRead);
    }
end:
    return RT{ ret, total };
}
std::tuple<int, size_t> File::read(
    std::function<bool(uint8_t const* const data, size_t const size)>
        didRead,
    size_t const eachRead0,
    ssize_t const limit) noexcept
{
    size_t total = 0;
    /* chk busy */
    if (this->ioBusy) {
        Warning("busy");
        return std::tuple<int, size_t>(-GetErrno(Errno::Busy), total);
    }
    IoBusyGuard iobusy(this->ioBusy);/* make io busy */
    (void)(iobusy);
    int ret, initfd;
    std::string initfilename;
    {
        /* get init */
        BoostScopedReadLock readLock(this->fdRwlock);
        initfd = this->fd;
        initfilename = this->filename;
    }
    /* check fd i.e. chk open */
    if (initfd < 0) {
        ret = -GetErrno(Errno::NotOpen);
        goto end;
    }
    {
    ssize_t const fileSize = this->__size();
    if (fileSize < 0) {
        ret = fileSize;
        goto end;
    }
    size_t maxRead;
    if ((limit < 0) || (limit > fileSize)) {
        maxRead = fileSize;
    } else {
        maxRead = limit;
    }
    size_t eachRead;
    if (eachRead0 <= 0) {
        eachRead = kFileIoUpperBound;
    } else if (eachRead0 > kMaxFileRdMem) {
        eachRead = kMaxFileRdMem;
    } else {
        eachRead = eachRead0;
    }
    if (eachRead > maxRead) {
        eachRead = maxRead;
    }
    SharedPtr<std::vector<uint8_t> > buffer;
    try {
        buffer.reset(new std::vector<uint8_t>());
        buffer->reserve(eachRead);
    } catch (std::exception const& e) {
        Error0("alloc buffer fail:  " << e.what())
        ret = -GetErrno(Errno::NoMem);
        goto end;
    }
    ssize_t didread;
    int fd;
    std::string filename;
    do {
        {
            // Get init incase changed
            BoostScopedReadLock readLock(this->fdRwlock);
            fd = this->fd;
            filename = this->filename;
        }
        // Check fd .. changed
        if ((fd != initfd) || (filename != initfilename)) {
            Error0("file changed")
            ret = -GetErrno(Errno::FileChanged);
            goto end;
        }
        buffer->resize(eachRead);
        {
            // Lock to read! (exclusive io)
            std::lock_guard<std::mutex> lock(this->ioMutex);
            if (eachRead <= (maxRead - total)) {
                didread = ::read(fd, buffer->data(), eachRead);
            } else {
                didread = ::read(fd, buffer->data(), maxRead - total);
            }
        }
        // Chk if fail
        if (didread < 0) {
            int const e = errno;
            Error0("read fail: " << e << " " << ::strerror(e)
                << " " << filename)
            if (!e) {
                ret = -GetErrno(Errno::ReadError);
            } else {
                ret = -GetErrno(e);
            }
            goto end;
        }
        // Success and acc total
        total += didread;
        // Success and callback
        buffer->resize(didread);
        if (didRead(buffer->data(), buffer->size())) {
            Warning("cancelled")
            ret = GetErrno(Errno::Cancelled);
            goto end;
        }
        // Check end
        if (0 == didread) {
            ret = 0;
            break;
        }
    } while (total < maxRead);
    }
end:
    this->jump2Offset(total);
    return std::tuple<int, size_t>(ret, total);
}
std::tuple<int32_t, uint64_t> File::write(
    void const* const data,
    uint64_t const maxWrite,
    int32_t const eachWrite0) noexcept
{
    using RT = std::tuple<int32_t, uint64_t>;
    uint64_t total = 0;
    if (!data) {
        Warning("invalid data")
        return RT{ -GetErrno(Errno::InvalidData), total };
    }
    // Chk io busy
    if (this->ioBusy) {
        Warning("busy")
        return RT{ -GetErrno(Errno::Busy), total };
    }
    // Make io busy
    IoBusyGuard iobusy(this->ioBusy);
    int ret;
    int primaryFd;
    std::string primaryFilename;
    {
        BoostScopedReadLock readLock(this->fdRwlock);
        primaryFd = this->fd;
        primaryFilename = this->filename;
    }
    // Check fd i.e. chk open
    if (primaryFd < 0) {
        ret = -GetErrno(Errno::NotOpen);
        goto end;
    }
    {
        uint64_t eachWrite;
        if (0 == eachWrite0) {
            eachWrite = kFileIoUpperBound;
        } else if (eachWrite0 < 0) {
            eachWrite = maxWrite;
        } else {
            eachWrite = eachWrite0;
        }
        if (eachWrite > maxWrite) {
            eachWrite = maxWrite;
        }
        int64_t didwrite;
        int fd;
        std::string filename;
        do {
            {
                BoostScopedReadLock readLock(this->fdRwlock);
                fd = this->fd;
                filename = this->filename;
            }
            // Check fd .. changed
            if ((fd != primaryFd) || (filename != primaryFilename)) {
                Error0("file changed")
                ret = -GetErrno(Errno::FileChanged);
                goto end;
            }
            {
                // Lock io to read! (exclusive io)
                std::lock_guard<std::mutex> lock(this->ioMutex);
                // Write!
                if (eachWrite <= (maxWrite - total)) {
                    didwrite = ::write(
                        fd,
                        reinterpret_cast<uint8_t const*>(data) + total,
                        eachWrite);
                } else {
                    didwrite = ::write(
                        fd,
                        reinterpret_cast<uint8_t const*>(data) + total,
                        maxWrite - total);
                }
            }
            // Chk if fail
            if (didwrite < 0) {
                int const e = errno;
                Error0("write fail")
                if (!e) {
                    ret = -GetErrno(Errno::ReadError);
                } else {
                    ret = -GetErrno(e);
                }
                goto end;
            }
            // Accumulate total
            total += didwrite;
            // Check end
            if (0 == didwrite) {
                ret = 0;
                break;
            }
        } while(total < maxWrite);
    }
end:
    return RT{ ret, total };
}
std::tuple<int, size_t> File::write(
    std::function<bool(size_t const wrote, size_t const rest)> didWrote,
    void const* const data,
    size_t const size,
    ssize_t const eachWrite0,
    ssize_t const limit) noexcept
{
    size_t total = 0;
    if (!data) {
        Warning("invalid data")
        return std::tuple<int, size_t>(-GetErrno(Errno::InvalidData), total);
    }
    // Chk busy
    if (this->ioBusy) {
        Warning("busy")
        return std::tuple<int, size_t>(-GetErrno(Errno::Busy), total);
    }
    IoBusyGuard iobusy(this->ioBusy);// make io busy
    (void)(iobusy);
    int ret, initfd;
    std::string initfilename;
    {
        // Get init
        BoostScopedReadLock readLock(this->fdRwlock);
        initfd = this->fd;
        initfilename = this->filename;
    }
    // Check fd i.e. chk open
    if (initfd < 0) {
        ret = -GetErrno(Errno::NotOpen);
        goto end;
    }
    {
    size_t maxWrite;
    if ((limit < 0) || (static_cast<size_t>(limit) > size)) {
        maxWrite = size;
    } else {
        maxWrite = limit;
    }
    size_t eachWrite;
    if (0 == eachWrite0) {
        eachWrite = kFileIoUpperBound;
    } else if (eachWrite0 < 0) {
        eachWrite = maxWrite;
    } else {
        eachWrite = eachWrite0;
    }
    if (eachWrite > maxWrite) {
        eachWrite = maxWrite;
    }
    ssize_t didwrite;
    int fd;
    std::string filename;
    do {
        {
            // Get init incase changed
            BoostScopedReadLock readLock(this->fdRwlock);
            fd = this->fd;
            filename = this->filename;
        }
        // Check fd .. changed
        if ((fd != initfd) || (filename != initfilename)) {
            Error0("file changed")
            ret = -GetErrno(Errno::FileChanged);
            goto end;
        }
        {
            // Lock to read! (exclusive io)
            std::lock_guard<std::mutex> lock(this->ioMutex);
            /* write! */
            if (eachWrite <= (maxWrite - total)) {
                didwrite = ::write(
                    fd,
                    reinterpret_cast<uint8_t const*>(data) + total,
                    eachWrite);
            } else {
                didwrite = ::write(
                    fd,
                    reinterpret_cast<uint8_t const*>(data) + total,
                    maxWrite - total);
            }
        }
        // Chk if fail
        if (didwrite < 0) {
            int const e = errno;
            Error0("write fail: " << e)
            if (!e) {
                ret = -GetErrno(Errno::ReadError);
            } else {
                ret = -GetErrno(e);
            }
            goto end;
        }
        // Acc total
        total += didwrite;
        // Succes and callback
        if (didWrote(total, maxWrite - total)) {
            Warning("cancelled")
            ret = GetErrno(Errno::Cancelled);
            goto end;
        }
        // Check end
        if (0 == didwrite) {
            ret = 0;
            break;
        }
    } while (total < maxWrite);
    }
end:
    return std::tuple<int, size_t>(ret, total);
}
#if 0
static int64_t ConsumeStream(
    FILE* const stream,
    std::function<bool(void const* const data, uint32_t const size)> const&
        freadConsumer,
    int64_t maxRead = -1)
{
    if ((!stream) || (!freadConsumer)) {
        return -EINVAL;
    }
    uint64_t rd;
    /* Read */
    {
        int32_t r;
        std::vector<uint8_t> buf(kPerReadBytes);
        rd = 0;
        do {
            ::memset(buf.data(), 0, buf.size());
            if ((maxRead < 0) || (maxRead - rd) >= kPerReadBytes) {
                r = ::fread(buf.data(), 1, kPerReadBytes, stream);
            } else if ((maxRead - rd) > 0) {
                r = ::fread(buf.data(), 1, maxRead - rd, stream);
            } else {
                break;
            }
            rd += r;
            if (r > 0) {
                if (freadConsumer(buf.data(), r)) {
                    break;
                }
            }
            if (::feof(stream)) {
                break;
            }
        } while(r > 0);
    }
    return rd;// OK
}
#endif
}
