#include "wxOpenCV.h"
#include "wxOpenCVProcessor.h"

class CalibrationProcessor: public wxOpenCVProcessor
{
public:
    CalibrationProcessor(wxOpenCVPanel *handler);
private:
    void init() override {}
    void uninit() override;
    void process(cv::Mat &img) override;
    void onSlider(int val) override;
    void onButton(int id) override ;
    void onClick(const cv::Point &pt) override ;

    cv::Mat outputImg_1, outputImg_2, outputImg_3;
    cv::Point calibPnt_, livePnt_;
    std::vector<cv::Point2f> pts_bitmap_, pts_img_;
    std::vector<cv::KeyPoint> keypointsLive_,keypointsPCB_ ;
    std::vector<cv::Point> pts_pcb_ {cv::Point(10,10), cv::Point(10,380), cv::Point(720,380), cv::Point(720,10), cv::Point(170,10) };

    float slidrVal_ = 1.0;
    bool calibClicked_ = false;
    bool calibDone_    = false;
    cv::Mat calibIMG_, calibDrawn_, mask_, calibBitmapWarped_, calibHomography, liveImgGray_, descriptorslive_, descriptorsPCB_;


};




