#include "wxOpenCVPanel.h"

wxBEGIN_EVENT_TABLE(wxOpenCVPanel, wxPanel)
      EVT_OPENCV_UPDATE(wxID_ANY, wxOpenCVPanel::OnOpenCVEvent)
      EVT_OPENCV_END_CAP(wxID_ANY, wxOpenCVPanel::OnOpenCVEndCapturing)
wxEND_EVENT_TABLE()


wxOpenCVPanel::wxOpenCVPanel(wxWindow *parent, wxWindowID winid, const wxPoint& pos, const wxSize& size, long style, const wxString& name):
    wxPanel(parent, winid, pos, size, style, name),
    imageRect_(0, 0, 0, 0)
{}

void wxOpenCVPanel::OnOpenCVEvent(wxOpenCVEvent &event)
{
    wxBitmap img(*(event.GetImage()));

    wxClientDC dc(this);

    wxSize dcSize = dc.GetSize();
    wxSize imageSize(img.GetWidth(), img.GetHeight());
    wxPoint off(0, 0);
    if(dcSize.x >= imageSize.x)
        off.x = (dcSize.x - imageSize.x)/2;
    if(dcSize.y >= imageSize.y)
        off.y = (dcSize.y - imageSize.y)/2;

    imageRect_ = wxRect(off, imageSize);
    SetMinSize(imageSize);

    dc.DrawBitmap(img, off);
}

void wxOpenCVPanel::OnOpenCVEndCapturing(wxOpenCVEvent &event)
{
	wxClientDC dc(this);
	dc.Clear();
}

