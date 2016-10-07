#include "../include/sift.h"

/*
void computeSift_left(mySIFT &sift_obj, Mat img_scene, Mat img_color, bool time_on){
	if(time_on){
		clock_t start, end;
		sift_obj.LoadImage(img_scene);

		start = clock();
		sift_obj.createDoG();
		end = clock();
		cout << "Blurred & Create DoGs : " << (double)(end - start) / CLOCKS_PER_SEC << "\n";
		
		start = clock();
		sift_obj.detectKeypoints();
		end = clock();
		cout << "Detect Keypoints : " << (double)(end - start) / CLOCKS_PER_SEC << "\n";
		
		start = clock();
		sift_obj.filterKeyPoints();
		end = clock();
		cout << "Filter Keypoints(FAST) : " << (double)(end - start) / CLOCKS_PER_SEC << "\n";
		
		start = clock();
		sift_obj.computeDescriptor();
		end = clock();
		cout << "Compute Descriptors : " << (double)(end - start) / CLOCKS_PER_SEC << "\n";

		cout << endl << endl;
		
		//start = clock();
		//match(haha, hoho, targetFile, img_scene, s);
		//end = clock();
		//cout << "Match : " << (double)(end - s) / CLOCKS_PER_SEC << "\n";
	}
	else{
		sift_obj.LoadImage(img_scene);
		sift_obj.createDoG();
		sift_obj.detectKeypoints();
		sift_obj.filterKeyPoints();
		// sift_obj.filterKeyPoints_Hessian_left(img_scene, img_color);
		sift_obj.computeDescriptor();
	}
}*/

void computeSift(mySIFT &sift_obj, Mat img_scene, Mat img_color, bool time_on){
	if(time_on){
		clock_t start, end;
		sift_obj.LoadImage(img_scene);

		start = clock();
		sift_obj.createDoG();
		end = clock();
		cout << "Blurred & Create DoGs : " << (double)(end - start) / CLOCKS_PER_SEC << "\n";
		
		start = clock();
		sift_obj.detectKeypoints();
		end = clock();
		cout << "Detect Keypoints : " << (double)(end - start) / CLOCKS_PER_SEC << "\n";
		
		start = clock();
		sift_obj.filterKeyPoints();
		end = clock();
		cout << "Filter Keypoints(FAST) : " << (double)(end - start) / CLOCKS_PER_SEC << "\n";
		
		start = clock();
		sift_obj.computeDescriptor();
		end = clock();
		cout << "Compute Descriptors : " << (double)(end - start) / CLOCKS_PER_SEC << "\n";

		cout << endl << endl;
		
		//start = clock();
		//match(haha, hoho, targetFile, img_scene, s);
		//end = clock();
		//cout << "Match : " << (double)(end - s) / CLOCKS_PER_SEC << "\n";
	}
	else{
		sift_obj.LoadImage(img_scene);
		sift_obj.createDoG();
		sift_obj.detectKeypoints();
		sift_obj.filterKeyPoints();
		// sift_obj.filterKeyPoints_Hessian(img_scene, img_color);
		sift_obj.computeDescriptor();
	}
}