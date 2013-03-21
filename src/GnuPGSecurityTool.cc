/*
 Project: Console-Secrets
 Copyright (C) 2013  Simone Pagan Griso
 Distributed WITHOUT ANY WARRANTY under the GPL-3.0 licence,
 see the LICENCE file or visit <http://www.gnu.org/licenses/>
 File: GnuPGSecurityTool.cc
 Description: Implements ISecurityTool with GnuPG libs
 Last Modified: $Id: GnuPGSecurityTool.cc 54 2013-03-20 21:54:57Z gridge $
*/

#include "GnuPGSecurityTool.h"
#include "ILog.h"

#include <locale.h>
#include <iomanip>

extern ILog *log;

using namespace std;
using namespace GNUPG;

//Declaration of static member for first gpgme initialization
bool GnuPGSecurityTool::m_gpgme_initialized = false;

GnuPGSecurityTool::GnuPGSecurityTool(string pName) : ISecurityTool(pName)
{
  InitGpgMe();
}

GnuPGSecurityTool::~GnuPGSecurityTool()
{
  CloseGpgMe();
}

IErrorHandler::StatusCode GnuPGSecurityTool::InitGpgMe()
{
  /*First we need to set-up general settings for the first usage of gpgme
   * This is needed only the first time. */
  gpgme_error_t err;
  if (!m_gpgme_initialized) {
    /* Initialize the locale environment.  */  
    m_gpgme_version = gpgme_check_version (NULL); //no explicit version requirement
    gpgme_set_locale (NULL, LC_CTYPE, setlocale (LC_CTYPE, NULL));
#ifdef LC_MESSAGES
    gpgme_set_locale (NULL, LC_MESSAGES, setlocale (LC_MESSAGES, NULL));
#endif  
    // check engine compatibility
    if (gpgme_engine_check_version(GPGME_PROTOCOL_OpenPGP) != GPG_ERR_NO_ERROR) {
      //error in underlying engine version
      log->say(ILog::FATAL, "Engine version is not compatible with gpgme version", this);
      return m_statusCode = SC_ERROR;
    }
    *log << ILog::INFO << "Performed the first initialization of gpgme. Version: "
	 << m_gpgme_version 
	 << ", protocol name: " << gpgme_get_protocol_name(GPGME_PROTOCOL_OpenPGP)
	 << this << ILog::endmsg;
    //ok, set initialized static flag
    m_gpgme_initialized = true;
  }

  //print some information about the current settings    
  if ((err = gpgme_get_engine_info(&m_engInfo)) != GPG_ERR_NO_ERROR) {
    *log << ILog::WARNING << "Error in retrieving gpgme engine information."
	 << "Error " << gpgme_err_code(err) << ": " << gpgme_strerror(err) 
	 << "(" << gpgme_strsource(err) << ")"
	 << this << ILog::endmsg;
  }   
  *log << ILog::VERBOSE << "GunPG engine info (if any):";
  if (m_engInfo->file_name)
    *log << " file name = " << m_engInfo->file_name;
  if (m_engInfo->home_dir)
    *log << ", home dir = " << m_engInfo->home_dir;
  *log << this << ILog::endmsg;

  // Now setup the context in which this instance will work in
  err = gpgme_new(&m_context);
  if(err != GPG_ERR_NO_ERROR) {
    *log << ILog::ERROR << "Error in creating context for GPG. "
	 << "Error " << gpgme_err_code(err) <<": " << gpgme_strerror(err) 
	 << "(" << gpgme_strsource(err) << ")"
	 << this << ILog::endmsg;
    return m_statusCode = SC_ERROR;
  }
  // set engine properties
  err = gpgme_ctx_set_engine_info (m_context, GPGME_PROTOCOL_OpenPGP,
				   m_engInfo->file_name,m_engInfo->home_dir);
  if (err != GPG_ERR_NO_ERROR) {
    *log << ILog::ERROR << "Error in setting engine properties for GPG. "
	 << "Error " << gpgme_err_code(err) <<": " << gpgme_strerror(err) 
	 << "(" << gpgme_strsource(err) << ")"
	 << this << ILog::endmsg;
    return m_statusCode = SC_ERROR;    
  }
  // set armored data type
  gpgme_set_armor(m_context, 1);

  // set the key search mode
  gpgme_keylist_mode_t currentKeySearchMode;
  currentKeySearchMode = gpgme_get_keylist_mode(m_context);
  // enable search in the local key-ring
  currentKeySearchMode |= GPGME_KEYLIST_MODE_LOCAL;
  err = gpgme_set_keylist_mode(m_context, currentKeySearchMode);
  if (err != GPG_ERR_NO_ERROR) {
    *log << ILog::ERROR << "Error in setting search type to local for keys. "
	 << "Error " << gpgme_err_code(err) <<": " << gpgme_strerror(err) 
	 << "(" << gpgme_strsource(err) << ")"
	 << this << ILog::endmsg;
    return m_statusCode = SC_ERROR;        
  }

  return m_statusCode = SC_OK;
}

