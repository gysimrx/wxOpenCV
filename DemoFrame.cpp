#include "DemoFrame.h"
#include "wxOpenCVPanel.h"

#include <wx/stattext.h>
#include <wx/sizer.h>
#include <wx/tglbtn.h>

#include <opencv2/opencv.hpp>

BEGIN_EVENT_TABLE(DemoFrame, wxFrame)
    EVT_CLOSE(DemoFrame::OnClose)
    EVT_MENU(idMenuQuit, DemoFrame::OnQuit)
    EVT_MENU(idMenuAbout, DemoFrame::OnAbout)
    EVT_MENU(idMenuStart, DemoFrame::OnStart)
    EVT_MENU(idMenuStop, DemoFrame::OnStop)
    EVT_UPDATE_UI(idMenuStart, DemoFrame::OnUpdateStart)
    EVT_UPDATE_UI(idMenuStop, DemoFrame::OnUpdateStop)
END_EVENT_TABLE()

DemoFrame::DemoFrame(wxFrame *frame, const wxString& title):
    wxFrame(frame, -1, title),
    opencv_(1)
{
    // create a menu bar
    wxMenuBar* mbar = new wxMenuBar();
    wxMenu* fileMenu = new wxMenu("");
    fileMenu->Append(idMenuQuit, _("&Quit\tAlt-F4"), _("Quit the application"));
    mbar->Append(fileMenu, _("&File"));

    wxMenu* ocvMenu = new wxMenu("");
    ocvMenu->Append(idMenuStart, _("&Start Video\t"), _("Start Video"));
    ocvMenu->Append(idMenuStop, _("&Stop Video\t"), _("Stop Video"));
    mbar->Append(ocvMenu, _("&OpenCV"));

    wxMenu* helpMenu = new wxMenu("");
    helpMenu->Append(idMenuAbout, _("&About\tF1"), _("Show info about this application"));
    mbar->Append(helpMenu, _("&Help"));

    SetMenuBar(mbar);

    this->SetSizeHints(wxDefaultSize, wxDefaultSize);

    wxBoxSizer *topSizer = new wxBoxSizer(wxHORIZONTAL);

    wxFlexGridSizer* fgSizer1;
    fgSizer1 = new wxFlexGridSizer(4, 2, 0, 0);
    fgSizer1->SetFlexibleDirection(wxBOTH);
    fgSizer1->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

    wxOpenCVPanel *panel1 = new wxOpenCVPanel(this, wxID_ANY, wxDefaultPosition, wxSize(480, 360), wxTAB_TRAVERSAL);
//    numberingFramesProcessor_ = new NumberingFramesProcessor(panel1);
//    opencv_.addProcessor(numberingFramesProcessor_);
//    numberingFramesProcessor_->registerLClickEvents(panel1);
    calibrationProcessor_ = new CalibrationProcessor(panel1);
    opencv_.addProcessor(calibrationProcessor_);
    calibrationProcessor_->registerLClickEvents(panel1);
    panel1->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_INACTIVECAPTION));

    fgSizer1->Add(panel1, 1, wxALL | wxEXPAND, 5);

    wxOpenCVPanel *panel2 = new wxOpenCVPanel(this, wxID_ANY, wxDefaultPosition, wxSize(480, 360), wxTAB_TRAVERSAL);
    calibrationProcessor_->addEventHandler(panel2);
   // cannyProcessor_ = new CannyProcessor(panel2);
   // opencv_.addProcessor(cannyProcessor_);
    panel2->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_INACTIVECAPTION));

    fgSizer1->Add(panel2, 1, wxEXPAND | wxALL, 5);

    wxStaticText *staticText = new wxStaticText(this, wxID_ANY, wxT("Process 1"), wxDefaultPosition, wxDefaultSize, 0);
    staticText->Wrap(-1);
    wxButton *button = new wxButton(this, wxID_ANY, "stop updates");
    wxBoxSizer *bsz = new wxBoxSizer(wxHORIZONTAL);
    bsz->Add(staticText);
    bsz->Add(button);
    calibrationProcessor_->registerButtonEvents(button);
 //   numberingFramesProcessor_->registerButtonEvents(button);
    fgSizer1->Add(bsz, 1, wxALL | wxALIGN_CENTER_HORIZONTAL, 5);

    staticText = new wxStaticText(this, wxID_ANY, wxT("Process 2"), wxDefaultPosition, wxDefaultSize, 0);
    staticText->Wrap(-1);
    fgSizer1->Add(staticText, 1, wxALL | wxALIGN_CENTER_HORIZONTAL, 5);

    wxOpenCVPanel *panel3 = new wxOpenCVPanel(this, wxID_ANY, wxDefaultPosition, wxSize(480, 360), wxTAB_TRAVERSAL);
