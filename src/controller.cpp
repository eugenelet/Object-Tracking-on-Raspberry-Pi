#include "../include/sift.h"
/**************************
        SIFT TOOLS
***************************/
#define PI 3.14159265358979323846264338327
#define SIFT_ORI_HIST_BINS 36// default number of bins in histogram for orientation assignment
#define SIFT_ORI_SIG 1.5f// determines gaussian sigma for orientation assignment
#define SIFT_ORI_RADIUS 3 * SIFT_ORI_SIG// determines the radius of the region used in orientation assignment


using namespace cv;
using namespace std;

int sokt;

int localSocket;
int remoteSocket;
int serverPort;
int doneFlag = 1;
int counter = 0;
static char* snd_PORT="10023";
static char* rcv_PORT="10024";
//static char* IP_ADDR="127.0.0.1";
static char* IP_ADDR="192.168.0.100";

/////////////////////////////////////////////////////////////////
int distance_obj = 0;
/////////////////////////////////////////////////////////////////


bool sigIntFlag = false;

void sigint(int a){
	cout << "SIG INT Captured!" << endl;
	cout << "Closing Socket Connections" << endl;
	unsigned char output[DATAGRAM_SIZE];
	output[0] = CONTROL;
	output[1] = SIG_INT;
	transmit(output);
	close_receiver();
	close_transmit();
	sigIntFlag = true;
}

pthread_mutex_t target_mutex;
pthread_mutex_t mtxSave;
pthread_mutex_t mtxRecv;
typedef struct target_choice_info{
    int* target_num;
    int* target_pick;
    Mat* img_scene;
};
unsigned char* receivedPacket = NULL;
bool receivedACKPacket = false;

void* ultra_thread(void* arg){

    while(1){
        pthread_mutex_lock(&mtxRecv);
        receivedPacket = receiver();
        pthread_mutex_unlock(&mtxRecv);
        if(receivedPacket!=NULL)
            if(receivedPacket[0] == ULTRA){
                distance_obj = (receivedPacket[4] << 24) | (receivedPacket[3] << 16) | (receivedPacket[2] << 8) | receivedPacket[1];
                cout << distance_obj << endl;
            }
            else if(receivedPacket[0] == ACK){
                receivedACKPacket = true;
            }
    }
}

void* getch_thread(void* arg){
    unsigned char output[DATAGRAM_SIZE];
	// unsigned char* receivedPacket;
    int bytes;
	bool wrongKey = false;
    int keyIn;
    target_choice_info *target_choice = (target_choice_info*) arg;
    while(1){
		tcflush(STDIN_FILENO, TCIFLUSH);	
		keyIn = getch();
        switch(keyIn){
            case 'a':{
                cout <<"left"<<endl;
                output[0] = CONTROL;
                output[1] = LEFT;
                transmit(output);
                break;
            }
            case 's':{
                cout <<"back"<<endl;
                output[0] = CONTROL;
                output[1] = BACK;
                transmit(output);
                break;
            }
            case 'd':{
                cout <<"right"<<endl;
                output[0] = CONTROL;
                output[1] = RIGHT;
                transmit(output);
                break;
            }
            case 'w':{
                if(distance_obj>15){
                    cout <<"forward"<<endl;
                    output[0] = CONTROL;
                    output[1] = FORWARD;
                    transmit(output);
                }
                else{
                    wrongKey = true;
                    cout << "Object too near!" << endl;
                }
                break;
            }
            case 'c':{
				pthread_exit(0);
                break;
            }
			case 'h':{
				cout <<"Shut Down Pi..."<<endl;
				output[0] = CONTROL;
				output[1] = SHUTDOWN;
				transmit(output);
				break;
			}
            case 'n':{
                wrongKey = true;
                pthread_mutex_lock(&target_mutex);
                if(*(target_choice->target_pick) < *(target_choice->target_num) - 1)
                    *(target_choice->target_pick) = *(target_choice->target_pick) + 1;
                else
                    *(target_choice->target_pick) = 0;
                cout << "Next Target. " << "(" << *(target_choice->target_pick) << ")"<< endl;
                pthread_mutex_unlock(&target_mutex);
                break;
            }
            case 'p':{
                char* saveFile = "snapshot.jpg";
                wrongKey = true;
                pthread_mutex_lock(&mtxSave);
                imwrite(saveFile, *(target_choice->img_scene) );
                pthread_mutex_unlock(&mtxSave);
                cout << "Picture saved as " << saveFile << endl;
                imshow("Snapshot", imread(saveFile));
                waitKey(30);
                break;
            }
            default:{
				cout <<"\n\n\n\n" ;
				cout << "================================" << endl;
                cout <<"W,A,S,D for Control." << endl <<
                    "P to take snapshot."<<endl <<
                    "N to pick next target. (Multi Target)"<<endl <<
					"C to terminate program."<<endl <<
					"H to Shutdown Pi." << endl;
				cout << "================================" << endl;
				cout <<"\n\n\n\n" ;
                wrongKey = true;
                break;
            }
        
        }
		if(wrongKey){
			wrongKey = false;
		}
		else{
            // pthread_mutex_lock(&mtxRecv);
			// receivedPacket = receiver();
            // pthread_mutex_unlock(&mtxRecv);
            if(receivedACKPacket){
                receivedACKPacket = false;
    			cout << "ACK!" << endl;
            }
		}
    }
    pthread_exit(NULL);
}

