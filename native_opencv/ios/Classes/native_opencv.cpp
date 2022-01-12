// CPP program to Stitch
// input images (panorama) using OpenCV
#include <iostream>
#include <fstream>

// Include header files from OpenCV directory
// required to stitch images.
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/stitching.hpp"

#include <opencv2/opencv.hpp>
#include <chrono>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32)
#define IS_WIN32
#endif

#ifdef __ANDROID__
#include <android/log.h>
#endif

#ifdef IS_WIN32
#include <windows.h>
#endif

#if defined(__GNUC__)
    // Attributes to prevent 'unused' function from being removed and to make it visible
    #define FUNCTION_ATTRIBUTE __attribute__((visibility("default"))) __attribute__((used))
#elif defined(_MSC_VER)
    // Marking a function for export
    #define FUNCTION_ATTRIBUTE __declspec(dllexport)
#endif

using namespace cv;
using namespace std;

Stitcher::Mode mode = Stitcher::PANORAMA;

long long int get_now() {
    return chrono::duration_cast<std::chrono::milliseconds>(
            chrono::system_clock::now().time_since_epoch()
    ).count();
}

void platform_log(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
#ifdef __ANDROID__
    __android_log_vprint(ANDROID_LOG_VERBOSE, "ndk", fmt, args);
#elif defined(IS_WIN32)
    char *buf = new char[4096];
    std::fill_n(buf, 4096, '\0');
    _vsprintf_p(buf, 4096, fmt, args);
    OutputDebugStringA(buf);
    delete[] buf;
#else
    vprintf(fmt, args);
#endif
    va_end(args);
}

// Avoiding name mangling
extern "C" {
    FUNCTION_ATTRIBUTE
    const char* version() {
        return CV_VERSION;
    }

    struct tokens: ctype<char>
    {
        tokens(): std::ctype<char>(get_table()) {}

        static std::ctype_base::mask const* get_table()
        {
            typedef std::ctype<char> cctype;
            static const cctype::mask *const_rc= cctype::classic_table();

            static cctype::mask rc[cctype::table_size];
            std::memcpy(rc, const_rc, cctype::table_size * sizeof(cctype::mask));

            rc[','] =  ctype_base::space;
            rc[' '] =  ctype_base::space;
            return &rc[0];
        }
    };

//    vector<string> getpathlist(string path_string){
//        string sub_string = path_string.substr(1,path_string.length()-2);
//        stringstream ss(sub_string);
//        ss.imbue( locale( locale(), new tokens()));
//        istream_iterator<std::string> begin(ss);
//        istream_iterator<std::string> end;
//        vector<std::string> pathlist(begin, end);
//        return pathlist;
//    }

    FUNCTION_ATTRIBUTE
    void process_image(char* inputImagePath, char* outputImagePath) {
        long long start = get_now();

        Mat input = imread(inputImagePath, IMREAD_GRAYSCALE);
        Mat threshed, withContours;

        vector<vector<Point>> contours;
        vector<Vec4i> hierarchy;

        adaptiveThreshold(input, threshed, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY_INV, 77, 6);
        findContours(threshed, contours, hierarchy, RETR_TREE, CHAIN_APPROX_TC89_L1);

        cvtColor(threshed, withContours, COLOR_GRAY2BGR);
        drawContours(withContours, contours, -1, Scalar(0, 255, 0), 4);

        imwrite(outputImagePath, withContours);

        int evalInMillis = static_cast<int>(get_now() - start);
        platform_log("Processing done in %dms\n", evalInMillis);
    }

    FUNCTION_ATTRIBUTE
    void stitch_image(char* inputImagePath, int size, char* outputImagePath) {
        string input_path_string =  inputImagePath;
                
        // Get path list.
        string sub_string = input_path_string.substr(1,input_path_string.length()-2);
        stringstream ss(sub_string);
        ss.imbue( locale( locale(), new tokens()));
        istream_iterator<std::string> begin(ss);
        istream_iterator<std::string> end;
        vector<std::string> image_vector_list(begin, end);
        
//        vector<string> image_vector_list = getpathlist(input_path_string);
        
        vector<Mat> imgs;

        for(auto k = image_vector_list.begin();k != image_vector_list.end(); ++k)
        {
            String  path = *k;
            Mat img = imread(path);
            imgs.push_back(img);
        }

        // Define object to store the stitched image
        Mat pano;

        // Create a Stitcher class object with mode panoroma
        Ptr<Stitcher> stitcher = Stitcher::create(mode);

        // Command to stitch all the images present in the image array
        Stitcher::Status status = stitcher->stitch(imgs, pano);

        if (status != Stitcher::OK)
        {
            // Check if images could not be stiched
            // status is OK if images are stiched successfully
            platform_log("Can't stitch images\n");
            return;
        }

        // Store a new image stiched from the given
        //set of images as "result.jpg"
        imwrite(outputImagePath, pano);

        // Show the result
        // imshow("Result", pano);

//        waitKey(0);
    }
}
