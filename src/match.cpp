#include "../include/sift.h"

int computeRow(Mat* target, int row){
	int accuRow = 0;
	for(int i = 0; i < row; i++)
		accuRow += target[i].rows;
	return accuRow;
}

int MaxCol(Mat* target, int target_num){
	int current_max = 0;
	for(int i = 0; i < target_num; i++)
		if(target[i].cols > current_max)
			current_max = target[i].cols;
	return current_max;
}

void match_multi(mySIFT* left, mySIFT& right, vector<char*> targetFile, Mat img_scene, int target_num, int target_pick)
{
    Mat* target = new Mat[target_num]; // = imread(targetFile1);//§Ú­n±m¦âªº
	vector< vector<Key_Point> > a;
	for(int i = 0; i < target_num; i++){
		target[i] = imread(targetFile[i]);
		vector<Key_Point>& tmp = left[i].keyPoints;
		a.push_back(tmp);
	}
	vector< Key_Point >& b  = right.keyPoints;
	
	Mat find = img_scene;
	int maxCol = MaxCol(target, target_num);
	Mat result = concatMultiImg(target, find, target_num, maxCol, target_pick);

	// vector< Point2f > obj1, obj2;
	// vector< Point2f > scene, scene2;
	vector< vector<Point2f> > obj;
	vector< vector<Point2f> > scene;
	for(int i = 0; i < target_num; i++){
		vector<Point2f> sub_obj;
		vector<Point2f> sub_scene;
		obj.push_back(sub_obj);
		scene.push_back(sub_scene);
		for(int j = 0; j < a[i].size(); j++){
			int index = -1;//index of minimum distance;
			double min = INT_MAX;
			int indexMin2 = -1;
			double min2 = INT_MAX;
		
			for (int k = 0; k < b.size(); k++){//¼É¤O¥h±½¨C¤@­Ó¥kÃäªºKey_Point:b[j]
				double dist = 0;
				for (int l = 0; l < 32; ++l)
					dist += (a[i][j].descriptor[l] - b[k].descriptor[l]) * (a[i][j].descriptor[l] - b[k].descriptor[l]);
				if (dist < min){//³Ì¤pªº­n³Q¨ú¥N¤F
					min2 = min;
					indexMin2 = index;
					min = dist;
					index = k;//¥ªÃäi match¨ì¥kÃäindex
				}
			}

			int B = rand() % 256;
			int G = rand() % 256;
			int R = rand() % 256;

			if (min < 0.5 * min2){//good matches
				int aScaleNum = a[i][j].layer / left[i].nLayersPerOctave;// == 0) ? 1 : 1.6;
				double aScaling = 1;
				for (int k = 0; k < aScaleNum; ++k){
					aScaling *= SCALE;
				}
				int bScaleNum = b[index].layer / right.nLayersPerOctave;// == 0) ? 1 : 1.6;//ÁY¤p´X­¿ªº¡A­n©ñ¤j¦^¨Ó
				double bScaling = 1;
				for (int k = 0; k < bScaleNum; ++k){
					bScaling *= SCALE;
				}
				// circle(result, Point(a1[i].col * aScaling, a1[i].row * aScaling), 3, Scalar(255, 0, 0), 1);
				// circle(result, Point(max(target1.cols, target2.cols) + b[index].col * bScaling, b[index].row * bScaling), 3, Scalar(0, 255, 0), 1);
				if(i == target_pick)
					line(result, Point(a[i][j].col * aScaling, computeRow(target, i) + a[i][j].row * aScaling), Point(maxCol + b[index].col * bScaling, b[index].row * bScaling), Scalar(B, G, R));
				obj.back().push_back(Point2f(a[i][j].col * aScaling, a[i][j].row * aScaling));
				scene.back().push_back(Point2f(b[index].col * bScaling, b[index].row * bScaling));
	        }
		}
	}


	Mat* H = new Mat[target_num];
	if(!scene.empty())
		for(int i = 0; i < target_num; i++)
			if(!obj[i].empty() && obj[i].size()>3 && scene[i].size()>3)
				H[i] = findHomography(obj[i], scene[i], CV_RANSAC);


	vector< vector<Point2f> > obj_corners;
	for(int i = 0; i < target_num; i++){
		vector< Point2f > obj_corners_sub(4);
		obj_corners_sub[0] = cvPoint(0, 0);
		obj_corners_sub[1] = cvPoint(left[i].blurredImgs[0].cols, 0);
		obj_corners_sub[2] = cvPoint(left[i].blurredImgs[0].cols, left[i].blurredImgs[0].rows);
		obj_corners_sub[3] = cvPoint(0, left[i].blurredImgs[0].rows);
		obj_corners.push_back(obj_corners_sub);
	}

    Point2f target_offset = cvPoint(maxCol, 0);

	vector< vector<Point2f> > computed_corners;
	for(int i = 0; i < target_num; i++){
		vector< Point2f > computed_corners_sub(4);
		if(!H[i].empty()){
			for (int j = 0; j < 4; j++)
				computed_corners_sub[j] = MatMulti(H[i], obj_corners[i][j]) + target_offset;//¦Û¤vºâ
			computed_corners.push_back(computed_corners_sub);
		}
		else{
			for (int j = 0; j < 4; j++)
				computed_corners_sub[j] = cvPoint(0,0);			
			computed_corners.push_back(computed_corners_sub);
		}
	}

    /////
    //int target_cols = max(target1.cols, target2.cols);
    for(int i = 0; i < target_num; i++){
    	// cout << computed_corners[i][0].x << " " << computed_corners[i][0].y << endl;
    	if(i == target_pick)
    		trackObject(computed_corners[i] , result, left[i], right, maxCol);
    	else{
		    line(result, computed_corners[i][0], computed_corners[i][1], Scalar((i*67)%256, (i*31)%256, (i*97)%256), 4);
		    line(result, computed_corners[i][1], computed_corners[i][2], Scalar((i*67)%256, (i*31)%256, (i*97)%256), 4);
		    line(result, computed_corners[i][2], computed_corners[i][3], Scalar((i*67)%256, (i*31)%256, (i*97)%256), 4);
		    line(result, computed_corners[i][3], computed_corners[i][0], Scalar((i*67)%256, (i*31)%256, (i*97)%256), 4);		
    	}
    }

    imshow("haha", result);
	// imwrite("result.jpg", result);
	waitKey(1);
}

