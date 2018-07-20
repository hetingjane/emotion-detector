#pragma once


#include <iostream>
#include <memory>
#include <chrono>
#include <thread>
#include <mutex>
#include <fstream>
#include <boost/filesystem.hpp>
#include <boost/timer/timer.hpp>

#include "Visualizer.h"
#include "ImageListener.h"

using namespace affdex;

class PlottingImageListener : public ImageListener
{

	std::mutex mMutex;
	std::deque<std::pair<Frame, std::map<FaceId, Face> > > mDataArray;

	double mCaptureLastTS;
	double mCaptureFPS;
	double mProcessLastTS;
	double mProcessFPS;
	std::ofstream &fStream;
	std::chrono::time_point<std::chrono::system_clock> mStartT;
	const bool mDrawDisplay;
	const int spacing = 20;
	const float font_size = 0.5f;
	const int font = cv::FONT_HERSHEY_COMPLEX_SMALL;
	Visualizer viz;

public:


	PlottingImageListener(std::ofstream &csv, const bool draw_display)
		: fStream(csv), mDrawDisplay(draw_display), mStartT(std::chrono::system_clock::now()),
		mCaptureLastTS(-1.0f), mCaptureFPS(-1.0f),
		mProcessLastTS(-1.0f), mProcessFPS(-1.0f)
	{

		fStream << "TimeStamp,faceId,interocularDistance,glasses,age,ethnicity,gender,dominantEmoji,";
		for (std::string angle : viz.HEAD_ANGLES) fStream << angle << ",";
		for (std::string emotion : viz.EMOTIONS) fStream << emotion << ",";
		for (std::string expression : viz.EXPRESSIONS) fStream << expression << ",";
		for (std::string emoji : viz.EMOJIS) fStream << emoji << ",";
		fStream << std::endl;
		fStream.precision(4);
		fStream << std::fixed;
	}

	cv::Point2f minPoint(VecFeaturePoint points)
	{
		VecFeaturePoint::iterator it = points.begin();
		FeaturePoint ret = *it;
		for (; it != points.end(); it++)
		{
			if (it->x < ret.x) ret.x = it->x;
			if (it->y < ret.y) ret.y = it->y;
		}
		return cv::Point2f(ret.x, ret.y);
	};

	cv::Point2f maxPoint(VecFeaturePoint points)
	{
		VecFeaturePoint::iterator it = points.begin();
		FeaturePoint ret = *it;
		for (; it != points.end(); it++)
		{
			if (it->x > ret.x) ret.x = it->x;
			if (it->y > ret.y) ret.y = it->y;
		}
		return cv::Point2f(ret.x, ret.y);
	};


	double getProcessingFrameRate()
	{
		std::lock_guard<std::mutex> lg(mMutex);
		return mProcessFPS;
	}

	double getCaptureFrameRate()
	{
		std::lock_guard<std::mutex> lg(mMutex);
		return mCaptureFPS;
	}

	int getDataSize()
	{
		std::lock_guard<std::mutex> lg(mMutex);
		return mDataArray.size();

	}

	std::pair<Frame, std::map<FaceId, Face>> getData()
	{
		std::lock_guard<std::mutex> lg(mMutex);
		std::pair<Frame, std::map<FaceId, Face>> dpoint = mDataArray.front();
		mDataArray.pop_front();
		return dpoint;
	}

	void onImageResults(std::map<FaceId, Face> faces, Frame image) override
	{
		std::lock_guard<std::mutex> lg(mMutex);
		mDataArray.push_back(std::pair<Frame, std::map<FaceId, Face>>(image, faces));
		std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
		std::chrono::milliseconds milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(now - mStartT);
		double seconds = milliseconds.count() / 1000.f;
		mProcessFPS = 1.0f / (seconds - mProcessLastTS);
		mProcessLastTS = seconds;
	};

	void onImageCapture(Frame image) override
	{
		std::lock_guard<std::mutex> lg(mMutex);
		mCaptureFPS = 1.0f / (image.getTimestamp() - mCaptureLastTS);
		mCaptureLastTS = image.getTimestamp();
	};

	void outputToFile(const std::map<FaceId, Face> faces, const double timeStamp)
	{
		if (faces.empty())
		{
			fStream << timeStamp << ",nan,nan,no,unknown,unknown,unknown,unknown,";
			for (std::string angle : viz.HEAD_ANGLES) fStream << "nan,";
			for (std::string emotion : viz.EMOTIONS) fStream << "nan,";
			for (std::string expression : viz.EXPRESSIONS) fStream << "nan,";
			for (std::string emoji : viz.EMOJIS) fStream << "nan,";
			fStream << std::endl;
		}
		for (auto & face_id_pair : faces)
		{
			Face f = face_id_pair.second;

			fStream << timeStamp << ","
				<< f.id << ","
				<< f.measurements.interocularDistance << ","
				<< viz.GLASSES_MAP[f.appearance.glasses] << ","
				<< viz.AGE_MAP[f.appearance.age] << ","
				<< viz.ETHNICITY_MAP[f.appearance.ethnicity] << ","
				<< viz.GENDER_MAP[f.appearance.gender] << ","
				<< affdex::EmojiToString(f.emojis.dominantEmoji) << ",";
			std::cout << " Dominant Emotion: " << affdex::EmojiToString(f.emojis.dominantEmoji) << std::endl;
			float *values = (float *)&f.measurements.orientation;
			for (std::string angle : viz.HEAD_ANGLES)
			{
				fStream << (*values) << ",";
				values++;
			}

			values = (float *)&f.emotions;
			for (std::string emotion : viz.EMOTIONS)
			{
				fStream << (*values) << ",";
				values++;
			}

			values = (float *)&f.expressions;
			for (std::string expression : viz.EXPRESSIONS)
			{
				fStream << (*values) << ",";
				values++;
			}

			values = (float *)&f.emojis;
			for (std::string emoji : viz.EMOJIS)
			{
				fStream << (*values) << ",";
				values++;
			}

			fStream << std::endl;
		}
	}

	std::vector<cv::Point2f> CalculateBoundingBox(VecFeaturePoint points)
	{

		std::vector<cv::Point2f> ret;

		//Top Left
		ret.push_back(minPoint(points));

		//Bottom Right
		ret.push_back(maxPoint(points));

		//Top Right
		ret.push_back(cv::Point2f(ret[1].x,
			ret[0].y));
		//Bottom Left
		ret.push_back(cv::Point2f(ret[0].x,
			ret[1].y));

		return ret;
	}

	void draw(const std::map<FaceId, Face> faces, Frame image)
	{

		const int left_margin = 30;

		cv::Scalar clr = cv::Scalar(0, 0, 255);
		cv::Scalar header_clr = cv::Scalar(255, 0, 0);

		std::shared_ptr<unsigned char> imgdata = image.getBGRByteArray();
		cv::Mat img = cv::Mat(image.getHeight(), image.getWidth(), CV_8UC3, imgdata.get());
		viz.updateImage(img);

		for (auto & face_id_pair : faces)
		{
			Face f = face_id_pair.second;
			VecFeaturePoint points = f.featurePoints;
			std::vector<cv::Point2f> bounding_box = CalculateBoundingBox(points);

			// Draw Facial Landmarks Points
			//viz.drawPoints(points);

			// Draw bounding box
			//viz.drawBoundingBox(bounding_box[0], bounding_box[1], f.emotions.valence);

			// Draw a face on screen
			//viz.drawFaceMetrics(f, bounding_box);
		}

		viz.showImage();
		std::lock_guard<std::mutex> lg(mMutex);
	}

};