#include <opencv2/opencv.hpp>
#include "opencv2/objdetect.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include <opencv2/face.hpp>
#include <glm/glm.hpp>
#include "Calibration.h"

class LandmarkDetector
{
public:
	cv::CascadeClassifier faceDetector;
	cv::VideoCapture cam;
	cv::Mat frame, gray, final_frame;

	cv::Ptr<cv::face::FacemarkLBF> facemark;
	LandmarkDetector(Calibration*);
	void SetupFeatureModels();
	float* Capture();

private:
	static const int blendfaces = 7;
	float* min_max[blendfaces][2];
	/*
	Left Eyebrow
	Right Eyebrow
	Jaw Open
	Left Smile
	Right Smile
	Left Eye
	Right Eye
	*/
	int face_pairs[blendfaces][2] = { {33, 19}, {33, 24}, {66, 62}, {8, 48}, {8, 54}, {37, 41}, {44, 46} };
	float face_range[blendfaces];
	float weight[blendfaces + 3];


	Calibration* calibrate;
	int calibration_counbter = 0;
	std::vector<float> sma_stream;
	std::vector<cv::Rect> faces;
	cv::Scalar color = cv::Scalar(255, 0, 0);
	std::vector< std::vector<cv::Point2f> > landmarks;
	cv::Rect r;
};

