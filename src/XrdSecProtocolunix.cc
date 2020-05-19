/******************************************************************************/
/*                                                                            */
/*                 X r d S e c P r o t o c o l u n i x . c c                  */
/*                                                                            */
/* (c) 2007 by the Board of Trustees of the Leland Stanford, Jr., University  */
/*                            All Rights Reserved                             */
/*   Produced by Andrew Hanushevsky for Stanford University under contract    */
/*              DE-AC02-76-SFO0515 with the Department of Energy              */
/*                                                                            */
/* This file is part of the XRootD software suite.                            */
/*                                                                            */
/* XRootD is free software: you can redistribute it and/or modify it under    */
/* the terms of the GNU Lesser General Public License as published by the     */
/* Free Software Foundation, either version 3 of the License, or (at your     */
/* option) any later version.                                                 */
/*                                                                            */
/* XRootD is distributed in the hope that it will be useful, but WITHOUT      */
/* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or      */
/* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public       */
/* License for more details.                                                  */
/*                                                                            */
/* You should have received a copy of the GNU Lesser General Public License   */
/* along with XRootD in a file called COPYING.LESSER (LGPL license) and file  */
/* COPYING (GPL license).  If not, see <http://www.gnu.org/licenses/>.        */
/*                                                                            */
/* The copyright holder's institutional names and contributor's names may not */
/* be used to endorse or promote products derived from this software without  */
/* specific prior written permission of the institution or contributor.       */
/******************************************************************************/

#include <unistd.h>
#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/types.h>

#include "XrdVersion.hh"

#include "XrdNet/XrdNetAddrInfo.hh"
#include "XrdOuc/XrdOucErrInfo.hh"
#include "XrdOuc/XrdOucUtils.hh"
#include "XrdSys/XrdSysHeaders.hh"
#include "XrdSys/XrdSysPthread.hh"
#include "XrdSec/XrdSecInterface.hh"

/******************************************************************************/
/*              X r d S e c P r o t o c o l u n i x   C l a s s               */
/******************************************************************************/

class XrdSecProtocolanon : public XrdSecProtocol
{
public:
        int                Authenticate  (XrdSecCredentials *cred,
                                          XrdSecParameters **parms,
                                          XrdOucErrInfo     *einfo=0);

        XrdSecCredentials *getCredentials(XrdSecParameters  *parm=0,
                                          XrdOucErrInfo     *einfo=0);

        XrdSecProtocolanon(const char *hname, XrdNetAddrInfo &endPoint)
                          : XrdSecProtocol("unix")
                          {Entity.host = strdup(hname);
                           Entity.name = strdup("?");
                           epAddr      = endPoint;
                           Entity.addrInfo = &epAddr;
                          }

        void              Delete() {delete this;}

private:

       ~XrdSecProtocolanon() {if (Entity.host) free(Entity.host);
                             }

XrdNetAddrInfo            epAddr;
};

/******************************************************************************/
/*             C l i e n t   O r i e n t e d   F u n c t i o n s              */
/******************************************************************************/
/******************************************************************************/
/*                        g e t C r e d e n t i a l s                         */
/******************************************************************************/


// This is just a stub implementation of the unix protocol; it should never
// be used by a client!
XrdSecCredentials *XrdSecProtocolanon::getCredentials(XrdSecParameters * /*noparm*/,
                                                      XrdOucErrInfo    * /*error*/)
{
   return nullptr;
}

/******************************************************************************/
/*               S e r v e r   O r i e n t e d   M e t h o d s                */
/******************************************************************************/
/******************************************************************************/
/*                          A u t h e n t i c a t e                           */
/******************************************************************************/

int XrdSecProtocolanon::Authenticate(XrdSecCredentials *cred,
                                     XrdSecParameters **parms,
                                     XrdOucErrInfo     *erp)
{
// Check if we have any credentials or if no credentials really needed.
// In either case, use host name as client name
//
   if (cred->size <= int(4) || !cred->buffer)
      {strncpy(Entity.prot, "host", sizeof(Entity.prot));
       Entity.name = const_cast<char *>("?");
       return 0;
      }

// Check if this is our protocol
//
   if (strcmp(cred->buffer, "unix"))
      {char msg[256];
       snprintf(msg, sizeof(msg),
                "Secunix: Authentication protocol id mismatch (unix != %.4s).",
                cred->buffer);
       if (erp) erp->setErrInfo(EINVAL, msg);
          else cerr <<msg <<endl;
       return -1;
      }

// All done
//
   return 0;
}
  
/******************************************************************************/
/*                X r d S e c p r o t o c o l u n i x I n i t                 */
/******************************************************************************/
  
extern "C"
{
char  *XrdSecProtocolunixInit(const char     mode,
                              const char    *parms,
                              XrdOucErrInfo *erp)
{
   return (char *)"";
}
}

/******************************************************************************/
/*              X r d S e c P r o t o c o l u n i x O b j e c t               */
/******************************************************************************/

XrdVERSIONINFO(XrdSecProtocolunixObject,secunix);
  
extern "C"
{
XrdSecProtocol *XrdSecProtocolunixObject(const char              mode,
                                         const char             *hostname,
                                               XrdNetAddrInfo   &endPoint,
                                         const char             *parms,
                                               XrdOucErrInfo    *erp)
{
   XrdSecProtocolanon *prot;

// Return a new protocol object
//
   if (!(prot = new XrdSecProtocolanon(hostname, endPoint)))
      {const char *msg = "Secanon: Insufficient memory for protocol.";
       if (erp) erp->setErrInfo(ENOMEM, msg);
          else cerr <<msg <<endl;
       return (XrdSecProtocol *)0;
      }

// All done
//
   return prot;
}
}
