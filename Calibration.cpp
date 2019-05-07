#include "Calibration.h"



Calibration::Calibration()
{

}

float* Calibration::Calibrate(std::vector<float>* sma_stream)
{
	std::vector<float>& sma_stream_ref = *sma_stream;
	std::vector < std::vector<float>> sorted_sma_stream(blendfaces);

	for (int j = 0; j< calibration_samples; j++)
	{
		for (int i = 0; i < blendfaces; i++)
		{
			//cv::Point2f cvp = sma_stream_ref[sma_counter];
			sorted_sma_stream[i].push_back(sma_stream_ref[sma_counter]);
			//std::cout << sorted_sma_stream[i][j] << " " << sma_stream_ref[sma_counter] << " " << i << " "  << j << std::endl;
			sma_counter++;
		}
	}

	// MIN MAX DEFINITIONS
	for (int i = 0; i < blendfaces; i++)
	{
		min_max[i][0] = *std::min_element(sorted_sma_stream[i].begin(), sorted_sma_stream[i].end());
		min_max[i][1] = *std::max_element(sorted_sma_stream[i].begin(), sorted_sma_stream[i].end());
	}
	Calibrated = true;
	printf("Calibrated\n");
	return &min_max[0][0];

}

