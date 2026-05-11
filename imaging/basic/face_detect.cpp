#include <dlib/opencv.h>
#include <dlib/image_processing/frontal_face_detector.h>
#include <opencv2/opencv.hpp>
#include <iostream>

int main() {
    // 打开默认摄像头
    cv::VideoCapture cap(0);
    if (!cap.isOpened()) {
        std::cerr << "无法打开摄像头!" << std::endl;
        return -1;
    }

    // 初始化 Dlib HOG 人脸检测器
    dlib::frontal_face_detector detector = dlib::get_frontal_face_detector();

    cv::Mat frame;
    while (true) {
        cap >> frame;  // 读取摄像头一帧
        if (frame.empty()) break;

        // 转换为 dlib 图像格式
        dlib::cv_image<dlib::bgr_pixel> dlib_frame(frame);

        // 检测人脸
        std::vector<dlib::rectangle> faces = detector(dlib_frame);

        // 在 OpenCV 图像上绘制矩形框
        for (auto& face : faces) {
            cv::rectangle(frame,
                          cv::Point(face.left(), face.top()),
                          cv::Point(face.right(), face.bottom()),
                          cv::Scalar(0, 255, 0), 2);
        }

        // 显示结果
        cv::imshow("Face Detection", frame);

        // 按 q 键退出
        if (cv::waitKey(1) == 'q') break;
    }

    cap.release();
    cv::destroyAllWindows();
    return 0;
}