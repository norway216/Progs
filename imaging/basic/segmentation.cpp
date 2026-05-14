// object_segmentation.cpp
#include <opencv2/opencv.hpp>
#include <iostream>

int main(int argc, char** argv) {
    if(argc < 2) {
        std::cout << "Usage: ./object_segmentation <image_path>" << std::endl;
        return -1;
    }

    std::string image_path = argv[1];

    // 1. 读取图片
    cv::Mat img = cv::imread(image_path);
    if(img.empty()) {
        std::cerr << "Cannot read image!" << std::endl;
        return -1;
    }

    cv::Mat img_gray, img_blur, img_thresh, mask;
    
    // 2. 灰度化
    cv::cvtColor(img, img_gray, cv::COLOR_BGR2GRAY);

    // 3. 高斯模糊
    cv::GaussianBlur(img_gray, img_blur, cv::Size(5,5), 0);

    // 4. Otsu 阈值化（二值化）
    cv::threshold(img_blur, img_thresh, 0, 255, cv::THRESH_BINARY + cv::THRESH_OTSU);

    // 5. 形态学操作，去噪声
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3,3));
    cv::morphologyEx(img_thresh, img_thresh, cv::MORPH_OPEN, kernel);

    // 6. 轮廓检测
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(img_thresh, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    // 7. 创建掩膜并绘制轮廓
    mask = cv::Mat::zeros(img.size(), CV_8UC3);
    for(size_t i=0;i<contours.size();i++) {
        cv::Scalar color(rand()%256, rand()%256, rand()%256);
        cv::drawContours(mask, contours, (int)i, color, cv::FILLED);
    }

    // 8. 显示结果
    cv::imshow("Original", img);
    cv::imshow("Threshold", img_thresh);
    cv::imshow("Segmented Objects", mask);

    cv::waitKey(0);
    cv::destroyAllWindows();

    return 0;
}