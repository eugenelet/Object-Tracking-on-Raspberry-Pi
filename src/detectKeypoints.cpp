#include "../include/sift.h"

void mySIFT::detectKeypoints()
{
	int size = blurredImgs.size() / nOctave - 1;//¨C¤@­Óoctave¡ADoGªºsize¡A²{¦b¬O4
	double sigmaTemp = sigma;

	for (int octave = 0; octave < nOctave; ++octave){	
		for (int layer = octave * nLayersPerOctave + 1; layer < octave * nLayersPerOctave + size - 1; ++layer){//DoGªºlayer
			//cout << octave << " " << layer << "\n";
			for (int row = 1; row < DoGs[layer].rows - 1; ++row){//octave * nLayersPerOctave
				for (int col = 1; col < DoGs[layer].cols - 1; ++col){//¬ÝDoGs[layer]¨º±iªº[row][col]¨ºÂI¬O¤£¬Omax min
					int value = DoGs[layer].at<int>(row, col);
					//cout << "value : " << value << " at " << row << ", " << col << "\n";
					bool isMax = true;
					bool isMin = true;
					for (int layerOffset = -1; layerOffset <= 1 && (isMin || isMax); ++layerOffset){
						for (int i = row - (DOG_DETECT_KPT_SIZE - 1) / 2; i <= row + (DOG_DETECT_KPT_SIZE - 1) / 2 && (isMin || isMax); ++i){
							for (int j = col - (DOG_DETECT_KPT_SIZE - 1) / 2; j <= col + (DOG_DETECT_KPT_SIZE - 1) / 2 && (isMin || isMax); ++j){
								if (i >= 0 && j >= 0 && i < DoGs[octave * nLayersPerOctave].rows && j < DoGs[octave * nLayersPerOctave].cols && i != row && j != col){//¦b½d³ò¤ºªº¸Ü
									int tester = DoGs[layer + layerOffset].at<int>(i, j);
									if (tester + 0 >= value)
										isMax = false;
									if (tester - 0 <= value)
										isMin = false;
								}
							}
						}
					}
					if (isMax)
						keyPoints.push_back(Key_Point(sigmaTemp, row, col, 1, layer));
					if (isMin)
						keyPoints.push_back(Key_Point(sigmaTemp, row, col, 0, layer));
				}
			}
			sigmaTemp *= k;
		}
	}
	//computeOrientationHist(keyPoints);
	// cout << keyPoints.size() << "\n";

}

void mySIFT::computeOrientationHist(vector< Key_Point >& keyPoints)// Computes a gradient orientation histogram at a specified pixel
{

	for (int i = 0; i < keyPoints.size(); ++i){//¹ï¨C­Ókeypointºâ¤è¦V
		vector< double > bins(SIFT_ORI_HIST_BINS, 0);
		Key_Point& thiskpt = keyPoints[i];
		Mat& image = blurredImgs[thiskpt.layer];
		int kptrow = thiskpt.row;
		int kptcol = thiskpt.col;
		//int temp1 = (int)image.at<uchar>(kptrow, kptcol + 1) - (int)image.at<uchar>(kptrow, kptcol - 1);
		//int temp2 = (int)image.at<uchar>(kptrow + 1, kptcol) - (int)image.at<uchar>(kptrow - 1, kptcol);
		//double magnitude = sqrt(temp1 * temp1 + temp2 * temp2);
		//double theta = atan2(temp2, temp1) * 180 / PI;
		//cout << setprecision(2) << fixed << setw(5) << magnitude << " " << theta << "\n";
		//system("pause");
		//double weightSum = 0;
		//int row = kptrow - SIFT_ORI_RADIUS;
		for (int row = kptrow - SIFT_ORI_RADIUS; row <= kptrow + SIFT_ORI_RADIUS; ++row)
			for (int col = kptcol - SIFT_ORI_RADIUS; col <= kptcol + SIFT_ORI_RADIUS; ++col){
				if (row >= 1 && row < image.rows - 1 && col >= 1 && col < image.cols - 1 && (row - kptrow) * (row - kptrow) + (col - kptcol) * (col - kptcol) < SIFT_ORI_RADIUS * SIFT_ORI_RADIUS){
					//¦b½d³ò¤º
					int temp1 = (int)image.at<uchar>(row, col + 1) - (int)image.at<uchar>(row, col - 1);
					int temp2 = (int)image.at<uchar>(row + 1, col) - (int)image.at<uchar>(row - 1, col);//pixel­Èªº®t
					double magnitude = sqrt(temp1 * temp1 + temp2 * temp2);
					double theta = atan2(temp2, temp1) * 180 / PI;
					if (theta < 0)
						theta += 360;
					double weight = exp(-0.5 * ((row - kptrow) * (row - kptrow) + (col - kptcol) * (col - kptcol)) / (SIFT_ORI_SIG * SIFT_ORI_SIG));
					int offset = ((int)theta % 10 < 5)? 0 : 1;
					bins[((int)theta / 10 + offset) % 36] += magnitude * weight;
					//weightSum += weight;
				}
			}
		int index = 0;
		for (int i = 1; i < SIFT_ORI_HIST_BINS; ++i)
			if (bins[i] > bins[index])
				index = i;

		thiskpt.orientation = index * 10;
	}
}