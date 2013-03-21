/*
 Project: Console-Secrets
 Copyright (C) 2013  Simone Pagan Griso
 Distributed WITHOUT ANY WARRANTY under the GPL-3.0 licence,
 see the LICENCE file or visit <http://www.gnu.org/licenses/>
 File: TuiStatusBar.cc
 Description: Text User Interface status bar, command bar and header management
 Last Modified: $Id: TuiStatusBar.cc 54 2013-03-20 21:54:57Z gridge $
*/

#include "TuiStatusBar.h"
#include "ILog.h"
#include "IConfigurationService.h"
#include "TuiSvc.h"
#include "MiscUtils.h"

#include <errno.h>

extern ILog *log;
extern IConfigurationService *cfgMgr;
extern TuiSvc *tuiSvc;

using namespace std;
using namespace Ncurses;
using namespace CSMUtils;

TuiStatusBar::TuiStatusBar(string pName) : IErrorHandler(pName)
{
  m_hwnd = 0;
  m_sbwnd = 0;
  m_headerHeight = 1; // 1 line for the header on the top
  if (COLS < 100)
    m_statusBarHeight = 3; // 3 lines = 2 (command bar) + 1 (status bar)
  else
    m_statusBarHeight = 2; // 2 lines = 1 (command bar) + 1 (status bar)
  m_maxDescriptionSize = 20;
  m_headerFixedString = "CSM v";
  m_headerFixedString += cfgMgr->version;  
}

TuiStatusBar::~TuiStatusBar()
{
  if (m_hwnd)
    delwin(m_hwnd);
  if (m_sbwnd)
    delwin(m_sbwnd);
}

void TuiStatusBar::Init()
{
  //now init windows
  m_hwnd = newwin(m_headerHeight, COLS, 0, 0);
  wattrset(m_hwnd, A_REVERSE);
  m_sbwnd = newwin(m_statusBarHeight, COLS, LINES - m_statusBarHeight, 0);
  keypad(m_sbwnd, TRUE);
}

void TuiStatusBar::HeaderBar(std::string pHeader, bool pRefresh)
{
  string std_header_left = m_headerFixedString;
  //pad with spaces
  size_t std_header_size =std_header_left.size();
  for (int p=std_header_left.size(); p<COLS; p++) 
    std_header_left+= " ";
  //attribute A_REVERSE already set at the beginning for this window
  wattron(m_hwnd, A_BOLD);
  mvwprintw(m_hwnd, 0,0,std_header_left.c_str());
  wattroff(m_hwnd, A_BOLD);
  wclrtoeol(m_hwnd);    

  if (!pHeader.empty()) {
    //now print pHeader in the middle
    size_t colHeader = (COLS - pHeader.size()) / 2;
    if (colHeader < std_header_size + 1)
      colHeader = std_header_size + 1;
    mvwprintw(m_hwnd, 0, colHeader, pHeader.c_str());    
  }

  //refresh window
  if (pRefresh)
    wrefresh(m_hwnd);  
}

int TuiStatusBar::GetHeaderHeight()
{
  return m_headerHeight;
}

