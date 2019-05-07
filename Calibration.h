#ifndef CALIBRATION_H
#define CALIBRATION_H
#include <Eigen/Sparse>
#include <opencv2/opencv.hpp>
const int calibration_samples = 500;

class Calibration
{
public:
	bool isCalibrate = false;
	bool Calibrated = false;
	Calibration();
	float* Calibrate(std::vector<float>*);
private:
	static const int blendfaces = 7;
	float min_max[blendfaces][2];
	int sma_counter = 0;
	std::vector<float> sma_max;
	std::vector<float> sma_min;
};
#endif;
