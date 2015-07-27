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
  m_nLinesForBrowsing = 10; 
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

  //now get the list of items and build the menu
  


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
  for (size_t it=0; it<m_loaListStr.size()+1; it++)
    free_item(m_loaList[it]);
  delete[] m_loaList;
  m_loaListStr.clear();
  m_loaList = 0;
  m_loaMenu = 0;

  return ITuiPage::Kill();

}

IErrorHandler::StatusCode TuiBrowse::Display()
{
  m_statusCode = SC_OK;

  return m_statusCode;
};

void TuiBrowse::SetNLinesForBrowsing(int pNLines)
{
  m_nLinesForBrowsing = pNLines;
}
