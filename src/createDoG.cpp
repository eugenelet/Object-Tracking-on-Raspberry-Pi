#include "../include/sift.h"

void mySIFT::createDoG()
{
	double tempSigma = sigma;

	for (int j = 0; j < nOctave; ++j){
		for (int i = 0; i < nLayersPerOctave - 1; ++i){//¥]§t­ì¹Ïªº¼Ò½k¹Ï¡A¤@­ÓOctave5±i¹Ï¡A±À4¦¸¡A¦]¬°²Ä¤@±i¬O­ì¹Ï
			blurredImgs.push_back(GaussianBlur(blurredImgs[j * nLayersPerOctave], tempSigma));
			tempSigma *= k;
			Mat DoG(blurredImgs[j * nLayersPerOctave + i + 1].rows, blurredImgs[j * nLayersPerOctave + i + 1].cols, CV_32SC1, Scalar(0));
			for (int row = 0; row < blurredImgs[j * nLayersPerOctave + i].rows; ++row)
				for (int col = 0; col < blurredImgs[j * nLayersPerOctave + i].cols; ++col){
					int diff = (int)blurredImgs[j * nLayersPerOctave + i + 1].at<uchar>(row, col) - (int)blurredImgs[j * nLayersPerOctave + i].at<uchar>(row, col);
					DoG.at<int>(row, col) = diff;
					//cout << diff << " " << DoG.at<int>(row, col) << "\n";
				}
			DoGs.push_back(DoG);
		}
		//°µ§¹¤@­Óoctave
		if (j != nOctave - 1){
			tempSigma = tempSigma / k / k / k / k;
			Mat src = *(blurredImgs.end() - 4);//blurredImgs[blurredImgs.size() - 3];
			Mat firstMatInNewOctave;
			resize(src, firstMatInNewOctave, Size(src.cols / SCALE, src.rows / SCALE), 0, 0, INTER_NEAREST);
			//imshow("hey", firstMatInNewOctave);
			//waitKey(0);
			blurredImgs.push_back(firstMatInNewOctave);//±À¶i¤U¤@¼hoctaveªº²Ä¤@±i¹Ï
			Mat empty;
			DoGs.push_back(empty);
		}
	}
}

double2D mySIFT::getGaussianKernel(double sigma)
{
	int kSize = cvRound(sigma * 1.5 + 1) | 1;//kSize¬°©_¼Æ¡A¶Ã©wªº
	//cout << sigma << " " << kSize << " ";
	vector< double > kernel_1D;
	
	int shift = (kSize - 1) / 2;
	double sum = 0;
	
	for (int i = 0; i < kSize; ++i){
		int x = i - shift;
		double temp = exp(-0.5*(x / sigma)*(x / sigma));
		sum += temp;
		kernel_1D.push_back( temp );
	}
	
	for (int i = 0; i < kSize; ++i)
		kernel_1D[i] = kernel_1D[i] / sum;

	double2D kernel_2D;
	for (int i = 0; i < kSize; ++i){
		vector < double > a;
		kernel_2D.push_back(a);
		for (int j = 0; j < kSize; ++j)
			kernel_2D[i].push_back( kernel_1D[j] * kernel_1D[i]);
	}
	//for (int i = 0; i < kSize; ++i){
	//	for (int j = 0; j < kSize; ++j)
	//		cout << setw(6) << setprecision(2) << kernel_2D[i][j];
	//	cout << "\n";
	//}
	return kernel_2D;
}

Mat mySIFT::GaussianBlur(const Mat& src, double sigma)//input (¹Ï¤ù, sigma), output (¼Ò½k¹Ï¤ù)
{
	double2D G_Kernel = getGaussianKernel(sigma);
	int kSize = G_Kernel.size();
	//kernel¸n¤W¥h¡A¶W¥X¹Ï¤ùªºpixel¸É0
	int shift = (kSize - 1) / 2;

	Mat output = src.clone();

	for (int row = 0; row < src.rows; ++row)
		for (int col = 0; col < src.cols; ++col){//­n¹ïimg[row][col]ªºpixel°µ¼Ò½k
			double sum = 0.0;
			double weightSum = 0.0;//0.0
			for (int i = row - shift; i <= row + shift; ++i)
				for (int j = col - shift; j <= col + shift; ++j){
					if (i >= 0 && j >= 0 && i < src.rows && j < src.cols){
						sum += src.at<uchar>(i, j) * G_Kernel[i - (row - shift)][j - (col - shift)];
						weightSum += G_Kernel[i - (row - shift)][j - (col - shift)];
					}
				}
			output.at<uchar>(row, col) = sum / weightSum;
			//cout << "sum : " << sum << "   " << (int)output.at<uchar>(row, col) << "\n";
		}
	//cout << "Finish Blurring with sigma : " << setprecision(2) << sigma << "\n";
	return output;
}
