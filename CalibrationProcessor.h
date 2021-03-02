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

    cv::Point calibPnt_;
    std::vector<cv::Point2f> pts_bitmap_;
    std::vector<cv::Point2f> pts_img_;
    int ptsCnt_ = 0;

    int slidrVal_;
    bool calibClicked_ = false;
    bool calibDone_    = false;
    cv::Mat calibIMG_, calibDrawn_;


};




