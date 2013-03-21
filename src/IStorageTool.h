/*
 Project: Console-Secrets
 Copyright (C) 2013  Simone Pagan Griso
 Distributed WITHOUT ANY WARRANTY under the GPL-3.0 licence,
 see the LICENCE file or visit <http://www.gnu.org/licenses/>
 File: IStorageTool.h
 Description: Interface for physical load/storage of data
 Last Modified: $Id: IStorageTool.h 54 2013-03-20 21:54:57Z gridge $
*/

#ifndef __ISTORAGE_TOOL__
#define __ISTORAGE_TOOL__

#include "ILog.h"
#include "SourceURI.h"

/* Interface for physical load/storage of data.
 * Define basic function to read/write a source of data.
 * Operate on raw data, in the format of a std::string.
 */
class IStorageTool : public IErrorHandler {
 protected:
  /* identifier of the source. 
   * For format see SourceURI class.
   * If you need to change source, create a new IStorage instance.
   */
  SourceURI m_source;

 public:
  IStorageTool(std::string pName, SourceURI pSource);
  ~IStorageTool();
  
  /* Store data to m_source.
   * @param pData data to store
   * @return status of operation
   */
  virtual StatusCode Store(std::string &pData) = 0;

  /* Load data from m_source.
   * @param reference to string where to store data
   * @return  status of operation
   */
  virtual StatusCode Load(std::string &pData) = 0;

  /* Check if source exists already.
   * @return SC_OK if source exists
   * @return SC_ERROR if an error occurred while checking
   * @return SC_NOT_FOUND if the source does not exists
   * @return SC_NOT_IMPLEMENTED if can't tell
   */
  virtual StatusCode StorageExists();

  // --- Accessors
  SourceURI GetSource();
};

#endif
