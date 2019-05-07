#include <opencv2/opencv.hpp>
const int sma = 5;
int sma_i = 0;
cv::Point2f sma_x[sma][68];
cv::Point2f sma_arr[68];

int sma_optim_arr[18] = {30, 19, 33, 24, 66, 62, 8, 48, 54, 37, 41, 44, 46, 31, 16, 0, 3, 13 };

void calculateSMA()
{

	for (int j = 0; j < 68; j++)
	{
		sma_arr[j] = cv::Point2f(0, 0);
	}

	for (int j = 0; j < 68; j++)
	{
		//int j = sma_optim_arr[z];
		for (int i = 0; i < sma; i++)
		{
			sma_arr[j] += sma_x[i][j];
		}
		sma_arr[j] /= sma;
	}
}