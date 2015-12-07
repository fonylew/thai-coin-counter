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

Coin::Coin(int iindex,int iradius,int icenterx,int icentery){
    index = iindex;
    radius = iradius;
    centerx = icenterx;
    centery = icentery;
}

//int Coin::Compare (const Coin& c) {
//   if (radius<c.radius) {
//      return -1;
//   }
//   else if (radius>c.radius) {
//      return 1;
//   }
//   return 0;
//}

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
        HoughCircles( gray, circles, CV_HOUGH_GRADIENT, 2, 100,200,110,50 );

        Coin coins[circles.size()];
        // EXPERIMENTAL
        vector<int> type;
        int ntype = 0;
        float sum = 0.0;
        map<float,int> valuencoin;
        vector<float> value {10,5,2,1,0.5,0.25};
        vector<float> tenratio {1.085,1.18,1.31,1.45,1.625};

        // Draw the circles detected
        for( size_t i = 0; i < circles.size(); i++ ){
            Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
            int radius = cvRound(circles[i][2]);
            //circle( src, center, 3, Scalar(0,0,255), -1, 8, 0 );// circle center
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

        // CLUSTERING
        sort(coins,coins+circles.size());
        //Test sorted
        cout<<endl<<"sort : ";

        //Initial value
        int prev_rad = 0, tentype=0 ,vindex=0,iindex=1;
        float ten_rad=0;
        float diff_rad, weight_diff, min_diff;
        bool hasTen = false;
        //There exists a coin
        if(circles.size()>0) {
            ntype = 1;
            prev_rad = coins[0].radius;
            diff_rad = 5;
            weight_diff =0.95;
            min_diff = 3;
        }
        for(int i=0;i<circles.size();i++){
            //count types
            //new type
            if(prev_rad - coins[i].radius > diff_rad){
                for(int j=iindex;j<value.size();j++){
                    if(ten_rad/coins[i].radius<tenratio[j]){
                        vindex=j;
                        iindex=j+1;
                        break;
                    }
                }
//                if(ten_rad/coins[i].radius < tenratio[1]) vindex = 1;       //5 baht
//                else if(ten_rad/coins[i].radius < tenratio[2]) vindex = 2;  //2 baht
//                else if(ten_rad/coins[i].radius < tenratio[3]) vindex = 3;  //1 baht
//                else if(ten_rad/coins[i].radius < tenratio[4]) vindex = 4;  //0.5 baht
//                else if(ten_rad/coins[i].radius < tenratio[5]) vindex = 5;  //0.25 baht

                cout <<" | ";
                ntype++;
                if(diff_rad > min_diff) diff_rad *= weight_diff;
                prev_rad = coins[i].radius;
            }

            //find 10 Baht
            if(!hasTen){
                int b10 =  src.at<Vec3b>(Point(coins[i].centerx,coins[i].centery))[0];
                int g10 =  src.at<Vec3b>(Point(coins[i].centerx,coins[i].centery))[1];
                int r10 =  src.at<Vec3b>(Point(coins[i].centerx,coins[i].centery))[2];
                int yellow_thres = 23;
                if(r10 >= b10+yellow_thres && g10 >= b10+yellow_thres){
                    hasTen = true;
                    tentype = ntype;
                    cout<<"(10)";
                    ten_rad = prev_rad;
                }
            }
            if(ntype == tentype){
                vindex = 0;
                cout<<"*";
                //average 10Baht's radius
                //ten_rad = (ten_rad+coins[i].radius)/2;
            }

            //Valid Coin
            if(vindex>=0&&hasTen) valuencoin[value[vindex]]++;

            //print sortednumber
            cout<<coins[i].radius<<">";
        }
        cout<<endl<<"Number of Types : "<<ntype<<endl;

        cout <<"Total Coins : "<<circles.size()<<endl;
        for (map<float,int>::iterator it = valuencoin.begin(); it != valuencoin.end(); ++it){
            cout << it->first << " => " << it->second << '\n';
            sum = sum + it->first*it->second;
        }
        cout <<"SUM : "<<sum<<endl;
        cout <<"============================================"<<endl;
        // Show your results
        //namedWindow( "Hough Circle "+to_string(p), CV_WINDOW_AUTOSIZE );
        // imshow( "Hough Circle "+to_string(p), src );
        //imshow( "Gray "+to_string(p), gray );
    }
    waitKey(0);
    return 0;
}
