//
// File: pxi.cc -- implementation of main classes for PXI.  [Plot X V2.0]
//

//****************************************************************************
// PXI: This program is designed to primarily plot data that was generated by*
// the Gambit program.  However, it can also read other data files that      *
// conform to the file layout.  The data basically consists of columns of data
// each representing a y-value corresponding to a lambda value.  Lamda values
// are in a separate column.
// The format is as follows:
// Note: all the strings in [] should appear EXACTLY as shown, including the
// ':', excluding the []
// comments are in /* or // and should not be in the data file
// [Dimensionality:]
//  # of datasets, # of datapoints for each dataset
//  2 3 3 means 2 data sets w/ 3 datapoints each
// [Settings:]
// LambdaMin		// minimum value of 'x'
// LambdaMax		// maximum value of 'x'
// LambdaDel		// step in 'x' (assumed to be LambdaDel-1 for Log plots)
// DataMin		// minimum value the data values take
// DataMax		// maximum value the data values take
// DataType		// Can be either 1-Log or 0-Linear

// The following is optional, and might not be used in future versions
// [Game:]
// Order of coefficient reading
// Game matrix
// Only works for 2 dataset files
// [Extra:]
// Extra information about the algorithm that generated the file
// ReadExtra		// Set to 0 unless certain that PXI can use that data
// Merror		// Tolerance
// DelP			// P-step

// Required again
// [DataFormat:]
// # of columns
// column # for lambda values
// column # for objective function values (0 for none)
// Now, column # of datapoint #i for dataset #j, in order i=1..DataPoints[j],
// j=1..DataSets
// last entry can be 0 to indicate that it is obtained by 1-Sum[strategies[1..#strategies-1]]
// [Data:]
// The data, columns separated by white space.

#include <math.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // WX_PRECOMP

#include "guishare/dlabout.h"

#include "expdata.h"
#include "pxi.h"
#include "pxichild.h"

#include "wx/sizer.h"
#include "wx/fontdlg.h"
#include "wx/colordlg.h"
#include "wx/print.h"
#include "wx/printdlg.h"

gOutput &operator<<(gOutput &op,const PxiPlot::LABELSTRUCT &l) 
  //{op<<l.x<<' '<<l.y<<' '<<l.label<<'\n';return op;}
{ return op; }

// The `main program' equivalent, creating the windows and returning the
// main frame
bool PxiApp::OnInit(void)
{
  wxConfig config("PXI");

  // Create the main frame window.
  PxiFrame *pxiFrame = new PxiFrame(0, "PXI", wxPoint(0, 0), wxSize(800, 600));
  // Set up the help system.
  wxString helpDir = wxGetWorkingDirectory();
  config.Read("Help-Directory", &helpDir);
  
  pxiFrame->Show(true);
  if (argc>1) pxiFrame->LoadFile( (wxString) (argv[1]));
  return true;
}

IMPLEMENT_APP(PxiApp)

#include "../bitmaps/pxi.xpm"

//=====================================================================
//                       class PxiFrame
//=====================================================================

PxiFrame::PxiFrame(wxFrame *p_parent, const wxString &p_title,
		   const wxPoint &p_position, const wxSize &p_size, long p_style) :
  wxFrame(p_parent, -1, p_title, p_position, p_size,p_style)
{
  SetIcon(wxICON(pxi));
    
  wxMenu *fileMenu = new wxMenu;
  fileMenu->Append(wxID_OPEN, "&Open", "Load file");
  fileMenu->Append(wxID_EXIT, "E&xit", "Exit PXI");

  wxMenu *helpMenu = new wxMenu;
  helpMenu->Append(wxID_HELP_CONTENTS, "&Contents", "Table of contents");
  helpMenu->Append(wxID_HELP_INDEX, "&Index", "Index of help file");
  helpMenu->AppendSeparator();
  helpMenu->Append(wxID_ABOUT, "&About", "About PXI");

  wxMenuBar *menu_bar = new wxMenuBar;
  menu_bar->Append(fileMenu, "&File");
  menu_bar->Append(helpMenu, "&Help");
  SetMenuBar(menu_bar);

  wxConfig config("PXI");
  m_fileHistory.Load(config);
  m_fileHistory.UseMenu(fileMenu);
  m_fileHistory.AddFilesToMenu();

  CreateStatusBar();
  MakeToolbar();
}


PxiFrame::~PxiFrame()
{
  wxConfig config("PXI");
  m_fileHistory.Save(config);
}