void TuiStatusBar::CommandBar(vector<pair<string, string> >pCommands, bool pRefresh)
{
  //position of entry, respect to m_sbwnd
  int startCol = 0; //respect to m_sbwnd (space for statusBar)
  int startRow = 1;
  int maxSize = m_maxDescriptionSize; //maximum size of description
  //clear command bar
  for (int cr=startRow; cr < m_statusBarHeight; cr++) {
    wmove(m_sbwnd, cr, startCol);
    wclrtoeol(m_sbwnd);  
  }
  //loop over map
  for (vector<pair<string, string> >::iterator it = pCommands.begin(); it != pCommands.end(); ++it) {
    //first print shortcut
    wattron(m_sbwnd, A_REVERSE);
    //assume shortcut will be one or few charachters, no check for maximum length
    mvwprintw(m_sbwnd, startRow, startCol, it->first.c_str());
    wattroff(m_sbwnd, A_REVERSE);
    //print description, do not exceed maxSize charachters in total (including shortcut)
    // leave 1 blank, account for blank between shortcut and description
    mvwprintw(m_sbwnd, startRow, startCol + it->first.size() + 1, it->second.substr(0, maxSize -1 - it->first.size() - 1).c_str());
    *log << ILog::DEBUG << "Adding to StatusBar new labels: " 
	 << it->first << " " << it->second << ", at position "
	 << startRow <<"," << startCol << this << ILog::endmsg;
    //ok, advance in col,row
    startRow++;
    if (startRow > m_statusBarHeight - 1) {
      startRow = 1; //respect to m_sbwnd (space for statusBar)
      startCol += maxSize;
      if (startCol >= COLS) {
	//ok, they're too many.. sorry :)
	break;
      }
    }
  } 

  //save for future internal reference
  m_commands = pCommands;

  //refresh and quit
  if (pRefresh)
    wrefresh(m_sbwnd);
}

void TuiStatusBar::CommandBar(bool pRefresh)
{
  //create empty vector;
  vector<pair<string, string> >pCommands;
  CommandBar(pCommands, pRefresh);
}

void TuiStatusBar::StatusBar(string pStatus, bool pRefresh)
{
  //write marker at the beginning of the status bar
  wattron(m_sbwnd, A_REVERSE);
  mvwprintw(m_sbwnd, 0, 0, " ");

  //write status message
  if (!pStatus.empty())
    mvwprintw(m_sbwnd, 0, 1, pStatus.c_str());

  //clear the rest of the line
  wclrtoeol(m_sbwnd);

  wattroff(m_sbwnd, A_REVERSE);

  //refresh window
  if (pRefresh)
    wrefresh(m_sbwnd);
}

