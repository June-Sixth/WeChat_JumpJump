#include <Opencv2/opencv.hpp>
#include <stdio.h>
#include <Windows.h>
#include <highgui.h>

using namespace cv;
using namespace std;

//��ͼ��g_srcImage
Mat g_srcImage;
//����ģ��ͼ��
Mat character = imread("character.png", IMREAD_GRAYSCALE);
Mat white_point = imread("white_point.png", IMREAD_GRAYSCALE);
//�˵�λ�ã��׵��λ��
Point Loc_man, Loc_white_point;
//��һ�����������λ��,����������
Point Loc_center = Point(0, 0);

//��ȡ�ֻ���ͼ
void get_screen_shot()
{
	system("adb shell screencap -p /sdcard/jump.png");
	system("adb pull /sdcard/jump.png E:/Jump/Jump");
	Mat srcImage = imread("jump.png");
	//��ȡ�м���Ҫ��һ��ͼ�񣬷�����
	Mat srcROI = srcImage(Range(666, 1332), Range(66, 1080-66));//1920,1080
	g_srcImage=srcROI.clone();
}

//ģ�ⰴѹ��Ļ����
void jump(int& get_distance)
{
	char Time[50];
	int press_time = get_distance * 1.35;
	sprintf(Time, "adb shell input swipe %d %d %d %d %d", Loc_center.x, Loc_center.y, Loc_center.x, Loc_center.y, press_time);
	cout << Time << endl;
	system(Time);
}

//�������
int get_distance(Point& first_point, Point& next_point)
{
	int A = first_point.x - next_point.x;
	int B = first_point.y - next_point.y;
	return int(pow(pow(A, 2) + pow(B, 2), 0.5));
}

//��ȡ�������Ĺ���ֵ
Point get_center(Mat& image)
{
	Canny(image, image, 3, 6);
	int white_x, white_y, white_lenth = 0;
	bool loop_end_flag = 0;
	for (int j = 0; j < int(image.rows); j++)
	{
		uchar *data = image.ptr<uchar>(j);
		for (int i = 0; i < image.cols; i++)
		{
			if (data[i] == 255)
			{
				white_y = j;
				white_x = i;
				white_lenth++;
				loop_end_flag = 1;
			}
		}
		if (loop_end_flag)
			break;
	}
	//Ϳ��
	for (int j = Loc_white_point.y - white_point.rows; j <= Loc_white_point.y; j++)
	{
		uchar *data = image.ptr<uchar>(j);
		for (int i = 0; i<image.cols; i++)
			data[i] = 0;
	}
	if (white_lenth > 5)//Բ����̨
	{
		//������������
		Point up = Point(white_x - white_lenth / 2, white_y);
		int dark_lenth=0;
		for (int j = white_y+1; j < image.rows; j++)
		{
			uchar *data = image.ptr<uchar>(j);
				dark_lenth++;
			if (data[up.x] == 255)
				break;
		}
		if (dark_lenth < 70)
			Loc_center = Point(up.x, up.y + 70);
		else
			Loc_center = Point(up.x, up.y + dark_lenth / 2);
	}
	else//������̨
	{
		//������������
		Point up = Point(white_x - white_lenth / 2, white_y);
		int dark_lenth = 0;
		for (int j = white_y+1; j < image.rows ; j++)
		{
			uchar *data = image.ptr<uchar>(j);
			if (data[up.x] == 0)
				dark_lenth++;
			if (data[up.x] == 255)
				break;
		}
		if (dark_lenth < 100)
			Loc_center = Point(white_x, white_y + 100);
		else
			Loc_center = Point(up.x, up.y + dark_lenth / 2);
	}
	resize(image, image, Size(image.cols / 2, image.rows / 2));
	imshow("gray_image", image);
	return Loc_center;
}
//ƥ������λ��
void match_man_location(Mat& src_image)
{
	//�������ͼ��resultImage(ע���С������)
	//���ԭͼ(������ͼ��)�ߴ�ΪW * H, ��ģ��ߴ�Ϊ w * h, ����ͼ��ߴ�һ����(W-w+1)*(H-h+1)  
	int width = src_image.cols - character.cols + 1;
	int height = src_image.rows - character.rows + 1;
	Mat result_image(Size(width, height), CV_32FC1);//��ͨ��32λ������
	//ģ��ƥ��
	matchTemplate(src_image, character, result_image, TM_SQDIFF_NORMED);
	//��һ��
	normalize(result_image, result_image, 0, 100, NORM_MINMAX);
	double minVal, maxVal;
	Point minLoc, maxLoc, matchLoc;
	minMaxLoc(result_image, &minVal, &maxVal, &minLoc, &maxLoc, Mat());
	rectangle(src_image, Rect(minLoc, Size(character.cols, character.rows)), Scalar(255, 255, 255), 1, 8, 0);
	//�е�
	matchLoc = Point(minLoc.x + character.cols / 2, minLoc.y + character.rows/2);
	//�����׵����浽�Ҷ�ͼ
	Mat srcROI = src_image(Range(matchLoc.y - white_point.rows / 2, matchLoc.y + white_point.rows / 2), Range(matchLoc.x - white_point.cols / 2, matchLoc.x + white_point.cols / 2));
	white_point.copyTo(srcROI);
	Loc_man = Point(matchLoc.x, matchLoc.y + character.rows / 2 - 25);
	circle(src_image, Loc_man, 2, Scalar(255, 255, 0), -1, 8, 0);
}
//ƥ��׵�λ�ã��뿽�����Ƚϵó��Ƿ�׼ȷ
void match_point_location(Mat& src_image)
{
	//�������ͼ��resultImage(ע���С������)
	//���ԭͼ(������ͼ��)�ߴ�ΪW * H, ��ģ��ߴ�Ϊ w * h, ����ͼ��ߴ�һ����(W-w+1)*(H-h+1)  
	//int width = src_image.cols - white_point.cols + 1;
	//int height = src_image.rows - white_point.rows + 1;
	Mat result_image;//(Size(width, height), CV_32FC1);//��ͨ��32λ������
	matchTemplate(src_image, white_point, result_image, TM_CCORR_NORMED);
	normalize(result_image, result_image, 0, 1, NORM_MINMAX);
	double result = 0;
	for (int j = 0; j < result_image.rows; j++)
	for (int i = 0; i < result_image.cols; i++)
	{
		double match_value = result_image.at<float>(j, i);
		if (match_value>=0.99)
		{
			circle(result_image, Point(i, j), 8, Scalar(0, 0, 0), -1, 8, 0);
			circle(src_image, Point(i+white_point.cols/2, j+white_point.rows/2), 8, Scalar(0, 0, 0), -1, 8, 0);
		}
	}
	imshow("result", result_image);

}
int main()
{
	Mat gray_image;
	while (true)
	{
		get_screen_shot();
		cvtColor(g_srcImage, gray_image, CV_BGR2GRAY);
		match_man_location(gray_image);
		match_point_location(gray_image);
		imshow("gray", gray_image);
		waitKey(0);
	}
}