BEGIN_EVENT_TABLE(PxiFrame, wxFrame)
  EVT_MENU(wxID_OPEN, PxiFrame::OnFileOpen) 
  EVT_MENU(wxID_EXIT, PxiFrame::OnCloseWindow)
  EVT_MENU_RANGE(wxID_FILE1, wxID_FILE9, PxiFrame::OnMRUFile)
  EVT_MENU(wxID_HELP_CONTENTS, PxiFrame::OnHelpContents)
  EVT_MENU(wxID_HELP_INDEX, PxiFrame::OnHelpIndex)
  EVT_MENU(wxID_ABOUT, PxiFrame::OnHelpAbout)
  EVT_CLOSE(PxiFrame::OnCloseWindow)
END_EVENT_TABLE()

void PxiFrame::OnFileOpen(wxCommandEvent &)
{
  wxString filename = wxFileSelector("Load data file", wxGetApp().CurrentDir(),
				  NULL, NULL, "*.pxi").c_str();
  if (filename == "") {
    return;
  }

  wxGetApp().SetCurrentDir(wxPathOnly(filename));
  LoadFile(filename);
}

void PxiFrame::OnMRUFile(wxCommandEvent &p_event)
{
  LoadFile(m_fileHistory.GetHistoryFile(p_event.GetId() - wxID_FILE1).c_str());
}

void PxiFrame::OnHelpContents(wxCommandEvent &)
{
}

void PxiFrame::OnHelpIndex(wxCommandEvent &)
{
}

void PxiFrame::OnHelpAbout(wxCommandEvent &)
{
  dialogAbout dialog(this, "About PXI...",
		     "PXI Quantal Response Plotting Program",
		     "Version 0.97 (alpha)");
  dialog.ShowModal();
}

void PxiFrame::LoadFile(const wxString &p_filename)
{    
  if (p_filename.Contains(".pxi")) {
    if (!wxFileExists(p_filename)) {
      wxMessageBox("File could not be found or opened");
      return;
    }
    else {
      m_fileHistory.AddFileToHistory(p_filename);
    }
    (void) new PxiChild(this,p_filename);
  }
  else {
    wxMessageBox("Unknown file type");
  }
}

#include "../sources/bitmaps/open.xpm"
#include "../sources/bitmaps/help.xpm"

void PxiFrame::MakeToolbar(void)
{
  wxToolBar *toolBar = CreateToolBar(wxNO_BORDER | wxTB_FLAT | wxTB_DOCKABLE |
				     wxTB_HORIZONTAL);

  toolBar->SetMargins(4, 4);

  toolBar->AddTool(wxID_OPEN, wxBITMAP(open), wxNullBitmap, false,
		   -1, -1, 0, "Open", "Open a saved datafile");
  toolBar->AddSeparator();
  toolBar->AddTool(wxID_HELP_CONTENTS, wxBITMAP(help), wxNullBitmap, false,
		   -1, -1, 0, "Help", "Table of contents");

  toolBar->Realize();
  toolBar->SetRows(1);
}

void PxiFrame::OnCloseWindow(wxCloseEvent &)
{
  Destroy();
}

//
// A general-purpose dialog box to display the description of the exception
//

void pxiExceptionDialog(const wxString &p_message, wxWindow *p_parent,
            long p_style /*= wxOK | wxCENTRE*/)
{
  wxString message = "An internal error occurred in PXI:\n" + p_message;
  wxMessageBox(message, "PXI Error", p_style, p_parent);
}


PxiPrintout::PxiPrintout(PxiPlot &c, char *title)
  : wxPrintout(title), canvas(c)
{ }

PxiPrintout::~PxiPrintout(void) 
{ }

bool PxiPrintout::OnPrintPage(int page)
{
  wxDC *dc = GetDC();
  if (dc) {
    dc->SetDeviceOrigin(0, 0);
    dc->SetUserScale(1.0, 1.0);
    
    canvas.Update(*dc,PXI_UPDATE_PRINTER);
    
    return true;
  }
  else
    return false;
}

bool PxiPrintout::OnBeginDocument(int startPage, int endPage)
{
  if (!wxPrintout::OnBeginDocument(startPage, endPage))
    return false;
  return true;
}

void PxiPrintout::GetPageInfo(int *minPage, int *maxPage, int *selPageFrom, int *selPageTo)
{
  //  int max =  (canvas.DrawSettings())->GetMaxPage();
  int max = 1;
  *minPage =1;
  *maxPage = max;
  *selPageFrom = 1;
  *selPageTo = max;
}

bool PxiPrintout::HasPage(int pageNum)
{
  return (pageNum == 1);
}



