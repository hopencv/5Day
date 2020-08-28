#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <iostream>


#define IMG_Width	640
#define IMG_Height	480
#define PERSPECTIVE_IMG_W 640
#define PERSPECTIVE_IMG_H 480


#define ASSIST_BASE_LINE 130
#define ASSIST_BASE_WIDTH 30


using namespace cv;
using namespace std;

Point2f Source[]={Point2f(0,0),Point2f(-270,330),Point2f(640,0),Point(640+270,330)};
Point2f Destination[]={Point2f(0,0),Point2f(0,PERSPECTIVE_IMG_H),Point2f(PERSPECTIVE_IMG_W,0),Point(PERSPECTIVE_IMG_W,PERSPECTIVE_IMG_H)};

Mat Perspective(Mat img)
{
    Mat Matrix, result_img;
    Matrix = getPerspectiveTransform(Source,Destination);
    warpPerspective(img,result_img,Matrix,Size(PERSPECTIVE_IMG_W,PERSPECTIVE_IMG_H));
    
    return result_img;
}

Mat Canny_Edge_Detection(Mat img)
{
    Mat mat_blur_img, mat_canny_img;
    blur(img,mat_blur_img,Size(3,3));
    Canny(mat_blur_img,mat_canny_img,40,130,3);

    return mat_canny_img;
}

int main(){

	int img_width, img_height;
	img_width = 640;
	img_height = 480;
	
	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////// OpenCV 변 수선 언///////////////////////////////////////////////////
	
	Mat mat_image_org_color;
	Mat mat_image_org_color_Overlay;
	Mat mat_image_org_gray;
	Mat mat_image_gray_result;
	Mat mat_image_canny_Edge;
	Mat image;
	
	Scalar GREEN(0,255,0);
	Scalar RED(0,0,255);
	Scalar BLUE(255,0,0);
	Scalar YELLOW(0,255,255);
	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	mat_image_org_color = imread("/home/pi/HancomMDS/AutoCar/C++/OpenCV/7/images/line_2_0.jpg");
	mat_image_org_color.copyTo(mat_image_org_color_Overlay);
	img_width = mat_image_org_color.size().width;
	img_height = mat_image_org_color.size().height;
	
	printf("Image size[%3d,%3d]\n", img_width, img_height);
	
	namedWindow("Display window", CV_WINDOW_NORMAL);
	resizeWindow("Display window", img_width, img_height);
	moveWindow("Display window", 10, 20);

	namedWindow("Gray Image window", CV_WINDOW_NORMAL);
	resizeWindow("Gray Image window", img_width, img_height);
	moveWindow("Gray Image window", 700, 20);

	namedWindow("Canny Edge window", CV_WINDOW_NORMAL);
	resizeWindow("Canny Edge window", img_width, img_height);
	moveWindow("Canny Edge window", 10, 520);
	
	while(1)
	{
		mat_image_org_color=imread("/home/pi/HancomMDS/AutoCar/C++/OpenCV/7/images/line_2_0.jpg");
		cvtColor(mat_image_org_color,mat_image_org_gray,CV_RGB2GRAY); //coloar to gray conversion
		//threshold(mat_image_org_gray,mat_image_canny_Edge,200,255,THRESHO_BINARY);
        
		mat_image_canny_Edge= Canny_Edge_Detection(mat_image_org_gray);
		image=Perspective(mat_image_canny_Edge);
		vector<Vec4i> linesP;
		//HoughLinesP(mat_image_canny_Edge,linesP,1,CV_PI/180,70,30,40);
		HoughLinesP(image,linesP,1,CV_PI/180,70,30,40);
		printf("Line Number : %3d\n",linesP.size());
        
		for(int i=0;i<linesP.size();i++)
		{
			Vec4i L = linesP[i];
			int cx1 = linesP[i][0];
			int cy1 = linesP[i][1];
			int cx2 = linesP[i][2];
			int cy2 = linesP[i][3];
            
			line(mat_image_org_color,Point(L[0],L[1]),Point(L[2],L[3]),Scalar(0,0,255),3,LINE_AA);
			printf("L : [%3d,%3d] , [%3d,%3d] \n" , L[0],L[1],L[2],L[3]);
			printf("H : [%3d,%3d] , [%3d,%3d] \n" , cx1,cy1,cx2,cy2);
		}	
		printf("\n\n\n");
        
		if(mat_image_org_color.empty())
		{
			cerr << "빈 영상입니다. \n";
			break;
		}
		int guide_width1 = 50;
		int guide_height1 = 30;
		int guide_l_center = 0 + 70;
		int guide_r_center = IMG_Width-70;;
		rectangle(mat_image_org_color_Overlay, Point(50, ASSIST_BASE_LINE-ASSIST_BASE_WIDTH), Point(IMG_Width-50, ASSIST_BASE_LINE + ASSIST_BASE_WIDTH), Scalar(0,255,0), 1, LINE_AA);
		line(mat_image_org_color_Overlay, Point(guide_l_center-guide_width1, ASSIST_BASE_LINE), Point(guide_l_center,ASSIST_BASE_LINE),Scalar(0,255,255), 1, 0);
		line(mat_image_org_color_Overlay, Point(guide_l_center, ASSIST_BASE_LINE), Point(guide_l_center+guide_width1,ASSIST_BASE_LINE),Scalar(0,255,255), 1, 0);
		line(mat_image_org_color_Overlay, Point(guide_l_center-guide_width1, ASSIST_BASE_LINE-guide_height1), Point(guide_l_center-guide_width1,ASSIST_BASE_LINE+guide_height1),Scalar(0,255,255), 1, 0);
		line(mat_image_org_color_Overlay, Point(guide_l_center+guide_width1, ASSIST_BASE_LINE-guide_height1), Point(guide_l_center+guide_width1,ASSIST_BASE_LINE+guide_height1),Scalar(0,255,255), 1, 0);		
		line(mat_image_org_color_Overlay, Point(guide_r_center-guide_width1, ASSIST_BASE_LINE), Point(guide_r_center,ASSIST_BASE_LINE),Scalar(0,255,255), 1, 0);
		line(mat_image_org_color_Overlay, Point(guide_r_center, ASSIST_BASE_LINE), Point(guide_r_center+guide_width1,ASSIST_BASE_LINE),Scalar(0,255,255), 1, 0);
		line(mat_image_org_color_Overlay, Point(guide_r_center-guide_width1, ASSIST_BASE_LINE-guide_height1), Point(guide_r_center-guide_width1,ASSIST_BASE_LINE+guide_height1),Scalar(0,255,255), 1, 0);
		line(mat_image_org_color_Overlay, Point(guide_r_center+guide_width1, ASSIST_BASE_LINE-guide_height1), Point(guide_r_center+guide_width1,ASSIST_BASE_LINE+guide_height1),Scalar(0,255,255), 1, 0);
		line(mat_image_org_color_Overlay, Point(IMG_Width/2, ASSIST_BASE_LINE - guide_height1*1.5),Point(IMG_Width/2, ASSIST_BASE_LINE + guide_height1*1.5), Scalar(255,255,255), 2, 0);				
		//line()
		//imshow("Display window", mat_image_org_color);
		imshow("Display window", mat_image_org_color_Overlay);
		imshow("Gray Image window",image /*mat_image_org_gray*/);
		imshow("Canny Edge window", mat_image_canny_Edge);
		if(waitKey(10)>0)
			break;	
	}
	destroyAllWindows();
		
	return 0;
}
