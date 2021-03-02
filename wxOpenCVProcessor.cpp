#include "wxOpenCVProcessor.h"
#include "wxOpenCVPanel.h"

using namespace cv;

wxOpenCVProcessor::wxOpenCVProcessor(wxOpenCVPanel *handler)
{
    addEventHandler(handler);
}

void wxOpenCVProcessor::sendOcvUpdateEvent(wxOpenCVPanel *handler,const cv::Mat &img)
{
    wxOpenCV::sendOcvUpdateEvent(handler, img);
}

void wxOpenCVProcessor::sendOcvEndCapturingEvent(wxOpenCVPanel *handler)
{
    wxOpenCV::sendOcvEndCapturingEvent(handler);
}

void wxOpenCVProcessor::registerSliderEvents(wxSlider *slider)
{
    if(!slider)return;
    slider->Bind(wxEVT_SCROLL_CHANGED, &wxOpenCVProcessor::OnSlider, this);
}

void wxOpenCVProcessor::registerLClickEvents(wxOpenCVPanel *panel)
{
    if(!panel)return;

    panel->Bind(wxEVT_LEFT_DOWN, &wxOpenCVProcessor::OnClick, this);
}

void wxOpenCVProcessor::registerButtonEvents(wxButton *button)
{
    if(!button)return;
    button->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &wxOpenCVProcessor::OnButton, this);
}

void wxOpenCVProcessor::addEventHandler(wxOpenCVPanel *handler)
{
    if(handler)
        handlers_.push_back(handler);
}

void wxOpenCVProcessor::OnSlider(wxScrollEvent &event)
{
    //wxMessageBox("scrolled");
    EvtQueueData elem = {
        .event = SLIDER,
        .id = -1,
        .data = std::vector<int>{event.GetPosition()}
    };
    queueMutex_.lock();
    queue_.push(elem);
    queueMutex_.unlock();
}

void wxOpenCVProcessor::OnButton(wxCommandEvent &event)
{

    EvtQueueData elem = {
        .event = BUTTON,
        .id = event.GetId(),
        .data = std::vector<int>{}
    };
    queueMutex_.lock();
    queue_.push(elem);
    queueMutex_.unlock();
}

void wxOpenCVProcessor::OnClick(wxMouseEvent &event)
{
    wxPoint pt = event.GetPosition();

    wxOpenCVPanel *panel = static_cast<wxOpenCVPanel *>(event.GetEventObject());

    wxRect imageRect = panel->getImageRect();
    if(!imageRect.Contains(pt)) return;
    pt -= imageRect.GetPosition();

    EvtQueueData elem = {
        .event = LCLICK,
        .id = -1,
        .data = std::vector<int>{pt.x, pt.y}
    };
    queueMutex_.lock();
    queue_.push(elem);
    queueMutex_.unlock();
}

void wxOpenCVProcessor::processQueue()
{
    EvtQueueData elem;
    elem.event = EMPTY;
    queueMutex_.lock();
    if(!queue_.empty())
    {
        elem = queue_.front();
        queue_.pop();
    }
    queueMutex_.unlock();

    if(elem.event == LCLICK)
        onClick(Point(elem.data[0], elem.data[1]));
    else if(elem.event == SLIDER)
        onSlider(elem.data[0]);
    else if(elem.event == BUTTON)
        onButton(elem.id);
}

