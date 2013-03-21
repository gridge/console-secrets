/*
 Project: Console-Secrets
 Copyright (C) 2013  Simone Pagan Griso
 Distributed WITHOUT ANY WARRANTY under the GPL-3.0 licence,
 see the LICENCE file or visit <http://www.gnu.org/licenses/>
 File: IdManagerTool.h
 Description: Manage id <--> source connections
 Last Modified: $Id: IdManagerTool.h 54 2013-03-20 21:54:57Z gridge $
*/

#ifndef __IDMANAGER_TOOL__
#define __IDMANAGER_TOOL__

#include "IErrorHandler.h"

#include <string>
#include <vector>
#include <map>

/** Tool for managing id <--> source connections
 * Provides unique id for new records, allows association Id <--> source.
 */
class IdManagerTool : public IErrorHandler {
 protected:
  typedef std::map<unsigned long, std::string> idDataType;
  ///map to store id <--> source associations
  std::map<unsigned long, std::string> m_idData;  
 public:
  IdManagerTool(std::string pName);
  ~IdManagerTool();

  /** Get new Id for the source pSource.
   * @return new Id, returns 0 if not succesfull.
   */
  virtual unsigned long GetNewId(std::string pSource);

  /** Determine which Source is associated with id pId.
   * @param pId Id of the source to be searched
   * @return source associated. Empty string if not found.
   */
  virtual std::string GetSource(unsigned long pId);

  /// Get list of id associated with source pSource
  virtual std::vector<unsigned long> GetIdList(std::string pSource);

  /// Free Id
  virtual StatusCode FreeId(unsigned long pId);
  
  /// Free all Id connected with source pSource
  virtual StatusCode FreeIdBySource(std::string pSource);
};

#endif
