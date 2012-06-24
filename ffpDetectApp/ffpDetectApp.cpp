// ffpDetectApp.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

namespace po = boost::program_options;
using namespace std;

template<class T>
ostream& operator<<(ostream& os, const vector<T>& v)
{
    copy(v.begin(), v.end(), ostream_iterator<T>(cout, " ")); 
    return os;
}

//int _tmain(int argc, _TCHAR* argv[])	// VS10
int main(int argc, char *argv[])		// Peter
{
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF ); // dump leaks at return
	//_CrtSetBreakAlloc(17534);
	
	int verbose_level_text;
	int verbose_level_images;
    bool useFileList = false;
	bool useImgs = false;
	std::string fn_fileList;
	std::vector<std::string> filenames; // Create vector to hold the filenames
	std::vector<Rect> rects;			// Create vector to hold the groundtruth (if there is any)
	
	try {
        po::options_description desc("Allowed options");
        desc.add_options()
            ("help,h", "produce help message")
            ("verbose-text,v", po::value<int>(&verbose_level_text)->implicit_value(2)->default_value(1,"minimal text output"),
                  "enable text-verbosity (optionally specify level)")
            ("verbose-images,w", po::value<int>(&verbose_level_images)->implicit_value(2)->default_value(1,"minimal image output"),
                  "enable image-verbosity (optionally specify level)")
            ("file-list,f", po::value< string >(), 
                  "a .lst file to process")
            ("input-file,i", po::value< vector<string> >(), "input image")
        ;

        po::positional_options_description p;
        p.add("input-file", -1);
        
        po::variables_map vm;
        po::store(po::command_line_parser(argc, argv).
                  options(desc).positional(p).run(), vm);
        po::notify(vm);
    
        if (vm.count("help")) {
            cout << "[ffpDetectApp] Usage: options_description [options]\n";
            cout << desc;
            return 0;
        }
        if (vm.count("file-list"))
        {
            cout << "[ffpDetectApp] Using file-list as input: " << vm["file-list"].as< string >() << "\n";
			useFileList = true;
			fn_fileList = vm["file-list"].as<string>();
        }
        if (vm.count("input-file"))
        {
            cout << "[ffpDetectApp] Using input images: " << vm["input-file"].as<vector<string>>() << "\n";
			useImgs = true;
			filenames = vm["input-file"].as<vector<string>>();
        }
        if (vm.count("verbose-text")) {
            cout << "[ffpDetectApp] Verbose level for text: " << vm["verbose-text"].as<int>() << "\n";
        }
        if (vm.count("verbose-images")) {
            cout << "[ffpDetectApp] Verbose level for images: " << vm["verbose-images"].as<int>() << "\n";
        }
    }
    catch(std::exception& e) {
        cout << e.what() << "\n";
        return 1;
    }
	if(useFileList==true && useImgs==true) {
		cout << "[ffpDetectApp] Error: Please either specify a file-list OR an input-file, not both!" << endl;
		return 1;
	} else if(useFileList==false && useImgs==false) {
		cout << "[ffpDetectApp] Error: Please either specify a file-list or an input-file to run the program!" << endl;
		return 1;
	}

	char* fn_detFrontal = "D:\\CloudStation\\libFD_patrik2011\\config\\fdetection\\fd_config_ffd_fd.mat";
	char* fn_regrSVR = "D:\\CloudStation\\libFD_patrik2011\\config\\fdetection\\fd_config_ffd_ra.mat";
	char* fn_regrWVR = "D:\\CloudStation\\libFD_patrik2011\\config\\fdetection\\fd_config_ffd_la.mat";
	
	FdImage *myimg;// = new FdImage();
	//myimg->load("D:\\CloudStation\\libFD_patrik2011\\data\\firstrun\\ws_220.tiff");
	//myimg->load("D:\\CloudStation\\libFD_patrik2011\\horse.jpg");
	//myimg->load("D:\\CloudStation\\libFD_patrik2011\\ws_220.jpg");
	//myimg->load("D:\\CloudStation\\libFD_patrik2011\\ws_115.jpg");
	
	//myimg->load("D:\\CloudStation\\libFD_patrik2011\\ws_220.tif");
//	myimg->load("D:\\CloudStation\\libFD_patrik2011\\ws_71.tif");
	//myimg->load("D:\\CloudStation\\libFD_patrik2011\\color_feret_00194_940128_hr.png");