int RowSum(Mat* target, int& target_num){
	int accuSum = 0;
	for(int i = 0; i < target_num; i++)
		accuSum += target[i].rows;
	return accuSum;
}

Mat concatMultiImg(Mat* target, Mat& scene, int& target_num, int& maxCol, int& target_pick)
{   
    //cout << i1.type();
    int rowSum = RowSum(target, target_num);
    Mat concat_out(max(rowSum, scene.rows), maxCol + scene.cols, target[0].type(), Scalar(0, 0, 0));

    for(int i = 0; i < target_num; i++)
    	for(int row = 0; row < target[i].rows; row++)
    		for(int col = 0; col < target[i].cols; col++)
    			concat_out.at<Vec3b>(row + computeRow(target, i), col) = target[i].at<Vec3b>(row, col);

    for(int row = 0; row < scene.rows; row++)
    	for(int col = 0; col < scene.cols; col++)
    		concat_out.at<Vec3b>(row, col + maxCol) = scene.at<Vec3b>(row, col);

    rectangle(concat_out, Point(0, computeRow(target, target_pick)), 
    	Point(target[target_pick].cols, computeRow(target, target_pick) + target[target_pick].rows), Scalar(0, 255, 255), 4);
    return concat_out;
}


void match(mySIFT& left, mySIFT& right, string targetFile, Mat img_scene, clock_t s)
{


	// cout << "keyPoints (AFTER): " << right.keyPoints.size() << endl << endl;
	vector< Key_Point >& a = left.keyPoints;
	vector< Key_Point >& b = right.keyPoints;
	
	Mat target = imread(targetFile);//§Ú­n±m¦âªº
	Mat find = img_scene;
	Mat result = concat2Img(target, find);

	vector< Point2f > obj;
	vector< Point2f > scene;

	for (int i = 0; i < a.size(); ++i){//¹ï¨C¤@­Ó¥ªÃäªºKey_Point:a[i]
		//Mat garha = target.clone();
		//Mat garho = find.clone();
		int index = -1;//index of minimum distance;
		double min = INT_MAX;
		int indexMin2 = -1;
		double min2 = INT_MAX;
		
		for (int j = 0; j < b.size(); ++j){//¼É¤O¥h±½¨C¤@­Ó¥kÃäªºKey_Point:b[j]
			double dist = 0;
			for (int k = 0; k < 32; ++k)
				dist += (a[i].descriptor[k] - b[j].descriptor[k]) * (a[i].descriptor[k] - b[j].descriptor[k]);
			if (dist < min){//³Ì¤pªº­n³Q¨ú¥N¤F
				min2 = min;
				indexMin2 = index;
				min = dist;
				index = j;//¥ªÃäi match¨ì¥kÃäindex
			}
		}
		//circle(garha, Point(a[i].col, a[i].row), 3, Scalar(0, 0, 255));
		//circle(garho, Point(b[index].col, b[index].row), 3, Scalar(0, 0, 255));
		//imshow("target", garha);
		//imshow("find", garho);
		//while (waitKey(0) != 'e')
		//	continue;

		int B = rand() % 256;
		int G = rand() % 256;
		int R = rand() % 256;

		if (min < 0.5 * min2){//good matches
			int aScaleNum = a[i].layer / left.nLayersPerOctave;// == 0) ? 1 : 1.6;
			double aScaling = 1;
			for (int k = 0; k < aScaleNum; ++k){
				aScaling *= SCALE;
			}
			int bScaleNum = b[index].layer / right.nLayersPerOctave;// == 0) ? 1 : 1.6;//ÁY¤p´X­¿ªº¡A­n©ñ¤j¦^¨Ó
			double bScaling = 1;
			for (int k = 0; k < bScaleNum; ++k){
				bScaling *= SCALE;
			}
			//cout << aScaling << " " << bScaling << "\n";
			circle(result, Point(a[i].col * aScaling, a[i].row * aScaling), 3, Scalar(255, 0, 0), 1);
			circle(result, Point(target.cols + b[index].col * bScaling, b[index].row * bScaling), 3, Scalar(0, 255, 0), 1);

			line(result, Point(a[i].col * aScaling, a[i].row * aScaling), Point(target.cols + b[index].col * bScaling, b[index].row * bScaling), Scalar(B, G, R));
			obj.push_back(Point2f(a[i].col * aScaling, a[i].row * aScaling));
			scene.push_back(Point2f(b[index].col * bScaling, b[index].row * bScaling));
		}
	}
	//µ²§ômatch

	if (obj.empty() || scene.empty())
		return;

	clock_t end = clock();
	//cout << "«e­±¥þ³¡ : " << (double)(end - s) / CLOCKS_PER_SEC << "\n";

	clock_t start = clock();
	
	Mat H;
	if (obj.size()>3 && scene.size()>3)
		H = findHomography(obj, scene, CV_RANSAC);
	else
		return;

	end = clock();
	//cout << "Find Homography Use " << (double)(end - start) / CLOCKS_PER_SEC << "\n\n\n";

	vector< Point2f > obj_corners(4);
	obj_corners[0] = cvPoint(0, 0);
	obj_corners[1] = cvPoint(left.blurredImgs[0].cols, 0);
	obj_corners[2] = cvPoint(left.blurredImgs[0].cols, left.blurredImgs[0].rows);
	obj_corners[3] = cvPoint(0, left.blurredImgs[0].rows);
	
	vector< Point2f > scene_corners(4);
	vector< Point2f > computed_corners(4);
	for (int i = 0; i < 4; ++i)
		computed_corners[i] = MatMulti(H, obj_corners[i]) + Point2f(left.blurredImgs[0].cols, 0);//¦Û¤vºâ

	trackObject(computed_corners, result, left, right, left.blurredImgs[0].cols);
    imshow("haha", result);
    // imwrite("result.jpg", result);
    waitKey(1);
}



