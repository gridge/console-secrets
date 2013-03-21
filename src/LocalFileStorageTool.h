/*
 Project: Console-Secrets
 Copyright (C) 2013  Simone Pagan Griso
 Distributed WITHOUT ANY WARRANTY under the GPL-3.0 licence,
 see the LICENCE file or visit <http://www.gnu.org/licenses/>
 File: LocalFileStorageTool.h
 Description: Implementation of IStorageTool for local file load/storage of data
 Last Modified: $Id: LocalFileStorageTool.h 54 2013-03-20 21:54:57Z gridge $
*/

#ifndef __LOCALFILESTORAGE_TOOL__
#define __LOCALFILESTORAGE_TOOL__

#include "IStorageTool.h"

#include <fstream>

/** Implementation of IStorageTool for local file load/storage of data.
 * Implement IO from file using fstream.
 */
class LocalFileStorageTool : public IStorageTool {
 protected:
  /// physical file
  std::fstream m_file;

  // --- Helper members
  /// Check if file exists
  bool FileExists(std::string strFilename);

  /// Get local file name from sourceURI
  std::string GetLocalFileName(SourceURI pSource);
 public:
  LocalFileStorageTool(std::string pName, SourceURI pSource);
  ~LocalFileStorageTool();

  /** Store to file.
   * See IStorageTool::Store for parameters.
   * Overwrites previous data.
   */
  virtual StatusCode Store(std::string &pData);  

  /** Load from file.
   * See IStorageTool::Load for parameters.
   */
  virtual StatusCode Load(std::string &pData);

  /** Check if storage exists. */
  virtual StatusCode StorageExists();

};

#endif
