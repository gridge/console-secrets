/*
 Project: Console-Secrets
 Copyright (C) 2015  Simone Pagan Griso
 Distributed WITHOUT ANY WARRANTY under the GPL-3.0 licence,
 see the LICENCE file or visit <http://www.gnu.org/licenses/>
 File: TuiBrowse.cc
 Description: Text User Interface browsing existing accounts
 Last Modified: $Id$
*/

#include "TuiBrowse.h"
#include "MiscUtils.h"
#include "ILog.h" 
#include "IConfigurationService.h"
#include "MultipleSourceIOSvc.h"
#include "TuiSvc.h"

extern ILog *log;
extern IConfigurationService *cfgMgr;
extern IIOService *ioSvc;
extern TuiSvc *tuiSvc;

using namespace Ncurses;
using namespace std;
using namespace CSMUtils;

TuiBrowse::TuiBrowse(std::string pName) : ITuiPage(pName)
{
  m_loaWnd = 0;
  m_loaMenu = 0;
  m_editAccountPage = 0;
  m_loaMenu = 0;
  m_loaList = 0;
  m_nLinesForBrowsing = static_cast<int>(LINES/2); // a bit more than half-screen
  m_nColsPerItem = 40;
  m_editInNewPage = false;
}

TuiBrowse::~TuiBrowse()
{

}

IErrorHandler::StatusCode TuiBrowse::Init(TuiStatusBar *pStatusBar)
{
  m_statusCode = SC_OK;
  ITuiPage::Init(pStatusBar);
  // --- define window and menu for account browsing
  if (!m_statusBar) {
    *log << ILog::FIXME << "A valid status bar is needed to initialize this page!"
	 << this << ILog::endmsg;
    return m_statusCode;
  }
  
  m_wnd_lines = m_nLinesForBrowsing;
  if (m_wnd_lines > LINES - (m_statusBar->GetHeaderHeight() + m_statusBar->GetBottomBarHeight())) {
    m_wnd_lines = LINES - (m_statusBar->GetHeaderHeight() + m_statusBar->GetBottomBarHeight());
    m_editInNewPage = true;
    *log << ILog::WARNING << "Resizing browsing window to: " << m_wnd_lines << this << ILog::endmsg;
  }
  m_wnd_cols = COLS;
  m_wnd_y = m_statusBar->GetHeaderHeight()+1;
  m_wnd_x = 0;
  m_wnd = newwin(m_wnd_lines, m_wnd_cols, m_wnd_y, m_wnd_x);
  keypad(m_wnd, TRUE);
  //now subwin for menu
  m_loaWnd = subwin(m_wnd, m_wnd_lines, m_wnd_cols, m_wnd_y, m_wnd_x+1); //leave one column of space/border
  *log << ILog::DEBUG << "Created browse window: " 
       << "H:" << m_wnd_lines << " W:" << m_wnd_cols << " Y:" << m_wnd_y << " X:" << m_wnd_x << this << ILog::endmsg;

  //now get the initial full list of items and build the menu
  vector<ARecord*> allRecords = ioSvc->GetAllAccounts();
  UpdateListRecords(allRecords);

  //Set shortcuts for command-bar in navigation mode
  SetCommandBarNavigation();

  // Finally init the account edit page
  m_editAccountPage = new TuiAccount("TuiViewAccount");
  m_editAccountPage->SetWndOffset(0, m_nLinesForBrowsing+1);
  m_statusCode = m_editAccountPage->Init(pStatusBar);
  if (m_statusCode >= SC_ERROR) {
    *log << ILog::ERROR << "Error in initializing view account page" << this << ILog::endmsg;
    m_editAccountPage = 0;    
    return m_statusCode;
  }

  return m_statusCode;
}

IErrorHandler::StatusCode TuiBrowse::Close()
{ 
  return ITuiPage::Close();
}

IErrorHandler::StatusCode TuiBrowse::Kill()
{
  *log << ILog::DEBUG << "Destoying browse window" << this << ILog::endmsg;
  if (m_loaWnd) {
    wclear(m_loaWnd);
    delwin(m_loaWnd);
  }
  m_loaWnd = 0;
  if (m_loaMenu) {
    free_menu(m_loaMenu);
  }
  if (m_loaList) {
    for (size_t it=0; it<m_listOfRecords.size()+1; it++)
      free_item(m_loaList[it]);
    delete[] m_loaList;
  }
  m_listOfRecords.clear();
  m_loaLabels.clear();
  m_loaList = 0;
  m_loaMenu = 0;

  return ITuiPage::Kill();

}