IErrorHandler::StatusCode GnuPGSecurityTool::CloseGpgMe()
{
  //free context
  gpgme_release(m_context);

  return SC_OK;
}

vector<string> GnuPGSecurityTool::GetKeyIdList(bool showAllKeys)
{
  vector<string> keyIdList;
  gpgme_error_t err;
  gpgme_key_t key;
  int gpgme_seckey_search = 1;
  if (showAllKeys)
    gpgme_seckey_search = 0;
  //search for all keys in the key-ring
  err = gpgme_op_keylist_start(m_context, "", gpgme_seckey_search);
  while (!err) {
    err = gpgme_op_keylist_next (m_context, &key);
    if (err)
      break;
    keyIdList.push_back(key->subkeys->keyid);
    gpgme_key_release(key);
   }
  if (gpg_err_code (err) != GPG_ERR_EOF) {
    *log << ILog::ERROR << "Error in retrieving list of keys."
	 << "Error " << gpgme_err_code(err) <<": " << gpgme_strerror(err) 
	 << "(" << gpgme_strsource(err) << ")"
	 << this << ILog::endmsg;
  }
  gpgme_op_keylist_end(m_context);

  return keyIdList;
}

vector<string> GnuPGSecurityTool::GetKeyListBriefDescr(bool showAllKeys)
{
  vector<string> keyList;
  gpgme_error_t err;
  gpgme_key_t key;
  int gpgme_seckey_search = 1;
  if (showAllKeys)
    gpgme_seckey_search = 0;
  //search for all keys in the key-ring
  err = gpgme_op_keylist_start(m_context, "", gpgme_seckey_search);
  while (!err) {
    ostringstream descr;
    err = gpgme_op_keylist_next (m_context, &key);
    if (err)
      break;
    descr << key->subkeys->keyid << ": ";
    //add principal user details
    /*
    if (key->uids && key->uids->name)
      descr << " " << key->uids->name;
    if (key->uids && key->uids->comment)
      descr << " (" << key->uids->comment << ")";
    if (key->uids && key->uids->email)
      descr << " <" << key->uids->email << ">";
    */
    //add user id
    descr << key->uids->uid;
    keyList.push_back(descr.str()); 
    gpgme_key_release(key);
   }
  if (gpg_err_code (err) != GPG_ERR_EOF) {
    *log << ILog::ERROR << "Error in retrieving list of keys."
	 << "Error " << gpgme_err_code(err) <<": " << gpgme_strerror(err) 
	 << "(" << gpgme_strsource(err) << ")"
	 << this << ILog::endmsg;
  }
  gpgme_op_keylist_end(m_context);

  return keyList;  
}

GnuPGSecurityTool::GpgKeyInfo GnuPGSecurityTool::GetKeyInfo(string pKeyId)
{
  GpgKeyInfo keyInfo;
  gpgme_error_t err;
  gpgme_key_t key;
  err = gpgme_op_keylist_start(m_context, pKeyId.c_str(), 0);
  err = gpgme_op_keylist_next (m_context, &key);
  if (err) {
    *log << ILog::ERROR << "Key not found in current key-ring: " 
	 << pKeyId << this << ILog::endmsg;
    return keyInfo;
  }
  FillKeyInfo(key, keyInfo);
  gpgme_key_release(key);
  err = gpgme_op_keylist_next (m_context, &key);  
  //we're expecting only one key to match
  if (gpg_err_code (err) != GPG_ERR_EOF) {
    *log << ILog::WARNING << "More than one key could have matched you search: " << pKeyId
	 << ". Error " << gpgme_err_code(err) <<": " << gpgme_strerror(err) 
	 << "(" << gpgme_strsource(err) << ")"
	 << this << ILog::endmsg;
  }
  gpgme_op_keylist_end(m_context);

  return keyInfo;
}

