/*
 Project: Console-Secrets
 Copyright (C) 2013  Simone Pagan Griso
 Distributed WITHOUT ANY WARRANTY under the GPL-3.0 licence,
 see the LICENCE file or visit <http://www.gnu.org/licenses/>
 File: csm.cc
 Description: Main CSM program
 Last Modified: $Id: csm.cc -1   $
*/

//Debug flag to perform ad-hoc tests
//#define DEBUG_CSM

#include <stdlib.h>
#include <string>
#include <sstream>
#include <getopt.h>
#include <algorithm>

// --- Base includes
#include "csm.h"

#include "ISearchTool.h"

using namespace std;

void CleanUp();
void Usage(char **argv);
std::string cleanForCSV(std::string str);

/** CSM Main function */
int main(int argc, char **argv)
{
  // --- global static init
  cfgMgr = 0;
  log = 0;
  runningSecurityChecks = 0;
  ioSvc = 0;
  tuiSvc = 0;
  // --- General initializations
  setlocale (LC_ALL, "");  
  // --- Define CSM services and tools
  // -- Configuration Manager Service: use hard-coded defualts
  string userHome = getenv("HOME");
  if (userHome.empty())
    userHome = ".";
  //cfgMgr = new LocalConfigHardcoded("ConfigSvc"); //use hard-coded settings
  cfgMgr = new LocalConfigSimpleTxt("ConfigSvc"); //use hard-coded settings
  cfgMgr->version = PACKAGE_VERSION; // Set the CSM Version
  // -- Log service
  log = new LogLocalFile("LogSvc");
  // set specific LogLocalFile settings
  log->SetLogDetail(ILog::DEBUG); //to print initialization of log file itself
  logFileName=userHome+"/.csm_log";
  dynamic_cast<LogLocalFile*>(log)->SetLocalFileName(logFileName);
  log->Init();
  // -- Running Security Service
  runningSecurityChecks = new DummyRunSecurityService("RunSecSvc");
  // -- IO Service, manage account. Multiple source management.
  ioSvc = new MultipleSourceIOSvc("IOSvc");

  // --- Parse command-line options and set services/tools options  
  // -- Command line settings
  CmdLineActions cfg_action = act_nActions;
  // - General settings  
  string cfg_configFile=userHome+string("/.console-secrets");
  std::string cfg_sourceURI; //defaults will be set later
  bool cfg_bruteForce=false;
  // - Search options
  bool cfg_verboseSearchResults=false;
  bool cfg_regexSearch=false;
  // - Create new source
  string cfg_userName;
  string cfg_userKey;
  // - Export to file
  string cfg_exportFilename;
  
  int c;
  //int digit_optind = 0;
  while (1) {
    //int this_option_optind = optind ? optind : 1;
    int option_index = 0;
    static struct option csm_options[] =
      {
	{"help", no_argument, 0, 1},
	//General options
	{"source", required_argument, 0, 's'},
	{"force", required_argument, 0, 'f'},
	{"config", required_argument, 0, 'c'},
	//Search options
	{"regexp", no_argument, 0, 'e'},
	{"verbose", no_argument, 0, 'v'},
	//Create source options
	{"create", no_argument, 0, 'C'},
	{"key", required_argument, 0, 'k'},
	{"user", required_argument, 0, 'u'},
  //Export to CSV file
  {"export-csv", required_argument, 0, 'x'},
	//trailer
	{0, 0, 0, 0}
      };    
    c = getopt_long (argc, argv, "hs:fc:evCk:u:x:", csm_options, &option_index); 

    if (c==-1)
      break;

    switch (c) {
    case 's':
      cfg_sourceURI = optarg;
      *log << ILog::INFO << "Source set from command-line: " << cfg_sourceURI << ILog::endmsg;
      break;
    case 'f':
      cfg_bruteForce=true;
      *log << ILog::INFO << "Setting BruteForce flag to true" << ILog::endmsg;
      break;
    case 'c':
      cfg_configFile = optarg;
      *log << ILog::INFO << "Setting configuration file: " << cfg_configFile << ILog::endmsg;
      break;
    case 'e':
      cfg_regexSearch=true;
      log->say(ILog::INFO, "Use of regexp search set from command-line");
      break;
    case 'v':
      cfg_verboseSearchResults=true;
      log->say(ILog::INFO, "Verbose search printout set from command-line");
      break;
    case 'C':
      cfg_action = act_createSource;
      log->say(ILog::INFO, "Source creation requested by command-line");
      break;
    case 'k':
      cfg_userKey = optarg;
      *log << ILog::INFO << "User key from command-line ID: " << cfg_userKey << ILog::endmsg;
      break;
    case 'u':
      cfg_userName = optarg;
      *log << ILog::INFO << "User name from command-line: " << cfg_userName << ILog::endmsg;
      break;
    case 'x':
      cfg_exportFilename = optarg;
      *log << ILog::INFO << "Export to CSV file:: " << cfg_exportFilename << ILog::endmsg;
      break;
    case 'h':
    default:
      Usage(argv);
      return 1;
    };
  } //end parse of command-line options

  if (cfg_action == act_nActions) {
    //set default action
    if (not cfg_exportFilename.empty()) {
      //export to CSV file
      cfg_action = act_export;
    } else if (optind < argc) {
      //arguments found, perform a quick search on them
      cfg_action = act_quickSearch;
    } else {
      //start gui
      cfg_action = act_startGui;
    }
  }

  vector<string> cmdLineArguments;
  if (optind < argc) {
    while (optind < argc)
      cmdLineArguments.push_back(argv[optind++]);
  }

  // --- Delayed-init operations for services/tools
  if (cfgMgr->Load(cfg_configFile) != IErrorHandler::SC_OK) {
    if (cfg_bruteForce)
      *log << ILog::ERROR;
    else
      *log << ILog::FATAL;
    *log << "Cannot load properly configuration file: " << cfg_configFile << ILog::endmsg;
    if (not cfg_bruteForce) {
      cerr << "FATAL: Can't load configuration file: " << cfg_configFile << endl;
      return CSM_WRONG_CONFIG;
    }
  }
  cfgMgr->SetBruteForce(cfg_bruteForce);
  if (cfg_regexSearch) cfgMgr->SetSearchType(SearchRequest::REGEX);
  if (not cfg_userKey.empty())
    cfgMgr->SetUserKey(cfg_userKey);
  if (not cfg_userName.empty())
    cfgMgr->SetUserName(cfg_userName);
  

  log->SetLogDetail(cfgMgr->logMessagesDetails);


  // --- Preliminary checks
  int currentSecurityLevel = runningSecurityChecks->Run();
  if (currentSecurityLevel < cfgMgr->minRunSecurityLevel) {
    ostringstream errMsg;
    errMsg << "Running Security checks failed. " 
	   << "Security: " << currentSecurityLevel
	   << " (required: " << cfgMgr->minRunSecurityLevel << ")" 
	   << endl;
    log->say(ILog::FATAL, errMsg.str());
    return CSM_SECURITY;
  }

#ifndef DEBUG_CSM
  if (cfgMgr->GetUserName().empty()) {
    //get default from environment
    cfgMgr->SetUserName(getenv("USER"));
    *log << ILog::INFO << "Getting username from environment: " << cfgMgr->GetUserName() << ILog::endmsg;
    if (cfgMgr->GetUserName().empty()) {
      log->say(ILog::ERROR, "Cannot retrieve username.");
      cerr << "ERROR: Cannot retrieve username from command-line or environment. Continuing with blank one." << ILog::endmsg;
    }
  }

  ioSvc->SetOwner(cfgMgr->GetUserName());
  ioSvc->SetKey(cfgMgr->GetUserKey());
  bool errorDuringSourceLoading=false;
  bool atLeastOneSourceLoaded=false;
  if (cfg_action != act_createSource) {
    //Load data from source
    SourceURI inSource;
    if (not cfg_sourceURI.empty()) {
      inSource.SetURI(cfg_sourceURI);
      if (ioSvc->Load(inSource) != IErrorHandler::SC_OK) {
	*log << ILog::ERROR << "ERROR loading source: " << inSource.GetFullURI() << ILog::endmsg;
	errorDuringSourceLoading=true;
      } else {
	atLeastOneSourceLoaded=true;
      }
    } else {
      //load from config file
      for (vector<string>::iterator sourceIt = cfgMgr->inputURI.begin(); sourceIt != cfgMgr->inputURI.end(); ++sourceIt) {
	inSource.SetURI(*sourceIt);
	if (ioSvc->Load(inSource) != IErrorHandler::SC_OK) {
	  *log << ILog::ERROR << "ERROR loading source: " << inSource.GetFullURI() << ILog::endmsg;	  
	  errorDuringSourceLoading=true;  
	} else {
	  atLeastOneSourceLoaded=true;
	}
      }
    }
    if (not atLeastOneSourceLoaded) {
      *log << ILog::FATAL << "No sources could be loaded. Exiting" << ILog::endmsg;
      cerr << "ERROR: No sources could be loaded. Exiting." << endl;
      return CSM_WRONG_CONFIG;
    }
  }

  if (cfg_action == act_quickSearch) {
    log->say(ILog::INFO, "Starting quick search");
    if (errorDuringSourceLoading) {
      cerr << "WARNING: An error occurred during source loading. Not all results may be available. Consult log file: " << logFileName <<  endl;
    }
    //perform a quick search and quit
    vector<ARecord*> resultSearch;
    string searchPattern;
    for (vector<string>::iterator itA = cmdLineArguments.begin();
	 itA != cmdLineArguments.end(); ++itA) {
      searchPattern = searchPattern + *itA;
      if (itA+1 != cmdLineArguments.end())
	searchPattern += string(" ");
    }
    resultSearch = ioSvc->Find(searchPattern);
    if (cfg_verboseSearchResults)
      cout << "CSM MATCHED RECORDS (" << ioSvc->GetSource().str()<< "): " << resultSearch.size() << endl;
    for (vector<ARecord*>::iterator it = resultSearch.begin(); it != resultSearch.end(); ++it) {
      if (cfg_verboseSearchResults) {
	cout << "===============" << endl;
	cout << " Account name: " << (*it)->GetAccountName() << endl;
	cout << " Account Id: "  << (*it)->GetAccountId() << endl;
	cout << " Creation date: " << (*it)->GetCreationTimeStr() << endl;
	cout << " Last Modification: " << (*it)->GetModificationTimeStr() << endl;
	cout << " Labels: ";
	for (ARecord::TLabelsIterator lit = (*it)->GetLabelsIterBegin(); lit != (*it)->GetLabelsIterEnd(); ++lit)
	  cout << *lit <<",";
	cout << endl;
	cout << " Essentials: ";
	for (ARecord::TEssentialsIterator eit = (*it)->GetEssentialsIterBegin(); eit != (*it)->GetEssentialsIterEnd(); ++eit)
	  cout << *eit <<",";
	cout << endl;
	cout << " Fields: " << endl;
	for (ARecord::TFieldsIterator fit = (*it)->GetFieldsIterBegin(); fit != (*it)->GetFieldsIterEnd(); ++fit)
	  cout << "  " << fit->first <<": " << fit->second << endl;
      } else { //non-verbose printing
	cout << "Account name: " << (*it)->GetAccountName() << endl;
	cout << " Last Modification: " << (*it)->GetModificationTimeStr() << endl;
	//just print essentials fields
	for (ARecord::TEssentialsIterator eit = (*it)->GetEssentialsIterBegin(); eit != (*it)->GetEssentialsIterEnd(); ++eit) {
	  if ((*it)->HasField(*eit))
	    cout << " " << *eit <<": " << (*it)->GetField(*eit) << endl;
	  else
	    *log << ILog::VERBOSE <<"Record Id << "<<(*it)->GetAccountId()<<" does not contain one of its Essentials: "<<*eit<<ILog::endmsg;
	}
      }
    }
    if (cfg_verboseSearchResults)
      cout << "===============" << endl;
  } else if (cfg_action == act_startGui) {
    //start GUI
    log->say(ILog::INFO, "Starting GUI");
    tuiSvc = new TuiSvc("TUI");
    if (errorDuringSourceLoading)
      tuiSvc->SetWelcomeMessage(string("WARNING: An error occurred during source loading. Not all results may be available. Consult log file: ")+logFileName);
    else
      tuiSvc->SetWelcomeMessage("Source data loaded successfully");
    tuiSvc->Run();
  } else if (cfg_action == act_createSource) {
    log->say(ILog::INFO, "Creating new source from command-line");
    if (cfgMgr->GetUserKey().empty()) {
      log->say(ILog::FATAL, "User key not provided");
      cerr << "User key not provided" << endl;
      Usage(argv);
      CleanUp();
      return CSM_ACTION_ERROR;
    }
    if (cfg_sourceURI.empty()) {
      if (cmdLineArguments.size() == 1) {
	cfg_sourceURI = cmdLineArguments[0];
      } else {
	log->say(ILog::FATAL, "More than one argument for source input provided.");
	Usage(argv);
	CleanUp();
	return CSM_ACTION_ERROR;
      }
    }
    SourceURI inSource(cfg_sourceURI);
    *log << ILog::INFO << "Creating source with URI: " << inSource.GetFullURI() << ILog::endmsg;
    //first check if source exists
    IErrorHandler::StatusCode resExists;
    resExists = ioSvc->SourceExists(inSource);
    if (resExists != IErrorHandler::SC_NOT_FOUND && resExists != IErrorHandler::SC_NOT_IMPLEMENTED) {
      if (resExists == IErrorHandler::SC_OK) {
	*log << ILog::ERROR << "Source already exists: " << inSource.GetFullURI() << ILog::endmsg;
	cerr << "ERROR: Source already exists." << endl;
      } else if (resExists == IErrorHandler::SC_ERROR) {
	*log << ILog::ERROR << "Can't check source existence: " << inSource.GetFullURI() << ILog::endmsg;
	cerr << "ERROR while checking for existence of source. Consult the log file: " << logFileName << endl;
      } else {
	*log << ILog::ERROR << "Unexpected error checking for source existence: " << resExists << "; for source: " << inSource.GetFullURI() << ILog::endmsg;
	cerr << "Unexpected error. Consult the log file: " << logFileName << endl;	
      }
      CleanUp();
      return CSM_ACTION_ERROR;
    }
    if (resExists == IErrorHandler::SC_NOT_IMPLEMENTED) {
      //Warn the user that we could not check if source exists or not
      *log << ILog::WARNING << "Couldn't check if source existed or not." << ILog::endmsg;
      cerr << "WARNING: Couldn't check if source existed or not." << endl;
      if (!cfgMgr->GetBruteForce()) {
	cerr << "Use --force option to force creation." << endl;
	CleanUp();
	return CSM_ACTION_ERROR;
      }
    }
    //ok, now create new source
    int retC;
    retC = ioSvc->NewSource(inSource, cfgMgr->GetUserName(), cfgMgr->GetUserKey());
    if (retC != IErrorHandler::SC_OK) {
      log->say(ILog::FATAL, "Error detected in creating new source");
      cerr << "Error in creating new source. Consult log file: " << logFileName << endl;
      CleanUp();
      return CSM_ACTION_ERROR;
    }
    retC = ioSvc->Store();
    if (retC != IErrorHandler::SC_OK) {
      log->say(ILog::FATAL, "Error detected in storing new source");
      cerr << "Error in storing new source. Consult log file: " << logFileName << endl;
      CleanUp();
      return CSM_ACTION_ERROR;
    }
    log->say(ILog::INFO, "New source created successfully");
    cout << "New Source created successfully: " << inSource.GetFullURI() << endl;    
  } if (cfg_action == act_export) {
    vector<string> csvColumns = {"Name", "Date", "Labels"};    
    log->say(ILog::INFO, "Starting export to CSV file");
    vector<ARecord*> resultSearch;
    resultSearch = ioSvc->GetAllAccounts(IIOService::ACCOUNTS_SORT_BYNAME);
    //First, find out all possible column names    
    for (vector<ARecord*>::iterator it = resultSearch.begin(); it != resultSearch.end(); ++it) {
      for (ARecord::TFieldsIterator fit = (*it)->GetFieldsIterBegin(); fit != (*it)->GetFieldsIterEnd(); ++fit) {
        if ( std::find_if(csvColumns.begin(), csvColumns.end(), [fit](std::string& element) -> bool { return (element == fit->first);}) == csvColumns.end() )
          csvColumns.push_back(fit->first);
      }
    }
    //Now write the output CSV file
    ofstream csvOut;
    csvOut.open(cfg_exportFilename.c_str());
    bool first_entry=true;
    for (vector<string>::iterator itColNames = csvColumns.begin(); itColNames != csvColumns.end(); ++itColNames) {
      std::string cleanStr = cleanForCSV(*itColNames);
      if (first_entry) first_entry = false;
      else csvOut << ", ";
      csvOut << cleanStr;
    } //loop over columns
    csvOut << endl;
    for (vector<ARecord*>::iterator it = resultSearch.begin(); it != resultSearch.end(); ++it) {
      map<string, string> outCsvRow;
      outCsvRow["Name"] = (*it)->GetAccountName();
      outCsvRow["Date"] = (*it)->GetModificationTimeStr();
      string labels;
      for (ARecord::TLabelsIterator lit = (*it)->GetLabelsIterBegin(); lit != (*it)->GetLabelsIterEnd(); ++lit) {
        if (not labels.empty()) {
          labels = labels + ", ";
        }
        labels = labels + *lit;
      }
      outCsvRow["Labels"] = labels;
      for (ARecord::TFieldsIterator fit = (*it)->GetFieldsIterBegin(); fit != (*it)->GetFieldsIterEnd(); ++fit) {
        outCsvRow[fit->first] = fit->second;
      }
      first_entry=true;
      for (vector<string>::iterator itColNames = csvColumns.begin(); itColNames != csvColumns.end(); ++itColNames) {
        if (first_entry) first_entry = false;
        else csvOut << ", ";
        map<string,string>::iterator itColVal;
        itColVal = outCsvRow.find(*itColNames);
        if ( itColVal != outCsvRow.end() ) {
          csvOut << cleanForCSV(itColVal->second);
        } 
      }
      csvOut << endl;
    }
    csvOut.close();
  } else {
    log->say(ILog::FATAL, "Unable to determine action");
    cerr << "Unrecognized action. Exiting." << endl;    
    CleanUp();
    return CSM_ACTION_ERROR;
  }

#else
  // --- TEST SECTION !!!
  log->SetLogDetail(ILog::DEBUG);
  SourceURI testSource("file://csm_test.txt.ct");
  //test brand-new log functionalisties
  *log << ILog::INFO << "Testing source: "
       << testSource.GetField(SourceURI::TYPE) << " :// " 
       << testSource.GetField(SourceURI::NAME) << " . "
       << testSource.GetField(SourceURI::FORMAT) << ILog::endmsg;

  *log << ILog::INFO << "off_t size = " << sizeof(off_t) << ILog::endmsg;

  //set key
  string mykey = "D941FEB9C37DBF71"; 
  MultipleSourceIOSvc *ioSvcMulti = dynamic_cast<MultipleSourceIOSvc*>(ioSvc);
  ioSvcMulti->NewSource(testSource, "pagan", mykey);
  // ioSvcMulti->NewSource(testSource, "pagan");
 
  int test = 4;

  if (test == 1 || test == 4) {
     //test storing
    ARecord *testRec = new ARecord();
    testRec->SetAccountName("DummyAccount1");
    testRec->AddLabel("DummyFolder1");
    testRec->AddDebugField(string("User"), string("pippo"));
    testRec->AddDebugField(string("Password"), string("cavolo"));
    testRec->AddEssential("Password");
    ioSvc->Add(testRec);
    testRec = new ARecord();
    testRec->SetAccountName("DummyAccount2");
    testRec->AddLabel("DummyFolder1");
    testRec->AddLabel("DummyFolder2");
    testRec->AddDebugField(string("User"), string("Bpippo"));
    testRec->AddDebugField(string("Password"), string("Bcavolo"));
    testRec->AddDebugField(string("Varie"), string("blah"));
    testRec->AddEssential("Password");
    testRec->AddEssential("User");
    ioSvc->Add(testRec);
    testRec = new ARecord();
    testRec->SetAccountName("DummyAccount3");
    testRec->AddDebugField(string("User"), string("Cpippo"));
    testRec->AddDebugField(string("Password"), string("Ccavolo"));
    testRec->AddDebugField(string("Multi"), string("This is a multiline input.\n This is the second line."));
    testRec->AddEssential("Password");
    ioSvc->Add(testRec);
    testRec = new ARecord();
    testRec->SetAccountName("DummyAccount4");
    testRec->AddDebugField(string("User"), string("Dpippo"));
    testRec->AddDebugField(string("Password"), string("Dcavolo"));
    ioSvc->Add(testRec);
    testRec = new ARecord();
    testRec->SetAccountName("DummyAccount5");
    testRec->AddDebugField(string("User"), string("Epippo"));
    testRec->AddDebugField(string("Password"), string("Ecavolo"));
    testRec->AddLabel("DummyFolder2");
    ioSvc->Add(testRec);
    ioSvc->Store();
  }
  if (test == 2) {
    //test loading
    ioSvc->Load();
    vector<ARecord*> resultSearch;
    resultSearch = ioSvc->Find("Dummy");
    for (vector<ARecord*>::iterator it = resultSearch.begin(); it != resultSearch.end(); ++it) {
      cout << "NEW RECORD LOADED:" << endl;
      cout << "  Account name: " << (*it)->GetAccountName() << endl;
      cout << "  Account Id = "  << (*it)->GetAccountId() << endl;
      cout << "  Labels: ";
      for (ARecord::TLabelsIterator lit = (*it)->GetLabelsIterBegin(); lit != (*it)->GetLabelsIterEnd(); ++lit)
	cout << *lit <<",";
      cout << endl;
      cout << "  Essentials: ";
      for (ARecord::TEssentialsIterator eit = (*it)->GetEssentialsIterBegin(); eit != (*it)->GetEssentialsIterEnd(); ++eit)
	cout << *eit <<",";
      cout << endl;      
      cout << "  Fields: " << endl;
      for (ARecord::TFieldsIterator fit = (*it)->GetFieldsIterBegin(); fit != (*it)->GetFieldsIterEnd(); ++fit)
	cout << "    " << fit->first <<" <--> " << fit->second << endl;
      cout << endl;
    }
    //re-write data
    ioSvc->Store();
  }
  if (test == 3) {
    ISecurityTool *ist = new GnuPGSecurityTool("TestGPGSecurity");
    GnuPGSecurityTool *gpgst = dynamic_cast<GnuPGSecurityTool *>(ist);
    cout << "Brief list of keys:" << endl;
    vector<string> keysDescr = gpgst->GetKeyListBriefDescr();
    for (vector<string>::iterator k=keysDescr.begin(); k!=keysDescr.end(); ++k)
      cout << *k << endl;
    string mykey = "D941FEB9C37DBF71";
    cout << endl << "Using the following key: " << mykey << endl;
    GnuPGSecurityTool::GpgKeyInfo kinfo = gpgst->GetKeyInfo(mykey);
    cout << kinfo.StringSummary();
    ist->SetKey(mykey);
    delete ist;    
  }
  if (test == 4) {
    //test TUI
    tuiSvc = new TuiSvc("TUI");
    tuiSvc->Run();
  }

#endif

  //clean-up and exit
  CleanUp();
  return CSM_OK;
}