IErrorHandler::StatusCode TuiBrowse::Display()
{
  m_statusCode = SC_OK;

  if (Status() != RUNNING) {
    //Need to refresh few things
    ITuiPage::Display();
    //display header, command bar. Do not clean status-bar now (keep last message)
    m_statusBar->HeaderBar(cfgMgr->GetTopMessage());
    SetCommandBarNavigation();
    //draw the main menu
    post_menu(m_loaMenu);
    //refresh screen
    wrefresh(m_wnd);  
  }

  set_current_item(m_loaMenu, m_loaList[0]);

  //enter the main loop
  int recordSelection = -1; 
  bool quitBrowsing=false;
  while (not quitBrowsing) {
    int c = getch();
    m_statusBar->StatusBar(); //clear status bar from previous messages. half_delay set to appropriate value.
    switch (c) {
    case KEY_DOWN:
      menu_driver(m_loaMenu, REQ_DOWN_ITEM);
      break;
    case KEY_UP:
      menu_driver(m_loaMenu, REQ_UP_ITEM);
      break;
    case KEY_LEFT:
      menu_driver(m_loaMenu, REQ_LEFT_ITEM);
      break;
    case KEY_RIGHT:
      menu_driver(m_loaMenu, REQ_RIGHT_ITEM);
      break;
    case KEY_NPAGE:
      menu_driver(m_loaMenu, REQ_SCR_DPAGE);
      break;
    case KEY_PPAGE:
      menu_driver(m_loaMenu, REQ_SCR_UPAGE);
      break;
    case KEY_ENTER: //Enter
    case 10: //Enter
    case 13: //Enter
      {
	//ok, find out what's the requested action and act accordingly
	ITEM *selItem = current_item(m_loaMenu);
	recordSelection = item_index(selItem);
	ViewRecord(m_listOfRecords[recordSelection]);
	break;
      }
      //now define shortcuts -- very inelegant sorry 
    case CTRL('c'):
      //quit
      quitBrowsing=true;
      break;
    case KEY_BACKSPACE:
      //clear last character of search pattern
      menu_driver(m_loaMenu, REQ_BACK_PATTERN);
      break;
    case KEY_CANCEL:
      //delete search pattern
      menu_driver(m_loaMenu, REQ_CLEAR_PATTERN);
      break;
    case CTRL('s'):
      //next search match
      menu_driver(m_loaMenu, REQ_NEXT_MATCH);
      break;
    case CTRL('a'):
      //previous search match
      menu_driver(m_loaMenu, REQ_PREV_MATCH);
      break;
    case CTRL('n'):
      m_statusBar->StatusBar("Not yet implemented");
      break;
    case CTRL('d'):
      m_statusBar->StatusBar("Not yet implemented");
      break;
    case CTRL('r'):
      {
	ITEM *selItem = current_item(m_loaMenu);
	recordSelection = item_index(selItem);
	RemoveRecord(m_listOfRecords[recordSelection]);
	break;      
      }
    default:
      //assume it's a search character
      if (c < 255) {
	*log << ILog::VERBOSE << "Add to search buffer: " << (char)c << this << ILog::endmsg;
	menu_driver(m_loaMenu, (char)c);
      }
    }
    wrefresh(m_wnd);
    // catch errors
    if (m_statusCode >= SC_FATAL) {
      *log << ILog::FATAL << "ERROR condition detected. Closing Tui."
	   << this << ILog::endmsg;
      return m_statusCode;
    }
  }

  wclear(m_loaWnd);
  wrefresh(m_loaWnd);

  return m_statusCode;
};

void TuiBrowse::SetNLinesForBrowsing(int pNLines)
{
  m_nLinesForBrowsing = pNLines;
}

void TuiBrowse::SetNColumnsPerItem(int pNCols) 
{
  m_nColsPerItem = pNCols;
}

