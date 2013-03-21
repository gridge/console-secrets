/*
 Project: Console-Secrets
 Copyright (C) 2013  Simone Pagan Griso
 Distributed WITHOUT ANY WARRANTY under the GPL-3.0 licence,
 see the LICENCE file or visit <http://www.gnu.org/licenses/>
 File: ISecurityTool.h
 Description: Interface for cryptography and password strenght checks
 Last Modified: $Id: ISecurityTool.h 54 2013-03-20 21:54:57Z gridge $
*/

#ifndef __ISECURITY_TOOL__
#define __ISECURITY_TOOL__

#include "IErrorHandler.h"

#include <string>

/** Intergace for cryptography and password strenght checks
 * Defines basic methods needed by CSM to crypt/decrypt and to make 
 * all the checks on the pasword strength.
 */
class ISecurityTool : public IErrorHandler {
 protected:
  std::string m_key; ///< identifies the key to be used
 public:
  ISecurityTool(std::string pName);
  ~ISecurityTool();

  //------------------------------
  // --- Key management methods
  //------------------------------
  /// Set the key associated
  virtual StatusCode SetKey(std::string pKey);
  /// Get the key associated
  virtual std::string GetKey();

  //------------------------------
  // --- Encryption methods
  //------------------------------
  /** Encrypt a given string.
   * @param pSource defines the source string to be crypted
   * @param pDest defines the string where to store the result
   * @return the status of the operation
   */
  virtual StatusCode Encrypt(const std::string &pSource, std::string &pDest) = 0;

  /** Decrypt a given string.
   * @param pSource defines the source string to be decrypted
   * @param pDest defines the string where to store the result
   * @return the status of the operation
   */
  virtual StatusCode Decrypt(const std::string &pSource, std::string &pDest) = 0;

  //------------------------------
  // --- Password-related methods
  //------------------------------
  /** Check password strength.
   * Allow an integer to be returned to define password strength.
   * See derived class for specific meaning.
   * @param pPpwd is the password being checkes
   * @return integer defining password strength (see derived classes)
   */
  //virtual int PasswordStrength(std::string pPpwd) = 0;

  /** Generate a password.
   * Routine used to generate a new random and possibly strong password.
   * @param pPpwd is the generated password
   * @return the status of the operation
   * @todo Think to make GenPassword and GetHash as static functions
   */
  //virtual StatusCode GenPassword(std::string &pPwd) = 0;

  // --- Hash functions
  /** Get Hash of the string.
   * Implements an hash algorithm for content checking.
   * See derived classes for details on the algorithm.
   */
  //virtual StatusCode GetHash(const std::string &pSource, std::string &pHash) = 0;

  //------------------------------
  // --- Utilities
  //------------------------------
  /* Clear safely this string. */
  static void ClearString(std::string& str);
  /* Clear safely this string bugger. */
  static void ClearStrBuffer(std::istringstream& str);

};

#endif