/*	CascadeERT *ert = new CascadeERT();
	//ert->wvm->load(fn_detFrontal);
	//ert->svm->load(fn_detFrontal);
	ert->wvr->load(fn_regrWVR);
	//ert->svr->load(fn_regrSVR);
	ert->wvr->init_for_image(myimg);
	ert->wvr->extract(myimg);
	std::vector<FdPatch*> candidates = ert->wvr->detect_on_image(myimg);
	
	char* fn = new char[500];
	cv::Mat tmp[9];
	for(int i=0; i<=8; i++) {
		tmp[i] = myimg->data_matbgr.clone();
		Logger->drawCenterpointsWithYawAngleColor(tmp[i], candidates, i);
		sprintf(fn, "yaw_%d.png", i);
		cv::imwrite(fn, tmp[i]);
	}*/

	const float DETECT_MAX_DIST_X = 0.33f;
	const float DETECT_MAX_DIST_Y = 0.33f;
	const float DETECT_MAX_DIFF_W = 0.33f;

	//std::string fn_fileList = "D:\\CloudStation\\libFD_patrik2011\\data\\firstrun\\theRealWorld_png.lst";
	//std::string fn_fileList = "H:\\featuredetection\\data\\lfw\\lfw-b_NAS.lst";
	//std::string fn_fileList = "H:\\featuredetection\\data\\feret_m2\\feret-frontal_m2_NAS.lst";
	
	if(useFileList) {
		std::ifstream fileList;
		fileList.open(fn_fileList, std::ios::in);
		if (!fileList.is_open()) {
			std::cout << "[ffpDetectApp] Error opening file list!" << std::endl;
			return 0;
		}
		std::string line;
		while( fileList.good() ) {
			std::getline(fileList, line);
			if(line=="") {
				continue;
			}
      		std::string buf; // Have a buffer string
			int l=0, r=0, b=0, t=0;
			std::stringstream ss(line); // Insert the string into a stream
			//while (ss >> buf)
			ss >> buf;	
			filenames.push_back(buf);
			ss >> l;
			ss >> t;
			ss >> r;
			ss >> b;
			//if(!(l==0 && t==0 && r==0 && b==0))
			rects.push_back(Rect(l, t, r, b));	// GT available or 0 0 0 0
			buf.clear();
			l=t=r=b=0;
		}
		fileList.close();
	}
	// Else useImgs==true: filesnames are already in "filenames", and no groundtruth available!

	// All filesnames now in "filenames", either way
	// All groundtruth now in "rects" IF available (read from .lst)

	int TOT = 0;
	int TACC = 0;
	int FACC = 0;
	int NOCAND = 0;
	int DONTKNOW = 0;


	CascadeWvmOeSvmOe* casc = new CascadeWvmOeSvmOe(fn_detFrontal);
	//std::vector<FdPatch*> result;
	for(unsigned int i=0; i< filenames.size(); i++) {
		myimg = new FdImage();
		myimg->load(filenames[i]);
		casc->init_for_image(myimg);
		casc->detect_on_image(myimg);
		TOT++;
		if(casc->candidates.size()<1) {
			std::cout << "[ffpDetectApp] No face-candidates at all found:  " << filenames[i] << std::endl;
			NOCAND++;
		} else {
			if(rects[i]==Rect(0, 0, 0, 0)) {//no groundtruth
				std::cout << "[ffpDetectApp] No ground-truth available, not counting anything: " << filenames[i] << std::endl;
				++DONTKNOW;
			} else {//we have groundtruth
				int gt_w = abs(rects[i].right-rects[i].left);
				int gt_h = abs(rects[i].top-rects[i].bottom);
				int gt_cx = rects[i].left+gt_w/2;
				int gt_cy = rects[i].top+gt_h/2;
				if (abs(gt_cx - casc->candidates[0]->c.x) < DETECT_MAX_DIST_X*(float)gt_w &&
					abs(gt_cy - casc->candidates[0]->c.y) < DETECT_MAX_DIST_Y*(float)gt_w &&
					abs(gt_w - casc->candidates[0]->w_inFullImg) < DETECT_MAX_DIFF_W*(float)gt_w       ) {
				
					std::cout << "[ffpDetectApp] TACC (1/1): " << filenames[i] << std::endl;
					TACC++;
				} else {
					std::cout << "[ffpDetectApp] Face not found, wrong position:  " << filenames[i] << std::endl;
					FACC++;
				}
			}//end no groundtruth
		}

		std::cout << std::endl;
		std::cout << "[ffpDetectApp] -------------------------------------" << std::endl;
		std::cout << "[ffpDetectApp] TOT:  " << TOT << std::endl;
		std::cout << "[ffpDetectApp] TACC:  " << TACC << std::endl;
		std::cout << "[ffpDetectApp] FACC:  " << FACC << std::endl;
		std::cout << "[ffpDetectApp] NOCAND:  " << NOCAND << std::endl;
		std::cout << "[ffpDetectApp] DONTKNOW:  " << DONTKNOW << std::endl;
		std::cout << "[ffpDetectApp] -------------------------------------" << std::endl;

		delete myimg;
	}

	std::cout << std::endl;
	std::cout << "[ffpDetectApp] =====================================" << std::endl;
	std::cout << "[ffpDetectApp] =====================================" << std::endl;
	std::cout << "[ffpDetectApp] TOT:  " << TOT << std::endl;
	std::cout << "[ffpDetectApp] TACC:  " << TACC << std::endl;
	std::cout << "[ffpDetectApp] FACC:  " << FACC << std::endl;
	std::cout << "[ffpDetectApp] NOCAND:  " << NOCAND << std::endl;
	std::cout << "[ffpDetectApp] DONTKNOW:  " << DONTKNOW << std::endl;
	std::cout << "[ffpDetectApp] =====================================" << std::endl;
	
