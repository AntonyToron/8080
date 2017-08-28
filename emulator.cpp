/*
  File name : emulator.cpp
  Author : Antony Toron

  Description : Contains the UI layer of the 8080_EMU

 */

#include <wx/wx.h>
#include <wx/menu.h>
#include <wx/sizer.h>
#include <wx/filename.h>
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

ImagePanel::ImagePanel(wxFrame *parent, wxString file, wxBitmapType format) :
  wxPanel(parent) {
  image.LoadFile(file, format);
}

void ImagePanel::paintEvent(wxPaintEvent & evt) {
  wxPaintDC dc(this);
  render(dc);
}

void ImagePanel::paintNow() {
  wxClientDC dc(this);
  render(dc);
}

void ImagePanel::render(wxDC & dc) {
  dc.DrawBitmap(image, 0, 0, false);
}


// IMAGE PANEL
			   
BEGIN_EVENT_TABLE (ImagePanel, wxPanel)
EVT_PAINT(ImagePanel::paintEvent)
END_EVENT_TABLE()

// CONSTRUCTOR OF THE CLASS, the list following : is the member initializer list
// it is necessary in this case to properly initialize as a wxFrame
MainFrame::MainFrame(const wxChar *title, int x, int y, int width, int height)
  : wxFrame((wxFrame *) NULL,
	    150,
	    title,
	    wxPoint(x, y),
	    wxSize(width, height),
	    wxDEFAULT_FRAME_STYLE,
	    _T("Main")
	    ) {
  // args: parent, id (default if -1), title, position, size
  // optional args: style, name (if to be referred to)

  menu_bar = (wxMenuBar *) NULL; // on ubuntu, this will display outside of window
  file_menu = (wxMenu *) NULL;

  // file dropdown, & in string indicates menu hotkey (alt + F opens file)
  file_menu = new wxMenu();
  file_menu->Append(BASIC_OPEN, _("&Open Custom ROM"));
  file_menu->Append(BASIC_ABOUT, _("&About"));
  file_menu->AppendSeparator();
  file_menu->Append(BASIC_EXIT, _("E&xit"), _("Exiting"));

  // add save states

  // toolbar
  /*
  toolbar = (wxToolBar *) NULL;
  toolbar = new wxToolBar(this, 150);

  toolbar->AddTool(wxID_EXIT, exit, 

  SetToolBar(toolbar);
  */
  
  
  // actual bar
  menu_bar = new wxMenuBar(1);
  menu_bar->Append(file_menu, _("&File"));
  SetMenuBar(menu_bar);

  status = new wxStatusBar(this, 150, 0, _T("StatusBar"));
  
  //CreateStatusBar(3);
  status->SetFieldsCount(3);
  status->SetStatusText("Ready", 0);
  status->SetStatusText("No ROM selected", 1);

  // can change these when necessary
  const int styles[] = { wxSB_RAISED, wxSB_RAISED, wxSB_FLAT };
  status->SetStatusStyles(3, styles);

  SetStatusBar(status);



  // display main image panel with ROMS
  wxInitAllImageHandlers();
  wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
  imagePanel = (ImagePanel *) NULL;

  //imagePanel = new ImagePanel(this, wxT("invaders_cabinet.jpg"), wxBITMAP_TYPE_JPEG);
  imagePanel = new ImagePanel(this, wxT("space_invader_icon1.png"), wxBITMAP_TYPE_PNG);
  sizer->Add(imagePanel,
	     1,
	     wxALIGN_CENTER | wxSHAPED | wxLEFT | wxTOP,
	     60);

  sizer->Add(
	     new wxButton(this, SELECT_ROM_BUTTON, "Select ROM"),
	     0,
	     wxALIGN_CENTER | wxALL,
	     40);
  
  SetSizer(sizer);

  //sizer->SetSizeHints(this);


  // DEFAULT
  rom = INVADERS;
  

  // can set working directory via wxGetCwd() into wxSetWorkingDirectory
}


// ~ IS THE DESTRUCTOR OF THE CLASS, CALLED WHEN NECESSARY TO CALL DESTRUCTOR
MainFrame::~MainFrame() {

}

// MAIN FRAME

BEGIN_EVENT_TABLE (MainFrame, wxFrame)
EVT_MENU (BASIC_EXIT, MainFrame::OnExit)
EVT_MENU (BASIC_ABOUT, MainFrame::OnAbout)
EVT_MENU (BASIC_OPEN, MainFrame::OnOpenFile)
EVT_BUTTON(SELECT_ROM_BUTTON, MainFrame::SelectROM)
END_EVENT_TABLE()

/*
  Define callbacks for menu options
 */

void MainFrame::OnOpenFile(wxCommandEvent & event) {
  wxFileDialog *openFileDialog = new wxFileDialog(this, _("Open ROM"), "", "", "All Files (*)|*", wxFD_OPEN, wxDefaultPosition);

  if (openFileDialog->ShowModal() == wxID_OK) {
    wxString path;
    //path.append(openFileDialog->GetDirectory());
    //path.append(wxFileName::GetPathSeparator());
    path.append(openFileDialog->GetFilename());

    status->SetStatusText(path, 2);
    status->SetStatusText("ROM Selected", 1);

    const int styles[] = { wxSB_RAISED, wxSB_RAISED, wxSB_RAISED };
    status->SetStatusStyles(3, styles);

    
  }
}

void MainFrame::OnAbout(wxCommandEvent & event) {
  wxString t = _T("About the 8080 Emulator:\nAuthor: Antony Toron\n 2017");
  
  wxMessageDialog aboutDialog(this, t, _T("About 8080_EMU"), wxOK | wxCANCEL);

  aboutDialog.ShowModal();
}

void MainFrame::OnExit(wxCommandEvent & event) {
  Close(true);
}

void MainFrame::SelectROM(wxCommandEvent & event) {
  printf ("ROM selected : %i\n", (int) rom);
  fflush (stdout);
}