vector<GnuPGSecurityTool::GpgKeyInfo> GnuPGSecurityTool::GetKeyList(bool showAllKeys)
{
  vector<GpgKeyInfo> keyList;
  gpgme_error_t err;
  gpgme_key_t key;
  int gpgme_seckey_search = 1;
  if (showAllKeys)
    gpgme_seckey_search = 0;
  //search for all keys in the key-ring
  err = gpgme_op_keylist_start(m_context, "", gpgme_seckey_search);
  while (!err) {
    GpgKeyInfo keyInfo;
    err = gpgme_op_keylist_next (m_context, &key);
    if (err)
      break;
    FillKeyInfo(key, keyInfo);
    keyList.push_back(keyInfo); 
    gpgme_key_release(key);
   }
  if (gpg_err_code (err) != GPG_ERR_EOF) {
    *log << ILog::ERROR << "Error in retrieving list of keys."
	 << "Error " << gpgme_err_code(err) <<": " << gpgme_strerror(err) 
	 << "(" << gpgme_strsource(err) << ")"
	 << this << ILog::endmsg;
  }
  gpgme_op_keylist_end(m_context);

  return keyList;  
}

IErrorHandler::StatusCode GnuPGSecurityTool::SetKey(std::string pKey)
{
  //first check if key exists and is usable for encryption
  GpgKeyInfo keyInfo = GetKeyInfo(pKey);
  if (!keyInfo.revoked &&
      !keyInfo.expired &&
      !keyInfo.disabled &&
      !keyInfo.invalid &&
      keyInfo.canEncrypt) {
    //ok this key is ok
    *log << ILog::VERBOSE << "Setting new key: " << pKey << this << ILog::endmsg;
    m_key = pKey;
  } else {
    //not a valid key
    *log << ILog::ERROR << "Selected key is not valid: " << pKey
	 << this << ILog::endmsg;
    return m_statusCode = SC_ERROR;
  }

  return SC_OK;    
}

void GnuPGSecurityTool::FillKeyInfo(const gpgme_key_t &key, GpgKeyInfo &keyInfo)
{
  //key info
  keyInfo.revoked = key->revoked;
  keyInfo.expired = key->expired;
  keyInfo.disabled = key->disabled;
  keyInfo.invalid = key->invalid;
  keyInfo.canEncrypt = key->can_encrypt;
  keyInfo.secret = key->secret;
  keyInfo.canSign = key->can_sign;
  keyInfo.canCertify = key->can_certify;
  keyInfo.canAuthenticate = key->can_authenticate;
  keyInfo.ownerTrustLevel = key->owner_trust;
  switch (key->owner_trust) {
  case GPGME_VALIDITY_UNKNOWN:
    keyInfo.ownerTrust = "unknown";
    break;
  case GPGME_VALIDITY_UNDEFINED:
    keyInfo.ownerTrust = "undefined";
    break;
  case GPGME_VALIDITY_NEVER:
    keyInfo.ownerTrust = "never";
    break;
  case GPGME_VALIDITY_MARGINAL:
    keyInfo.ownerTrust = "marginal";
    break;
  case GPGME_VALIDITY_FULL:
    keyInfo.ownerTrust = "full";
    break;
  case GPGME_VALIDITY_ULTIMATE:
    keyInfo.ownerTrust = "ultimate";
    break;
  default:
    keyInfo.ownerTrust = "ERROR";
    break;    
  }
  //subkey info
  keyInfo.keyId = key->subkeys->keyid;
  keyInfo.fingerprint = key->subkeys->fpr;
  //uid info
  if (key->uids) {
    keyInfo.userId = key->uids->uid;
    if (key->uids->name)
      keyInfo.userName = key->uids->name;
    if (key->uids->comment)
      keyInfo.userComment = key->uids->comment;
    if (key->uids->email)
      keyInfo.userEmail = key->uids->email;    
  }
}

IErrorHandler::StatusCode GnuPGSecurityTool::GpgDataToString(const gpgme_data_t &pGpgData, string &pDest)
{
  gpgme_error_t err;
  //first go to the beginning of the data buffer
  err = gpgme_data_seek(pGpgData, 0, SEEK_SET);
  if (err != GPG_ERR_NO_ERROR) {
    *log << ILog::ERROR << "GpgDataToString, error reading data."
	 << "Error " << gpgme_err_code(err) <<": " << gpgme_strerror(err) 
	 << "(" << gpgme_strsource(err) << ")"
	 << this << ILog::endmsg;
    return SC_ERROR;
  }
  size_t readBytes;
  const int sizeBuffer = 10240;
  char tmpBuf[sizeBuffer+1];
  while ((readBytes = gpgme_data_read(pGpgData, tmpBuf, sizeBuffer)) > 0) {
    //append to destination string
    pDest += string(tmpBuf, readBytes);
  }
  return SC_OK;
}

