#include <Opencv2/opencv.hpp>
#include <stdio.h>
#include <Windows.h>
#include <highgui.h>

using namespace cv;
using namespace std;

//ȫ�ֱ���
Mat g_srcImage;//��ͼ�ò�ͼ
Mat gray_image;//�����ûҶ�ͼ
Mat character = imread("character.png", IMREAD_GRAYSCALE);//������ģ��ͼ�񣬻Ҷ�ͼ
Mat white_point = imread("white_point.png", IMREAD_GRAYSCALE);//���ذ׵�ģ��ͼ�񣬻Ҷ�ͼ

Point Loc_man = Point(0, 0);//�˵�λ��
Point Loc_white_point = Point(0, 0); //�׵��λ��,
Point Loc_match = Point(0, 0);//���ڶԱ�ƥ��Ч���İ׵����������
Point Loc_center = Point(0, 0);//��һ�����������λ��,ͬʱ���ڵ����Ļ

bool left_or_right = 1;//�������0�Ұ��1
int D;//Ҫ���ľ���

//��ȡ�ֻ���ͼ
void get_screen_shot()
{
	system("adb shell screencap -p /sdcard/jump.png");
	system("adb pull /sdcard/jump.png E:/auto_jump/auto_jump");
	Mat srcImage = imread("jump.png");
	//��ȡ�м���Ҫ��һ��ͼ�񣬷�����
	Mat srcROI = srcImage(Range(666, 1332), Range(0, 1080));//1920,1080
	g_srcImage = srcROI.clone();
}

