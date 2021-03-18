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

    cv::Point calibPnt_, livePnt_;
    std::vector<cv::Point2f> pts_bitmap_;
    std::vector<cv::Point2f> pts_img_;
    std::vector<cv::KeyPoint> keypointsCalib_, keypointsCalibSorted_, keypointsLive_,keypointsBlackedOut_ ;
    std::vector<cv::Point> pts_polygon_ {cv::Point(10,10), cv::Point(28,360), cv::Point(716,377), cv::Point(702,23), cv::Point(325,100) }; // 80/111.
//    std::vector<cv::Point> pts_polygon_ {cv::Point(80,110), cv::Point(80,284), cv::Point(314,144), cv::Point(548,282), cv::Point(548,110), cv::Point(314,24) }; cv::Point(314,369)
    int ptsCnt_ = 0;

    int slidrVal_ = 0;
    bool calibClicked_ = false;
    bool calibDone_    = false;
    cv::Mat calibIMG_, calibDrawn_, mask_, calibBitmapWarped_, h, initalKeypoints_, liveImgGray_, descriptors1_, descriptors2_, blackedOutDescriptors_;


};




