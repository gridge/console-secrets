/*
 Project: Console-Secrets
 Copyright (C) 2013  Simone Pagan Griso
 Distributed WITHOUT ANY WARRANTY under the GPL-3.0 licence,
 see the LICENCE file or visit <http://www.gnu.org/licenses/>
 File: IErrorHandler.h
 Description: Base class for handling return codes and error messages.
 Last Modified: $Id: IErrorHandler.h 54 2013-03-20 21:54:57Z gridge $
*/

#ifndef __IERROR_HANDLER__
#define __IERROR_HANDLER__

#include <string>

class ILog;

/** Base class for handling return codes and error messages.
 * This class is also uses to identify services and tools by name
 * by the logger (ILog).
 * All tools/services derives from this to allow proper error handling
 */
class IErrorHandler {
 public:
  /// Common return codes for Services/Tools methods
  enum StatusCodes {
    SC_OK, ///< Action executed succesfully
    SC_WARNING, ///< Action executed, but non-standard adjustement had to be made
    SC_ERROR, ///< Action not succeded
    SC_NOT_IMPLEMENTED, ///< Action not implemented
    SC_NOT_FOUND, ///< Object not found
    SC_ABORT, ///< Request abort of current operation (typically by user-request)
    SC_FATAL ///< Request abort of program
  };
  typedef StatusCodes StatusCode; ///< define default return code type for Services/Tools methods
 protected:
  std::string m_errorMsg; ///< Store eventual error/warning Message
  StatusCodes m_statusCode; ///< Current status code of the service/tool
  std::string m_name; ///< Name of service/tool
 public:
  IErrorHandler(std::string pName);
  virtual ~IErrorHandler();

  /// Return and clear Error Message
  StatusCode GetErrorMsg(std::string &pMsg);
  /// Return and clear Error Message
  std::string GetErrorMsg();

  /// Friend class to set fast error Message
  friend class ILog;

};

#endif