//ģ�ⰴѹ��Ļ����
void jump(int& get_distance)
{
	char Time[50];
	int press_time = get_distance * 1.35;
	sprintf(Time, "adb shell input swipe %d %d %d %d %d", Loc_center.x, Loc_center.y+400, Loc_center.x, Loc_center.y+400, press_time);
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

//ƥ��������
void match_man_location(Mat& gray_image)
{
	Loc_man = Point(0, 0), Loc_white_point = Point(0, 0), Loc_match = Point(0, 0);
	Mat result_image;
	//ģ��ƥ��
	matchTemplate(gray_image, character, result_image, TM_SQDIFF_NORMED);
	//��һ��
	normalize(result_image, result_image, 0, 1, NORM_MINMAX);
	double minVal, maxVal;
	Point minLoc, maxLoc;
	minMaxLoc(result_image, &minVal, &maxVal, &minLoc, &maxLoc, Mat());
	//rectangle(gray_image, Rect(minLoc, Size(character.cols, character.rows)), Scalar(255, 255, 255), 1, 8, 0);
	//��ģ�����ĵ�
	Loc_match = Point(minLoc.x + character.cols / 2, minLoc.y + character.rows / 2);
	
	//Loc_match����������ͼ���Ҳ�����
	if (Loc_match.x < gray_image.cols / 2)
		left_or_right = 1;
	else 		//Loc_match���ң�������ͼ���������	
		left_or_right = 0;
	//�����׵����浽�Ҷ�ͼ��Ϊƥ��ȵıȽϲ���
	Mat srcROI = gray_image(Range(Loc_match.y - white_point.rows / 2, Loc_match.y + white_point.rows / 2), Range(Loc_match.x - white_point.cols / 2, Loc_match.x + white_point.cols / 2));
	white_point.copyTo(srcROI);
	Loc_man = Point(Loc_match.x, Loc_match.y + character.rows / 2 - 15);
	circle(gray_image, Loc_man, 2, Scalar(255, 255, 0), -1, 8, 0);
}

//ƥ��׵�λ�ã��뿽�����Ƚϵó��Ƿ�׼ȷ
void match_point_location(Mat& gray_image)
{
	//�������ͼ��resultImage(ע���С������)
	//���ԭͼ�ߴ�ΪW * H, ��ģ��ߴ�Ϊ w * h, ����ͼ��ߴ�һ����(W-w+1)*(H-h+1)  
	int width = gray_image.cols - white_point.cols + 1;
	int height = gray_image.rows - white_point.rows + 1;
	//���ͼ��Ϊ��ͨ��32λ������
	Mat result_image(Size(width, height), CV_32FC1);
	matchTemplate(gray_image, white_point, result_image, TM_CCORR_NORMED);
	normalize(result_image, result_image, 0, 1, NORM_MINMAX);
	for (int j = 0; j < result_image.rows; j++)
	{
		
		if (left_or_right)//ɨ���Ұ���Ļ
		for (int i = Loc_match.x + white_point.cols; i < result_image.cols; i++)
		{
			double match_value = result_image.at<float>(j, i);
			if (match_value >= 0.95)
			{
				Loc_white_point = Point(i + white_point.cols / 2, j + white_point.rows / 2-5);
				//circle(gray_image, Loc_white_point, 8, Scalar(0, 0, 0), -1, 8, 0);
				//circle(result_image, Point(i, j), 8, Scalar(0, 0, 0), -1, 8, 0);
			}
		}
		else//ɨ�������Ļ

		for (int i = 0; i <= Loc_match.x-white_point.cols; i++)
		{
			double match_value = result_image.at<float>(j, i);
			if (match_value >= 0.95)
			{
				Loc_white_point = Point(i + white_point.cols / 2, j + white_point.rows / 2-5);
				//circle(gray_image, Loc_white_point, 8, Scalar(0, 0, 0), -1, 8, 0);
				//circle(result_image, Point(i, j), 8, Scalar(0, 0, 0), -1, 8, 0);
			}
		}
	}
	//imshow("result", result_image);
}


//��ȡ�������Ĺ���ֵ
void get_center(Mat& gray_image)
{
	Canny(gray_image, gray_image, 4, 7, 3);
	//��һ�α�����������������ɨ�裬�ҵ����
	int white_x=0, white_y=0, white_lenth = 0;//��ɫ����ʼλ��x��y����ɫ���ص�ĸ���
	bool loop_end_flag = 0;//��������ѭ��

	if (left_or_right)//ɨ���Ұ���Ļ
		for (int j1 = 0; j1 < int(gray_image.rows); j1++)
		{
			//��ָ�뷨����
			uchar *data = gray_image.ptr<uchar>(j1);
			for (int i1 = Loc_match.x + character.cols*0.7; i1 < gray_image.cols; i1++)
			{
				if (data[i1] == 255)//��Χ����ֵ
				{
					if (i1<gray_image.cols*0.2)
						break;//����������뷶Χ��������������canny��������˰�ɫ��������ֱ������һ�����±���
					white_y = j1;
					white_x = i1;
					white_lenth++;
					loop_end_flag = 1;
				}
			}
			if (loop_end_flag)
				break;
		}
	else//ɨ�������Ļ
	for (int j1 = 0; j1 < int(gray_image.rows); j1++)
	{
		//��ָ�뷨����
		uchar *data = gray_image.ptr<uchar>(j1);
		for (int i1 = 0; i1 < Loc_match.x - character.cols*0.7; i1++)
		{
			if (data[i1] == 255)//��Χ����ֵ
			{
				if (i1 < gray_image.cols*0.2)
					break;//����������뷶Χ��������������canny��������˰�ɫ��������ֱ������һ�����±���
				white_y = j1;
				white_x = i1;
				white_lenth++;
				loop_end_flag = 1;
			}
		}
		if (loop_end_flag)
			break;
	}

	//�ڶ��α���
	int edge_x = white_x, delta_y = 0, bias = 3, value = 0;
	if (white_lenth > 5)bias = 10;
	for (int j2 = white_y; j2 <= white_y + 200; j2++)
	{
		for (int i2 = min(edge_x + bias,gray_image.cols-1); i2 >= edge_x - bias; i2--)
		{
			value = gray_image.at<uchar>(j2, i2);
			if (value == 255)
			{
				if (edge_x < i2)
					delta_y = 0;
				else
					delta_y++;
				edge_x = i2;
				break;
			}
		}
		if (delta_y >= 4)
		{
			Loc_center = Point(white_x - white_lenth / 2, j2 - delta_y - 6);
			break;
		}
		else if (white_lenth>5)
			Loc_center = Point(white_x - white_lenth / 2, white_y+65);
		else
			Loc_center = Point(white_x - white_lenth / 2, white_y+90);
	}
	//imshow("canny", gray_image);
}

int main()
{
	int jump_number = 30;
	while (jump_number)
	{
		get_screen_shot();
		cvtColor(g_srcImage, gray_image, CV_BGR2GRAY);
		match_man_location(gray_image);
		circle(g_srcImage, Loc_man, 8, Scalar(0, 0, 255), -1, 8, 0);
		match_point_location(gray_image);
		circle(g_srcImage, Loc_white_point, 8, Scalar(255, 0, 0), -1, 8, 0);
		get_center(gray_image);
		circle(g_srcImage, Loc_center, 8, Scalar(0, 255, 0), -1, 8, 0);
		//imshow("gray", gray_image);
		imshow("jump", g_srcImage);
		if (Loc_white_point==Point(0,0))//δƥ�䵽�׵�
			D = get_distance(Loc_man, Loc_center);
		else
			D = get_distance(Loc_man, Loc_white_point);
		jump(D);
		waitKey(3 * D);
		jump_number--;
	}
}