//    contourProcessor_ = new ContourProcessor(panel3);
//    opencv_.addProcessor(contourProcessor_);
    calibrationProcessor_->addEventHandler(panel3);
    panel3->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_INACTIVECAPTION));

    fgSizer1->Add(panel3, 1, wxEXPAND | wxALL, 5);

    wxOpenCVPanel *panel4 = new wxOpenCVPanel(this, wxID_ANY, wxDefaultPosition, wxSize(480, 360), wxTAB_TRAVERSAL);
  // calibrationProcessor_ = new CalibrationProcessor(panel4);
//    opencv_.addProcessor(calibrationProcessor_);
//    calibrationProcessor_->registerButtonEvents(button);
    panel4->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_INACTIVECAPTION));


    fgSizer1->Add(panel4, 1, wxEXPAND | wxALL, 5);

    staticText = new wxStaticText(this, wxID_ANY, wxT("process 3"), wxDefaultPosition, wxDefaultSize, 0);
    staticText->Wrap(-1);
    fgSizer1->Add(staticText, 1, wxALL | wxALIGN_CENTER_HORIZONTAL, 5);

    staticText = new wxStaticText(this, wxID_ANY, wxT("Process 4"), wxDefaultPosition, wxDefaultSize, 0);
    staticText->Wrap(-1);
    fgSizer1->Add(staticText, 1, wxALL | wxALIGN_CENTER_HORIZONTAL, 5);


    topSizer->Add(fgSizer1, 1, wxEXPAND, 5);

    wxSlider *sldr = new wxSlider(this, wxID_ANY, 1, 1, 100, wxDefaultPosition, wxDefaultSize, wxSL_VERTICAL|wxSL_INVERSE);
   // numberingFramesProcessor_->registerSliderEvents(sldr);
    topSizer->Add(sldr, 0, wxEXPAND, 5);
    sldr = new wxSlider(this, wxID_ANY, 0, 0, 100, wxDefaultPosition, wxDefaultSize, wxSL_VERTICAL|wxSL_INVERSE);
    topSizer->Add(sldr, 0, wxEXPAND, 5);

    this->SetSizer(topSizer);
    this->Layout();
    topSizer->Fit(this);

    this->Centre(wxBOTH);

    // create a status bar with some information about the used wxWidgets version
    CreateStatusBar(2);
    SetStatusText(_("Hello to wxOpenCV!"), 0);
    SetStatusText(_("idle"), 1);
}

void DemoFrame::OnClose(wxCloseEvent &event)
{
    close();
}

void DemoFrame::OnQuit(wxCommandEvent &event)
{
    close();
}

void DemoFrame::close()
{
    opencv_.stopCapture();
    Destroy();
}

void DemoFrame::OnAbout(wxCommandEvent &event)
{
    wxMessageBox(_("wxOpenCV"), _("Welcome to..."));
}

void DemoFrame::OnStart(wxCommandEvent& event)
{
    opencv_.startCapture();
}

void DemoFrame::OnStop(wxCommandEvent& event)
{
    opencv_.stopCapture();
}

void DemoFrame::OnUpdateStart(wxUpdateUIEvent& event)
{
    event.Enable(!opencv_.isCapturing());
}

void DemoFrame::OnUpdateStop(wxUpdateUIEvent& event)
{
    event.Enable(opencv_.isCapturing());
}