void CleanUp()
{
  if (log) log->say(ILog::INFO, "Final clean-up.");
  // --- Free CSM services and tools
  if (ioSvc) delete ioSvc;
  if (cfgMgr) delete cfgMgr;
  if (runningSecurityChecks) delete runningSecurityChecks;
  if (log) delete log;
}

void Usage(char **argv)
{
  string strHelp_userDefault;
  strHelp_userDefault = getenv("USER");

  cerr << "CSM " << cfgMgr->GetCSMVersion() << " -- a c++ Console password (and other Secrets) Management utility. " << std::endl;
  cerr << "Copyright (C) 2013 Simone Pagan Griso" << std::endl;
  cerr << "This program comes with ABSOLUTELY NO WARRANTY; This is free software, and you are welcome to redistribute it under certain conditions;" << std::endl;
  cerr << "see the file LICENCE, distributed with the source of this program, for furhter information." << std::endl;
  cerr << "General options:" << endl;
  cerr << "\t -s, --source\t Set input source." << endl;
  cerr << "\t -c, --config\t Set configuration file." << endl;
  cerr << "\t -f, --force\t Force actions (use if you know what you're doing)" << endl;
  cerr << "\t -h, --help\t Print help screen" << std::endl;
  cerr << endl;
  cerr << "* " <<  argv[0] << " [options]" << std::endl;
  cerr << "If only general options (or none) are given, launch the GUI." << endl;
  cerr << endl;
  cerr << "* " << argv[0] << " [options] searchString" << std::endl;  
  cerr << "Free text search for *searchString* pattern. All general options are also valid." << std::endl;
  cerr << "\t -e, --regexp\t Interpret *searchString* as regular expression" << endl;
  cerr << "\t -v, --verbose\t Full printout of matched records (default: only \"Essentials\" fields)" << endl;
  cerr << endl;
  cerr << "* " << argv[0] << " (--create | -C) --key myKey [options] (newSourceName | --source newSourceName)" << std::endl;
  cerr << "Create a new source with name *newSourceName* for storing your passwords. All general options are also valid." << endl;
  cerr << "\t -k, --key\t Set the secret key identifier to be used for encryption" << endl;
  cerr << "\t -u, --user\t Set username to be associated to the source (default: " << strHelp_userDefault << ")" << endl;
  cerr << "* " << argv[0] << " (--export | -x outputFile) " << std::endl;
  cerr << "Export to outputFile in CSV format. All general options are also valid." << endl;

  cerr << std::endl;
}



string cleanForCSV(string str)
{
  //escape characters
  std::string::size_type n = 0;
  if ((str.find(",") != string::npos) or str[0] == '"' ) {
    //contains at least one comma or starts with a ", quote the string
    n=0;
    while ( ( n = str.find( "\"", n ) ) != std::string::npos )
      {
        str.replace( n, 1, "\"\"" ); //double-quote escape of single-quote
        n += 2;
      }
    str = string("\"") + str + string("\"");
  }
  return str;
}