//static int waitflag = 0;
//Mat grayimage;
//while (true)
//{
//	get_screen_shot();
//	cvtColor(g_srcImage, grayimage, CV_BGR2GRAY);
//	Loc_man = match_template_location(grayimage, character);
//	Loc_white_point = match_template_location(grayimage, white_point);
//
//	Loc_center = get_center(grayimage);
//	circle(g_srcImage, Loc_center, 10, Scalar(0, 0, 0), -1, 8, 0);
//	resize(g_srcImage, g_srcImage, Size(g_srcImage.cols / 2, g_srcImage.rows / 2));
//	imshow("jump", g_srcImage);
//	int D;
//	if (get_distance(Loc_center, Loc_white_point) > 80)//�����������ֵ��ģ��ƥ��ֵ������
//	{
//		D = get_distance(Loc_man, Loc_center);//�Է�������ֵΪ׼
//		waitflag = 0;
//	}
//	else
//	{
//		D = get_distance(Loc_man, Loc_white_point) + 6;//��ģ��ƥ��ֵΪ׼
//		waitflag = 1;
//	}
//	jump(D);
//	if (waitflag)
//		waitKey(2 * D);
//	waitKey(3 * D);
////}
//void match_man_location(Mat& src_image)
//{
//	//�������ͼ��resultImage(ע���С������)
//	//���ԭͼ(������ͼ��)�ߴ�ΪW * H, ��ģ��ߴ�Ϊ w * h, ����ͼ��ߴ�һ����(W-w+1)*(H-h+1)  
//	int width = src_image.cols - character.cols + 1;
//	int height = src_image.rows - character.rows + 1;
//	Mat result_image(Size(width, height), CV_32FC1);//��ͨ��32λ������
//	//ģ��ƥ��
//	matchTemplate(src_image, character, result_image, CV_TM_CCOEFF_NORMED);
//	//��һ��
//	normalize(result_image, result_image, 0, 100, NORM_RELATIVE);
//	//����result_image,����ɸѡ����:����1����ֵ����0.9,����2:�κ�ѡ�еĵ���x�����y�����϶�Ҫ����һ�����5(���⻭�߿���Ӱ�����)
//	double result = 0;
//	int match_x = 0, match_y = 0;
//	for (int j = 0; j < result_image.rows; j++)
//	for (int i = 0; i < result_image.cols; i++)
//	{
//		double match_value = result_image.at<float>(j, i);
//		if (match_value >result)
//		{
//			result = match_value;
//			match_x = i;
//			match_y = j;
//		}
//	}
//	printf("%d\n", result);
//	rectangle(result_image, Rect(Point(match_x, match_y), Size(character.cols, character.rows)), Scalar(255, 255, 255), 1, 8, 0);
//	rectangle(src_image, Rect(Point(match_x, match_y), Size(character.cols, character.rows)), Scalar(255, 255, 255), 1, 8, 0);
//	imshow("result", result_image);
//
//}