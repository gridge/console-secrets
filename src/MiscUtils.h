/*
 Project: Console-Secrets
 Copyright (C) 2013  Simone Pagan Griso
 Distributed WITHOUT ANY WARRANTY under the GPL-3.0 licence,
 see the LICENCE file or visit <http://www.gnu.org/licenses/>
 File: MiscUtils.h
 Description: Collection of utilites
 Last Modified: $Id$
*/

#ifndef __CSMUTILS__
#define __CSMUTILS__

#include <string>
#include <vector>
#include <sstream>
#include <stdio.h>
#include <time.h>

/** Namespace for utility contaner.
 * This namespace collects various utilities used inside CSM.
 */
namespace CSMUtils {

  // ----------------------------------------
  // --- String-related utilities
  // ----------------------------------------  

  /** Create a list of strings with a given separator from a list.
   * @param pList is the input vector containing the list of elements
   * @param pDelim defines the delimiter between elements
   * @return concatenated list of string-elements separated with pDelim
   */
  std::string CreateListStr(std::vector<std::string> pList, std::string pDelim=",");

  /** Split string containing a list of element with a given separator into a vector.
   * @param pStr is the input string
   * @param pDelim defines the delimiter between elements
   * @return vector of elements
   */
  std::vector<std::string> SplitListStr(std::string pStr, std::string pDelim=",");

  /** Pad a string with spaces leaving it at the center. 
   * @param pStr input string
   * @param pLength lenght of output string
   * @return padded string
   */   
  std::string PadStrCenter(std::string pStr, size_t pLength);

  /** Trim leading/trailing (pad) charachters.
   * @param pStr input string
   * @param pPad charachter to be "trimmed"
   * @param string with removed leading/trailing occurrences of pPad
   */
  std::string TrimStr(std::string pStr, const char pPad=' ');

  /** Parse string to check for a valid date and return tm record */
  int parseStrDate(std::string timeStr, struct tm* returnTime);

}

#endif