IErrorHandler::StatusCode GnuPGSecurityTool::Decrypt(const std::string &pSource, std::string &pDest)
{
  gpgme_error_t err;
  gpgme_data_t source;
  gpgme_data_t dest;
  gpgme_decrypt_result_t  decryptResult = 0;
  
  //point to source buffer
  err = gpgme_data_new_from_mem(&source, pSource.c_str(), pSource.size(), 0);  
  if (err != GPG_ERR_NO_ERROR) {
    *log << ILog::ERROR << "Error in reading data to decrypt."
	 << "Error " << gpgme_err_code(err) <<": " << gpgme_strerror(err) 
	 << "(" << gpgme_strsource(err) << ")"
	 << this << ILog::endmsg;
    return m_statusCode = SC_ERROR;
  }
  //create dest buffer
  err = gpgme_data_new(&dest);
  if (err != GPG_ERR_NO_ERROR) {
    *log << ILog::ERROR << "Error in creating output data buffer to decrypt."
	 << "Error " << gpgme_err_code(err) <<": " << gpgme_strerror(err) 
	 << "(" << gpgme_strsource(err) << ")"
	 << this << ILog::endmsg;
    return m_statusCode = SC_ERROR;
  }
  //decrypt, a retry of 3 times is allowed for bad passphrase error
  int nTry = 0;
  while (nTry < 3) {
    err = gpgme_op_decrypt(m_context, source, dest);
    if (err == GPG_ERR_NO_ERROR) 
      break; //exit loop
    if (err != GPG_ERR_BAD_PASSPHRASE) {
      *log << ILog::ERROR << "Error in decrypting data."
	   << "Error " << gpgme_err_code(err) <<": " << gpgme_strerror(err) 
	   << "(" << gpgme_strsource(err) << ")"
	   << this << ILog::endmsg;
      return m_statusCode = SC_ERROR;
    }
    *log << ILog::ERROR << "Bad passphrase, Try n. " << ++nTry << this << ILog::endmsg;
  }
  if (err != GPG_ERR_NO_ERROR) {
    //At this point is a bad passphrase... for 3 times!
    *log << ILog::ERROR << "Error in decrypting data."
	 << "Error " << gpgme_err_code(err) <<": " << gpgme_strerror(err) 
	 << "(" << gpgme_strsource(err) << ")"
	 << this << ILog::endmsg;
    return m_statusCode = SC_ERROR;
  }

  //verify that there were no unsupported algorithms
  decryptResult = gpgme_op_decrypt_result(m_context);
  if (decryptResult && decryptResult->unsupported_algorithm) {
    *log << ILog::WARNING << "Unsopported algorithms were encountered: "
	 << decryptResult->unsupported_algorithm 
	 << this << ILog::endmsg;
  }
  //save the key used to decrypt for future usage
  if (decryptResult && decryptResult->recipients) {
    //save the key of the first recipient, if a key was not already set
    SetKey(decryptResult->recipients->keyid);
    *log << ILog::DEBUG << "Saving auto-determined key for source: "
	 << m_key << this << ILog::endmsg;
  }

  //now retreve result
  StatusCode sc;
  sc = GpgDataToString(dest, pDest);
  if (sc != SC_OK) {
    *log << ILog::ERROR << "Error in retrieving output of decrypt operation."
	 << this << ILog::endmsg;
    return m_statusCode = sc;
  }

  //free buffers and return
  gpgme_data_release(dest);
  gpgme_data_release(source);
  
  return SC_OK;
}

