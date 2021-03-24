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
    Mat dstImage = img.clone(), Bitmap, Bitmap1, Bitmap2, Calibgray, backgroundBlacked;
    Bitmap1 = imread("Outline.png");
    Bitmap2 = imread("I2C.png");
    Bitmap = Bitmap1 + Bitmap2;
    cv::resize(Bitmap,Bitmap, cv::Size(),0.6,0.6);
    const int MAX_FEATURES = 3000;
    Ptr<Feature2D> orb = ORB::create(MAX_FEATURES);

    std::vector<cv::Point2f> polyPnts, polyPntsConvexSorted;
    char str_[40];
    float GOOD_MATCH_PERCENT = slidrVal_;

    if(calibClicked_)
    {
        calibIMG_ = img.clone();
        cvtColor(calibIMG_, Calibgray,COLOR_BGR2GRAY);
        calibClicked_ = false;
        mask_ = Mat::zeros(Bitmap1.size(), CV_8UC3);
        for( size_t i = 0; i< pts_polygon_.size(); i++ )
            fillPoly(mask_, pts_polygon_, Scalar(255, 255,255));
        cv::resize(mask_, mask_, cv::Size(), 0.6, 0.6);
    }

    if(!calibDone_)
    {
        if(pts_bitmap_.size() == 4)
            outputImg_1 = calibIMG_.clone();
        else
            outputImg_1 = Bitmap.clone();

        if( (pts_bitmap_.size() ==4) && (pts_img_.size()==4))
            h = findHomography(pts_bitmap_, pts_img_);

        if(!h.empty())
        {
            /****** Warp Mask and Bitmap **************/
            Mat im_temp = calibIMG_.clone(), im_temp_mask, calibImgClone = calibIMG_.clone();
            im_temp_mask = im_temp.clone();
            warpPerspective(Bitmap, im_temp, h, im_temp.size());
            warpPerspective(mask_, im_temp_mask, h, im_temp_mask.size());

            calibBitmapWarped_ = im_temp.clone();       //Store warped Bitmap for later use with 2nd Homography
            calibDrawn_ = calibIMG_ + im_temp;

            /********* Get Contours *******************************************/
            cvtColor(im_temp_mask, im_temp_mask, COLOR_BGR2GRAY);
            Mat nonConvex = im_temp_mask.clone();
            blur( nonConvex, nonConvex, Size(3,3) );
            Canny( nonConvex, nonConvex,  3.0, 255.0 );
            std::vector<std::vector<Point> > contours;
            std::vector<Vec4i> hierarchy;
            findContours( nonConvex, contours, hierarchy, RETR_LIST, CHAIN_APPROX_SIMPLE );

            /*********Black out non used AREA of Calib IMG *******************************************/
            mask_ = Mat::zeros(calibIMG_.size(), CV_8UC3);
            for( size_t i = 0; i< contours.at(0).size(); i++ )
                fillPoly(mask_,contours.at(0), Scalar(255, 255,255));
            bitwise_and(calibImgClone, mask_, backgroundBlacked);
            cvtColor(backgroundBlacked, backgroundBlacked,COLOR_BGR2GRAY);
            orb->detectAndCompute(backgroundBlacked, Mat(), keypointsBlackedOut_, blackedOutDescriptors_);
            calibIMG_ = calibDrawn_.clone();
            calibDone_ = true;
        }
    }
    else
    {
        /********* Live HOMOGRAPHY *******************************************/
        Mat liveImgTemp = img.clone();
        cvtColor(liveImgTemp, liveImgGray_, COLOR_BGR2GRAY);
        orb->detectAndCompute(liveImgGray_, Mat(), keypointsLive_, descriptorslive_);
        if(!keypointsLive_.empty())
        {
            std::vector<DMatch> matches;
            Ptr<DescriptorMatcher> matcher = DescriptorMatcher::create("BruteForce-Hamming");
            matcher->match(blackedOutDescriptors_, descriptorslive_, matches, Mat());

            // Sort matches by score
            std::sort(matches.begin(), matches.end());

            // Remove not so good matches
             int numGoodMatches = matches.size() * GOOD_MATCH_PERCENT;
            matches.erase(matches.begin()+numGoodMatches, matches.end());

            // Extract location of good matches
            std::vector<Point2f> points1, points2;
            for( size_t i = 0; i < matches.size(); i++ )
            {
                points1.push_back( keypointsBlackedOut_[ matches[i].queryIdx ].pt );
                points2.push_back( keypointsLive_[ matches[i].trainIdx ].pt );
            }
            Mat h2 = findHomography( points1, points2, RANSAC ); //Normal 17.02ä
            if(!h2.empty())
            {
                // Warp the calibBitamp again and add up with realtime IMG
                warpPerspective(calibBitmapWarped_, liveImgTemp, h2, liveImgTemp.size());
                sprintf(str_, "pnts1:%06i pnts2:%06i",int(points1.size()), int(points2.size()) );
                cv::putText(liveImgTemp, str_, Point(10,50), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(155), 2, 8);
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
            outputImg_2 = imread("NoIMG.png");
            sprintf(str_, "pt: %03i",int(pts_bitmap_.size()) );
            cv::putText(outputImg_2, str_, Point(10,50), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(155), 2, 8);
        }
    }
    if(calibIMG_.empty())
        outputImg_3 = img.clone();
    else
        outputImg_3 = calibIMG_.clone();

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
