/*
 Project: Console-Secrets
 Copyright (C) 2013  Simone Pagan Griso
 Distributed WITHOUT ANY WARRANTY under the GPL-3.0 licence,
 see the LICENCE file or visit <http://www.gnu.org/licenses/>
 File: MiscUtils.h
 Description: Collection of utilites
 Last Modified: $Id$
*/

#include "MiscUtils.h"

#include <sstream>

using namespace std;

string CSMUtils::CreateListStr(vector<string> pList, string pDelim)
{
  ostringstream outStr;
  for (vector<string>::iterator its = pList.begin(); its != pList.end(); ++its) {
    outStr << *its;
    outStr << pDelim;
  }
  //delete last separator
  return outStr.str().substr(0, outStr.str().size() - pDelim.size());
}

vector<string> CSMUtils::SplitListStr(std::string pStr, string pDelim)
{
  //parse pStr. it's a pDelim-separated list of fields
  vector<string> outList;
  string str = pStr;
  size_t cutAt;
  while( (cutAt = str.find_first_of(pDelim)) != str.npos ) {
    if(cutAt > 0) {
      outList.push_back(str.substr(0,cutAt));
    }
    str = str.substr(cutAt+pDelim.size());
  }
  if(str.length() > 0) 
    outList.push_back(str);
  return outList;
}

string CSMUtils::PadStrCenter(string pStr, size_t pLength)
{
  if (pStr.size() > pLength)
    return pStr.substr(0, pLength);
  string padStr = pStr;
  bool padRight=true;
  for (size_t p=pStr.size(); p<pLength; p++) {
    if (padRight)
      padStr.insert(0, " ");
    else
      padStr.append(" ");
    //alternate one space right, one left
    padRight = !padRight;
  }
  return padStr;
}

string CSMUtils::TrimStr(std::string pStr, const char pPad)
{
  //trim leading occurrences
  size_t start=0;
  while (pStr[start] == pPad)
    start++;
  //trim trailing occurrences
  size_t end=pStr.length() - 1;
  while (pStr[end] == pPad)
    end--;
  return pStr.substr(start, end-start+1);
}
