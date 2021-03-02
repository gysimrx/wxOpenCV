#include "CalibrationProcessor.h"
#include "wxOpenCV.h"
using namespace cv;

CalibrationProcessor::CalibrationProcessor(wxOpenCVPanel *handler):
    wxOpenCVProcessor(handler)
{}

void CalibrationProcessor::uninit()
{
    sendOcvEndCapturingEvent(handlers_[0]);
}

void CalibrationProcessor::process(cv::Mat &img)
{
    Mat dstImage = img, Bitmap, Bitmap1, Bitmap2, h, imgUserInput, dst;
    Mat Calibgray, descriptors1, descriptors2;
    std::vector<cv::KeyPoint> keypointsCalib;
    Ptr<SIFT> siftPtr = SIFT::create();
    Bitmap1 = imread("Outline.png");
    Bitmap2 = imread("I2C.png");
    Bitmap = Bitmap1 + Bitmap2;
    cv::Point pnt;
    cv::resize(Bitmap,Bitmap, cv::Size(),0.6,0.6);

    const int MAX_FEATURES = 3000;
	const float GOOD_MATCH_PERCENT = 0.25f;

    Ptr<Feature2D> orb = ORB::create(MAX_FEATURES);

    if(calibClicked_)
    {
        calibIMG_ = img;
        cvtColor(calibIMG_, Calibgray,COLOR_BGR2GRAY);
        // SiftFeatureDetector detector;
      //  siftPtr->detect(Calibgray,keypointsCalib);
        orb->detectAndCompute(Calibgray, Mat(), keypointsCalib, descriptors1);
         drawKeypoints(Calibgray, keypointsCalib,dstImage);
        calibClicked_ = false;
    }

    if(!calibDone_)
    {
        if(pts_bitmap_.size() == 4)
            imgUserInput = calibIMG_;
        else
            imgUserInput = Bitmap;

        if( (pts_bitmap_.size() ==4) && (pts_img_.size()==4))
            h = findHomography(pts_bitmap_, pts_img_);

        if(!h.empty())
        {
            //cv::resize(Bitmap,Bitmap, cv::Size(),1.6,1.6);
            Mat im_temp = calibIMG_.clone();
            warpPerspective(Bitmap, im_temp, h, im_temp.size());
            fillConvexPoly(calibIMG_, pts_img_, 4, Scalar(0), 21);

            calibDrawn_ = calibIMG_ + im_temp;
            calibDone_ = true;
        }
    }
    else
    {
       calibIMG_ = calibDrawn_;

    }

    if(imgUserInput.empty())
        imgUserInput = imread("NoIMG.png");
    if(calibIMG_.empty())
        calibIMG_ = imread("NoIMG.png");
    if(dstImage.empty())
        dstImage = imread("NoIMG.png");

    sendOcvUpdateEvent(handlers_[0], imgUserInput);
    sendOcvUpdateEvent(handlers_[1], calibIMG_);
    sendOcvUpdateEvent(handlers_[2], dstImage);

    pnt = calibPnt_;
}

void CalibrationProcessor::onSlider(int val)
{
   slidrVal_ = 1.0 + val/100.0;
}


void CalibrationProcessor::onButton(int id)
{
    calibClicked_ = !calibClicked_;
}

void CalibrationProcessor::onClick(const cv::Point &pt)
{
    if(pts_bitmap_.size() < 4)
        pts_bitmap_.push_back(Point2f(pt));
    else
    {
        if(pts_img_.size()< 4)
            pts_img_.push_back(Point2f(pt));
    }



}