Point2f MatMulti(Mat matrix, Point2f point)//¬Ý¬Ý¸òsource code¤@¤£¤@¼Ë
{
	double input[3] = { point.x, point.y, 1 };

	vector < double > output(3);

	for (int row = 0; row < 3; ++row){
		double sum = 0;
		for (int col = 0; col < 3; ++col){
			sum += matrix.at< double >(row, col) * input[col];
		}
		output[row] = sum;
	}
	//for (int i = 0; i < 3; ++i)
	//	cout << output[i] << "\n";
	
	if (output[2] == 0.0)
		return Point2f(0, 0);
	else
		return Point2f((int)(output[0] / output[2]), (int)(output[1] / output[2]));
}

Mat concat2Img(Mat& i1, Mat& i2)
{	
	//cout << i1.type();
	Mat i3(max(i1.rows, i2.rows), i1.cols + i2.cols, i1.type(), Scalar(0, 0, 0));

	for (int row = 0; row < i3.rows; ++row)
		for (int col = 0; col < i3.cols; ++col){
			if (col < i1.cols && row < i1.rows)//¥Î¥ªÃäªº½Æ»s
				i3.at<Vec3b>(row, col) = i1.at<Vec3b>(row, col);
			else if (col >= i1.cols)
				i3.at<Vec3b>(row, col) = i2.at<Vec3b>(row, col - i1.cols);
		}
	return i3;
}
