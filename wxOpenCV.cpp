#include "wxOpenCV.h"
#include "wxOpenCVPanel.h"
#include "wxOpenCVProcessor.h"

#include <opencv2/opencv.hpp>

#include <mutex>
#include <condition_variable>

using namespace cv;

wxImage *cv2wx(const Mat &img)
{
    Mat rgbImage;

    if(img.channels()==1)
		cvtColor(img, rgbImage, COLOR_GRAY2RGB);
	else if (img.channels() == 4)
		cvtColor(img, rgbImage, COLOR_BGRA2RGB);
    else
		cvtColor(img, rgbImage, COLOR_BGR2RGB);

	size_t imageSize = rgbImage.rows * rgbImage.cols * rgbImage.channels();
    wxImage *wxImg = new wxImage(rgbImage.cols, rgbImage.rows, (unsigned char*)malloc(imageSize), false);

    if(wxImg)
        memcpy(wxImg->GetData(), rgbImage.data, imageSize);

    return wxImg;
}

Mat wx2cv(wxImage &wx)
{
    Mat image(Size(wx.GetWidth(), wx.GetHeight()), CV_8UC3, wx.GetData());
    cvtColor(image, image, COLOR_RGB2BGR);

    return image;
}

class wxOpenCV::Impl
{
public:
    VideoCapture videoCapture_;
    std::list<Mat> frameSequence_;
    std::mutex mutexFrameSequence;
    std::mutex mutexProcess_;
    Mat srcImg_;
    std::condition_variable condProcess_;
};

wxOpenCV::wxOpenCV(int videoCapturingDeviceIndex):
    impl_(std::make_unique<Impl>()),
    videoCapturingDeviceIndex_(videoCapturingDeviceIndex),
    stop_(true)
{}

// TODO (danselmi#1#): stop threads and delete Processors:
wxOpenCV::~wxOpenCV() = default;

void wxOpenCV::startCapture()
{
    if(isCapturing()) return;

//    impl_->videoCapture_.set(CAP_PROP_FRAME_WIDTH, 1024);
//    impl_->videoCapture_.set(CAP_PROP_FRAME_HEIGHT, 768);

    impl_->videoCapture_.open(videoCapturingDeviceIndex_);
    if (!impl_->videoCapture_.isOpened()) return;

    stop_ = false;
    threads_.push_back(std::thread(&wxOpenCV::capture, this));
    threads_.push_back(std::thread(&wxOpenCV::control, this));
    for(auto prc : processors_)
        threads_.push_back(std::thread(&wxOpenCV::process, this, prc));
}


void wxOpenCV::stopCapture()
{
    if(!isCapturing()) return;

    stop_ = true;
    impl_->condProcess_.notify_all();
    for (auto& thread : threads_)
        thread.join();
    impl_->videoCapture_.release();
    threads_.clear();
}

bool wxOpenCV::isCapturing()
{
    return !stop_;
}

int getNewId()
{
    static int id = 0;
    return id++;
}

void wxOpenCV::capture()
{
	Mat capframe;
	while (!stop_)
	{
		impl_->videoCapture_.read(capframe);
		impl_->mutexFrameSequence.lock();
		impl_->frameSequence_.push_front(capframe);
		impl_->mutexFrameSequence.unlock();
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
}

void wxOpenCV::control()
{
	while (!stop_)
	{
		if (!impl_->frameSequence_.empty())
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			impl_->mutexFrameSequence.lock();
			impl_->mutexProcess_.lock();
			impl_->srcImg_ = impl_->frameSequence_.back();
			impl_->frameSequence_.pop_back();
			impl_->mutexFrameSequence.unlock();
			impl_->mutexProcess_.unlock();
			impl_->condProcess_.notify_all();
		}
	}
}

void wxOpenCV::process(wxOpenCVProcessorInterface *processor)
{
    if(!processor) return;

    processor->init();

    while(!stop_)
    {
        processor->processQueue();

		std::unique_lock<std::mutex> locker(impl_->mutexProcess_);
		impl_->condProcess_.wait(locker);
		Mat srcImg = impl_->srcImg_.clone();
		locker.unlock();

		processor->process(srcImg);
    }

    processor->uninit();
}

void wxOpenCV::addProcessor(wxOpenCVProcessorInterface *proc)
{
    processors_.push_back(proc);
}

void wxOpenCV::sendOcvUpdateEvent(wxEvtHandler *handler, const Mat &img)
{
    if(handler)
    {
        wxImage *imwx = cv2wx(img);
        if(imwx)
        {
            wxOpenCVEvent event(wxEVT_OPENCV_UPDATE);
            event.SetImage(imwx); // Store the data in the event
            wxPostEvent(handler, event);
        }
    }
}

void wxOpenCV::sendOcvEndCapturingEvent(wxEvtHandler *handler)
{
	if(handler)
    {
        wxOpenCVEvent event(wxEVT_OPENCV_END_CAP);
        wxPostEvent(handler, event);
    }
}

wxDEFINE_EVENT(wxEVT_OPENCV_UPDATE, wxOpenCVEvent);
wxDEFINE_EVENT(wxEVT_OPENCV_END_CAP, wxOpenCVEvent);

wxOpenCVEvent::wxOpenCVEvent(wxEventType commandType, int id):
    wxCommandEvent(commandType, id),
    img_(nullptr)
{}

wxOpenCVEvent::~wxOpenCVEvent()
{
    delete img_;
}

wxOpenCVEvent::wxOpenCVEvent(const wxOpenCVEvent& event):
    wxCommandEvent(event),
    img_(nullptr)
{
    if(!event.img_) return;
	const size_t imageSize = event.img_->GetWidth() * event.img_->GetHeight() * 3;
    img_ = new wxImage(event.img_->GetWidth(), event.img_->GetHeight(), (unsigned char*)malloc(imageSize), false);
    memcpy(img_->GetData(), event.img_->GetData(), imageSize);
}

// Required for sending with wxPostEvent()
wxEvent* wxOpenCVEvent::Clone() const
{
    return new wxOpenCVEvent(*this);
}