std::mutex mtxCam;

void task(VideoCapture* cap, Mat* frame){
	while(1){
		//cap->grab();
		mtxCam.lock();
		*cap >> *frame;
		mtxCam.unlock();
		imshow("Video Thread", *frame);
		waitKey(1);
	}
}

int listdir(char *dir, vector<char*>& content, int* target_num) {
    *target_num = 0;
    struct dirent *dp;
    DIR *fd;

    if ((fd = opendir(dir)) == NULL) {
        fprintf(stderr, "listdir: can't open %s\n", dir);
        return 0;
    }
    while ((dp = readdir(fd)) != NULL) {
        if (!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, ".."))
            continue;    /* skip self and parent */
        char * tmp = (char *) malloc(1 + strlen(dir) + 1 +strlen(dp->d_name));
        strcpy(tmp, dir);
        strcat(tmp, "/");            
        strcat(tmp, dp->d_name);            
        content.push_back(tmp);
        printf("%d %s/%s\n",*target_num, dir, dp->d_name);
        (*target_num)++;
    }
    cout << "Total Files: " << *target_num << endl;
    closedir(fd);
}


int main(int argc, char* argv[])
{
    srand(time(NULL));
    bool time_on = false;
    bool multi_on = false;
    if (argc > 1)
        for(int i = 1; i < argc; i++){
            if(strcmp(argv[i], "time") == 0)
                time_on = true; 
            if(strcmp(argv[i], "multi") == 0)
                multi_on = true;    
            if(strcmp(argv[i], "-h") == 0){
                cout << "time: Measure time." << endl;
                cout << "multi: Multi target." << endl;
                return 0;
            }
        }

    Mat img_scene;

    vector<char*> dirContent;
    
    int* target_num = new int;
    int* target_pick = new int;
    if(multi_on)
        listdir("target", dirContent, target_num);
    else
        *target_num = 1;
    *target_pick = 0;

    target_choice_info *targetChoiceInfo = new target_choice_info;
    targetChoiceInfo->target_num = target_num;
    targetChoiceInfo->target_pick = target_pick;
    targetChoiceInfo->img_scene = &img_scene;
    pthread_t  getch_t;
    pthread_t  ultra_t;
    transmit_init(IP_ADDR, snd_PORT);  
    receiver_init(rcv_PORT); 
    pthread_create(&getch_t, NULL, getch_thread, targetChoiceInfo);
    pthread_create(&ultra_t, NULL, ultra_thread, targetChoiceInfo);

    signal(SIGINT, sigint);

    //----------------------------------------------------------
    //OpenCV Code
    //----------------------------------------------------------
    cv::VideoCapture vcap;
    vcap.set(CV_CAP_PROP_BUFFERSIZE, 3);
    cv::Mat frame;

//    const std::string videoStreamAddress = "http://192.168.0.100:8090/?action=stream"; 
    const std::string videoStreamAddress = "http://192.168.0.100:8080/stream/video.mjpeg"; 
    /* it may be an address of an mjpeg stream, 
    e.g. "http://user:pass@cam_address:8081/cgi/mjpg/mjpg.cgi?.mjpg" */

    //open the video stream and make sure it's opened
    if(!vcap.open(videoStreamAddress)) {
        std::cout << "Error opening video stream or file" << std::endl;
        return -1; 
    }  

    vcap>>frame;
    //vcap.grab();
    
    thread t(task, &vcap, &frame);

    ///////////////////////////////////////////////////////////////////////////////////////////
    
    ///////////////////////////////////////////////////////////////////////////////////////////

    clock_t start, end;
    // char** targetFile = new char*[*target_num];//"target.jpg";
    char* targetFile;
    mySIFT* sift_target = new mySIFT[*target_num];
    char* str1 = "target/target";
    char* str2 = ".jpg";
    Mat* target = new Mat[*target_num];
    char buffer[10];
    if(multi_on){
        for(int i = 0; i < *target_num; i++){
            sift_target[i] = mySIFT(1.414, 1.414, 3);//sigma k
            // sprintf(buffer, "%d", i);
            // char * tmp = (char *) malloc(1 + strlen(str1) + strlen(buffer) + strlen(str2) );
            // strcpy(tmp, str1);
            // strcat(tmp, buffer);
            // strcat(tmp, str2);
            // targetFile[i] = tmp;
            target[i] = imread(dirContent[i], CV_LOAD_IMAGE_GRAYSCALE);
            computeSift(sift_target[i], target[i], imread(dirContent[i]), time_on);
        }
    }
    else{
        sift_target[0] = mySIFT(1.414, 1.414, 3);//sigma k
        targetFile = "target.jpg";
        target[0] = imread(targetFile, CV_LOAD_IMAGE_GRAYSCALE);
        computeSift(sift_target[0], target[0], imread(targetFile), time_on);
    }

    while (1){

        if(sigIntFlag){
            pthread_cancel(getch_t);
            return 0;
        }

        if(!frame.empty()){
            mtxCam.lock();
            pthread_mutex_lock(&mtxSave);
            frame.copyTo(img_scene);
            pthread_mutex_unlock(&mtxSave);
            mtxCam.unlock();
        }

        clock_t s;
        s = clock();
        // Mat img_scene;
        // cap >> img_scene;
        clock_t start, end;

        mySIFT hoho(1.414, 1.414, 1);//sigma k
        //hoho.LoadImage(imageName2);
        Mat imgScene;
        cvtColor(img_scene, imgScene, CV_BGR2GRAY);
        computeSift(hoho, imgScene, img_scene, time_on);
        //start = clock();
        if(multi_on){
            pthread_mutex_lock(&target_mutex);
            match_multi(sift_target, hoho, dirContent, img_scene, *target_num, *target_pick);
            pthread_mutex_unlock(&target_mutex);
        }
        else
            match(sift_target[0], hoho, targetFile, img_scene, s);

        //end = clock();
        //cout << "Match : " << (double)(end - s) / CLOCKS_PER_SEC << "\n";

    }

    //system("pause");
    return 0;
}

void mySIFT::LoadImage(Mat imgOri)
{
    blurredImgs.push_back(imgOri);//­ì¹Ï¬O©ñindex0
}


	
