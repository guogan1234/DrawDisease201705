/* 区域类型病害合并 */
#include "stdafx.h"
#include "cv.h"  
#include <iostream>
#include <io.h>
#include <string>
#include <vector>
#include <stack>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

Mat mergeContours(Mat img,int ret);  // 合并区域病害
Mat mergeLines(Mat img,int ret);     // 合并线条病害

void getSkeleton(Mat &srcimage);  // 获取骨架图,输入为单通道、二值化后的图像
void getEight(Mat img, Mat &eight, vector<Point> &points); // 探讨八联通区域
void connectNearest(Mat &eight, vector<Point> &points, int ret, vector<int> conLabels); // 连接最近的两个点
double getEuclidean(Point A, Point B); // 欧氏距离
vector<int> getPair(vector<Point> points, vector<vector<Point>> contours);  // 轮廓和点配对
string num2str(int i);

int main()
{
	double t = (double)getTickCount();

	Mat lineImg = imread("line.jpg");
	Mat lineResult = mergeLines(lineImg,60);
	imwrite("line_result.jpg", lineResult);

	//namedWindow("lineResult",WINDOW_NORMAL);
	//imshow("lineResult", lineResult);

	Mat contourImg = imread("contour.jpg");
	Mat contourResult = mergeContours(contourImg, 100);
	imwrite("contour_Result.jpg", contourResult);

	//namedWindow("contourResult", WINDOW_NORMAL);
	//imshow("contourResult", contourResult);

	Mat mergeImg = lineResult & contourResult;
	imwrite("merge_Img.jpg", mergeImg);

	namedWindow("mergeImg", WINDOW_NORMAL);
	imshow("mergeImg", mergeImg);

	t = ((double)getTickCount() - t) / (getTickFrequency());
	std::cout << "算法耗时: " << t << "s" << endl;

	waitKey();
	destroyAllWindows();
	getchar();
	return 0;
}

Mat mergeContours(Mat img, int ret)
{
	Mat gray, thImg;
	if (img.channels()>1) cvtColor(img, gray, COLOR_BGR2GRAY);
	else img.copyTo(gray);
	threshold(gray, thImg, 200, 255, THRESH_BINARY_INV);

	vector<vector<Point>> contours;
	Mat draw = Mat(thImg.size(), CV_8U, Scalar(0));
	findContours(thImg, contours, RETR_EXTERNAL, CHAIN_APPROX_TC89_L1); //CV_RETR_CCOMP, KCOS
	for (int i = 0; i < contours.size(); i++)
	{
		drawContours(draw, contours, i, Scalar(255), -1, 8);
	}
	//imshow("draw",draw);
	cout << "Before Merge : contours.size() = " << contours.size() << endl;

	Mat closeImg;
	morphologyEx(draw, closeImg, MORPH_CLOSE, getStructuringElement(MORPH_RECT, Size(ret, ret)));
	//imshow("closeImg", closeImg);

	Mat draw2 = Mat(thImg.size(), CV_8UC3, Scalar(255, 255, 255));
	findContours(closeImg, contours, RETR_EXTERNAL, CHAIN_APPROX_TC89_L1); //CV_RETR_CCOMP, KCOS
	for (int i = 0; i < contours.size(); i++)
	{
		drawContours(draw2, contours, i, Scalar(255, 0, 0), 5, 8);
	}
	cout << "After Merge :contours.size() = " << contours.size() << endl;
	return draw2;
}

Mat mergeLines(Mat sick, int ret)
{
	Mat gray, thImg, eight, dilateImg,draw,red;

	if (sick.channels()>1) cvtColor(sick, gray, COLOR_BGR2GRAY);
	else sick.copyTo(gray);
	threshold(gray, thImg, 200, 255, THRESH_BINARY_INV);

	getSkeleton(thImg);

	thImg.copyTo(eight);
	vector<Point> points;

	getEight(thImg, eight, points);

	vector<vector<Point>> contours;
	vector<int> conLabels;
	findContours(thImg, contours, RETR_EXTERNAL, CHAIN_APPROX_TC89_L1); //CV_RETR_CCOMP, KCOS
	conLabels = getPair(points, contours);
	
	connectNearest(eight, points, ret, conLabels);

	morphologyEx(eight, dilateImg, MORPH_DILATE, getStructuringElement(MORPH_RECT, Size(5, 5)));
	draw = Mat(dilateImg.size(), CV_8UC3, Scalar(255, 255, 255));
	red = Mat(dilateImg.size(), CV_8UC3, Scalar(0, 0, 255)); 
	red.copyTo(draw,dilateImg);
	return draw;
}

