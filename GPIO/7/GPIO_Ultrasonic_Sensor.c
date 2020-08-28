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



#define IMG_Width	640
#define IMG_Height	480

using namespace cv;
using namespace std;

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

void sig_Handler(int sig);

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
    
    unsigned char test,receive_char;
    
    int img_width, img_height;
	img_width = 640;
	img_height = 480;
	
	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////// OpenCV 변 수선 언///////////////////////////////////////////////////
	

    
    if(GPIO_control_setup() == -1)
    {
        return -1;
    }
    
    signal(SIGINT, sig_Handler);
 
    test='B';
    
    
    while(1)
    {
        printf("%6.3lf [cm]\n", ultrasonic_sensor());
       delay(100);
       // test = getch();
        
        test = 'y';
        
        switch(test)
        {
            case 'w': //가속(전)
                motor_control_r(pwm_r);
                motor_control_l(pwm_l);
                pwm_r++;
                pwm_l++;
                if(pwm_r>100) pwm_r = 100;
                if(pwm_l>100) pwm_l = 100;
                break;
            case 's': // 정지
                motor_control_r(0);
                motor_control_l(0);
                break;
                
            case 'x': //감속
                motor_control_r(pwm_r);
                motor_control_l(pwm_l);
                pwm_r--;
                pwm_l--;
                if(pwm_r<-100) pwm_r = 100;
                if(pwm_l<-100) pwm_l = 100;
                break;
            case 'a': //왼쪽 
                motor_control_r(pwm_r);
                motor_control_l(pwm_l);
                pwm_r++;
                pwm_l--;
                if(pwm_r>100) pwm_r = 100;
                if(pwm_l<-100) pwm_l = -100;
                break;
            case 'd': // 오른쪽 
                motor_control_r(pwm_r);
                motor_control_l(pwm_l);
                pwm_r--;
                pwm_l++;
                if(pwm_r<-100) pwm_r = -100;
                if(pwm_l>100) pwm_l = 100;
                break;
            case 'p':
                motor_control_r(0);
                motor_control_l(0);
                exit(0);
                break;
            
        }
        
    }

    return 0;

}

void sig_Handler(int sig)
{
    printf("\n\n\n\nProgram and Stop Motor!\n\n\n");
    motor_control_r(0);
    motor_control_l(0);
    exit(0); 
}
