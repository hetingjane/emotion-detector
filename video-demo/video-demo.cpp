#include <iostream>
#include <memory>
#include <chrono>
#include <fstream>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <boost/filesystem.hpp>
#include <boost/timer/timer.hpp>
#include <boost/program_options.hpp>

#include "VideoDetector.h"
#include "PhotoDetector.h"
#include "AffdexException.h"

#include "AFaceListener.hpp"
#include "PlottingImageListener.hpp"
#include "StatusListener.hpp"


using namespace std;
using namespace affdex;

int main(int argsc, char ** argsv)
{



	std::map<boost::filesystem::path, bool> VIDEO_EXTS = { { boost::filesystem::path(".avi"), 1 },
	{ boost::filesystem::path(".mov"), 1 },
	{ boost::filesystem::path(".flv"), 1 },
	{ boost::filesystem::path(".webm"), 1 },
	{ boost::filesystem::path(".wmv"), 1 },
	{ boost::filesystem::path(".mp4"), 1 } };
	affdex::path DATA_FOLDER;
	affdex::path videoPath;

	int process_framerate = 15;
	bool draw_display = false;
	bool loop = false;
	unsigned int nFaces = 1;
	int faceDetectorMode = (int)FaceDetectorMode::LARGE_FACES;

	const int precision = 2;
	std::cerr.precision(precision);
	std::cout.precision(precision);

	namespace po = boost::program_options; // abbreviate namespace
	po::options_description description("Project for demoing the Affdex SDK VideoDetector class (processing video files).");
	description.add_options()
		("help,h", po::bool_switch()->default_value(false), "Display this help message.")
#ifdef _WIN32
		("data,d", po::wvalue< affdex::path >(&DATA_FOLDER)->default_value(affdex::path(L"data"), std::string("data")), "Path to the data folder")
		("input,i", po::wvalue< affdex::path >(&videoPath)->required(), "Video file to processs")
#else // _WIN32
		("data,d", po::value< affdex::path >(&DATA_FOLDER)->default_value(affdex::path("data"), std::string("data")), "Path to the data folder")
		("input,i", po::value< affdex::path >(&videoPath)->required(), "Video file to processs")
#endif // _WIN32
		("pfps", po::value< int >(&process_framerate)->default_value(30), "Processing framerate.")
		("draw", po::value< bool >(&draw_display)->default_value(false), "Draw video on screen.")
		("faceMode", po::value< int >(&faceDetectorMode)->default_value((int)FaceDetectorMode::SMALL_FACES), "Face detector mode (large faces vs small faces).")
		("numFaces", po::value< unsigned int >(&nFaces)->default_value(1), "Number of faces to be tracked.")
		("loop", po::value< bool >(&loop)->default_value(false), "Loop over the video being processed.")
		;
	po::variables_map args;
	try
	{
		po::store(po::command_line_parser(argsc, argsv).options(description).run(), args);
		if (args["help"].as<bool>())
		{
			std::cout << description << std::endl;
			return 0;
		}
		po::notify(args);
	}
	catch (po::error& e)
	{
		std::cerr << "ERROR: " << e.what() << std::endl << std::endl;
		std::cerr << "For help, use the -h option." << std::endl << std::endl;
		return 1;
	}

	// Parse and check the data folder (with assets)
	if (!boost::filesystem::exists(DATA_FOLDER))
	{
		std::cerr << "Data folder doesn't exist: " << std::string(DATA_FOLDER.begin(), DATA_FOLDER.end()) << std::endl;
		std::cerr << "Try specifying the folder through the command line" << std::endl;
		std::cerr << description << std::endl;
		return 1;
	}
	try
	{
		std::shared_ptr<Detector> detector;

		//Initialize out file
		boost::filesystem::path csvPath(videoPath);
		boost::filesystem::path fileExt = csvPath.extension();
		csvPath.replace_extension(".csv");
		std::ofstream csvFileStream(csvPath.c_str());

		if (!csvFileStream.is_open())
		{
			std::cerr << "Unable to open csv file " << csvPath << std::endl;
			return 1;
		}

		if (VIDEO_EXTS[fileExt]) // IF it is a video file.
		{
			detector = std::make_shared<VideoDetector>(process_framerate, nFaces, (affdex::FaceDetectorMode) faceDetectorMode);
		}
		else //Otherwise it's a photo
		{
			detector = std::make_shared<PhotoDetector>(nFaces, (affdex::FaceDetectorMode) faceDetectorMode);
		}


		//VideoDetector videoDetector(process_framerate, nFaces, (affdex::FaceDetectorMode) faceDetectorMode);



		std::cout << "Max num of faces set to: " << detector->getMaxNumberFaces() << std::endl;
		std::string mode;
		switch (detector->getFaceDetectorMode())
		{
		case FaceDetectorMode::LARGE_FACES:
			mode = "LARGE_FACES";
			break;
		case FaceDetectorMode::SMALL_FACES:
			mode = "SMALL_FACES";
			break;
		default:
			break;
		}

		std::cout << "Face detector mode set to: " << mode << std::endl;
		shared_ptr<PlottingImageListener> listenPtr(new PlottingImageListener(csvFileStream, draw_display));

		detector->setDetectAllEmotions(true);
		detector->setDetectAllExpressions(true);
		detector->setDetectAllEmojis(true);
		detector->setDetectAllAppearances(true);
		detector->setClassifierPath(DATA_FOLDER);
		detector->setImageListener(listenPtr.get());


		detector->start();    //Initialize the detectors .. call only once

		do
		{
			shared_ptr<StatusListener> videoListenPtr = std::make_shared<StatusListener>();
			detector->setProcessStatusListener(videoListenPtr.get());
			if (VIDEO_EXTS[fileExt])
			{
				((VideoDetector *)detector.get())->process(videoPath); //Process a video
			}
			else
			{
				//videoPath is of type std::wstring on windows, but std::string on other platforms.
				cv::Mat img = cv::imread(std::string(videoPath.begin(), videoPath.end()));

				// Create a frame
				Frame frame(img.size().width, img.size().height, img.data, Frame::COLOR_FORMAT::BGR);

				((PhotoDetector *)detector.get())->process(frame); //Process an image
			}

			do
			{
				if (listenPtr->getDataSize() > 0)
				{
					std::pair<Frame, std::map<FaceId, Face> > dataPoint = listenPtr->getData();
					Frame frame = dataPoint.first;
					std::map<FaceId, Face> faces = dataPoint.second;


					if (draw_display)
					{
						listenPtr->draw(faces, frame);
					}

					std::cerr << "timestamp: " << frame.getTimestamp()
						<< " cfps: " << listenPtr->getCaptureFrameRate()
						<< " pfps: " << listenPtr->getProcessingFrameRate()
						<< " faces: " << faces.size();

					listenPtr->outputToFile(faces, frame.getTimestamp());
				}
			} while (VIDEO_EXTS[fileExt] && (videoListenPtr->isRunning() || listenPtr->getDataSize() > 0));
		} while (loop);

		detector->stop();
		csvFileStream.close();

		std::cout << "Output written to file: " << csvPath << std::endl;
	}
	catch (AffdexException ex)
	{
		std::cerr << ex.what();
	}

	return 0;
}