string num2str(int i)
{
	stringstream ss;
	ss << i;
	return ss.str();
}

vector<int> getPair(vector<Point> points, vector<vector<Point>> contours)
{
	double flag;
	vector<int> conLabels = vector<int>(points.size());
	for (int i = 0; i < points.size(); i++)
	{
		for (int j = 0; j < contours.size(); j++)
		{
			flag = pointPolygonTest(contours[j], points[i], false);
			if (0 == flag)
			{
				conLabels[i] = j;
			}
		}
	}
	return conLabels;
}


double getEuclidean(Point A, Point B)
{
	return sqrt(pow(A.x - B.x, 2) + pow(A.y - B.y, 2));
}

void connectNearest(Mat &eight, vector<Point> &points, int ret, vector<int> conLabels)
{
	double tempD;
	vector<double> dists;
	for (int i = 0; i < points.size(); i++)
	{
		int flag = 1;  // flag控制设置最初最小值的语句在每个次循环只运行一次
		int minV_idx = -1;  // 记录每个点满足要求的最小值时的点的索引
		double minValue;
		for (int j = i; j < points.size(); j++)
		{
			if (i == j) continue;
			tempD = getEuclidean(points[i], points[j]);
			if (conLabels[i] != conLabels[j] && tempD<ret) // 处在不同轮廓才记录
			{
				if (flag == 1)
				{ // 只运行一次
					minValue = tempD;
					minV_idx = j;
					flag == 0;
					continue;
				}
				if (tempD < minValue)
				{
					minValue = tempD;
					minV_idx = j;
				}
			}
		}
		if (-1 != minV_idx) line(eight, points[i], points[minV_idx], Scalar(255), 1);
	}
}

void getEight(Mat img, Mat &eight, vector<Point> &points)
{
	int dataSum = 0;
	for (int i = 1; i < img.rows - 1; i++)
	{
		uchar* data_last = img.ptr<uchar>(i - 1);
		uchar* data = img.ptr<uchar>(i);
		uchar* data_next = img.ptr<uchar>(i + 1);
		for (int j = 1; j < (img.cols - 1); j++)
		{
			if (data[j] == 0) continue;
			dataSum = data[j - 1] + data[j + 1] + data_last[j] + data_last[j - 1] + data_last[j + 1] + data_next[j] + data_next[j - 1] + data_next[j + 1];
			if (dataSum == 255)
			{
				//circle(eight, Point(j, i), 3, Scalar(255)); // 是否标记出线的端点
				points.push_back(Point(j, i));
			}
		}
	}
}

