/*
 Project: Console-Secrets
 Copyright (C) 2013  Simone Pagan Griso
 Distributed WITHOUT ANY WARRANTY under the GPL-3.0 licence,
 see the LICENCE file or visit <http://www.gnu.org/licenses/>
 File: ITuiPage.h
 Description: Text User Interface general page
 Last Modified: $Id: ITuiPage.cc 54 2013-03-20 21:54:57Z gridge $
*/

#include "ITuiPage.h"
#include "TuiStatusBar.h"
#include "ILog.h"

extern ILog *log;

using namespace std;

ITuiPage::ITuiPage(string pName) : IErrorHandler(pName)
{
  m_pageStatus = NONE;
  m_statusBar = 0;
  m_wnd = 0;
  m_wnd_lines = 0;
  m_wnd_cols = 0;
  m_wnd_y = 0;
  m_wnd_x = 0;  
}

ITuiPage::~ITuiPage()
{
  if (m_wnd)
    delwin(m_wnd);
}

ITuiPage::PageStates ITuiPage::Status()
{
  return m_pageStatus;
}

void ITuiPage::SetPageStatus(PageStates pStatus)
{
  m_pageStatus = pStatus;
  *log << ILog::VERBOSE << "New page stauts:";
  switch (m_pageStatus) {
  case NONE:
    *log << "NONE";
    break;
  case CLEARED:
    *log << "CLEARED";
    break;
  case RUNNING:
    *log << "RUNNING";
    break;
  default:
    *log << "Invalid state. Fix me please!";    
  }
  *log << this << ILog::endmsg;
}

IErrorHandler::StatusCode ITuiPage::Init(TuiStatusBar *pStatusBar)
{
  if (pStatusBar)
    m_statusBar = pStatusBar;
  SetPageStatus(CLEARED);
  return m_statusCode = SC_OK;
}

IErrorHandler::StatusCode ITuiPage::Close()
{
  if (m_wnd) {
    if (m_statusBar) {
      m_statusBar->HeaderBar();
      //m_statusBar->StatusBar(); //we want to leave last status bar message
      m_statusBar->CommandBar();
    }
    wclear(m_wnd);
    wrefresh(m_wnd);
  }
  SetPageStatus(CLEARED);
  return m_statusCode = SC_OK;
}

IErrorHandler::StatusCode ITuiPage::Kill()
{
  if (m_wnd) {
    Close();
    delwin(m_wnd);
  }
  m_wnd = 0;
  SetPageStatus(NONE);
  return m_statusCode = SC_OK;
}

IErrorHandler::StatusCode ITuiPage::Display()
{
  wclear(m_wnd);
  wrefresh(m_wnd);
  SetPageStatus(RUNNING);
  return m_statusCode = SC_OK;
}

