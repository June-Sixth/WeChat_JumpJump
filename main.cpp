#include "Opencv2/opencv.hpp"
#include "stdio.h"
#include "highgui.h"
#include "Windows.h"

using namespace cv;
using namespace std;

//全局变量
Mat g_srcImage;
Mat g_grayImage;
Mat g_Character;
Point g_man_Loc;
Point g_next_jump_point=cvPoint(0,0);

//获取手机截图
void get_screen_shot()
{
	system("adb shell screencap -p /sdcard/jump_src.png");
	system("adb pull /sdcard/jump_src.png E:/Jump_jump_OpenCV/OpenCV");
}

//获取character坐标
Point Get_character_Loc(Mat& srcimg, Mat& Templete_img_man)
{
	Mat g_dstImage;
	matchTemplate(srcimg, Templete_img_man, g_dstImage, TM_SQDIFF_NORMED);
	static double minVal, maxVal;
	Point minLoc, maxLoc, matchLoc;
	minMaxLoc(g_dstImage, &minVal, &maxVal, &minLoc, &maxLoc, Mat());
	printf("匹配最小值为：%d\n", minVal);
	matchLoc = minLoc; //matchLoc是最佳匹配的区域左上角点
	rectangle(g_srcImage, Rect(matchLoc, Size(Templete_img_man.cols, Templete_img_man.rows)), Scalar(255, 255, 0), 1, 8, 0);
	g_man_Loc = Point(matchLoc.x + Templete_img_man.cols*0.5, matchLoc.y + Templete_img_man.rows-2);
	circle(g_srcImage, g_man_Loc, 2, Scalar(255, 255, 0), -1, 8, 0);
	return g_man_Loc;
}

//计算距离
int Get_distance(Point& first_point, Point& next_point)
{
	int A = first_point.x - next_point.x;
	int B = first_point.y - next_point.y;
	return int(pow(pow(3 * A, 2) + pow(3 * B, 2), 0.5));
}

//跳
void jump(int& get_distance)
{
	char Time[50];
	int press_time = get_distance * 1.35;
	sprintf(Time, "adb shell input swipe %d %d %d %d %d", g_next_jump_point.x, g_next_jump_point.y, g_next_jump_point.x, g_next_jump_point.y, press_time);
	cout << Time << endl;
	system(Time);
	Sleep(press_time*2.5);
}

void load_image()
{
	get_screen_shot();
	Sleep(10);
	g_srcImage = imread("jump_src.png");
	while (g_srcImage.empty() | g_Character.empty())
	{
		cout << "Picture loading failed !" << endl;
		Sleep(10);
	}
	resize(g_srcImage, g_srcImage, cvSize(g_srcImage.cols / 3, g_srcImage.rows / 3));
	cvtColor(g_srcImage, g_grayImage, CV_BGR2GRAY);
	g_man_Loc = Get_character_Loc(g_grayImage, g_Character);
	imshow("jump", g_srcImage);
	Mat canny_image;
	Canny(g_grayImage, canny_image, 3, 6, 3);
	imshow("canny", canny_image);
}

void on_mouse(int event, int x, int y, int flags, void* param)
{
	switch (event)
	{
	case CV_EVENT_LBUTTONDOWN:
	{
								 g_next_jump_point = Point(x, y);
								// printf("人坐标%d,%d\n", g_man_Loc.x, g_man_Loc.y);
								// printf("鼠标点击坐标%d,%d\n", x, y);
								 int D = Get_distance(g_man_Loc, g_next_jump_point);
								 //printf("距离%d\n", D);
								 jump(D);
								 load_image();
	}
		break;
	case CV_EVENT_RBUTTONDOWN:
		load_image();
		break;
	}
	
}


int main()
{
	g_Character =imread("character.png", IMREAD_GRAYSCALE);
	resize(g_Character, g_Character, cvSize(g_Character.cols / 3, g_Character.rows / 3));
	load_image();
	g_man_Loc = Get_character_Loc(g_grayImage, g_Character);
	imshow("jump", g_srcImage);
	cvSetMouseCallback("jump", on_mouse);
	cvWaitKey(0);
}

