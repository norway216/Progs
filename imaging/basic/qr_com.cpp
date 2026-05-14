// qr_web_link.cpp
#include <opencv2/opencv.hpp>
#include <qrencode.h>
#include <iostream>

int main() {
    // 1. 设置二维码内容为网页超链接
    std::string url = "https://www.google.com";

    // 2. 生成二维码
    QRcode* qrcode = QRcode_encodeString(url.c_str(), 0, QR_ECLEVEL_Q, QR_MODE_8, 1);
    if (!qrcode) {
        std::cerr << "Failed to generate QR code!" << std::endl;
        return -1;
    }

    int qr_size = qrcode->width;
    int scale = 10;  // 每个模块放大倍数
    int img_size = qr_size * scale;

    // 3. 创建 OpenCV 图像
    cv::Mat qr_img(img_size, img_size, CV_8UC1, cv::Scalar(255));

    // 4. 填充二维码黑白模块
    unsigned char* data = qrcode->data;
    for (int y = 0; y < qr_size; y++) {
        for (int x = 0; x < qr_size; x++) {
            unsigned char b = data[y * qr_size + x] & 0x01 ? 0 : 255; // 黑=0，白=255
            cv::rectangle(qr_img,
                cv::Point(x * scale, y * scale),
                cv::Point((x + 1) * scale - 1, (y + 1) * scale - 1),
                cv::Scalar(b),
                cv::FILLED);
        }
    }

    // 5. 保存二维码
    cv::imwrite("web_link_qrcode.png", qr_img);
    std::cout << "QR code saved as web_link_qrcode.png" << std::endl;

    // 6. 可选显示
    cv::imshow("QR Code", qr_img);
    cv::waitKey(0);
    cv::destroyAllWindows();

    QRcode_free(qrcode);
    return 0;
}