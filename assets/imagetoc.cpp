#include <iostream>
#include <opencv2/opencv.hpp>

// Quick rough tool I wrote by hand to convert monochrome images to patterns
// Temporary tool

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cout << "Invalid number of parameters " << argc << ". Expected 2." << std::endl;
        return -1;
    }

    std::string filename = argv[1];
    std::cout << "Opening \"" << filename << "\"..." << std::endl;

    cv::Mat image = cv::imread(filename);
    std::cout << "Image resolution: " << image.size << std::endl;

    std::cout << "Uint8 pattern[] = {" << std::endl;
    for (int y{0}; y < image.rows; ++y) {
        std::cout << "    ";
        for (int x{0}; x < image.cols; ++x) {
            cv::Vec3b pixel = image.at<cv::Vec3b>(y, x);
            float value = (pixel[0] + pixel[1] + pixel[2]) / 3.0f;
            if (value > 200)
                std::cout << "1,";
            else
                std::cout << "0,";
        }
        if (y != image.rows-1) std::cout << std::endl;
    }
    std::cout << std::endl << "};" << std::endl;
    std::cout << "// width = " << image.cols << ", height = " << image.rows << std::endl;

    return 0;
}