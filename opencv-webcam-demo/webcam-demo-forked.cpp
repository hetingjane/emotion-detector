#include <iostream>
#include <memory>
#include <chrono>
#include <thread>
#include <opencv\highgui.h>
#include <filesystem>

#include "Frame.h"
#include "Face.h"
#include "ImageListener.h"
#include "FrameDetector.h"
#include "AffdexException.h"

using namespace std;
using namespace affdex;

float last_timestamp = -1.0f;
float capture_fps = -1.0f;
float process_last_timestamp = -1.0f;
float process_fps = -1.0f;


class PlottingImageListener : public ImageListener
{
public:
	void onImageResults(std::map<FaceId, Face> faces, Frame image) {

		shared_ptr<byte> imgdata = image.getBGRByteArray();
		cv::Mat img = cv::Mat(image.getHeight(), image.getWidth(), CV_8UC3, imgdata.get());
		for (int i = 0; i < faces.size(); i++)
		{
			Face f = faces[i];
			float smile_score = f.getSmileScore();
			int n = f.getFeaturePointCount();
			VecFeaturePoint points = f.getFeaturePoints();
			for (auto& point : points)	//Draw face feature points.
			{
				cv::circle(img, cv::Point(point.x, point.y), 1.0f, cv::Scalar(0, 0, 255));
			}

			//Output the results of the different classifiers.
			cv::putText(img, "Smile: " + std::to_string(f.getSmileScore()), cv::Point(30, 30), cv::FONT_HERSHEY_COMPLEX, 0.5f, cv::Scalar(0, 0, 255));
			cv::putText(img, "BrowFurrow: " + std::to_string(f.getBrowFurrowScore()), cv::Point(30, 50), cv::FONT_HERSHEY_COMPLEX, 0.5f, cv::Scalar(0, 0, 255));
			cv::putText(img, "BrowRaise: " + std::to_string(f.getBrowRaiseScore()), cv::Point(30, 70), cv::FONT_HERSHEY_COMPLEX, 0.5f, cv::Scalar(0, 0, 255));
			cv::putText(img, "LipCornerDepressor: " + std::to_string(f.getLipCornerDepressorScore()), cv::Point(30, 90), cv::FONT_HERSHEY_COMPLEX, 0.5f, cv::Scalar(0, 0, 255));
			cv::putText(img, "Engagement: " + std::to_string(f.getEngagementScore()), cv::Point(30, 110), cv::FONT_HERSHEY_COMPLEX, 0.5f, cv::Scalar(0, 0, 255));
			cv::putText(img, "Valence: " + std::to_string(f.getValenceScore()), cv::Point(30, 130), cv::FONT_HERSHEY_COMPLEX, 0.5f, cv::Scalar(0, 0, 255));


			//Calculate the processing framerate, output both the processing + capture framerate
			if (process_last_timestamp >= 0.0f)
			{
				process_fps = 1.0f / (image.getTimestamp() - process_last_timestamp);
				cv::putText(img, "capture fps: " + std::to_string(capture_fps), cv::Point(img.cols - 200, 30), cv::FONT_HERSHEY_COMPLEX, 0.5f, cv::Scalar(0, 0, 255));
				cv::putText(img, "process fps: " + std::to_string(process_fps), cv::Point(img.cols - 200, 50), cv::FONT_HERSHEY_COMPLEX, 0.5f, cv::Scalar(0, 0, 255));
			}
			process_last_timestamp = image.getTimestamp();
		}
		cv::imshow("analyze-image", img);

		cv::waitKey(30);
	};

	void onImageCapture(Frame image) override
	{};
};

int main(int argsc, char ** argsv)
{

	try {
		// Parse and check the data folder (with assets)
		const std::wstring AFFDEX_DATA_DIR = L"C:\\Program Files (x86)\\Affectiva\\Affdex SDK\\data";
		const std::wstring AFFDEX_LICENSE_FILE = L"affdex.license";

		int framerate = 30;
		int process_frame_rate = 30;
		int buffer_length = 2;

		FrameDetector frameDetector(buffer_length, process_frame_rate);		// Init the FrameDetector Class
		shared_ptr<ImageListener> listenPtr(new PlottingImageListener());	// Instanciate the ImageListener class

		cv::VideoCapture webcam(0);	//Connect to the first webcam
		webcam.set(CV_CAP_PROP_FPS, framerate);	//Set webcam framerate.
		std::cerr << "Setting the webcam frame rate to: " << framerate << std::endl;
		auto start_time = std::chrono::system_clock::now();
		if (!webcam.isOpened())
		{
			std::cerr << "Error opening webcam!" << std::endl;
			return 1;
		}

		//Initialize detectors
		frameDetector.setDetectSmile(true);
		frameDetector.setDetectBrowFurrow(true);
		frameDetector.setDetectBrowRaise(true);
		frameDetector.setDetectLipCornerDepressor(true);
		frameDetector.setDetectEngagement(true);
		frameDetector.setDetectValence(true);
		frameDetector.setClassifierPath(AFFDEX_DATA_DIR);
		frameDetector.setLicensePath(AFFDEX_LICENSE_FILE);
		frameDetector.setImageListener(listenPtr.get());
		//Start the frame detector thread.
		frameDetector.start();

		do {
			cv::Mat img;
			if (!webcam.read(img))	//Capture an image from the camera
			{
				std::cerr << "Failed to read frame from webcam! " << std::endl;
				break;
			}

			//Calculate the Image timestamp and the capture frame rate;
			const auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start_time);
			const float seconds = milliseconds.count() / 1000.f;
			// Create a frame
			Frame f(img.size().width, img.size().height, img.data, Frame::COLOR_FORMAT::BGR, seconds);
			capture_fps = 1.0f / (seconds - last_timestamp);
			last_timestamp = seconds;
			std::cerr << "Capture framerate = " << capture_fps << std::endl;
			frameDetector.process(f);  //Pass the frame to detector
		} while (!GetAsyncKeyState(VK_ESCAPE));

		frameDetector.stop();	//Stop frame detector thread
	}
	catch (AffdexException ex)
	{
		std::cerr << "Encountered an AffdexException " << ex.what();
		return 1;
	}
	catch (std::exception ex)
	{
		std::cerr << "Encountered an exception " << ex.what();
		return 1;
	}
	catch (std::runtime_error err)
	{
		std::cerr << "Encountered a runtime error " << err.what();
		return 1;
	}

	return 0;
}