/*
 Project: Console-Secrets
 Copyright (C) 2013  Simone Pagan Griso
 Distributed WITHOUT ANY WARRANTY under the GPL-3.0 licence,
 see the LICENCE file or visit <http://www.gnu.org/licenses/>
 File: TuiSvc.cc
 Description: Text User Interface Service
 Last Modified: $Id: TuiSvc.cc 54 2013-03-20 21:54:57Z gridge $
*/

#include "TuiSvc.h"

#include "ILog.h"
#include "IConfigurationService.h"
#include "IIOService.h"
#include "MultipleSourceIOSvc.h"

extern ILog *log;
extern IConfigurationService *cfgMgr;
extern IIOService *ioSvc;

#include <utility>

using namespace std;
using namespace Ncurses;

TuiSvc::TuiSvc(string pName) : IErrorHandler(pName)
{
  m_mwnd = 0;
  m_statusBar = 0;
  m_mainMenuPage = 0;
  m_newAccountPage = 0;
}

TuiSvc::~TuiSvc()
{

}

IErrorHandler::StatusCode TuiSvc::Run()
{
  //init ncurses
  m_statusCode = InitTui();
  if (m_statusCode >= SC_ERROR) {
    *log << ILog::ERROR << "Error initializing NCurses" << this << ILog::endmsg;
    CloseTui();
    return SC_ERROR;
  }

  //display main menu
  MainMenu();

  //close ncurses main screen
  m_statusCode = CloseTui();
  if (m_statusCode >= SC_ERROR) {
    *log << ILog::ERROR << "Error closing NCurses" << this << ILog::endmsg;
  }
  return m_statusCode;
}

IErrorHandler::StatusCode TuiSvc::InitTui()
{
  *log << ILog::INFO << "Entering Text User Interface" << this << ILog::endmsg;
  //init ncurses
  m_mwnd = initscr();
  //setup desired features
  //if (cbreak() != OK) //do not wait EOL to flush buffer
  //  return SC_ERROR;
  //halfdelay(50); //5s timeout for input of char -- decided that we prefer raw mode.
  raw(); //do not wait EOL to flush buffer. Handle Ctrl+C charachter and similar directly, no signal generated.
  keypad(m_mwnd, TRUE); //enable keypad
  noecho(); //disable echo
  nonl(); //disable automatic translations of '\n' in line-feed
  curs_set(0);   //disable cursor, by default
  
  // Init the header and status bar windows
  m_statusBar = new TuiStatusBar("StatusBar");
  m_statusBar->Init();
  m_statusBar->HeaderBar("", false);
  m_statusBar->StatusBar("", false);
  m_statusBar->CommandBar(false);  

  //refresh all the screen
  refresh();

  //store screen properties
  getmaxyx(m_mwnd, m_screenRows, m_screenColumns);

  *log << ILog::VERBOSE << "Initialized screen with " << LINES << " lines and "
       << COLS << " columns" << this << ILog::endmsg;
    
  return SC_OK;
}

IErrorHandler::StatusCode TuiSvc::CloseTui()
{
  //back to the cursor
  curs_set(1);
  //delete status bar
  delete m_statusBar;
  //end main window
  if (endwin() != OK)
    return SC_ERROR;

  m_mwnd = 0;

  *log << ILog::INFO << "Closed Text User Interface" << this << ILog::endmsg;

  return SC_OK;
}

void TuiSvc::MainMenu()
{
  //create main menu page
  if (!m_mainMenuPage)
    m_mainMenuPage = new TuiMainMenu("TuiMenu");
  m_mainMenuPage->Init(m_statusBar);

  if (not m_welcomeMessage.empty()) {
    m_statusBar->StatusBar(m_welcomeMessage);
    m_welcomeMessage.clear(); //display once only.
  }
  bool quitTui = false;
  while (!quitTui) {
    //display main menu. Returns when a choice is operated or an error ocurred.
    m_mainMenuPage->Display(); 
    if (m_mainMenuPage->menuSelection == TuiMainMenu::NOT_VALID) {
      //error occurred
      std::string errmsg;
      m_statusCode = m_mainMenuPage->GetErrorMsg(errmsg);
      *log << ILog::ERROR << "Main menu failed. Status code = " << m_statusCode << ": "
	   << errmsg << this << ILog::endmsg;
      return;
    }
    switch (m_mainMenuPage->menuSelection) {
    case TuiMainMenu::NEW_ACCOUNT:
      m_mainMenuPage->Close();
      NewAccount();
      break;
    case TuiMainMenu::BROWSE:
      m_statusBar->StatusBar("Browse not implemented yet. Sorry.");
      break;
    case TuiMainMenu::SEARCH:
      m_statusBar->StatusBar("Search not implemented yet. Sorry.");
      break;
    case TuiMainMenu::CONFIGURE:
      m_statusBar->StatusBar("Configure not implemented yet. Sorry.");
      break;
    case TuiMainMenu::QUIT:
      quitTui = true;
      break;
    default:
      *log << ILog::FIXME << "Main menu options not handled by TuiSvc: " << (int)m_mainMenuPage->menuSelection
	   << this << ILog::endmsg;
      //be nice and keep running
    }
  }

  //close everything and qui TUI
  m_mainMenuPage->Kill();
  if (m_mainMenuPage)
    delete m_mainMenuPage;
}

void TuiSvc::NewAccount()
{
  //we want a fresh instance.. do we?
  if (m_newAccountPage)
    delete m_newAccountPage;

  m_newAccountPage = new TuiAccount("TuiNewAcc");
  m_statusCode = m_newAccountPage->Init(m_statusBar);
  if (m_statusCode >= SC_ERROR) {
    *log << ILog::ERROR << "Error in intiaizling new account page."
	 << this << ILog::endmsg;
    delete m_newAccountPage;
    m_newAccountPage = 0;
    return;
  }

  // call new account page
  m_newAccountPage->NewAccount();

  // add new record to IOSvc
  m_statusCode = m_newAccountPage->GetErrorMsg(m_errorMsg);
  if (m_statusCode < SC_ERROR) {
    m_statusCode = ioSvc->SetSource(m_newAccountPage->GetSource()); // destination for IOService
    if (m_statusCode >= SC_ERROR) {
      *log << ILog::FIXME << "Internal error in source selection: " << m_newAccountPage->GetSource().str()
	   << this << ILog::endmsg;
      return;
    }
    if (ioSvc->Add(m_newAccountPage->GetRecord()) == SC_OK)
      m_statusBar->StatusBar("New Account successfully added.");
    else
      m_statusBar->StatusBar("ERROR adding new account. Consult log file.");
  } else if (m_statusCode == SC_ABORT) {
    *log << ILog::INFO << "Nothing to change or user abort changes." << this << ILog::endmsg;
    m_statusBar->StatusBar("Aborted new account creation.");
  } else {
    *log << ILog::ERROR << "Error reported by NewAccount Page. New account not added." << this << ILog::endmsg;    
    m_statusBar->StatusBar("ERROR adding new account. Consult log file.");
  }

  m_newAccountPage->Kill();

  //delete and quit
  delete m_newAccountPage;
  m_newAccountPage = 0;
}

// ----------------------------------------
// Accessors

int TuiSvc::GetScreenRows()
{
  return m_screenRows;
}

int TuiSvc::GetScreenCols()
{
  return m_screenColumns;
}

IErrorHandler::StatusCode TuiSvc::SetWelcomeMessage(std::string msg)
{
  m_welcomeMessage = msg;
  return SC_OK;
}
