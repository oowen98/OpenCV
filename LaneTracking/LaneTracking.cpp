/*
Owen Ren

Project to track lanes given a video
1. Frames are passed through a Canny Detector to identify outlines
2. Output of the Canny Detector are then passed through a mask that filters everything outside a given set of coordinates
	-> This is a very fixed method of tracking objects, will need to improve this in the future
3. Lane Segments are then passed through the Hough Transform to identify the lane line equations
4. Line equations are then used to draw lines over the original frame.

This has been a great learning process understanding the basics of image recognition and will serve as a starting point for the final project

*/

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>	

using namespace std;
using namespace cv;

//Function identifiers

int PlayVideo(String VideoName); //Plays Video
Mat Canny(Mat frame); //Passes Frames through the Canny Detector 
Mat LaneSegmenting(Mat frame); //From the Canny Detector, segments only the Lanes
Mat Calculate_Lines(Mat frame, vector<Vec4i> HoughTransform); //Calculate the Line equations of the lane

int main(void) {

	String VideoFile1 = "C:/Users/Owen/Desktop/Photo Albums/GoPro Car Driving Footage/StraightLanes2Muted_Trimmed.mp4";
	String VideoFile2 = "C:/Users/Owen/Desktop/Photo Albums/GoPro Car Driving Footage/CurvedLanesMuted.mp4";


	PlayVideo(VideoFile1);

	return 0;
}

int PlayVideo(String VideoName) {

	//Open the video
	VideoCapture cap(VideoName);

	if (!cap.isOpened()) {
		cout << "Cannot open the video file" << endl;
		cin.get();
		return -1;
	}

	double fps = cap.get(CAP_PROP_FPS);
	cout << "Frames per second " << fps << endl;

	String WindowName = "Lane Tracking Video";
	namedWindow(WindowName, WINDOW_NORMAL);

	String WindowName2 = "Processed Video";
	namedWindow(WindowName2, WINDOW_NORMAL);
	
	String WindowName3 = "LaneSegment Video";
	namedWindow(WindowName3, WINDOW_NORMAL);

	while (true){

		Mat frame;
		
		bool b_Success = cap.read(frame);

		if (!b_Success) {
			cout << "Reached End of Video" << endl;
			break;
		}

		Mat ProcessedFrame;
		Mat LaneSegments;
		Mat Drawnlanes;
		ProcessedFrame = Canny(frame);
		LaneSegments = LaneSegmenting(ProcessedFrame);
		imshow(WindowName3, LaneSegments);
		
		//Perform Hough Transform to indentify lines from the video frame
		vector<Vec4i> lines; //Vector that stores the detected lines from Hough Transform
		HoughLinesP(LaneSegments, lines, 1, CV_PI / 180, 40, 30, 200); //May need to modify this to identify the lines

		Drawnlanes = Calculate_Lines(frame, lines);
		cout << "Lines: " << lines[0];
		//cout << "Lines: " << lines[0][0] << "," << lines[0][1] << "," << lines[0] << "," << lines[1][0] << "," << lines[1] << endl;
		//cout << "Line Size: " << lines.size() << endl;
		//Show frame in the video
		imshow(WindowName, frame);
		imshow(WindowName2, Drawnlanes);
		

		if (waitKey(10) == 27) {
			cout << "ESC Key has been pressed" << endl;
			break;
		}

	}
	cap.release();
	destroyAllWindows();
	return 0;

}

//Returns processed frame using Canny Detector that detects all edges
Mat Canny(Mat frame) {

	Mat grey;
	Mat GaussianBlur_frame;
	Mat CannyOutput;

	//Need to modify, yellow lanes are harder to differentiate
	int CannyLowerthreshold = 37;
	int CannyMaxLowerthreshold = 100;

	//Convert frame to grey scale
	cvtColor(frame, grey, COLOR_RGB2GRAY);

	//Apply a 5x5 Gaussian Blue (Canny Function also does this for us)
	GaussianBlur(grey, GaussianBlur_frame, Size(5, 5), 0);

	//Canny Lower Threshold Trackbar to tune the intensity
	//String windowName = "Canny TrackBar";
	//namedWindow(windowName, WINDOW_AUTOSIZE);

	//createTrackbar("Min Threshold", windowName, &CannyLowerthreshold, CannyMaxLowerthreshold);

	//Canny - Filters noise, finds intensity gradient (Sobel), Non-maximum suppression, Hysteresis thresholding
	Canny(GaussianBlur_frame, CannyOutput, CannyLowerthreshold, (CannyLowerthreshold*3));

	return CannyOutput;

}

//Returns Processed Frame with only the lane
Mat LaneSegmenting(Mat frame) {

	//Want to segment the videos to remove everything other than the lane itself
	int height = frame.rows;
	cout << "Height: " << height << endl;
	cout << "Width " << frame.cols << endl;
	//May need to modify the triangle - How can we dynamically modify this? ML?
	//Segments only the lines within the triangle
	//(x,y)
	//(0-1920),(0-1080) 
	//Height: 0 is the top
	//Width: 0 is the most left
	Point Polygon[1][3];
	Polygon[0][0] = Point(600, 900); //Left Most Point
	Polygon[0][1] = Point(1600, 900); //Right Most Point
	Polygon[0][2] = Point(1080, 650); //Top point


	const Point* ppt[1] = { Polygon[0] };
	int npt[] = { 3 };

	//Create an image with zero intensity with the same dimensions as frame
	Mat mask = Mat::zeros(frame.rows, frame.cols, CV_8U);

	//Fill the area bounded by the polygon with values of 1 from the created mask
	fillPoly(mask, ppt, npt, 1, Scalar(255,255,255), 8);
	//AND the mask and the frame to only show the lane segments
	Mat LaneSegment;
	bitwise_and(frame, mask, LaneSegment);

	return LaneSegment;
}

//From the Hough Transform, calculate the respective lines
Mat Calculate_Lines(Mat frame, vector<Vec4i> HoughTransform) {

	vector<Vec2i> Calculated_lines;
	Mat lanes;
	int line_size = HoughTransform.size();
	//Point line1, line2;

	Mat drawn_lines;

	for (int i = 0; i < line_size; i++) {

		Vec4i l = HoughTransform[i];

		Scalar GreenColor = Scalar(0, 255, 0);
		line(frame, Point(l[0], l[1]), Point(l[2], l[3]), GreenColor, 5, 8);
		
		cout << Point(l[0], l[1]) << " , " << Point(l[2], l[3]) << endl;
	}


	return frame;
}