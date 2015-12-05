#include <iostream>
#include <opencv2/opencv.hpp>
using namespace std;
using namespace cv;

class Coin{
    public:
        int index;
        int radius;
        int value;
        int centerx;
        int centery;
        Coin();
        Coin(int iindex,int iradius,int icenterx,int icentery);
        bool operator () (const Coin& a, const Coin& b) const {
            if (a.radius > b.radius) return true;
            if (a.radius < b.radius) return false;
            return a.index < b.index;
        }
};

Coin::Coin(){
}

Coin::Coin(int iindex,int iradius,int icenterx,int icentery){
    index = iindex;
    radius = iradius;
    centerx = icenterx;
    centery = icentery;
}

int main(){
    for(int p=1;p<=4;p++){
        Mat src, gray;
        const float kScaleFactor = 0.6;
        src = imread( "coin"+to_string(p)+".jpg", 1 );
        resize(src, src, cvSize(0, 0), kScaleFactor, kScaleFactor);

        cvtColor( src, gray, CV_BGR2GRAY );

        //threshold & morpholody
        threshold(gray,gray,114,255,3);
        Mat const structure_elem2 = getStructuringElement(MORPH_RECT, Size(2, 2));
        morphologyEx(gray, gray,MORPH_OPEN, structure_elem2);
        Mat const structure_elem = getStructuringElement(MORPH_RECT, Size(2, 2));
        morphologyEx(gray, gray, MORPH_CLOSE, structure_elem);

        // Reduce the noise so we avoid false circle detection
        medianBlur(gray,gray,9);
        GaussianBlur( gray, gray, Size(7.5, 5.5), 2, 2 );

        vector<Vec3f> circles;

        // Apply the Hough Transform to find the circles
        //for testcoin.jpg
        HoughCircles( gray, circles, CV_HOUGH_GRADIENT, 2, 100,200,100,50 );

        Coin coins[circles.size()];

        // Draw the circles detected
        for( size_t i = 0; i < circles.size(); i++ ){
            Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
            int radius = cvRound(circles[i][2]);
            circle( src, center, 3, Scalar(0,0,255), -1, 8, 0 );// circle center
            circle( src, center, radius, Scalar(0,255,0), 1, 8, 0 );// circle outline
            coins[i] = Coin(i,radius,circles[i][0],circles[i][1]);
            string a = to_string(i)+":"+to_string(radius);
            putText( src, a , center, CV_FONT_HERSHEY_COMPLEX, 1,Scalar(i, i, 255), 3, 8 );
            //cout<<to_string(coins[i].radius)<<endl;
            cout <<"["<<i<<"] "<< "center : " << center << "\nradius : " << radius << endl;
        }

        //for ratio loop
        cout <<"MATRIX"<<endl;
        float m[circles.size()][circles.size()];
        for (size_t i = 0; i < circles.size(); i++) {
            for (size_t j = 0; j < circles.size(); j++) {
                m[i][j] = (float)coins[j].radius/(float)coins[i].radius;
                cout<<m[i][j]<<" ";
            }
            cout<<endl;
        }

        cout <<"Total Objects : "<<circles.size()<<endl;
        cout <<"============================================"<<endl;
        // Show your results
        namedWindow( "Hough Circle "+to_string(p), CV_WINDOW_AUTOSIZE );
        imshow( "Hough Circle "+to_string(p), src );
        imshow( "Gray "+to_string(p), gray );
    }
    waitKey(0);
    return 0;
}
