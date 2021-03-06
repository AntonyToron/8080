/*
  File name : emulator.h
  Author : Antony Toron

  Description : This defines the interface for the emulator and declares
  the wxwidgets app.

 */

enum {BASIC_EXIT = 1, BASIC_OPEN = 100, BASIC_ABOUT = 200};



enum {SELECT_ROM_BUTTON = wxID_HIGHEST + 1,
      DIPSWITCH_BUTTON = wxID_HIGHEST + 2,
      SAVE_DIPS_BUTTON = wxID_HIGHEST + 3};

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

class DipswitchDialog: public wxDialog {
 public:
  DipswitchDialog(wxWindow * parent, wxWindowID id, const wxString & title,
		  const wxPoint & pos = wxDefaultPosition,
		  const wxSize & size = wxDefaultSize,
		  long style = wxDEFAULT_DIALOG_STYLE);
  ~DipswitchDialog();

 private:
  void onOk(wxCommandEvent & event);

  wxSpinCtrl * currentDIPS_bank1[8];
  wxSpinCtrl * currentDIPS_bank2[8];

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
  
  std::map<ROM, DIPSettings_T> DIPS;
  
  void OnOpenFile (wxCommandEvent & event);
  void OnAbout (wxCommandEvent & event);
  void OnExit (wxCommandEvent & event);
  void SelectROM (wxCommandEvent & event);
  void OpenDipswitch (wxCommandEvent & event);
  void checkIfDipswitchSettingsExist();
  void saveDipswitchSettings();

  DECLARE_EVENT_TABLE() // alerts compiler that this class will have event table
};
  
