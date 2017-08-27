/*
  File name : emulator.cpp
  Author : Antony Toron

  Description : Contains the UI layer of the 8080_EMU

 */

#include <wx/wx.h>
#include "emulator.h"

// constructs Emualator and provides entry point
IMPLEMENT_APP(Emulator)

// THIS IS CALLED ON STARTUP
bool Emulator::OnInit() {
  // _T stands for text, turns literal into a Unicode wide character literal
  // if compiling with unicode support
  MainFrame *frame = new MainFrame(_T("8080_EMU"), 50, 50, 500, 700);

  // show the window
  frame->Show(true); // inherited from wxWindow
  SetTopWindow(frame);
  return true;
}


// CONSTRUCTOR OF THE CLASS, the list following : is the member initializer list
// it is necessary in this case to properly initialize as a wxFrame
MainFrame::MainFrame(const wxChar *title, int x, int y, int width, int height)
  : wxFrame((wxFrame *) NULL,
	    -1,
	    title,
	    wxPoint(x, y),
	    wxSize(width, height),
	    wxDEFAULT_FRAME_STYLE,
	    _T("Main")
	    ) {
  // args: parent, id (default if -1), title, position, size
  // optional args: style, name (if to be referred to)
  
}


// ~ IS THE DESTRUCTOR OF THE CLASS, CALLED WHEN NECESSARY TO CALL DESTRUCTOR
MainFrame::~MainFrame() {

}
