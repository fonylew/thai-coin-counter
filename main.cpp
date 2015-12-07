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
        int type;
        Coin();
        Coin(int iindex,int iradius,int icenterx,int icentery,int itype);
        bool Compare(const Coin& c) const {
           if (radius<c.radius) return false;
           return true;
        }
        bool operator == (const Coin& c) const {
            return Compare(c);
        }

        bool operator < (const Coin& c) const {
            return Compare(c);
        }
        bool operator () (const Coin& a, const Coin& b) const {
            if (a.Compare(b)) return false;
            return true;
        }
};

Coin::Coin(){
}

Coin::Coin(int iindex,int iradius,int icenterx,int icentery, int itype){
    index = iindex;
    radius = iradius;
    centerx = icenterx;
    centery = icentery;
    type = itype;
}

int main(){
    cout<<"Enter file name : ";
    string filename;
    cin>>filename;
    Mat src, gray;
    const float kScaleFactor = 0.6;
    src = imread( filename, 1 );
    resize(src, src, cvSize(0, 0), kScaleFactor, kScaleFactor);

    cvtColor( src, gray, CV_BGR2GRAY );

    //threshold & morpholody
    threshold(gray,gray,114,255,3);
    Mat const structure_elem2 = getStructuringElement(MORPH_RECT, Size(1, 1));
    morphologyEx(gray, gray,MORPH_OPEN, structure_elem2);
    Mat const structure_elem = getStructuringElement(MORPH_RECT, Size(2, 2));
    morphologyEx(gray, gray, MORPH_CLOSE, structure_elem);

    // Reduce the noise so we avoid false circle detection
    medianBlur(gray,gray,9);
    GaussianBlur( gray, gray, Size(7.5, 5.5), 2, 2 );



    vector<Vec3f> circles;

    // Apply the Hough Transform to find the circles
    HoughCircles( gray, circles, CV_HOUGH_GRADIENT, 2, 100,200,110,33 );

    Coin coins[circles.size()];
    // EXPERIMENTAL
    vector<int> type;
    int ntype = 0;
    map<int,int> valuencoin;

    // Draw the circles detected
    for( size_t i = 0; i < circles.size(); i++ ){
        Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
        int radius = cvRound(circles[i][2]);
        //circle( src, center, 3, Scalar(0,0,255), -1, 8, 0 );// circle center
        circle( src, center, radius, Scalar(0,255,0), 1, 8, 0 );// circle outline
        coins[i] = Coin(i,radius,circles[i][0],circles[i][1],0);
        string a = to_string(i)+":"+to_string(radius);
        putText( src, a , center, CV_FONT_HERSHEY_COMPLEX, 1,Scalar(i, i, 255), 3, 8 );
        //cout<<to_string(coins[i].radius)<<endl;
    }

    //for ratio loop
    float m[circles.size()][circles.size()];
    for (size_t i = 0; i < circles.size(); i++) {
        for (size_t j = 0; j < circles.size(); j++) {
            m[i][j] = (float)coins[j].radius/(float)coins[i].radius;
        }
    }

    // CLUSTERING
    sort(coins,coins+circles.size());

    //Initial value
    int prev_rad = 0;
    float diff_rad, min_diff;
    //There exists a coin
    if(circles.size()>0) {
        ntype = 1;
        prev_rad = coins[0].radius;
        diff_rad = 5;
        min_diff = 3;
    }
    for(int i=0;i<circles.size();i++){
        //count types
        if(prev_rad - coins[i].radius > diff_rad){
            ntype++;
            prev_rad = coins[i].radius;
        }
        coins[i].type = ntype;
    }

    //find 10 baht
    int temp_type = 1;
    int tenbahttype = 0;
    for (int i=0;i<circles.size();i++) {
        if (coins[i].type == temp_type) {
            int btemp = src.at<Vec3b>(Point(coins[i].centerx,coins[i].centery))[0];
            int rtemp = src.at<Vec3b>(Point(coins[i].centerx,coins[i].centery))[1];
            int gtemp = src.at<Vec3b>(Point(coins[i].centerx,coins[i].centery))[2];
            if (rtemp - btemp >= 23 && gtemp - btemp >= 23) {
                tenbahttype = temp_type;
                break;
            } else {
                temp_type++;
            }
        }
    }
    int tenBahtMaxRadius;
    for (int i=0;i<circles.size();i++) {
        if (coins[i].type == tenbahttype) {
            tenBahtMaxRadius = coins[i].radius;
            break;
        }
    }

    //find max of each type
    int maxRadiusOfEachType[ntype];
    int valueIndexOfEachType[ntype];
    double valueOfEachType[8] = {0,10,5,2,1,0.5,0.25,0};
    temp_type = 1;
    for (int i=0;i<circles.size();i++) {
        if (coins[i].type == temp_type) {
            maxRadiusOfEachType[temp_type-1] = coins[i].radius;
            temp_type++;
        }
    }

    //calculate value of each type
    int valueIndex = 0;
    int startTenBaht;
    int index = 1;
    for (int i = 0; i < ntype; i++) {
        if (maxRadiusOfEachType[i] == tenBahtMaxRadius) startTenBaht = i;
    }
    for (int i = 0; i<ntype-1; i++) {
        if (i < startTenBaht) {
            valueIndexOfEachType[i] = 0;
        } else {
            if (maxRadiusOfEachType[i]-maxRadiusOfEachType[i+1] <= 15) {
                valueIndexOfEachType[i] = index;
                valueIndexOfEachType[i+1] = index+1;
                index++;
            } else {
                valueIndexOfEachType[i] = index;
                valueIndexOfEachType[i+1] = index+2;
                index += 2;
            }
        }
    }

    //sum cost
    int c = -1, i = 0;
    double sum = 0;
    while (i < circles.size()) {
        c++;
        if (coins[i].radius == maxRadiusOfEachType[c]) {
            int temp = coins[i].type;
            while (coins[i].type == temp) {
                sum += valueOfEachType[valueIndexOfEachType[c]];
                i++;
            }
        }
    }
    cout <<"Number of coins : "<<circles.size()<<endl;
    cout<<"Summation : "<<sum<<endl;
    cout <<"-----------------------"<<endl;
    cin>>filename;
    waitKey(0);
    return 0;
}
