#include "LandmarkDetector.h"
#include "sma_filer.h"

LandmarkDetector::LandmarkDetector(Calibration* calibrate)
{
	this->calibrate = calibrate;
	SetupFeatureModels();
}

void LandmarkDetector::SetupFeatureModels()
{
	faceDetector = cv::CascadeClassifier("lbpcascade_frontalface.xml");
	facemark = cv::face::FacemarkLBF::create();
	facemark->loadModel("lbfmodel.yaml");
	cam = cv::VideoCapture(0);
}

float* LandmarkDetector::Capture()
{
	if (cam.read(frame))
	{
		cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
		faceDetector.detectMultiScale(gray, faces, 1.2);

		bool success = facemark->fit(frame, faces, landmarks);
		if (success)
		{
			
			r = faces[0];

			landmarks[0] = cv::Mat(cv::Mat(landmarks[0]));
			for (size_t j = 0; j < landmarks[0].size(); j++) {
				sma_x[sma_i][j] = landmarks[0][j];
				//if (j > 16 && j < 27 || j >= 36 && j <= 47 || j >= 48 && j <= 67)
				if(j < landmarks[0].size()-1)
				{
					line(frame, sma_arr[j], sma_arr[j+1], color, 1, cv::LINE_8, 0);
					circle(frame, sma_arr[j], 3, color, -1);
				}
			}
			sma_i++;

			if (sma_i == sma)
			{
				calculateSMA();
				sma_i = 0;
			}

			if (calibrate->isCalibrate)
			{
				calibration_counbter++;
				//printf("Collecting Samples...\n");
				for (int i = 0; i < blendfaces; i++)
				{
					sma_stream.push_back((sma_arr[face_pairs[i][0]] - sma_arr[face_pairs[i][1]]).y / (sma_arr[16] - sma_arr[0]).x);
				}
			}

			if (calibration_counbter == calibration_samples)
			{
				calibration_counbter = 0;
				calibrate->isCalibrate = false;
				printf("Calibrating...\n");
				min_max[0][0] = calibrate->Calibrate(&sma_stream);
				for (int i = 0; i < blendfaces; i++)
				{
					// (2*i +1) and 2*i -> memory mapping (array strides)
					face_range[i] = *(min_max[0][0] + (2*i + 1)) - *(min_max[0][0] + 2*i);
				}
			}
			glm::vec2 X1 = glm::vec2(330, 237);
			glm::vec2 X2 = glm::vec2(370, 237);
			glm::vec2 Y1 = glm::vec2(sma_arr[15].x, sma_arr[15].y);
			glm::vec2 Y2 = glm::vec2(sma_arr[16].x, sma_arr[16].y);
			glm::vec2 X_z_rot = X2 - X1;
			glm::vec2 Y_z_rot = Y2 - Y1;
			float theta = glm::dot(X_z_rot, Y_z_rot);
			//std::cout << theta << std::endl;
			//std::cout << sma_arr[3] << "  " << sma_arr[13] << std::endl;
			//std::cout << ((sma_arr[4] - sma_arr[0]).y / (sma_arr[16] - sma_arr[0]).x) << std::endl;
			weight[9] = (theta / 255) / 2.0;
			weight[7] = 1 - ((sma_arr[31] - sma_arr[3]).x / (sma_arr[16] - sma_arr[0]).x) / 0.35;
			weight[8] = 1 - ((sma_arr[33] - sma_arr[30]).y / (sma_arr[16] - sma_arr[0]).x) / 0.08;
			if (calibrate->Calibrated)
			{
				for (int i = 0; i < blendfaces; i++)
				{
					float range_val = (sma_arr[face_pairs[i][0]] - sma_arr[face_pairs[i][1]]).y / (sma_arr[16] - sma_arr[0]).x;
					range_val -= *(min_max[0][0] + 2*i);
					weight[i] = range_val / face_range[i];
				}
			}
		}
		cv::flip(frame, final_frame, 1);
		cv::imshow("Facial Landmark Detection", final_frame);
	}
	return weight;
}