//	Logger->drawBoxes(myimg->data_matbgr, casc->candidates);
//	cv::imwrite("fdsa.png", myimg->data_matbgr);

	//delete fn;
	
	//delete ert;
	delete casc;
	//delete myimg;

	return 0;
}


	// TODO important:
	
// implement OE and then test on lfw-big 1k

// Copy and = c'tors
	// pub/private
	// ALL in RegressorWVR.h/cpp is the same as in DetWVM! Except the classify loop AND threshold loading. -> own class (?)
	// Logger.drawscales
	// Logger draw 1 scale only, and points with color instead of boxes
	// logger filter lvls etc
	// CascadeWvmOeSvmOe should be a VDetVec... and returnFilterSize should return wvm->filtersizex... etc
	// problem when 2 diff. featuredet run on same scale
	// results dir from config etc
	// Diff. patch sizes: Cascade is a VDetectorVM, and calculates ONE subsampfac for the master-detector in his size. Then, for second det with diff. patchsize, calc remaining pyramids.
	// Test limit_reliability (SVM)	
//  WVM/R: bisschen viele *thresh*...?
// wie verhaelt sich alles bei GRAY input image?? (imread, Logger)

// @MR: Warum "-b" ? ComparisonRegr.xlsx 6grad systemat. fehler da ML +3.3, MR -3.3


/*	Possible workflows:
		- standard: det->load(), det->initForImg(), det->extract(), det->detect_on_img()...
		- standard: det->load(), det->initForImg(), det->detect_on_patchvec()...
		- det->load(), det->detect_on_patchvec()... TODO: TEST THIS!
		- det->load(), det->classify but I have to supply a valid patch! (my responsibility) TEST THIS


*/



/* 
/	Todo:
	* I think the whole det-naming system ["..."] collapses when someone uses custom names (which we have to when using features)
/	* Filelists
	* optimizations (eg const)
/	* dump_BBList der ffp
	* OE: write field in patch, fout=1 -> passed, fout=0 failed OE
	* RVR/RVM
	* Why do we do (SVM)
	this->support[is][y*filter_size_x+x] = (unsigned char)(255.0*matdata[k++]);	 // because the training images grey level values were divided by 255;
	  but with the WVM, support is all float instead of uchar.

	 * erasing from the beginning of a vector is a slow operation, because at each step, all the elements of the vector have to be shifted down one place. Better would be to loop over the vector freeing everything (then clear() the vector. (or use a list, ...?) Improve speed of OE
	 * i++ --> ++i (faster)
*/









		/*cv::Mat color_img, color_hsv;
		int h_ = 0;   // H : 0 179, Hue
int s_ = 255; // S : 0 255, Saturation
int v_ = 255; // V : 0 255, Brightness Value
	const char *window_name = "HSV color";
	cv::namedWindow(window_name);
	cv::createTrackbar("H", window_name, &h_, 180, NULL, NULL);
	cv::createTrackbar("S", window_name, &s_, 255, NULL, NULL);
	cv::createTrackbar("V", window_name, &v_, 255, NULL, NULL);

	while(true) {
		color_hsv = cv::Mat(cv::Size(320, 240), CV_8UC3, cv::Scalar(h_,s_,v_));
		cv::cvtColor(color_hsv, color_img, CV_HSV2BGR);
		cv::imshow(window_name, color_img);
		int c = cv::waitKey(10);
		if (c == 27) break;
	}
	cv::destroyAllWindows();*/