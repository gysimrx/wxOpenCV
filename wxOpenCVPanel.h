#ifndef WXOPENCVPANEL_H
#define WXOPENCVPANEL_H

#include <wx/wx.h>
#include "wxOpenCV.h"


class wxOpenCVPanel: public wxPanel
{
public:
    wxOpenCVPanel(
        wxWindow *parent,
        wxWindowID winid = wxID_ANY,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxTAB_TRAVERSAL | wxNO_BORDER,
        const wxString& name = wxPanelNameStr
    );

    const wxRect &getImageRect(){return imageRect_;}
private:
    void OnOpenCVEvent(wxOpenCVEvent &event);
    void OnOpenCVEndCapturing(wxOpenCVEvent &event);

    //void OnClick(wxMouseEvent &event);

    wxRect imageRect_;

    DECLARE_EVENT_TABLE()
};

#endif