IErrorHandler::StatusCode TuiStatusBar::StatusBar(string pQuestion, string &pAnswer, TStatusBarInput pInputType)
{
  m_statusCode = SC_OK;

  //-- First print the question
  string pRealQ = pQuestion;
  pRealQ += "?";
  StatusBar(pRealQ, true);
  //-- Now retrieve the answer

  //- create a form of one field to get the answer
  WINDOW *sbQuestionWnd = derwin(m_sbwnd, 1, tuiSvc->GetScreenCols() - pRealQ.length() - 1, 0, pRealQ.length()+1);
  *log << ILog::DEBUG << "Created window: H: 1, W:" << (int)(tuiSvc->GetScreenCols() - pRealQ.length() - 1)
       << ", Y: 0, X:" << (int)(pRealQ.length()+1) << this << ILog::endmsg;
  FORM *sbQuestionForm;
  FIELD *sbQuestionF[2];
  //one space left for status-bar msg, 1 b/w question and answer
  if (pInputType == SBIN_YN) {
    sbQuestionF[0]= new_field( 1, 1, 0, 1, 0, 0 ); //just one char
  } else {
    sbQuestionF[0]= new_field( 1, tuiSvc->GetScreenCols() - pRealQ.length() - 2, 0, 1, 0, 0 );
  }
  *log << "Created new field: H: 1, W:" << (int)(tuiSvc->GetScreenCols() - pRealQ.length() - 2)
       << "Y:0, X:" <<  1 << this << ILog::endmsg;
  /*
  field_opts_on(sbQuestionF[0], O_VISIBLE);
  field_opts_on(sbQuestionF[0], O_ACTIVE);
  field_opts_on(sbQuestionF[0], O_PUBLIC);
  field_opts_on(sbQuestionF[0], O_WRAP);
  field_opts_off(sbQuestionF[0], O_BLANK);
  */
  field_opts_off(sbQuestionF[0], O_AUTOSKIP);
  field_opts_off(sbQuestionF[0], O_STATIC); //dynamic fields
  set_max_field(sbQuestionF[0], 65534);
  set_field_back(sbQuestionF[0], A_UNDERLINE);
  sbQuestionF[1] = NULL;
  sbQuestionForm = new_form(sbQuestionF);
  set_form_win(sbQuestionForm, m_sbwnd);
  set_form_sub(sbQuestionForm, sbQuestionWnd);

  // setup status bar for aborting, save previous one
  vector<pair<string, string> > savedCB; //quick command bar
  vector<pair<string, string> > smallCB; //quick command bar
  savedCB = m_commands;
  smallCB.push_back( make_pair<string, string>("^C", "Cancel") );
  CommandBar(smallCB); //post small command bar, allows user abort

  // print and get the input
  post_form(sbQuestionForm);
  set_current_field(sbQuestionForm, sbQuestionF[0]);
  form_driver(sbQuestionForm, REQ_BEG_FIELD);
  curs_set(1); //should be at the active field
  bool quitQuestion = false;
  int ch;

  while (!quitQuestion) {
    wrefresh(m_sbwnd);
    ch = wgetch(m_sbwnd);
    if (pInputType == SBIN_YN) {
      //only y/n or variants accepted
      if (ch == 'y' || ch == 'Y') {
	pAnswer = "Y";
	quitQuestion = true;	
      } else if (ch == 'n' || ch == 'N') {
	pAnswer = "N";
	quitQuestion = true;
      } else if (ch == CTRL('c')) {
	m_statusCode = SC_ABORT;
	quitQuestion = true;
      }
      //otherwise print ask next character (only y/n accepted)
    } else {
      //all other types, string-like editing with movements
      switch (ch) {
	// moving actions
      case KEY_RIGHT:
	//todo: check end of string
	form_driver(sbQuestionForm, REQ_NEXT_CHAR);
	break;
      case KEY_LEFT:
	form_driver(sbQuestionForm, REQ_PREV_CHAR);
	break;
      case CTRL('e'):
	form_driver(sbQuestionForm, REQ_END_FIELD);
	break;
      case CTRL('a'):
	form_driver(sbQuestionForm, REQ_BEG_FIELD);
	break;
	
	// Editing actions
      case KEY_BACKSPACE:
      case CTRL('?'):
      case CTRL('h'):
      case 127: //XFree 4 style
	form_driver(sbQuestionForm, REQ_DEL_PREV);      
	break;
      case CTRL('d'):
      case KEY_DC:
	form_driver(sbQuestionForm, REQ_DEL_CHAR);      
      break;
      case CTRL('k'):
	form_driver(sbQuestionForm, REQ_CLR_EOF);      
	break;
	
	//quit question loop
      case CTRL('c'):
	m_statusCode = SC_ABORT;
	quitQuestion = true;
	break;
      case KEY_ENTER:
      case 10:
      case 13:
	{
	  //ok input acquired.
	  form_driver(sbQuestionForm, REQ_BEG_FIELD);
	  pAnswer = TrimStr( static_cast<char *>( field_buffer(sbQuestionF[0], 0) ) );
	  m_statusCode = SC_OK;
	  quitQuestion = true;
	  break;      
	}

	//form driver
      default:
	form_driver(sbQuestionForm, ch);
	break;
      } // end key switch
    }
  } // end input loop

  *log << ILog::DEBUG << "Question: '" << pQuestion << "', Answer: '" << pAnswer << "'" << this << ILog::endmsg;

  // -- quit
  unpost_form(sbQuestionForm);
  free_form(sbQuestionForm);
  free_field(sbQuestionF[0]);
  delwin(sbQuestionWnd);
  StatusBar();  //clear
  CommandBar(savedCB); //restore previous command bar
  curs_set(0);
  *log << ILog::DEBUG << "Quitting." << this << ILog::endmsg;
  return m_statusCode;
}

int TuiStatusBar::GetBottomBarHeight()
{
  return m_statusBarHeight; //includes command bar
}
