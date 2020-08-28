#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <wiringPi.h>
#include <wiringSerial.h>
#include <termio.h>
#include <softPwm.h>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <iostream>



#define IMG_Width	640
#define IMG_Height	480

using namespace cv;
using namespace std;

#define ASSIST_BASE_LINE 300
#define ASSIST_BASE_WIDTH 60

#define GPIO0 0  //Physical 11
#define GPIO3 3  //Physical 15


#define ENA 1 //Physical 
#define IN1 4 //Physical 
#define IN2 5 //Physical 

#define ENB 0 //Physical 
#define IN3 2 //Physical 
#define IN4 3 //Physical 

#define MAX_PWM_DUTY 100

/////////Ultrasonic Sensor ////////////

#define Trig 21
#define Echo 22

/////////Serial Com. ///////////
#define baud_rate 115200

int guide_width1 = 50;
int guide_height1 = 30;
int guide_center = IMG_Width/2;
Mat Draw_Guide_Line(Mat img);
void sig_Handler(int sig);


Mat Canny_Edge_Detection(Mat img)
{
    Mat mat_blur_img, mat_canny_img;
    blur(img,mat_blur_img,Size(3,3));
    Canny(mat_blur_img,mat_canny_img,40,130,3);

    return mat_canny_img;
}
int getch(void)
{
    int ch;
    struct termios buf;
    struct termios save;

    tcgetattr(0, &save);
    buf = save;
    buf.c_lflag &= ~(ICANON|ECHO);
    buf.c_cc[VMIN] = 1;
    buf.c_cc[VTIME] = 0;
    tcsetattr(0, TCSAFLUSH, &buf);
    ch = getchar();
    tcsetattr(0, TCSAFLUSH, &save);
    return ch;
}


int GPIO_control_setup(void)
{
    if(wiringPiSetup()==-1)
    {
        printf("wiringPi Setup error !\n");
        return -1;
    }

    pinMode(ENA,OUTPUT);
    pinMode(IN1,OUTPUT);
    pinMode(IN2,OUTPUT);
    
    pinMode(ENB,OUTPUT);
    pinMode(IN3,OUTPUT);
    pinMode(IN4,OUTPUT);
    
    pinMode(Trig,OUTPUT);
    pinMode(Echo,INPUT);
    
    softPwmCreate(ENA,1,MAX_PWM_DUTY);
    softPwmCreate(ENB,1,MAX_PWM_DUTY);
    
    
    softPwmWrite(ENA,0);
    softPwmWrite(ENB,0);
    
    return 0;
    
    
}

void motor_control_r(int pwm)
{
    if(pwm>0) 
    {
        digitalWrite(IN1,LOW);
        digitalWrite(IN2,HIGH);
        softPwmWrite(ENA,pwm);
    }
    else if(pwm == 0)
    {
        digitalWrite(IN1,LOW);
        digitalWrite(IN2,LOW);
        softPwmWrite(ENA,0);
    }
    else
    {
        digitalWrite(IN1,HIGH);
        digitalWrite(IN2,LOW);
        softPwmWrite(ENA,-pwm);
    }
}

void motor_control_l(int pwm)
{
    if(pwm>0) 
    {
        digitalWrite(IN3,LOW);
        digitalWrite(IN4,HIGH);
        softPwmWrite(ENB,pwm);
    }
    else if(pwm == 0)
    {
        digitalWrite(IN3,LOW);
        digitalWrite(IN4,LOW);
        softPwmWrite(ENB,0);
    }
    else
    {
        digitalWrite(IN3,HIGH);
        digitalWrite(IN4,LOW);
        softPwmWrite(ENB,-pwm);
    }
}

float ultrasonic_sensor(void)
{
    long start_time, end_time;
    long temp_time1, temp_time2;
    int duration;
    float distance;
    
    digitalWrite(Trig,LOW);
    delayMicroseconds(5);
    digitalWrite(Trig,HIGH);
    delayMicroseconds(10);
    digitalWrite(Trig,LOW);
    
    delayMicroseconds(200); //wait for burst signal. 40kHz X 8 = 8 X 25us = 200 
    //printf("200msec \n");
    temp_time1 = micros();

    while(digitalRead(Echo) == LOW) //wait for Echo pin High 
    {
        temp_time2 = micros();
        duration = temp_time2 - temp_time1;
        if(duration > 1000) return -1;
    } 
    
   // printf("%d\n",duration);
    
    start_time = micros();
 

    while(digitalRead(Echo) == HIGH)  //wait for Echo pin Low
    {  
        temp_time2 = micros();
        duration = temp_time2 - start_time;
        if(duration >2000) return -1;
        
    } 
    end_time = micros();

    
    duration = end_time - start_time;
    
    distance = duration/ 58;
    
    
    return distance;
}



