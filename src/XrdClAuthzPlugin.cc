
#include <fcntl.h>

#include "XrdCl/XrdClPlugInInterface.hh"
#include "XrdCl/XrdClFile.hh"
#include "XrdCl/XrdClURL.hh"
#include "XrdVersion.hh"

using namespace XrdCl;

XrdVERSIONINFO(XrdClGetPlugIn, XrdClAuthzPlugIn)

namespace AuthzPlugIn
{

std::string
NormalizeToken(const std::string &input_token)
{
    static const std::string whitespace = " \t\f\n\v\r";
    static const std::string nonheader_whitespace = "\r\n";
    auto begin = input_token.find_first_not_of(whitespace);
    if (begin == std::string::npos) {return "";}

    std::string token = input_token.substr(begin);
    auto end = token.find_last_not_of(whitespace);
    token = token.substr(0, end + 1);

        // If non-permitted header characters are present ("\r\n"),
        // then this token is not permitted.
    if (token.find(nonheader_whitespace) != std::string::npos) {return "";}

    return token;
}

std::string
FindTokenInFile(const std::string &token_file)
{
    #ifdef DEBUG
        printf("Looking for token in file %s\n", token_file.c_str());
    #endif
    int fd = open(token_file.c_str(), O_RDONLY);
    if (fd == -1) {
        return "";
    }

    static const size_t max_size = 16384;

    std::vector<char> input_buffer;
    input_buffer.reserve(max_size);

    ssize_t cur_size = 0;

    ssize_t retval;
    do {
        retval = read(fd, &input_buffer[cur_size], max_size - cur_size);
        if (retval != -1) cur_size += retval;
    } while ((retval > 0) || ((retval == -1) && (errno == EAGAIN || errno == EINTR)));

    close(fd);

    if (retval == -1) {
        return "";
    }

    std::string token(&input_buffer[0], cur_size);

    return NormalizeToken(token);
}

std::string
DiscoverToken()
{
    const char *bearer_token = getenv("BEARER_TOKEN");
    std::string token;
    if (bearer_token && *bearer_token &&
        !(token = NormalizeToken(bearer_token)).empty())
     {
        return token;
    }

    const char *bearer_token_file = getenv("BEARER_TOKEN_FILE");
    if (bearer_token_file &&
        !(token = FindTokenInFile(bearer_token_file)).empty())
    {
        return token;
    }

    uid_t euid = geteuid();
    std::string fname = "/bt_u";
    fname += std::to_string(euid);

    const char *xdg_runtime_dir = getenv("XDG_RUNTIME_DIR");
    if (xdg_runtime_dir) {
        std::string xdg_token_file = std::string(xdg_runtime_dir) + fname;
        if (!(token = FindTokenInFile(xdg_token_file)).empty()) {
            return token;
        }
    }

    return FindTokenInFile("/tmp" + fname);
}

std::string
CustomizeURL(const std::string &input_url)
{
    URL parsed_url(input_url);
    const char *xcache_host = getenv("XCACHE_HOST");
    const char *xcache_port = getenv("XCACHE_PORT");
    if (parsed_url.GetHostName() == "xcache" && xcache_host)
    {
        parsed_url.SetHostName(xcache_host);
        if (xcache_port) {
            int port = 0;
            try {
                port = std::stol(xcache_port);
            } catch (...) {}
            if (port) parsed_url.SetPort(port);
        }
    }
    auto token = DiscoverToken();
    if (!token.empty()) {
        URL::ParamsMap pmap = parsed_url.GetParams();
        auto iter = pmap.find("authz");
        if (iter == pmap.end()) {
            pmap["authz"] = std::string("Bearer%20") + token;
            parsed_url.SetParams(pmap);
        }
    }
    return parsed_url.GetURL();
}

class File : public XrdCl::FilePlugIn
{
public:
    File() : m_file(false) {}
    virtual ~File() {}

    virtual
    XRootDStatus Open(
        const std::string &url,
        OpenFlags::Flags   flags,
        Access::Mode       mode,
        ResponseHandler   *handler,
        uint16_t           timeout)
    override
    {
        return m_file.Open(CustomizeURL(url), flags, mode, handler, timeout);
    }

    virtual
    XRootDStatus Close(
        ResponseHandler *handler,
        uint16_t         timeout)
    override
    {
        return m_file.Close(handler, timeout);
    }

    virtual XRootDStatus Stat(
        bool             force,
        ResponseHandler *handler,
        uint16_t         timeout)
    override
    {
        return m_file.Stat(force, handler, timeout);
    }

    virtual XRootDStatus Read(
        uint64_t         offset,
        uint32_t         size,
        void            *buffer,
        ResponseHandler *handler,
        uint16_t         timeout)
    override
    {
        return m_file.Read(offset, size, buffer, handler, timeout);
    }

    virtual XRootDStatus Write(
        uint64_t         offset,
        uint32_t         size,
        const void      *buffer,
        ResponseHandler *handler,
        uint16_t         timeout)
    override
    {
        return m_file.Write(offset, size, buffer, handler, timeout);
    }

    virtual XRootDStatus Sync(
        ResponseHandler *handler,
        uint16_t         timeout)
    override
    {
        return m_file.Sync(handler, timeout);
    }

    virtual XRootDStatus Truncate(
        uint64_t         size,
        ResponseHandler *handler,
        uint16_t         timeout)
    override
    {
        return m_file.Truncate(size, handler, timeout);
    }

