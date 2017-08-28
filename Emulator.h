/*
  File name : Emulator.h
  Author : Antony Toron

  Description : This defines the interface for the emulator and declares
  the wxwidgets app.

 */

enum {BASIC_EXIT = 1, BASIC_OPEN = 100, BASIC_ABOUT = 200};



enum {SELECT_ROM_BUTTON = wxID_HIGHEST + 1};

class Emulator : public wxApp {
 public:
  virtual bool OnInit(); // DECLARED AS VIRTUAL SO THAT WXAPP DEFAULT IS OVERWRITTEN
};

class ImagePanel : public wxPanel {
 public:
  wxBitmap image;

  ImagePanel(wxFrame *parent, wxString file, wxBitmapType format);

  void paintEvent(wxPaintEvent & evt);
  void paintNow();

  void render(wxDC & dc);
  //void mouseDown(wxMouseEvent & event);

  DECLARE_EVENT_TABLE()
};

class MainFrame : public wxFrame {
 public:
  MainFrame(const wxChar *title, int x, int y, int width, int height);
  ~MainFrame();

  wxMenuBar *menu_bar; // actual menu bar
  wxMenu *file_menu; // file dropdown menu
  wxStatusBar *status;
  ImagePanel *imagePanel;

  ROM rom;
  
  
  void OnOpenFile (wxCommandEvent & event);
  void OnAbout (wxCommandEvent & event);
  void OnExit (wxCommandEvent & event);
  void SelectROM (wxCommandEvent & event);

  DECLARE_EVENT_TABLE() // alerts compiler that this class will have event table
};
  