int main(void)
{
    
    int fd;
    int pwm_r =0;
    int pwm_l =0;
    
    
    int img_width, img_height;
	img_width = 640;
	img_height = 480; 
	
	
	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////// OpenCV 변 수선 언///////////////////////////////////////////////////
	
	Mat mat_image_org_color;
	Mat mat_image_org_gray;
	Mat mat_image_gray_result;
	Mat mat_image_canny_Edge;
	Mat image;
	Mat mat_image_org_color_Overlay;
	Mat mat_image_line_image =Mat(img_height,img_width,CV_8UC1,Scalar(0));
    	
	Scalar GREEN(0,255,0);
	Scalar RED(0,0,255);
	Scalar BLUE(255,0,0);
	Scalar YELLOW(0,255,255);
	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	mat_image_org_color = imread("/home/pi/HancomMDS/AutoCar/C++/Line_Tracer/1/images/line_1_0.jpg");
    mat_image_org_color.copyTo(mat_image_org_color_Overlay);
    if(mat_image_org_color.empty()){
        cerr << "빈 영상입니다.\n";
        return -1;
    }
    img_width = mat_image_org_color.size().width;
	img_height = mat_image_org_color.size().height;
    
    				

    
    namedWindow("Display window", CV_WINDOW_NORMAL);
	resizeWindow("Display window", img_width, img_height);
	moveWindow("Display window", 10, 20);

	namedWindow("Gray Image window", CV_WINDOW_NORMAL);
	resizeWindow("Gray Image window", img_width, img_height);
	moveWindow("Gray Image window", 700, 20);

	namedWindow("Canny Edge window", CV_WINDOW_NORMAL);
	resizeWindow("Canny Edge window", img_width, img_height);
	moveWindow("Canny Edge window", 10, 520);
	
    
    if(GPIO_control_setup() == -1)
    {
        return -1;
    }
    
    signal(SIGINT, sig_Handler);
 
    ///test='B';
    
    
    while(1)
    {
        
        cvtColor(mat_image_org_color, mat_image_org_gray, CV_RGB2GRAY); 		// color to gray conversion
		//Threshold(mat_image_org_gray, mat_image_binary, 127, 255, THRESH_BINARY);
        mat_image_canny_Edge=Canny_Edge_Detection(mat_image_org_gray);
        //
         mat_image_org_color_Overlay=Draw_Guide_Line(mat_image_org_color);
         
        vector<Vec4i> linesP;
		HoughLinesP(mat_image_canny_Edge,linesP,1,CV_PI/180,70,30,40);
		//HoughLinesP(image,linesP,1,CV_PI/180,70,30,40);
		printf("Line Number : %3d\n",linesP.size());
        
		for(int i=0;i<linesP.size();i++)
		{
			Vec4i L = linesP[i];
			//int cx1 = linesP[i][0];
			//int cy1 = linesP[i][1];
			//int cx2 = linesP[i][2];
			//int cy2 = linesP[i][3];
			line(mat_image_line_image,Point(L[0],L[1]),Point(L[2],L[3]),Scalar(255),1,LINE_AA);
			line(mat_image_org_color_Overlay,Point(L[0],L[1]),Point(L[2],L[3]),Scalar(0,0,255),1,LINE_AA);
			printf("L : [%3d,%3d] , [%3d,%3d] \n" , L[0],L[1],L[2],L[3]);
			//printf("H : [%3d,%3d] , [%3d,%3d] \n" , cx1,cy1,cx2,cy2);
		}	
		printf("\n\n\n");
        
       
        
		imshow("Display window", mat_image_org_color_Overlay);
		//imshow("Display window", mat_image_org_color);
		imshow("Gray Image window",mat_image_line_image);
		imshow("Canny Edge window", mat_image_canny_Edge);

		if(waitKey(10)>0){
			break;
		}
		
        
     }
        

    return 0;

}

Mat Draw_Guide_Line(Mat img)
{
    Mat result_img;
    img.copyTo(result_img);
    rectangle(result_img, Point(50, ASSIST_BASE_LINE-ASSIST_BASE_WIDTH), Point(IMG_Width-50, ASSIST_BASE_LINE + ASSIST_BASE_WIDTH), Scalar(0,255,0), 1, LINE_AA);
    line(result_img, Point(guide_center-guide_width1, ASSIST_BASE_LINE), Point(guide_center,ASSIST_BASE_LINE),Scalar(0,255,255), 1, 0);
    line(result_img, Point(guide_center, ASSIST_BASE_LINE), Point(guide_center+guide_width1,ASSIST_BASE_LINE),Scalar(0,255,255), 1, 0);
    line(result_img, Point(guide_center-guide_width1, ASSIST_BASE_LINE-guide_height1), Point(guide_center-guide_width1,ASSIST_BASE_LINE+guide_height1),Scalar(0,255,255), 1, 0);
    line(result_img, Point(guide_center+guide_width1, ASSIST_BASE_LINE-guide_height1), Point(guide_center+guide_width1,ASSIST_BASE_LINE+guide_height1),Scalar(0,255,255), 1, 0);		
    line(result_img, Point(IMG_Width/2, ASSIST_BASE_LINE - guide_height1*1.5),Point(IMG_Width/2, ASSIST_BASE_LINE + guide_height1*1.5), Scalar(255,255,255), 2, 0);
    return result_img;
}

void sig_Handler(int sig)
{
    printf("\n\n\n\nProgram and Stop Motor!\n\n\n");
    motor_control_r(0);
    motor_control_l(0);
    exit(0); 
}
