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
    Mat dstImage = img.clone(), Bitmap, Bitmap1, Bitmap2, imgUserInput;
    Mat Calibgray;

    Ptr<SIFT> siftPtr = SIFT::create();
    Bitmap1 = imread("OutlineForm.png");
    Bitmap2 = imread("I2C.png");
    Bitmap = Bitmap1 + Bitmap2;
    cv::resize(Bitmap,Bitmap, cv::Size(),0.6,0.6);
    const int MAX_FEATURES = 3000;
    Ptr<Feature2D> orb = ORB::create(MAX_FEATURES);
    std::vector<cv::Point2f> polyPnts, polyPntsConvexSorted;
    if(calibClicked_)
    {
        calibIMG_ = img.clone();
//        calibIMG_ = imread("StaticImg.jpg");        //Comment if the live feed of a Camera should be used;
        cvtColor(calibIMG_, Calibgray,COLOR_BGR2GRAY);
        // SiftFeatureDetector detector;
        // siftPtr->detect(Calibgray,keypointsCalib_);
        orb->detectAndCompute(Calibgray, Mat(), keypointsCalib_, descriptors1_);
        drawKeypoints(Calibgray, keypointsCalib_,initalKeypoints_);
        calibClicked_ = false;

        mask_ = Mat::zeros(Bitmap1.size(), CV_8UC3);
        for( size_t i = 0; i< pts_polygon_.size(); i++ )
            fillPoly(mask_, pts_polygon_, Scalar(255, 255,255));
        imwrite("runtimeIMGs/pntsMask.jpg",mask_);
        cv::resize(mask_, mask_, cv::Size(), 0.6, 0.6);
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
            imwrite("runtimeIMGs/calibFirst.jpg",calibIMG_);
            Mat im_temp = calibIMG_.clone(), im_temp_mask, calibImgClone = calibIMG_.clone();
            im_temp_mask = im_temp.clone();
            warpPerspective(Bitmap, im_temp, h, im_temp.size());

            if(mask_.size() == Bitmap.size())
            {
                warpPerspective(mask_, im_temp_mask, h, im_temp_mask.size());
                imwrite("runtimeIMGs/maskwarped.jpg", im_temp_mask);
            }
            calibBitmapWarped_ = im_temp;
            calibDrawn_ = calibIMG_ + im_temp;
            cvtColor(im_temp_mask, im_temp_mask, COLOR_BGR2GRAY);
            imwrite("runtimeIMGs/maskwarped.jpg", im_temp_mask);
            findNonZero(im_temp_mask, polyPnts);
            convexHull(polyPnts, polyPntsConvexSorted);

            /*********NON CONVEX*******************************************/
            Mat nonConvex = im_temp_mask.clone();
            imwrite("runtimeIMGs/nonConvexBegin.jpg", nonConvex);
            blur( nonConvex, nonConvex, Size(3,3) );
            Canny( nonConvex, nonConvex,  3.0, 255.0 );
            std::vector<std::vector<Point> > contours;
            std::vector<Vec4i> hierarchy;
            imwrite("runtimeIMGs/nonConvexCanny.jpg", nonConvex);
            findContours( nonConvex, contours, hierarchy, RETR_LIST, CHAIN_APPROX_SIMPLE );
            imwrite("runtimeIMGs/maskContours.jpg", im_temp_mask);

            std::vector<cv::Point2f> pntsCalib, pntsCalibInsidePoly;
            KeyPoint::convert(keypointsCalib_, pntsCalib);
            bool MEASUREDISTANCE = false ;
            for(int i = 0; i < pntsCalib.size(); i++)
            {
//                if( pointPolygonTest(polyPntsConvexSorted, pntsCalib.at(i), MEASUREDISTANCE) == 1 ) // // 1 = inside, -1 = outside, 0 = OnEdge
                if( pointPolygonTest(contours.at(0), pntsCalib.at(i), MEASUREDISTANCE) == 1 ) // // 1 = inside, -1 = outside, 0 = OnEdge
                    pntsCalibInsidePoly.push_back(pntsCalib.at(i));

            }
            KeyPoint::convert(pntsCalibInsidePoly, keypointsCalibSorted_);
            drawKeypoints(calibDrawn_, keypointsCalibSorted_, calibDrawn_);

            /*********Black OUT non used AREA*******************************************/
            Mat backgroundBlacked;
            mask_ = Mat::zeros(calibIMG_.size(), CV_8UC3);
            for( size_t i = 0; i< contours.at(0).size(); i++ )
            {
                fillPoly(mask_,contours.at(0), Scalar(255, 255,255));
            }
            bitwise_and(calibImgClone, mask_, backgroundBlacked);
            imwrite("runtimeIMGs/bitwiseIMG.jpg",backgroundBlacked);
            cvtColor(backgroundBlacked, backgroundBlacked,COLOR_BGR2GRAY);
            orb->detectAndCompute(backgroundBlacked, Mat(), keypointsBlackedOut_, blackedOutDescriptors_);

            char str_[40];
            cv::Point pos (10,50);
            float Scale (0.8);
            sprintf(str_, "pnt:%03i srt:%03i cntrs:%06i ",int(pntsCalib.size()), int(pntsCalibInsidePoly.size()),int(contours.size()) );
            cv::putText(calibDrawn_, str_, pos, cv::FONT_HERSHEY_SIMPLEX, Scale, cv::Scalar(155), 2, 8);
            calibDone_ = true;
            calibIMG_ = calibDrawn_.clone();
        }
    }
    else
    {
        /********* Live HOMOGRAPHY *******************************************/
        Mat liveImgTemp = img.clone();
        imwrite("runtimeIMGs/liveImgTemp1.jpg", liveImgTemp);
        cvtColor(liveImgTemp, liveImgGray_, COLOR_BGR2GRAY);
        orb->detectAndCompute(liveImgGray_, Mat(), keypointsLive_, descriptors2_);
        if(!keypointsLive_.empty())
        {
            std::vector<DMatch> matches;
            Ptr<DescriptorMatcher> matcher = DescriptorMatcher::create("BruteForce-Hamming");
            matcher->match(blackedOutDescriptors_, descriptors2_, matches, Mat());
            // Sort matches by score
    //        std::sort(matches.begin(), matches.end());

    //        // Remove not so good matches
    //        const int numGoodMatches = matches.size() * GOOD_MATCH_PERCENT;
    //        matches.erase(matches.begin()+numGoodMatches, matches.end());

            // Extract location of good matches
            std::vector<Point2f> points1, points2;
            for( size_t i = 0; i < matches.size(); i++ )
            {
                points1.push_back( keypointsBlackedOut_[ matches[i].queryIdx ].pt );
                points2.push_back( keypointsLive_[ matches[i].trainIdx ].pt );
            }
            Mat h2;
            h2 = findHomography( points1, points2, RANSAC ); //Normal 17.02ä
            if(!h2.empty())
            {
                Mat combinedHomography = h2 * h;
                // Warp the calibBitamp again and add up with realtime IMG
                warpPerspective(calibBitmapWarped_, liveImgTemp, h2, liveImgTemp.size());
                char str_[40];
                cv::Point pos (10,50);
                float Scale (0.8);
                sprintf(str_, "pnts1:%06i pnts2:%06i",int(points1.size()), int(points2.size()) );
                cv::putText(liveImgTemp, str_, pos, cv::FONT_HERSHEY_SIMPLEX, Scale, cv::Scalar(155), 2, 8);
                calibIMG_ = liveImgTemp + img ;
                imwrite("runtimeIMGs/calibIMG_finsished.jpg",calibIMG_);
            }
        }
    }

    if(imgUserInput.empty())
        imgUserInput = imread("NoIMG.png");
    if(dstImage.empty())
        dstImage = imread("NoIMG.png");
    if(initalKeypoints_.empty())
        initalKeypoints_ = imread("NoIMG.png");

    sendOcvUpdateEvent(handlers_[0], imgUserInput);
    if(pts_bitmap_.size() == 4)
    {
       if(calibDone_)
            sendOcvUpdateEvent(handlers_[1], calibDrawn_);
        else
            sendOcvUpdateEvent(handlers_[1], img);
    }
    else
    {
        Mat gotPnts = initalKeypoints_.clone();
        char str_[16];
        cv::Point pos (10,50);
        float Scale (1.0);
        sprintf(str_, "pt: %03i",int(pts_bitmap_.size()) );
        cv::putText(gotPnts, str_, pos, cv::FONT_HERSHEY_SIMPLEX, Scale, cv::Scalar(155), 2, 8);
        sendOcvUpdateEvent(handlers_[1], gotPnts);
    }
        if(calibIMG_.empty())
            sendOcvUpdateEvent(handlers_[2], img);
        else
            sendOcvUpdateEvent(handlers_[2], calibIMG_);
}

void CalibrationProcessor::onSlider(int val)
{

    if(val > 254)
        slidrVal_ = 255;
    else
        slidrVal_ = val;
    if(val < 2)
        slidrVal_ = 1;
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