#ifdef UNUSED
void MyPrintout::DrawPageOne(wxDC *dc)
{
/* You might use THIS code if you were scaling
* graphics of known size to fit on the page.
    */
    int w, h;
    
    // We know the graphic is 200x200. If we didn't know this,
    // we'd need to calculate it.
    float maxX = 200;
    float maxY = 200;
    
    // Let's have at least 50 device units margin
    float marginX = 50;
    float marginY = 50;
    
    // Add the margin to the graphic size
    maxX += (2*marginX);
    maxY += (2*marginY);
    
    // Get the size of the DC in pixels
    dc->GetSize(&w, &h);
    
    // Calculate a suitable scaling factor
    float scaleX=(float)(w/maxX);
    float scaleY=(float)(h/maxY);
    
    // Use x or y scaling factor, whichever fits on the DC
    float actualScale = wxMin(scaleX,scaleY);
    
    // Calculate the position on the DC for centring the graphic
    float posX = (float)((w - (200*actualScale))/2.0);
    float posY = (float)((h - (200*actualScale))/2.0);
    
    // Set the scale and origin
    dc->SetUserScale(actualScale, actualScale);
    dc->SetDeviceOrigin( (long)posX, (long)posY );
    
    frame->Draw(*dc);
}

void MyPrintout::DrawPageTwo(wxDC *dc)
{
/* You might use THIS code to set the printer DC to ROUGHLY reflect
* the screen text size. This page also draws lines of actual length 5cm
* on the page.
    */
    // Get the logical pixels per inch of screen and printer
    int ppiScreenX, ppiScreenY;
    GetPPIScreen(&ppiScreenX, &ppiScreenY);
    int ppiPrinterX, ppiPrinterY;
    GetPPIPrinter(&ppiPrinterX, &ppiPrinterY);
    
    // This scales the DC so that the printout roughly represents the
    // the screen scaling. The text point size _should_ be the right size
    // but in fact is too small for some reason. This is a detail that will
    // need to be addressed at some point but can be fudged for the
    // moment.
    float scale = (float)((float)ppiPrinterX/(float)ppiScreenX);
    
    // Now we have to check in case our real page size is reduced
    // (e.g. because we're drawing to a print preview memory DC)
    int pageWidth, pageHeight;
    int w, h;
    dc->GetSize(&w, &h);
    GetPageSizePixels(&pageWidth, &pageHeight);
    
    // If printer pageWidth == current DC width, then this doesn't
    // change. But w might be the preview bitmap width, so scale down.
    float overallScale = scale * (float)(w/(float)pageWidth);
    dc->SetUserScale(overallScale, overallScale);
    
    // Calculate conversion factor for converting millimetres into
    // logical units.
    // There are approx. 25.1 mm to the inch. There are ppi
    // device units to the inch. Therefore 1 mm corresponds to
    // ppi/25.1 device units. We also divide by the
    // screen-to-printer scaling factor, because we need to
    // unscale to pass logical units to DrawLine.
    
    // Draw 50 mm by 50 mm L shape
    float logUnitsFactor = (float)(ppiPrinterX/(scale*25.1));
    float logUnits = (float)(50*logUnitsFactor);
    dc->SetPen(* wxBLACK_PEN);
    dc->DrawLine(50, 250, (long)(50.0 + logUnits), 250);
    dc->DrawLine(50, 250, 50, (long)(250.0 + logUnits));
    
    dc->SetFont(* wxGetApp().m_testFont);
    dc->SetBackgroundMode(wxTRANSPARENT);
    
    dc->DrawText("Some test text", 200, 200 );
    
    { // GetTextExtent demo:
        wxString words[7] = {"This ", "is ", "GetTextExtent ", "testing ", "string. ", "Enjoy ", "it!"};
        long w, h;
        long x = 200, y= 250;
        wxFont fnt(15, wxSWISS, wxNORMAL, wxNORMAL);
        
        dc->SetFont(fnt);
        for (int i = 0; i < 7; i++) {
            dc->GetTextExtent(words[i], &w, &h);
            dc->DrawRectangle(x, y, w, h);
            dc->DrawText(words[i], x, y);
            x += w;
        }
        dc->SetFont(* wxGetApp().m_testFont);
    }
    
    // TESTING
    
    int leftMargin = 20;
    int rightMargin = 20;
    int topMargin = 20;
    int bottomMargin = 20;
    
    int pageWidthMM, pageHeightMM;
    GetPageSizeMM(&pageWidthMM, &pageHeightMM);
    
    float leftMarginLogical = (float)(logUnitsFactor*leftMargin);
    float topMarginLogical = (float)(logUnitsFactor*topMargin);
    float bottomMarginLogical = (float)(logUnitsFactor*(pageHeightMM - bottomMargin));
    float rightMarginLogical = (float)(logUnitsFactor*(pageWidthMM - rightMargin));
    
    dc->SetPen(* wxRED_PEN);
    dc->DrawLine( (long)leftMarginLogical, (long)topMarginLogical, 
        (long)rightMarginLogical, (long)topMarginLogical);
    dc->DrawLine( (long)leftMarginLogical, (long)bottomMarginLogical, 
        (long)rightMarginLogical,  (long)bottomMarginLogical);
    
    WritePageHeader(this, dc, "A header", logUnitsFactor);
}


