/*
 Project: Console-Secrets
 Copyright (C) 2013  Simone Pagan Griso
 Distributed WITHOUT ANY WARRANTY under the GPL-3.0 licence,
 see the LICENCE file or visit <http://www.gnu.org/licenses/>
 File: IFormatterTool.h
 Description: Interface for data formatter tools
 Last Modified: $Id: IFormatterTool.h 54 2013-03-20 21:54:57Z gridge $
*/

#ifndef __IFORMATTER_TOOL__
#define __IFORMATTER_TOOL__

#include <vector>

#include "ILog.h"
#include "ARecord.h"

/** Interface for data formatter tools
 * Format data from/to std::vector<ARecord *> to/from std::string to be 
 * written/read by an IStorageTool instance.
 */
class IFormatterTool : public IErrorHandler {
 protected:
  /** Stores the format applied.
   * Initialized in the constructor, it depends on the derived-classes implementations
   * and specify the format applied to input data.
   */
  std::string m_format;
 public:
  IFormatterTool(std::string pName);
  IFormatterTool(std::string pName, std::string pFormat);
  ~IFormatterTool();

  /** Format vector of records in a string.
   * Streams the vector of ARecord in a string ready to be processed and sent to persistent storage.
   * @param pData input vector of records
   * @param pFormattedString output formatted string
   * @param pBruteForce ignore errors and try to code what we can
   * @return StatusCode of the operation. If failed and not pBruteForce, pFormattedString is empty.
   */
  virtual StatusCode Code(std::vector<ARecord *> pData, std::string &pFormattedString, int pBruteForce=0) = 0;

  /** Read a formatted string and decode it in vector of records.
   * Streams a given string in a vector of ARecord elements.
   * This function does/can *not* lock the record.
   * @param pFormattedString input formatted string
   * @param pData output vector of ARecord decoded from pFormattedString
   * @param pBruteForce ignore errors and try to decode what we can
   * @return StatusCode of the operation. If failed and not pBruteForce, pData is empty.
   */
  virtual StatusCode Decode(std::string &pFormattedString, std::vector<ARecord *> &pData, int pBruteForce=0) = 0;

  /// return m_format.
  std::string GetFormat();

};

#endif
