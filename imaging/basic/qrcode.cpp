// qr_code.cpp
#include <opencv2/opencv.hpp>
#include <qrencode.h>
#include <iostream>

int main() {
    std::string text = "https://www.example.com";

    // 1. 使用 libqrencode 生成二维码
    QRcode *qrcode = QRcode_encodeString(text.c_str(), 0, QR_ECLEVEL_Q, QR_MODE_8, 1);
    if (!qrcode) {
        std::cerr << "Failed to generate QR code!" << std::endl;
        return -1;
    }

    int qr_size = qrcode->width;
    int scale = 10;  // 每个模块放大倍数
    int img_size = qr_size * scale;

    // 2. 创建 OpenCV Mat 图像
    cv::Mat qr_img(img_size, img_size, CV_8UC1, cv::Scalar(255));

    // 3. 填充二维码黑白模块
    unsigned char *data = qrcode->data;
    for (int y = 0; y < qr_size; y++) {
        for (int x = 0; x < qr_size; x++) {
            unsigned char b = data[y*qr_size + x] & 0x01 ? 0 : 255; // 0=黑,255=白
            cv::rectangle(qr_img,
                cv::Point(x*scale, y*scale),
                cv::Point((x+1)*scale-1, (y+1)*scale-1),
                cv::Scalar(b),
                cv::FILLED
            );
        }
    }

    // 4. 显示二维码
    cv::imshow("QR Code", qr_img);
    cv::imwrite("qrcode.png", qr_img);
    std::cout << "QR code saved as qrcode.png" << std::endl;

    cv::waitKey(0);
    QRcode_free(qrcode);
    return 0;
}