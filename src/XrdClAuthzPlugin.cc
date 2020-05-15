
#include "XrdCl/XrdClPlugInInterface.hh"
#include "XrdCl/XrdClFile.hh"
#include "XrdCl/XrdClURL.hh"
#include "XrdVersion.hh"

using namespace XrdCl;

XrdVERSIONINFO(XrdClGetPlugIn, XrdClAuthzPlugIn)

namespace AuthzPlugIn
{

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
    const char *token = getenv("BEARER_TOKEN");
    if (token) {
        URL::ParamsMap pmap = parsed_url.GetParams();
        auto iter = pmap.find("authz");
        if (iter == pmap.end()) {
            pmap["authz"] = std::string("Bearer ") + token;
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

  //----------------------------------------------------------------------------
  //! An interface for file plug-ins
  //----------------------------------------------------------------------------
  class FileSystemPlugIn
  {
    public:
      //------------------------------------------------------------------------
      //! Destructor
      //------------------------------------------------------------------------
      virtual ~FileSystemPlugIn() {}

      //------------------------------------------------------------------------
      //! @see XrdCl::FileSystem::Locate
      //------------------------------------------------------------------------
      virtual XRootDStatus Locate( const std::string &path,
                                   OpenFlags::Flags   flags,
                                   ResponseHandler   *handler,
                                   uint16_t           timeout )
      {
        (void)path; (void)flags; (void)handler; (void)timeout;
        return XRootDStatus( stError, errNotImplemented );
      }

      //------------------------------------------------------------------------
      //! @see XrdCl::FileSystem::DeepLocate
      //------------------------------------------------------------------------
      virtual XRootDStatus DeepLocate( const std::string &path,
                                       OpenFlags::Flags   flags,
                                       ResponseHandler   *handler,
                                       uint16_t           timeout )
      {
        (void)path; (void)flags; (void)handler; (void)timeout;
        return XRootDStatus( stError, errNotImplemented );
      }

      //------------------------------------------------------------------------
      //! @see XrdCl::FileSystem::Mv
      //------------------------------------------------------------------------
      virtual XRootDStatus Mv( const std::string &source,
                               const std::string &dest,
                               ResponseHandler   *handler,
                               uint16_t           timeout )
      {
        (void)source; (void)dest; (void)handler; (void)timeout;
        return XRootDStatus( stError, errNotImplemented );
      }

      //------------------------------------------------------------------------
      //! @see XrdCl::FileSystem::Query
      //------------------------------------------------------------------------
      virtual XRootDStatus Query( QueryCode::Code  queryCode,
                                  const Buffer    &arg,
                                  ResponseHandler *handler,
                                  uint16_t         timeout )
      {
        (void)queryCode; (void)arg; (void)handler; (void)timeout;
        return XRootDStatus( stError, errNotImplemented );
      }

      //------------------------------------------------------------------------
      //! @see XrdCl::FileSystem::Truncate
      //------------------------------------------------------------------------
      virtual XRootDStatus Truncate( const std::string &path,
                                     uint64_t           size,
                                     ResponseHandler   *handler,
                                     uint16_t           timeout )
      {
        (void)path; (void)size; (void)handler; (void)timeout;
        return XRootDStatus( stError, errNotImplemented );
      }

      //------------------------------------------------------------------------
      //! @see XrdCl::FileSystem::Rm
      //------------------------------------------------------------------------
      virtual XRootDStatus Rm( const std::string &path,
                               ResponseHandler   *handler,
                               uint16_t           timeout )
      {
        (void)path; (void)handler; (void)timeout;
        return XRootDStatus( stError, errNotImplemented );
      }

      //------------------------------------------------------------------------
      //! @see XrdCl::FileSystem::MkDir
      //------------------------------------------------------------------------
      virtual XRootDStatus MkDir( const std::string &path,
                                  MkDirFlags::Flags  flags,
                                  Access::Mode       mode,
                                  ResponseHandler   *handler,
                                  uint16_t           timeout )
      {
        (void)path; (void)flags; (void)mode; (void)handler; (void)timeout;
        return XRootDStatus( stError, errNotImplemented );
      }

      //------------------------------------------------------------------------
      //! @see XrdCl::FileSystem::RmDir
      //------------------------------------------------------------------------
      virtual XRootDStatus RmDir( const std::string &path,
                                  ResponseHandler   *handler,
                                  uint16_t           timeout )
      {
        (void)path; (void)handler; (void)timeout;
        return XRootDStatus( stError, errNotImplemented );
      }

      //------------------------------------------------------------------------
      //! @see XrdCl::FileSystem::ChMod
      //------------------------------------------------------------------------
      virtual XRootDStatus ChMod( const std::string &path,
                                  Access::Mode       mode,
                                  ResponseHandler   *handler,
                                  uint16_t           timeout )
      {
        (void)path; (void)mode; (void)handler; (void)timeout;
        return XRootDStatus( stError, errNotImplemented );
      }

      //------------------------------------------------------------------------
      //! @see XrdCl::FileSystem::Ping
      //------------------------------------------------------------------------
      virtual XRootDStatus Ping( ResponseHandler *handler,
                                 uint16_t         timeout )
      {
        (void)handler; (void)timeout;
        return XRootDStatus( stError, errNotImplemented );
      }

      //------------------------------------------------------------------------
      //! @see XrdCl::FileSystem::Stat
      //------------------------------------------------------------------------
      virtual XRootDStatus Stat( const std::string &path,
                                 ResponseHandler   *handler,
                                 uint16_t           timeout )
      {
        (void)path; (void)handler; (void)timeout;
        return XRootDStatus( stError, errNotImplemented );
      }

      //------------------------------------------------------------------------
      //! @see XrdCl::FileSystem::StatVFS
      //------------------------------------------------------------------------
      virtual XRootDStatus StatVFS( const std::string &path,
                                    ResponseHandler   *handler,
                                    uint16_t           timeout )
      {
        (void)path; (void)handler; (void)timeout;
        return XRootDStatus( stError, errNotImplemented );
      }

      //------------------------------------------------------------------------
      //! @see XrdCl::FileSystem::Protocol
      //------------------------------------------------------------------------
      virtual XRootDStatus Protocol( ResponseHandler *handler,
                                     uint16_t         timeout = 0 )
      {
        (void)handler; (void)timeout;
        return XRootDStatus( stError, errNotImplemented );
      }

      //------------------------------------------------------------------------
      //! @see XrdCl::FileSystem::DirlList
      //------------------------------------------------------------------------
      virtual XRootDStatus DirList( const std::string   &path,
                                    DirListFlags::Flags  flags,
                                    ResponseHandler     *handler,
                                    uint16_t             timeout )
      {
        (void)path; (void)flags; (void)handler; (void)timeout;
        return XRootDStatus( stError, errNotImplemented );
      }

      //------------------------------------------------------------------------
      //! @see XrdCl::FileSystem::SendInfo
      //------------------------------------------------------------------------
      virtual XRootDStatus SendInfo( const std::string &info,
                                     ResponseHandler   *handler,
                                     uint16_t           timeout )
      {
        (void)info; (void)handler; (void)timeout;
        return XRootDStatus( stError, errNotImplemented );
      }

      //------------------------------------------------------------------------
      //! @see XrdCl::FileSystem::Prepare
      //------------------------------------------------------------------------
      virtual XRootDStatus Prepare( const std::vector<std::string> &fileList,
                                    PrepareFlags::Flags             flags,
                                    uint8_t                         priority,
                                    ResponseHandler                *handler,
                                    uint16_t                        timeout )
      {
        (void)fileList; (void)flags; (void)priority; (void)handler;
        (void)timeout;
        return XRootDStatus( stError, errNotImplemented );
      }

      //------------------------------------------------------------------------
      //! @see XrdCl::FileSystem::SetProperty
      //------------------------------------------------------------------------
      virtual bool SetProperty( const std::string &name,
                                const std::string &value )
      {
        (void)name; (void)value;
        return false;
      }

      //------------------------------------------------------------------------
      //! @see XrdCl::FileSystem::GetProperty
      //------------------------------------------------------------------------
      virtual bool GetProperty( const std::string &name,
                                std::string &value ) const
      {
        (void)name; (void)value;
        return false;
      }
  };

class Factory
{
    public:
      //------------------------------------------------------------------------
      //! Destructor
      //------------------------------------------------------------------------
      virtual ~Factory() {}

      //------------------------------------------------------------------------
      //! Create a file plug-in for the given URL
      //------------------------------------------------------------------------
      virtual FilePlugIn *CreateFile( const std::string &url ) {return new File();}

      //------------------------------------------------------------------------
      //! Create a file system plug-in for the given URL
      //------------------------------------------------------------------------
      virtual FileSystemPlugIn *CreateFileSystem( const std::string &url ) {return nullptr;}
};

}


extern "C"
void *XrdClGetPlugIn( const void *arg )
{
    (void)arg;
    return new AuthzPlugIn::Factory();
}
