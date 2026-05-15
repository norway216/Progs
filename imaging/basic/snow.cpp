#include <opencv2/opencv.hpp>
#include <vector>
#include <cstdlib>
#include <ctime>

struct Snowflake {
    cv::Point2f pos;
    float speed;
    float size;
    float depth;
};

int main() {
    srand((unsigned int)time(0));
    const int width = 1280;
    const int height = 720;
    const int numSnowflakes = 500; // 雪花数量
    cv::Mat frame(height, width, CV_8UC3, cv::Scalar(0,0,0));

    std::vector<Snowflake> snowflakes(numSnowflakes);
    for(auto &s : snowflakes) {
        s.pos = cv::Point2f(rand()%width, rand()%height);
        s.depth = (float)(rand()%100)/100.0f; // 0~1，越大越近
        s.size = 2 + s.depth*6; // 近的雪花更大
        s.speed = 1 + s.depth*4; // 近的雪花下落更快
    }

    while(true) {
        frame.setTo(cv::Scalar(0,0,0));
        for(auto &s : snowflakes) {
            cv::circle(frame, s.pos, (int)s.size, cv::Scalar(255,255,255), -1);
            s.pos.y += s.speed;
            s.pos.x += sin(s.pos.y*0.01)*2; // 模拟风
            if(s.pos.y > height) {
                s.pos.y = -s.size;
                s.pos.x = rand()%width;
            }
        }

        cv::imshow("Snowfall", frame);
        char c = (char)cv::waitKey(30);
        if(c == 27) break; // ESC退出
    }

    cv::destroyAllWindows();
    return 0;
}