IErrorHandler::StatusCode GnuPGSecurityTool::Encrypt(const std::string &pSource, std::string &pDest)
{
  if (m_key.empty()) {
    log->say(ILog::ERROR, "Trying to encrypt with empty key.", this);
    return m_statusCode = SC_ERROR;
  }

  gpgme_error_t err;
  gpgme_data_t source;
  gpgme_data_t dest;

  //get key to encrypt, get the first key
  gpgme_key_t key[2];
  err = gpgme_op_keylist_start(m_context, m_key.c_str(), 0);
  err = gpgme_op_keylist_next (m_context, key);
  if (err) {
    *log << ILog::ERROR << "Key not found in current key-ring: " 
	 << m_key << this << ILog::endmsg;
    return m_statusCode = SC_ERROR;
  }
  key[1] = 0; //set to NULL the second entry

  //point to source buffer
  err = gpgme_data_new_from_mem(&source, pSource.c_str(), pSource.size(), 0);  
  if (err != GPG_ERR_NO_ERROR) {
    *log << ILog::ERROR << "Error in reading data to encrypt."
	 << "Error " << gpgme_err_code(err) <<": " << gpgme_strerror(err) 
	 << "(" << gpgme_strsource(err) << ")"
	 << this << ILog::endmsg;
    return m_statusCode = SC_ERROR;
  }

  //create dest buffer
  err = gpgme_data_new(&dest);
  if (err != GPG_ERR_NO_ERROR) {
    *log << ILog::ERROR << "Error in creating output data buffer to encrypt."
	 << "Error " << gpgme_err_code(err) <<": " << gpgme_strerror(err) 
	 << "(" << gpgme_strsource(err) << ")"
	 << this << ILog::endmsg;
    return m_statusCode = SC_ERROR;
  }

  //encrypt text
  gpgme_encrypt_flags_t flags;
  flags = GPGME_ENCRYPT_NO_ENCRYPT_TO; //only specified recipient, no defaults please
  err = gpgme_op_encrypt(m_context, key, flags, source, dest);
  if (err != GPG_ERR_NO_ERROR) {
    *log << ILog::ERROR << "Error in encrypting data."
	 << "Error " << gpgme_err_code(err) <<": " << gpgme_strerror(err) 
	 << "(" << gpgme_strsource(err) << ")"
	 << this << ILog::endmsg;
    return m_statusCode = SC_ERROR;    
  }

  //retrieve result
  StatusCode sc;
  sc = GpgDataToString(dest, pDest);
  if (sc != SC_OK) {
    *log << ILog::ERROR << "Error in retrieving output of encrypt operation."
	 << this << ILog::endmsg;
    return m_statusCode = sc;
  }
  
  //release key and buffers
  gpgme_key_release(key[0]);  
  gpgme_data_release(dest);
  gpgme_data_release(source);

  return SC_OK;
}

GnuPGSecurityTool::GpgKeyInfo::GpgKeyInfo()
{
  revoked = true;
  expired = true;
  disabled = true;
  invalid = true;
  canEncrypt = false;
  secret = false;
  canSign = false;
  canCertify = false;
  canAuthenticate = false;
  ownerTrustLevel = -1;
}
 

GnuPGSecurityTool::GpgKeyInfo::~GpgKeyInfo()
{
  
}

string GnuPGSecurityTool::GpgKeyInfo::StringSummary()
{
  ostringstream outStr;
  outStr << right << setw(18) << "Key Id: "          << left << keyId << endl;
  outStr << right << setw(18) << "Key Fingerprint: " << left << fingerprint << endl;
  outStr << right << setw(18) << "User Id: "         << left << userId << endl;  
  outStr << right << setw(18) << "User Name: "       << left << userName << endl;
  outStr << right << setw(18) << "User Comment: "    << left << userComment << endl;
  outStr << right << setw(18) << "User E-mail: "     << left << userEmail << endl;
  outStr << right << setw(18) << "Owner Trust: "     << left << ownerTrust << endl;
  outStr << right << setw(18) << "Expired: "         << left << (expired ? "yes" : "no") << endl;
  outStr << right << setw(18) << "Revoked: "         << left << (revoked ? "yes" : "no") << endl;
  outStr << right << setw(18) << "Disabled: "        << left << (disabled ? "yes" : "no") << endl;
  outStr << right << setw(18) << "Invalid: "         << left << (invalid ? "yes" : "no") << endl;
  outStr << right << setw(18) << "Secret: "          << left << (secret ? "yes" : "no") << endl;
  outStr << right << setw(18) << "Can Encrypt: "     << left << (canEncrypt ? "yes" : "no") << endl;
  outStr << right << setw(18) << "Can Sign: "        << left << (canSign ? "yes" : "no") << endl;
  outStr << right << setw(18) << "Can Certify: "     << left << (canCertify ? "yes" : "no") << endl;
  outStr << right << setw(18) << "Can Authenticate: "<< left << (canAuthenticate ? "yes" : "no") << endl;
  
  return outStr.str();
}
