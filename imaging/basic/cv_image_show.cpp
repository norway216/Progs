#include <opencv2/opencv.hpp>
#include <iostream>

int main() {
    // 图像宽高
    const int width = 640;
    const int height = 480;

    // 创建一张 8 位 3 通道彩色图
    cv::Mat image(height, width, CV_8UC3);

    // 生成随机数填充图像
    cv::randu(image, cv::Scalar(0, 0, 0), cv::Scalar(256, 256, 256));

    // 显示图像
    cv::imshow("Random Image", image);

    // 等待按键
    cv::waitKey(0);

    return 0;
}