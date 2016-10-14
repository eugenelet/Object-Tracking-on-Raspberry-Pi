#include "../include/sift.h"

void mySIFT::computeDescriptor()
{
	for (int i = 0; i < keyPoints.size(); ++i){//¹ï¨C­Ókeypoint­nºâ¥X¤@­Ódescriptor(¥ý¸Õ¸Õ¬Ý32ºû)
		Key_Point& kpt = keyPoints[i];
		int kptRow = kpt.row;
		int kptCol = kpt.col;
		vector< double > temp;
		
		//¥ª¤W8x8
		temp = DescriptorHelper(kptRow - 7, kptCol - 7, kpt.layer);
		for (int j = 0; j < 8; ++j)
			kpt.descriptor.push_back(temp[j]);
		//¥k¤W8x8
		temp = DescriptorHelper(kptRow - 7, kptCol, kpt.layer);
		for (int j = 0; j < 8; ++j)
			kpt.descriptor.push_back(temp[j]);
		//¥ª¤U8x8
		temp = DescriptorHelper(kptRow, kptCol - 7, kpt.layer);
		for (int j = 0; j < 8; ++j)
			kpt.descriptor.push_back(temp[j]);
		//¥k¤U8x8
		temp = DescriptorHelper(kptRow, kptCol, kpt.layer);
		for (int j = 0; j < 8; ++j)
			kpt.descriptor.push_back(temp[j]);

	}
}

vector< double > mySIFT::DescriptorHelper(int Row, int Col, int layer)//¥ª¤W¨¤row, column
{
	Mat& image = blurredImgs[layer];
	vector< double > bins(8, 0);

	for (int r = Row; r <= Row + 7; ++r){
		for (int c = Col; c <= Col + 7; ++c){
			if (r >= 1 && c >= 1 && r < image.rows - 1 && c < image.cols - 1){
				int temp1 = (int)image.at<uchar>(r, c + 1) - (int)image.at<uchar>(r, c - 1);
				int temp2 = (int)image.at<uchar>(r + 1, c) - (int)image.at<uchar>(r - 1, c);//pixel­Èªº®t
				double magnitude = abs(temp1) + abs(temp2);//sqrt(temp1 * temp1 + temp2 * temp2);
				double theta = atan2(temp2, temp1) * 180 / PI;
				if (theta < 0)
					theta += 360;

				//int offset = ((int)theta % 45 < 23) ? 0 : 1;
				//bins[((int)theta / 45 + offset) % 8] +=  magnitude;
				bins[(int)theta / 45] += magnitude;// 9/6§ïªº

			}
		}
	}
	return bins;
}