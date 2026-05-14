#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <random>

using namespace cv;
using namespace std;

static Scalar randomColor(int id)
{
    RNG rng(id * 12345);
    return Scalar(rng.uniform(50, 255), rng.uniform(50, 255), rng.uniform(50, 255));
}

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        cout << "Usage: ./multi_object_segmentation <image_path>" << endl;
        return -1;
    }

    string imagePath = argv[1];

    Mat src = imread(imagePath);
    if (src.empty())
    {
        cerr << "Error: cannot read image: " << imagePath << endl;
        return -1;
    }

    Mat srcShow = src.clone();

    // 1. 灰度化
    Mat gray;
    cvtColor(src, gray, COLOR_BGR2GRAY);

    // 2. 高斯滤波，降低噪声
    Mat blurred;
    GaussianBlur(gray, blurred, Size(5, 5), 0);

    // 3. Otsu 自动阈值分割
    Mat binary;
    threshold(
        blurred,
        binary,
        0,
        255,
        THRESH_BINARY | THRESH_OTSU
    );

    // 如果目标是黑色、背景是白色，则反转
    int whitePixels = countNonZero(binary);
    int totalPixels = binary.rows * binary.cols;
    if (whitePixels > totalPixels / 2)
    {
        bitwise_not(binary, binary);
    }

    // 4. 形态学开运算：去除小噪点
    Mat kernel = getStructuringElement(MORPH_ELLIPSE, Size(3, 3));

    Mat opening;
    morphologyEx(binary, opening, MORPH_OPEN, kernel, Point(-1, -1), 2);

    // 5. 膨胀得到确定背景区域
    Mat sureBg;
    dilate(opening, sureBg, kernel, Point(-1, -1), 3);

    // 6. 距离变换，得到目标中心区域
    Mat dist;
    distanceTransform(opening, dist, DIST_L2, 5);

    // 归一化，便于观察
    Mat distDisplay;
    normalize(dist, distDisplay, 0, 1.0, NORM_MINMAX);

    // 7. 提取确定前景区域
    double maxVal;
    minMaxLoc(dist, nullptr, &maxVal);

    Mat sureFg;
    threshold(dist, sureFg, 0.4 * maxVal, 255, THRESH_BINARY);
    sureFg.convertTo(sureFg, CV_8U);

    // 8. 未知区域 = 背景 - 前景
    Mat unknown;
    subtract(sureBg, sureFg, unknown);

    // 9. 连通域标记
    Mat markers;
    int numLabels = connectedComponents(sureFg, markers);

    // Watershed 要求：
    // 背景不能是 0，所以整体 +1
    markers = markers + 1;

    // 未知区域设置为 0
    for (int y = 0; y < unknown.rows; y++)
    {
        for (int x = 0; x < unknown.cols; x++)
        {
            if (unknown.at<uchar>(y, x) == 255)
            {
                markers.at<int>(y, x) = 0;
            }
        }
    }

    // 10. Watershed 分割
    watershed(src, markers);

    // 11. 根据 markers 生成彩色分割图
    Mat result = Mat::zeros(src.size(), CV_8UC3);
    Mat boundary = src.clone();

    vector<int> areaCount(numLabels + 2, 0);

    for (int y = 0; y < markers.rows; y++)
    {
        for (int x = 0; x < markers.cols; x++)
        {
            int label = markers.at<int>(y, x);

            if (label == -1)
            {
                // Watershed 边界
                result.at<Vec3b>(y, x) = Vec3b(0, 0, 255);
                boundary.at<Vec3b>(y, x) = Vec3b(0, 0, 255);
            }
            else if (label <= 1)
            {
                // 背景
                result.at<Vec3b>(y, x) = Vec3b(0, 0, 0);
            }
            else
            {
                Scalar c = randomColor(label);
                result.at<Vec3b>(y, x) = Vec3b(
                    static_cast<uchar>(c[0]),
                    static_cast<uchar>(c[1]),
                    static_cast<uchar>(c[2])
                );

                if (label >= 0 && label < static_cast<int>(areaCount.size()))
                {
                    areaCount[label]++;
                }
            }
        }
    }

    // 12. 统计每个目标轮廓和外接矩形
    cout << "Detected objects:" << endl;

    for (int label = 2; label < numLabels + 1; label++)
    {
        Mat objectMask = markers == label;

        vector<vector<Point>> contours;
        findContours(objectMask, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

        if (contours.empty())
        {
            continue;
        }

        double area = contourArea(contours[0]);
        if (area < 50)
        {
            continue;
        }

        Rect bbox = boundingRect(contours[0]);

        rectangle(srcShow, bbox, Scalar(0, 255, 0), 2);
        putText(
            srcShow,
            to_string(label - 1),
            Point(bbox.x, max(0, bbox.y - 5)),
            FONT_HERSHEY_SIMPLEX,
            0.6,
            Scalar(0, 0, 255),
            2
        );

        cout << "Object " << label - 1
             << " | Area: " << area
             << " | Rect: x=" << bbox.x
             << ", y=" << bbox.y
             << ", w=" << bbox.width
             << ", h=" << bbox.height
             << endl;
    }

    // 13. 显示图像
    imshow("Original", src);
    imshow("Binary", binary);
    imshow("Opening", opening);
    imshow("Distance Transform", distDisplay);
    imshow("Watershed Result", result);
    imshow("Detected Objects", srcShow);
    imshow("Boundary", boundary);

    // 14. 保存结果
    imwrite("binary.png", binary);
    imwrite("segmentation_result.png", result);
    imwrite("detected_objects.png", srcShow);
    imwrite("watershed_boundary.png", boundary);

    cout << endl;
    cout << "Saved files:" << endl;
    cout << "  binary.png" << endl;
    cout << "  segmentation_result.png" << endl;
    cout << "  detected_objects.png" << endl;
    cout << "  watershed_boundary.png" << endl;

    waitKey(0);
    destroyAllWindows();

    return 0;
}