void MyFrame::OnPrintSetup(wxCommandEvent& WXUNUSED(event))
{
    wxPrintDialogData printDialogData(* m_printData);
    wxPrintDialog printerDialog(this, & printDialogData);

    printerDialog.GetPrintDialogData().SetSetupDialog(TRUE);
    printerDialog.ShowModal();

    (*m_printData) = printerDialog.GetPrintDialogData().GetPrintData();
}

void MyFrame::OnPageSetup(wxCommandEvent& WXUNUSED(event))
{
    (*m_pageSetupData) = * m_printData;

    wxPageSetupDialog pageSetupDialog(this, m_pageSetupData);
    pageSetupDialog.ShowModal();

    (*m_printData) = pageSetupDialog.GetPageSetupData().GetPrintData();
    (*m_pageSetupData) = pageSetupDialog.GetPageSetupData();
}

#if defined(__WXMSW__) && wxTEST_POSTSCRIPT_IN_MSW
void MyFrame::OnPrintPS(wxCommandEvent& WXUNUSED(event))
{
    wxPostScriptPrinter printer(m_printData);
    MyPrintout printout("My printout");
    printer.Print(this, &printout, TRUE);

    (*m_printData) = printer.GetPrintData();
}

void MyFrame::OnPrintPreviewPS(wxCommandEvent& WXUNUSED(event))
{
    // Pass two printout objects: for preview, and possible printing.
    wxPrintDialogData printDialogData(* m_printData);
    wxPrintPreview *preview = new wxPrintPreview(new MyPrintout, new MyPrintout, & printDialogData);
    wxPreviewFrame *frame = new wxPreviewFrame(preview, this, "Demo Print Preview", wxPoint(100, 100), wxSize(600, 650));
    frame->Centre(wxBOTH);
    frame->Initialize();
    frame->Show(TRUE);
}

void MyFrame::OnPrintSetupPS(wxCommandEvent& WXUNUSED(event))
{
    wxPrintDialogData printDialogData(* m_printData);
    wxGenericPrintDialog printerDialog(this, & printDialogData);

    printerDialog.GetPrintDialogData().SetSetupDialog(TRUE);
    printerDialog.ShowModal();

    (*m_printData) = printerDialog.GetPrintDialogData().GetPrintData();
}

void MyFrame::OnPageSetupPS(wxCommandEvent& WXUNUSED(event))
{
    (*m_pageSetupData) = * m_printData;

    wxGenericPageSetupDialog pageSetupDialog(this, m_pageSetupData);
    pageSetupDialog.ShowModal();

    (*m_printData) = pageSetupDialog.GetPageSetupData().GetPrintData();
    (*m_pageSetupData) = pageSetupDialog.GetPageSetupData();
}
#endif
#endif // UNUSED

//
// From old gmisc.cc
//

// Find String In File -- uses a file pointer to find a string in the file.
// This is useful for quickly getting to the section of the file you need
int FindStringInFile(gInput &in,const char *s)
{
  char fsif_str[200];
  in.seekp(0L);	// go to the start of the file
  do {
    try {
      in >> fsif_str;
    }
    catch (...) {
      return 0;
    }
  } while (strcmp(fsif_str,s)!=0 && !in.eof() /* && in.IsValid() */);
  if (in.eof() /*|| !in.IsValid()*/) return 0; else return 1;
}

//
// Template instantiations (for now, should be moved -- TLT)
//

#include "base/garray.imp"
#include "base/gblock.imp"
#include "base/grarray.imp"
#include "base/glist.imp"

template class gArray<FileHeader>;
template class gBlock<FileHeader>;

template class gArray<PxiPlot::LABELSTRUCT>;
template class gBlock<PxiPlot::LABELSTRUCT>;

template class gArray<DataLine>;
template class gBlock<DataLine>;

template class gArray<DataLine *>;
template class gBlock<DataLine *>;

template class gList<DataLine *>;

template class gArray<gBlock<bool> >;
template class gBlock<gBlock<bool> >;

gOutput &operator<<(gOutput &p_file, const gBlock<bool> &)
{ return p_file; }

gOutput &operator<<(gOutput &p_file, const gBlock<double> &)
{ return p_file; }

gOutput &operator<<(gOutput &p_file, const gBlock<int> &)
{ return p_file; }

template class gRectArray<gBlock<double> >;

//
// For global stream instances
//
gStandardInput _gin;
gInput &gin = _gin;

gStandardOutput _gout;
gOutput &gout = _gout;

gStandardOutput _gerr;
gOutput &gerr = _gerr;