    virtual XRootDStatus VectorRead(
        const ChunkList &chunks,
        void            *buffer,
        ResponseHandler *handler,
        uint16_t         timeout)
    override
    {
        return m_file.VectorRead(chunks, buffer, handler, timeout);
    }

    virtual XRootDStatus Fcntl(
        const Buffer    &arg,
        ResponseHandler *handler,
        uint16_t         timeout)
    override
    {
        return m_file.Fcntl(arg, handler, timeout);
    }

    virtual XRootDStatus Visa(
        ResponseHandler *handler,
        uint16_t         timeout)
    override
    {
        return m_file.Visa(handler, timeout);
    }

    virtual bool IsOpen() const override
    {
        return m_file.IsOpen();
    }

    virtual bool SetProperty(
        const std::string &name,
        const std::string &value)
    {
        return m_file.SetProperty(name, value);
    }

    virtual bool GetProperty(
        const std::string &name,
              std::string &value) const
    override
    {
        return m_file.GetProperty(name, value);
    }

private:
    XrdCl::File m_file;
};

class FileSystem : public XrdCl::FileSystemPlugIn
{
public:

    FileSystem(const std::string &url) : m_fs(CustomizeURL(url)) {}

    virtual ~FileSystem() {}

    virtual XRootDStatus Locate(
        const std::string &path,
        OpenFlags::Flags   flags,
        ResponseHandler   *handler,
        uint16_t           timeout)
    override
    {
        return m_fs.Locate(path, flags, handler, timeout);
    }

    virtual XRootDStatus Mv(
        const std::string &source,
        const std::string &dest,
        ResponseHandler   *handler,
        uint16_t           timeout)
    override
    {
        return m_fs.Mv(source, dest, handler, timeout);
    }

    virtual XRootDStatus Query(
        QueryCode::Code  queryCode,
        const Buffer    &arg,
        ResponseHandler *handler,
        uint16_t         timeout)
    override
    {
        return m_fs.Query(queryCode, arg, handler, timeout);
    }

    virtual XRootDStatus Truncate(
        const std::string &path,
        uint64_t           size,
        ResponseHandler   *handler,
        uint16_t           timeout)
    override
    {
        return m_fs.Truncate(path, size, handler, timeout);
    }

    virtual XRootDStatus Rm(
        const std::string &path,
        ResponseHandler   *handler,
        uint16_t           timeout)
    override
    {
        return m_fs.Rm(path, handler, timeout);
    }

    virtual XRootDStatus MkDir(
        const std::string &path,
        MkDirFlags::Flags  flags,
        Access::Mode       mode,
        ResponseHandler   *handler,
        uint16_t           timeout)
    override
    {
        return m_fs.MkDir(path, flags, mode, handler, timeout);
    }

    virtual XRootDStatus RmDir(
        const std::string &path,
        ResponseHandler   *handler,
        uint16_t           timeout)
    override
    {
        return m_fs.RmDir(path, handler, timeout);
    }

    virtual XRootDStatus ChMod(
        const std::string &path,
        Access::Mode       mode,
        ResponseHandler   *handler,
        uint16_t           timeout)
    override
    {
        return m_fs.ChMod(path, mode, handler, timeout);
    }

    virtual XRootDStatus Ping(
        ResponseHandler *handler,
        uint16_t         timeout)
    override
    {
        return m_fs.Ping(handler, timeout);
    }

    virtual XRootDStatus Stat(
        const std::string &path,
        ResponseHandler   *handler,
        uint16_t           timeout)
    {
        return m_fs.Stat(path, handler, timeout);
    }

    virtual XRootDStatus StatVFS(
        const std::string &path,
        ResponseHandler   *handler,
        uint16_t           timeout)
    {
        return m_fs.StatVFS(path, handler, timeout);
    }

    virtual XRootDStatus Protocol(
        ResponseHandler *handler,
        uint16_t         timeout = 0)
    override
    {
        return m_fs.Protocol(handler, timeout);
    }

    virtual XRootDStatus DirList(
        const std::string   &path,
        DirListFlags::Flags  flags,
        ResponseHandler     *handler,
        uint16_t             timeout)
    override
    {
        return m_fs.DirList(path, flags, handler, timeout);
    }

    virtual XRootDStatus SendInfo(
        const std::string &info,
        ResponseHandler   *handler,
        uint16_t           timeout)
    override
    {
        return m_fs.SendInfo(info, handler, timeout);
    }

    virtual XRootDStatus Prepare(
        const std::vector<std::string> &fileList,
        PrepareFlags::Flags             flags,
        uint8_t                         priority,
        ResponseHandler                *handler,
        uint16_t                        timeout)
    override
    {
        return m_fs.Prepare(fileList, flags, priority, handler, timeout);
    }

    virtual bool SetProperty(
        const std::string &name,
        const std::string &value)
    override
    {
        return m_fs.SetProperty(name, value);
    }

    virtual bool GetProperty(
        const std::string &name,
        std::string &value) const
    override
    {
        return m_fs.GetProperty(name, value);
    }

private:
    XrdCl::FileSystem m_fs;
};

class Factory
{
std::unique_ptr<FileSystemPlugIn> CurFS;

public:


    virtual ~Factory() {}

    virtual FilePlugIn *CreateFile( const std::string &url ) {return new File();}

    virtual FileSystemPlugIn *CreateFileSystem( const std::string &url ) {
      if (!CurFS)
         CurFS.reset(new FileSystem(url));
      return CurFS.get();
   }
};

}


extern "C"
void *XrdClGetPlugIn( const void *arg )
{
    (void)arg;
    return new AuthzPlugIn::Factory();
}
