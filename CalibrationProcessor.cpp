#include "CalibrationProcessor.h"
#include "wxOpenCV.h"
#include <opencv2/xfeatures2d.hpp>

using namespace cv;
using namespace cv::xfeatures2d;

CalibrationProcessor::CalibrationProcessor(wxOpenCVPanel *handler):
    wxOpenCVProcessor(handler)
{}

void CalibrationProcessor::uninit()
{
    sendOcvEndCapturingEvent(handlers_[0]);
}

void CalibrationProcessor::process(cv::Mat &img)
{
    Mat Bitmap, Bitmap1, Bitmap2, Calibgray, backgroundBlacked;
    char str_[40];
    const int MAX_FEATURES = 3000;
    Bitmap1 = imread("Outline.png");
    Bitmap2 = imread("I2C.png");
    Bitmap  = Bitmap1 + Bitmap2;
    cv::resize(Bitmap,Bitmap, cv::Size(),0.6,0.6);

    Ptr<SIFT> descriptor = SIFT::create(MAX_FEATURES);
    Ptr<DescriptorMatcher> matcher = DescriptorMatcher::create(DescriptorMatcher::BRUTEFORCE);

    /**** Slider changes percentage of good Matches ****/
    float GOOD_MATCH_PERCENT = slidrVal_;

    if(calibClicked_)
    {
        /**** Reset calibration***/
        pts_bitmap_.clear();
        pts_img_.clear();
        calibDone_ = false;
        calibHomography.release();

        /**** Saving calibration Image and create the Mask ****/
        calibIMG_ = img.clone();
        cvtColor(calibIMG_, Calibgray,COLOR_BGR2GRAY);
        calibClicked_ = false;
        mask_ = Mat::zeros(Bitmap1.size(), CV_8UC3);
        for( size_t i = 0; i< pts_pcb_.size(); i++ )
            fillPoly(mask_, pts_pcb_, Scalar(255, 255,255));
        cv::resize(mask_, mask_, cv::Size(), 0.6, 0.6);
    }

    if(!calibDone_)
    {
        if(pts_bitmap_.size() == 4)
            outputImg_1 = calibIMG_.clone();
        else
            outputImg_1 = Bitmap.clone();

        if( (pts_bitmap_.size() ==4) && (pts_img_.size()==4))
            calibHomography = findHomography(pts_bitmap_, pts_img_);

        if(!calibHomography.empty())
        {
            /**** Warp Mask and Bitmap ****/
            Mat im_temp = calibIMG_.clone(), im_temp_mask, calibImgClone = calibIMG_.clone();
            im_temp_mask = im_temp.clone();
            warpPerspective(Bitmap, im_temp, calibHomography, im_temp.size());
            warpPerspective(mask_, im_temp_mask, calibHomography, im_temp_mask.size());

            /**** Store warped Bitmap for later use with 2nd Homography and draw it on calibration image ****/
            calibBitmapWarped_ = im_temp.clone();
            calibDrawn_ = calibIMG_ + im_temp;

            /**** "Cut out" PCB and get Keypoints and Compute Descriptors for Live use ****/
            bitwise_and(calibImgClone, im_temp_mask, backgroundBlacked);
//            cvtColor(backgroundBlacked, backgroundBlacked,COLOR_BGR2GRAY);
            cvtColor(calibImgClone, backgroundBlacked,COLOR_BGR2GRAY);


            descriptor->detectAndCompute(backgroundBlacked, Mat(), keypointsPCB_, descriptorsPCB_);
            calibIMG_ = calibDrawn_.clone();
            calibDone_ = true;
        }
    }
    else
    {

        /********* Live HOMOGRAPHY *******************************************/
        Mat liveImgTemp = img.clone();
        cvtColor(liveImgTemp, liveImgGray_, COLOR_BGR2GRAY);
        descriptor->detectAndCompute(liveImgGray_, Mat(), keypointsLive_, descriptorslive_);
        if(!keypointsLive_.empty())
        {
            std::vector<DMatch> matches;
            matcher->match(descriptorsPCB_, descriptorslive_, matches, Mat());

            // Sort matches by score
            std::sort(matches.begin(), matches.end());

            // Remove not so good matches
            int numGoodMatches = matches.size() * GOOD_MATCH_PERCENT;
            matches.erase(matches.begin()+numGoodMatches, matches.end());

            // Extract location of good matches
            std::vector<Point2f> points1, points2;
            for( size_t i = 0; i < matches.size(); i++ )
            {
                points1.push_back( keypointsPCB_[ matches[i].queryIdx ].pt );
                points2.push_back( keypointsLive_[ matches[i].trainIdx ].pt );
            }
            Mat liveHomgraphy = findHomography( points1, points2, RANSAC ); //Normal 17.02ä
            if(!liveHomgraphy.empty())
            {
                // Warp the calibBitamp again and add up with realtime IMG
                warpPerspective(calibBitmapWarped_, liveImgTemp, liveHomgraphy, liveImgTemp.size());
                calibIMG_ = liveImgTemp + img ;
            }
        }
    }

   if(calibDone_)
        outputImg_1 = calibDrawn_.clone();
    else
    {
        if(pts_bitmap_.size() == 0)
            outputImg_2 = img.clone();
        else
        {
            /**** Print out how many clicks on the Picture are registered ****/
            outputImg_2 = imread("NoIMG.png");
            sprintf(str_, "pt: %03i",int(pts_bitmap_.size()) );
            cv::putText(outputImg_2, str_, Point(10,50), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(155), 2, 8);
        }
    }

    outputImg_3 = calibIMG_.clone();
    /**** Prevent crashes, caused by Sending empty Images ****/
    if(outputImg_1.empty())
        outputImg_1 = img.clone();
    if(outputImg_2.empty())
        outputImg_2 = img.clone();
    if(outputImg_3.empty())
        outputImg_3 = img.clone();

    sendOcvUpdateEvent(handlers_[0], outputImg_1);
    sendOcvUpdateEvent(handlers_[1], outputImg_2);
    sendOcvUpdateEvent(handlers_[2], outputImg_3);
}

void CalibrationProcessor::onSlider(int val)
{
        slidrVal_ =  val / 100.0;
        outputImg_2 = imread("NoIMG.png");
         char str_[40];
        sprintf(str_, "pt: %02f val: %04d", slidrVal_, val );
        cv::putText(outputImg_2, str_, Point(10,50), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(155), 2, 8);
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