IErrorHandler::StatusCode TuiBrowse::UpdateListRecords(vector<ARecord*> &newList)
{
  //clear previous list, if any
  if (m_loaMenu) {
    unpost_menu(m_loaMenu);
    free_menu(m_loaMenu);
  }
  if (m_loaList) {
    for (size_t it=0; it<m_listOfRecords.size()+1; it++)
      free_item(m_loaList[it]);
    delete[] m_loaList;
  }
  m_loaList = 0;
  m_loaMenu = 0;
  m_listOfRecords.clear();
  m_loaLabels.clear();

  //Now assign new list and create menu items
  // format of items: ModificationDate, AccountName
  m_listOfRecords = newList;
  m_loaList = new ITEM*[m_listOfRecords.size()+1];
  for (unsigned int idxR = 0; idxR < m_listOfRecords.size(); ++idxR) {
    m_loaLabels.push_back(make_pair(m_listOfRecords[idxR]->GetModificationTimeStr(),
				    m_listOfRecords[idxR]->GetAccountName()));
    m_loaList[idxR] = new_item(m_loaLabels[idxR].first.c_str(),m_loaLabels[idxR].second.c_str());
  }
  m_loaList[m_listOfRecords.size()] = new_item(NULL,NULL); //end-of-array
  m_loaMenu = new_menu((ITEM**)m_loaList);
  
  //now set menu properties
  //  set_menu_spacing(m_loaMenu, 3, 2, 0);
  menu_opts_on(m_loaMenu, O_SHOWMATCH);
  int nColumns = static_cast<int>(COLS / m_nColsPerItem);
  *log << ILog::DEBUG << "Adapted display to n. columns =" << nColumns << "(" << COLS << " columns available" << this << ILog::endmsg;
  set_menu_format(m_loaMenu, m_wnd_lines-1, nColumns);
  set_menu_mark(m_loaMenu, NULL);
  int err = set_menu_win(m_loaMenu, m_wnd);  
  if (err != 0) {
    *log << ILog::ERROR << "Error associating menu to window: " << err << this << ILog::endmsg;
    return m_statusCode;
  }
  err = set_menu_sub(m_loaMenu, m_loaWnd);  
  if (err != 0) {
    *log << ILog::ERROR << "Error associating menu to sub-window: " << err << this << ILog::endmsg;
    return m_statusCode;
  }

  return m_statusCode = SC_OK;
}

void TuiBrowse::SetCommandBarNavigation()
{
  std::vector< std::pair<std::string, std::string> > m_commands;
  m_commands.push_back(make_pair("^C", "Main Menu"));
  m_commands.push_back(make_pair("Del","Clear search"));
  m_commands.push_back(make_pair("^S", "Next match"));
  m_commands.push_back(make_pair("^A", "Prev. match"));
  m_commands.push_back(make_pair("^N", "Order by Name"));
  m_commands.push_back(make_pair("^D", "Order by Date"));
  m_commands.push_back(make_pair("^R", "Remove record"));
  

  m_statusBar->CommandBar(m_commands);
}

void TuiBrowse::ViewRecord(ARecord *record) 
{
  //Load record into viewing page and *lock* it
  m_editAccountPage->SetRecord(record);
  m_editAccountPage->LockFields();

  //Launch record viewing
  m_editAccountPage->Display();

  //check if account was modified simply looking if fields are still locked
  if (not m_editAccountPage->IsFieldsLocked()) {
    m_statusCode = m_editAccountPage->GetErrorMsg(m_errorMsg);
    if (m_statusCode < SC_ERROR) {
      //record was passed by pointer, so it's up-to-date, need to trigger writing to disk though
      m_statusCode = ioSvc->Store();
      if (m_statusCode >= SC_ERROR) {
	*log << ILog::INFO << "Error saving changes." << this << ILog::endmsg;
	m_statusBar->StatusBar("ERROR saving changes. Changes likely not saved.");
      } else {
	*log << ILog::INFO << "Saved changes to record." << this << ILog::endmsg;
	m_statusBar->StatusBar("Successfully saved changes.");
      }
    } else if (m_statusCode == SC_ABORT) {
      *log << ILog::INFO << "Aborting editing of account." << this << ILog::endmsg;
      m_statusBar->StatusBar("Account editing aborted. Changes were not saved.");
    } else {
      //error
      *log << ILog::INFO << "Error editing of account." << this << ILog::endmsg;
      m_statusBar->StatusBar("ERROR in editing account. Changes were not saved.");      
    }

  }
  
  //restore command bar, clean window
  m_editAccountPage->Close();
  SetCommandBarNavigation();
}

void TuiBrowse::RemoveRecord(ARecord *record)
{
  //ask confirmation before proceeding
  std::string removeAnswer;
  m_statusBar->StatusBar("Are you sure to remove this record PERMANENTLY?(yes/no)", removeAnswer);
  if (removeAnswer == "yes") {
    std::string nameOfRemoved = record->GetAccountName();
    //remove record
    m_statusCode = ioSvc->Remove(record);
    if (m_statusCode >= SC_ERROR) {
      m_statusBar->StatusBar("ERROR removing record. Aborting removal.");
      return;
    }
    //print confirmation and update current list of records
    if (m_statusCode < SC_ERROR) m_statusBar->StatusBar(string("Removed record: ")+nameOfRemoved);
    vector<ARecord*> newListRecords = ioSvc->GetAllAccounts();
    UpdateListRecords(newListRecords);
    post_menu(m_loaMenu);
    //refresh screen
    wrefresh(m_wnd);  
  } else {
    m_statusBar->StatusBar("Removal CANCELLED.");
  }  
  SetCommandBarNavigation();
}