void getSkeleton(Mat &srcimage)
{
	vector<Point> deletelist1;
	int Zhangmude[9];
	int nl = srcimage.rows;
	int nc = srcimage.cols;
	while (true)
	{
		for (int j = 1; j<(nl - 1); j++)
		{
			uchar* data_last = srcimage.ptr<uchar>(j - 1);
			uchar* data = srcimage.ptr<uchar>(j);
			uchar* data_next = srcimage.ptr<uchar>(j + 1);
			for (int i = 1; i<(nc - 1); i++)
			{
				if (data[i] == 255)
				{
					Zhangmude[0] = 1;
					if (data_last[i] == 255) Zhangmude[1] = 1;
					else  Zhangmude[1] = 0;
					if (data_last[i + 1] == 255) Zhangmude[2] = 1;
					else  Zhangmude[2] = 0;
					if (data[i + 1] == 255) Zhangmude[3] = 1;
					else  Zhangmude[3] = 0;
					if (data_next[i + 1] == 255) Zhangmude[4] = 1;
					else  Zhangmude[4] = 0;
					if (data_next[i] == 255) Zhangmude[5] = 1;
					else  Zhangmude[5] = 0;
					if (data_next[i - 1] == 255) Zhangmude[6] = 1;
					else  Zhangmude[6] = 0;
					if (data[i - 1] == 255) Zhangmude[7] = 1;
					else  Zhangmude[7] = 0;
					if (data_last[i - 1] == 255) Zhangmude[8] = 1;
					else  Zhangmude[8] = 0;
					int whitepointtotal = 0;
					for (int k = 1; k < 9; k++)
					{
						whitepointtotal = whitepointtotal + Zhangmude[k];
					}
					if ((whitepointtotal >= 2) && (whitepointtotal <= 6))
					{
						int ap = 0;
						if ((Zhangmude[1] == 0) && (Zhangmude[2] == 1)) ap++;
						if ((Zhangmude[2] == 0) && (Zhangmude[3] == 1)) ap++;
						if ((Zhangmude[3] == 0) && (Zhangmude[4] == 1)) ap++;
						if ((Zhangmude[4] == 0) && (Zhangmude[5] == 1)) ap++;
						if ((Zhangmude[5] == 0) && (Zhangmude[6] == 1)) ap++;
						if ((Zhangmude[6] == 0) && (Zhangmude[7] == 1)) ap++;
						if ((Zhangmude[7] == 0) && (Zhangmude[8] == 1)) ap++;
						if ((Zhangmude[8] == 0) && (Zhangmude[1] == 1)) ap++;
						if (ap == 1)
						{
							if ((Zhangmude[1] * Zhangmude[7] * Zhangmude[5] == 0) && (Zhangmude[3] * Zhangmude[5] * Zhangmude[7] == 0))
							{
								deletelist1.push_back(Point(i, j));
							}
						}
					}
				}
			}
		}
		if (deletelist1.size() == 0) break;
		for (size_t i = 0; i < deletelist1.size(); i++)
		{
			Point tem;
			tem = deletelist1[i];
			uchar* data = srcimage.ptr<uchar>(tem.y);
			data[tem.x] = 0;
		}
		deletelist1.clear();

		for (int j = 1; j<(nl - 1); j++)
		{
			uchar* data_last = srcimage.ptr<uchar>(j - 1);
			uchar* data = srcimage.ptr<uchar>(j);
			uchar* data_next = srcimage.ptr<uchar>(j + 1);
			for (int i = 1; i<(nc - 1); i++)
			{
				if (data[i] == 255)
				{
					Zhangmude[0] = 1;
					if (data_last[i] == 255) Zhangmude[1] = 1;
					else  Zhangmude[1] = 0;
					if (data_last[i + 1] == 255) Zhangmude[2] = 1;
					else  Zhangmude[2] = 0;
					if (data[i + 1] == 255) Zhangmude[3] = 1;
					else  Zhangmude[3] = 0;
					if (data_next[i + 1] == 255) Zhangmude[4] = 1;
					else  Zhangmude[4] = 0;
					if (data_next[i] == 255) Zhangmude[5] = 1;
					else  Zhangmude[5] = 0;
					if (data_next[i - 1] == 255) Zhangmude[6] = 1;
					else  Zhangmude[6] = 0;
					if (data[i - 1] == 255) Zhangmude[7] = 1;
					else  Zhangmude[7] = 0;
					if (data_last[i - 1] == 255) Zhangmude[8] = 1;
					else  Zhangmude[8] = 0;
					int whitepointtotal = 0;
					for (int k = 1; k < 9; k++)
					{
						whitepointtotal = whitepointtotal + Zhangmude[k];
					}
					if ((whitepointtotal >= 2) && (whitepointtotal <= 6))
					{
						int ap = 0;
						if ((Zhangmude[1] == 0) && (Zhangmude[2] == 1)) ap++;
						if ((Zhangmude[2] == 0) && (Zhangmude[3] == 1)) ap++;
						if ((Zhangmude[3] == 0) && (Zhangmude[4] == 1)) ap++;
						if ((Zhangmude[4] == 0) && (Zhangmude[5] == 1)) ap++;
						if ((Zhangmude[5] == 0) && (Zhangmude[6] == 1)) ap++;
						if ((Zhangmude[6] == 0) && (Zhangmude[7] == 1)) ap++;
						if ((Zhangmude[7] == 0) && (Zhangmude[8] == 1)) ap++;
						if ((Zhangmude[8] == 0) && (Zhangmude[1] == 1)) ap++;
						if (ap == 1)
						{
							if ((Zhangmude[1] * Zhangmude[3] * Zhangmude[5] == 0) && (Zhangmude[3] * Zhangmude[1] * Zhangmude[7] == 0))
							{
								deletelist1.push_back(Point(i, j));
							}
						}
					}
				}
			}
		}
		if (deletelist1.size() == 0) break;
		for (size_t i = 0; i < deletelist1.size(); i++)
		{
			Point tem;
			tem = deletelist1[i];
			uchar* data = srcimage.ptr<uchar>(tem.y);
			data[tem.x] = 0;
		}
		deletelist1.clear();
	}
}

