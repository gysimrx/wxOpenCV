#ifndef WXOPENCVMAIN_H
#define WXOPENCVMAIN_H

#include <wx/wx.h>

#include "wxOpenCV.h"
#include "NumberingFramesProcessor.h"
#include "CannyProcessor.h"
#include "ContourProcessor.h"
#include "CalibrationProcessor.h"

namespace{
    static const int idMenuQuit = wxNewId();
    static const int idMenuAbout = wxNewId();
    static const int idMenuStart = wxNewId();
    static const int idMenuStop = wxNewId();
}

class DemoFrame: public wxFrame
{
public:
    DemoFrame(wxFrame *frame, const wxString& title);
    ~DemoFrame() = default;
private:

    void OnClose(wxCloseEvent& event);
    void OnQuit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
    void OnStart(wxCommandEvent& event);
    void OnStop(wxCommandEvent& event);
    void OnUpdateStart(wxUpdateUIEvent& event);
    void OnUpdateStop(wxUpdateUIEvent& event);

    void close();

    wxOpenCV opencv_;
    NumberingFramesProcessor *numberingFramesProcessor_;
    CannyProcessor *cannyProcessor_;
    ContourProcessor *contourProcessor_;
    CalibrationProcessor *calibrationProcessor_;

    DECLARE_EVENT_TABLE()
};

#endif
