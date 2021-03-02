#ifndef WXOPENCVPROCESSOR_H
#define WXOPENCVPROCESSOR_H

#include <wx/wx.h>

#include <opencv2/opencv.hpp>

#include <mutex>
#include <queue>

class wxOpenCVPanel;

class wxOpenCVProcessorInterface
{
public:
    virtual void init() = 0;
    virtual void uninit() = 0;
    virtual void process(cv::Mat &img) = 0;

    virtual void processQueue() = 0;
};

class wxOpenCVProcessor: public wxOpenCVProcessorInterface
{
protected:
    wxOpenCVProcessor(wxOpenCVPanel *handler);
    void sendOcvUpdateEvent(wxOpenCVPanel *handler, const cv::Mat &img);
    void sendOcvEndCapturingEvent(wxOpenCVPanel *handler);

    std::vector<wxOpenCVPanel*> handlers_;
public:
    void registerSliderEvents(wxSlider *slider);
    void registerButtonEvents(wxButton *button);
    void registerLClickEvents(wxOpenCVPanel *panel);
    void addEventHandler(wxOpenCVPanel *handler);

private:
    void init() override = 0;
    void uninit() override = 0;
    void process(cv::Mat &img) override = 0;
    virtual void onSlider(int val){}
    virtual void onButton(int id){}
    virtual void onClick(const cv::Point &pt){}

    //void OnClick(const wxPoint &pt);
    void OnClick(wxMouseEvent &event);
    void OnSlider(wxScrollEvent &event);
    void OnButton(wxCommandEvent &event);

    enum EventQueueType
    {
        LCLICK,
        SLIDER,
        BUTTON,
        EMPTY
    };
    struct EvtQueueData
    {
        EventQueueType event;
        int id;
        std::vector<int> data;
    };
    std::queue<EvtQueueData> queue_;
    std::mutex queueMutex_;

    void processQueue() override;
};

#endif
