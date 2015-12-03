#include <iostream>
#include <opencv2/opencv.hpp>
using namespace std;
using namespace cv;
int main()
{
for(int p=1;p<=4;p++){
  Mat src, gray;
  const float kScaleFactor = 0.6;
  src = imread( "coin"+to_string(p)+".jpg", 1 );
  resize(src, src, cvSize(0, 0), kScaleFactor, kScaleFactor);
  /*
    Mat dst, cdst;
    Canny(src, dst, 50, 200, 3);
    cvtColor(dst, cdst, CV_GRAY2BGR);

    vector<Vec2f> lines;
    // detect lines
    //for wall.jpg
    //HoughLines(dst, lines, 1, CV_PI/180, 220, 0, 0 );
    //for building.jpg
    HoughLines(dst, lines, 1, CV_PI/180, 250, 0, 0 );

    // draw lines
    for( size_t i = 0; i < lines.size(); i++ )
    {
        float rho = lines[i][0], theta = lines[i][1];
        Point pt1, pt2;
        double a = cos(theta), b = sin(theta);
        double x0 = a*rho, y0 = b*rho;
        pt1.x = cvRound(x0 + 1000*(-b));
        pt1.y = cvRound(y0 + 1000*(a));
        pt2.x = cvRound(x0 - 1000*(-b));
        pt2.y = cvRound(y0 - 1000*(a));
        line( cdst, pt1, pt2, Scalar(0,0,255), 1, CV_AA);
    }
    imshow("source", src);
    imshow("detected lines", cdst);

    waitKey();
    return 0;
    */

  cvtColor( src, gray, CV_BGR2GRAY );

  // Reduce the noise so we avoid false circle detection
  GaussianBlur( gray, gray, Size(7.5, 5.5), 2, 2 );

  vector<Vec3f> circles;

  // Apply the Hough Transform to find the circles
  //for testcoin.jpg
  HoughCircles( gray, circles, CV_HOUGH_GRADIENT, 2, 100,200,100,50 );
  //for wall.jpg
  //HoughCircles( gray, circles, CV_HOUGH_GRADIENT, 2, 32.0, 30, 550 );

  // Draw the circles detected
  for( size_t i = 0; i < circles.size(); i++ )
  {
      Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
      int radius = cvRound(circles[i][2]);
      circle( src, center, 3, Scalar(0,0,255), -1, 8, 0 );// circle center
      circle( src, center, radius, Scalar(0,255,0), 3, 8, 0 );// circle outline
      string a = to_string(i)+":"+to_string(radius);
      putText( src, a , center, CV_FONT_HERSHEY_COMPLEX, 1,Scalar(i, i, 255), 3, 8 );
      cout <<"["<<i<<"] "<< "center : " << center << "\nradius : " << radius << endl;
   }
  cout<<"============================================"<<endl;
  // Show your results
  namedWindow( "Hough Circle "+to_string(p), CV_WINDOW_AUTOSIZE );
  imshow( "Hough Circle "+to_string(p), src );
}
  waitKey(0);
  return 0;

}
