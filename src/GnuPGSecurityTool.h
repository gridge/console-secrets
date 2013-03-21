/*
 Project: Console-Secrets
 Copyright (C) 2013  Simone Pagan Griso
 Distributed WITHOUT ANY WARRANTY under the GPL-3.0 licence,
 see the LICENCE file or visit <http://www.gnu.org/licenses/>
 File: GnuPGSecurityTool.h
 Description: Implements ISecurityTool with GnuPG libs
 Last Modified: $Id: GnuPGSecurityTool.h 54 2013-03-20 21:54:57Z gridge $
*/

#ifndef __GNUPGSECURITY_TOOL__
#define __GNUPGSECURITY_TOOL__

#include "ISecurityTool.h"

#include <vector>
#include <string>

/** Containes GnuPG C libraries
 * In particular we use:
 * - gpgme to interface with gpg
 * - libcrypt for password checking/generation
 * The underlying protocol used for encryption is the OpenPGP.
 */
namespace GNUPG {
#include "gpgme.h"
}

/** Implements ISecurityTool with GnuPG libs
 * Uses libcrypt and GpgMe interface to GunPG libs to implement
 * encryption/decryption and password checks.
 * The gpgme library needs to be initialized only once, then each instance of the class
 * can use a new context to perform its own operations.
 * Keys information are stored in this class (one key for each instance), so that
 * it can me connected to one IIOService which uses this tool.
 * The key ID will be used to uniquely identify a pub/sec key pair and stored in the
 * field ISecurityTool::m_key.
 * Instances without keys are also allowed in case only password generation/check is needed.
 */
class GnuPGSecurityTool : public ISecurityTool {
 protected:  
  // --- GpgMe library
  /** Init GpgMe library.
   * Initialize the locale (only he first time an instance of GnuPGSecurityTool is created)
   * and setup the context the instance we'll work on.
   */
  StatusCode InitGpgMe();

  /// Release context
  StatusCode CloseGpgMe();

  /// Keep track of the first initialization of gpgme library
  static bool m_gpgme_initialized;

  // Members used to talk to gpgme library and store info
  GNUPG::gpgme_ctx_t m_context; ///< Context used by class instance for gpg operations
  GNUPG::gpgme_engine_info_t m_engInfo; ///< Store engine configuration
  std::string m_gpgme_version; ///< Store version of gpgme library

 public:
  /** Class to contain key info.
   * List all relevant information for CSM of the OpenPGP key.
   * This is a summary only.
   */
  class GpgKeyInfo {
  public:
    //uid properties
    std::string userId; ///< id of principal user of the key
    std::string userName; ///< name of principal user (optional)
    std::string userEmail; ///< email of principal user (optional)
    std::string userComment; ///< comment field of principal user (optional)
    //key properties
    std::string ownerTrust; ///< key owner trust (string)
    bool revoked; ///< key has been revoked
    bool expired; ///< key has expired
    bool disabled; ///< key has been disabled
    bool invalid; ///< key is invalid
    bool canEncrypt; ///< key can encryot data
    bool secret; ///< key contains secret key
    bool canSign; ///< key can sign data
    bool canCertify; ///< key can certify data
    bool canAuthenticate; ///< key can authenticate data
    int ownerTrustLevel; ///< key owner trust (integer)    
    //subkey properties
    std::string keyId;
    std::string fingerprint; ///< Fingerprint of the key
  public:
    GpgKeyInfo();
    ~GpgKeyInfo();

    //helper members
    /// convert summary information into a human-readeable string
    std::string StringSummary();
  };

 protected:
  /// Helper to fill GpgKeyInfo from gpgme_key_t structure
  void FillKeyInfo(const GNUPG::gpgme_key_t &key, GpgKeyInfo &keyInfo);

 public:
  GnuPGSecurityTool(std::string pName);
  ~GnuPGSecurityTool();

  //------------------------------
  // --- Key management methods
  //------------------------------
  /// Override base function to check if key is present in accessible keyrings
  virtual StatusCode SetKey(std::string pKey);

  /** Get detailed list of keys in the key-ring
   * Only keys with public+secrey key are returned by default.
   * @param showAllKeys if true all keys in the key-ring are showed.
   * @return vector of GpgKeyInfo containintg keys properties
   */
  virtual std::vector<GpgKeyInfo> GetKeyList(bool showAllKeys=false);

  /** Get list of keys Id in the key-ring
   * Only keys with public+secrey key are returned by default.
   * @param showAllKeys if true all keys in the key-ring are showed.
   * @return list of keys identifiers
   */
  virtual std::vector<std::string> GetKeyIdList(bool showAllKeys=false);

  /** Get detailed property for a given key in the key-ring
   * @param pKeyId is the ID of the key
   * @return GpgKeyInfo containintg key properties
   * Note: if multiple keys satisfy search requirement, a warning is written
   * in the log and the first one is returned.
   */
  virtual GpgKeyInfo GetKeyInfo(std::string pKeyId);

  /** Get list of keys id and brief description.
   * Return vector of strings in the format "<keyId>: <name> (<comment>) < <email> >"
   */
  virtual std::vector<std::string> GetKeyListBriefDescr(bool showAllKeys=false);

  //------------------------------
  // --- Encryption methods
  //------------------------------
 protected:
  //Helper functions
  StatusCode GpgDataToString(const GNUPG::gpgme_data_t &pGpgData, std::string &pDest);
 public:
  ///Encrypt data using stored key, see ISecurityTool::Crypt
  virtual StatusCode Encrypt(const std::string &pSource, std::string &pDest);  

  ///Decrypt data using stored key, see ISecurityTool::Decrypt
  virtual StatusCode Decrypt(const std::string &pSource, std::string &pDest);

};

#endif
