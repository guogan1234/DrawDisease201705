#include "stdafx.h"
#include "cv.h"  
#include <io.h>
#include <stack>
#include <string>
#include <vector>
#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

struct SickProperties
{
	Point site;
	double lenOrArea;
	int type;   // 1: 线 ,0: 区域
};

// 计算病害信息,true为线状病害,false区域病害
vector<SickProperties> computeSickProperties(Mat img,bool type); 

// vector排序算子
bool algoComp(const Point &A, const Point &B);

int main()
{
	Mat img = imread("2.jpg");
	
	vector<SickProperties> sickMsg = computeSickProperties(img, false);

	waitKey();
	destroyAllWindows();
	return 0;
}

bool algoComp(const Point &A, const Point &B)
{
	if (A.x < B.x) return true;
	return false;
}

vector<SickProperties> computeSickProperties(Mat img, bool type)
{
	Mat gray, thImg, draw;
	vector<vector<Point>> contours;

	cvtColor(img, gray, COLOR_BGR2GRAY);
	threshold(gray, thImg, 200, 255, THRESH_BINARY_INV);
	findContours(thImg, contours, RETR_EXTERNAL, CHAIN_APPROX_TC89_L1); //CV_RETR_CCOMP, KCOS
	draw = Mat(thImg.size(), CV_8UC3, Scalar(0, 0, 0));

	for (int i = 0; i < contours.size(); i++)
	{
		drawContours(draw, contours, i, Scalar(255, 255, 255), 1, 8);
	}
	namedWindow("draw", WINDOW_NORMAL);
	imshow("draw", draw);

	vector<SickProperties> SickMsg;
	SickProperties sickProperty;
	double lenOrArea;
	for (int i = 0; i < contours.size(); i++)
	{
		type ? lenOrArea = arcLength(contours[i], true)/2 : lenOrArea = contourArea(contours[i]);
		
		sort(contours[i].begin(), contours[i].end(), algoComp);
		//circle(draw, contours[i][0],50,Scalar(0,0,255),10);
		sickProperty.site = contours[i][0]; // 起始坐标site
		sickProperty.lenOrArea = lenOrArea;
		sickProperty.type = type;
		
		SickMsg.push_back(sickProperty);  
	}

	for (int i = 0; i < SickMsg.size(); i++)
	{
		if (SickMsg[i].site.y > round(img.rows / 2))
		{
			SickMsg[i].site.y = img.rows - SickMsg[i].site.y;
		}
	}

	for (int i = 0; i < SickMsg.size(); i++)
	{
		cout << "-------------SickMsg["<<i<<"]--------------" << endl;
		cout << "type: "<< SickMsg[i].type << endl;
		cout << "site: "<< SickMsg[i].site << endl;
		cout << "lenOrArea: "<<SickMsg[i].lenOrArea << endl;
	}
	
	namedWindow("draw result", WINDOW_NORMAL);
	imshow("draw result", draw);

	return SickMsg;
}