#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <stack>
#include <queue>
#include <random>
#include <ctime>
#include <algorithm>

struct Cell {
    int r;
    int c;

    bool operator==(const Cell& other) const {
        return r == other.r && c == other.c;
    }
};

int main() {
    // ========== 1) 迷宫逻辑尺寸 ==========
    const int rows = 21;
    const int cols = 21;

    // 原来是 20，现在扩大一倍 -> 40
    const int cellSize = 40;

    // 1 = 墙, 0 = 路
    std::vector<std::vector<int>> maze(rows, std::vector<int>(cols, 1));

    std::mt19937 rng(static_cast<unsigned int>(std::time(nullptr)));

    auto inBoundsForGen = [&](int r, int c) {
        return r > 0 && r < rows - 1 && c > 0 && c < cols - 1;
    };

    // ========== 2) DFS 生成迷宫 ==========
    Cell start{1, 1};
    maze[start.r][start.c] = 0;

    std::stack<Cell> st;
    st.push(start);

    const std::vector<std::pair<int, int>> directions = {
        {-2, 0}, {2, 0}, {0, -2}, {0, 2}
    };

    while (!st.empty()) {
        Cell current = st.top();
        std::vector<Cell> neighbors;

        for (const auto& d : directions) {
            int nr = current.r + d.first;
            int nc = current.c + d.second;

            if (inBoundsForGen(nr, nc) && maze[nr][nc] == 1) {
                neighbors.push_back({nr, nc});
            }
        }

        if (!neighbors.empty()) {
            std::uniform_int_distribution<int> dist(0, static_cast<int>(neighbors.size()) - 1);
            Cell next = neighbors[dist(rng)];

            int wallR = (current.r + next.r) / 2;
            int wallC = (current.c + next.c) / 2;

            maze[wallR][wallC] = 0;
            maze[next.r][next.c] = 0;

            st.push(next);
        } else {
            st.pop();
        }
    }

    // ========== 3) 设置入口和出口 ==========
    Cell entrance{1, 0};
    Cell exit{rows - 2, cols - 1};

    maze[entrance.r][entrance.c] = 0;
    maze[exit.r][exit.c] = 0;

    // ========== 4) 颜色定义（BGR） ==========
    cv::Vec3b wallColor    = cv::Vec3b(40, 40, 40);       // 墙：深灰
    cv::Vec3b roadColor    = cv::Vec3b(240, 240, 220);    // 路：浅色
    cv::Vec3b enterColor   = cv::Vec3b(60, 200, 60);      // 入口：绿
    cv::Vec3b exitColor    = cv::Vec3b(60, 60, 220);      // 出口：红
    cv::Vec3b traceColor   = cv::Vec3b(0, 255, 255);      // 走过路径：黄
    cv::Vec3b playerColor  = cv::Vec3b(255, 0, 0);        // 当前位置：蓝

    const int imgH = rows * cellSize;
    const int imgW = cols * cellSize;

    // ========== 5) 画基础迷宫 ==========
    auto drawBaseMaze = [&]() -> cv::Mat {
        cv::Mat image(imgH, imgW, CV_8UC3);

        for (int r = 0; r < rows; ++r) {
            for (int c = 0; c < cols; ++c) {
                cv::Vec3b color = (maze[r][c] == 1) ? wallColor : roadColor;

                if (r == entrance.r && c == entrance.c) {
                    color = enterColor;
                } else if (r == exit.r && c == exit.c) {
                    color = exitColor;
                }

                for (int y = r * cellSize; y < (r + 1) * cellSize; ++y) {
                    for (int x = c * cellSize; x < (c + 1) * cellSize; ++x) {
                        image.at<cv::Vec3b>(y, x) = color;
                    }
                }
            }
        }

        // 画网格线
        for (int r = 0; r <= rows; ++r) {
            cv::line(image,
                     cv::Point(0, r * cellSize),
                     cv::Point(imgW, r * cellSize),
                     cv::Scalar(200, 200, 200), 1);
        }

        for (int c = 0; c <= cols; ++c) {
            cv::line(image,
                     cv::Point(c * cellSize, 0),
                     cv::Point(c * cellSize, imgH),
                     cv::Scalar(200, 200, 200), 1);
        }

        return image;
    };

    // ========== 6) BFS 求最短路径 ==========
    auto inBounds = [&](int r, int c) {
        return r >= 0 && r < rows && c >= 0 && c < cols;
    };

    std::vector<std::vector<int>> visited(rows, std::vector<int>(cols, 0));
    std::vector<std::vector<Cell>> parent(rows, std::vector<Cell>(cols, {-1, -1}));

    std::queue<Cell> q;
    q.push(entrance);
    visited[entrance.r][entrance.c] = 1;

    const std::vector<std::pair<int, int>> moveDirs = {
        {-1, 0}, {1, 0}, {0, -1}, {0, 1}
    };

    bool found = false;

    while (!q.empty()) {
        Cell cur = q.front();
        q.pop();

        if (cur == exit) {
            found = true;
            break;
        }

        for (const auto& d : moveDirs) {
            int nr = cur.r + d.first;
            int nc = cur.c + d.second;

            if (inBounds(nr, nc) && !visited[nr][nc] && maze[nr][nc] == 0) {
                visited[nr][nc] = 1;
                parent[nr][nc] = cur;
                q.push({nr, nc});
            }
        }
    }

    if (!found) {
        std::cout << "没有找到从入口到出口的路径！" << std::endl;
        return 0;
    }

    // 回溯最短路径
    std::vector<Cell> path;
    Cell cur = exit;
    while (!(cur == entrance)) {
        path.push_back(cur);
        cur = parent[cur.r][cur.c];
    }
    path.push_back(entrance);
    std::reverse(path.begin(), path.end());

    // ========== 7) 动态走迷宫 ==========
    cv::Mat baseMaze = drawBaseMaze();

    cv::namedWindow("Dynamic Maze", cv::WINDOW_AUTOSIZE);

    for (size_t i = 0; i < path.size(); ++i) {
        cv::Mat frame = baseMaze.clone();

        // 画已经走过的路径
        for (size_t j = 0; j <= i; ++j) {
            int r = path[j].r;
            int c = path[j].c;

            // 不覆盖入口出口颜色太多，只在中间区域画一个小矩形
            int x1 = c * cellSize + cellSize / 4;
            int y1 = r * cellSize + cellSize / 4;
            int x2 = c * cellSize + 3 * cellSize / 4;
            int y2 = r * cellSize + 3 * cellSize / 4;

            cv::rectangle(frame,
                          cv::Point(x1, y1),
                          cv::Point(x2, y2),
                          cv::Scalar(traceColor[0], traceColor[1], traceColor[2]),
                          -1);
        }

        // 当前人物位置（画圆）
        int pr = path[i].r;
        int pc = path[i].c;
        cv::Point center(pc * cellSize + cellSize / 2, pr * cellSize + cellSize / 2);
        int radius = cellSize / 3;

        cv::circle(frame,
                   center,
                   radius,
                   cv::Scalar(playerColor[0], playerColor[1], playerColor[2]),
                   -1);

        // 显示提示文字
        std::string text = "Walking through the maze...";
        cv::putText(frame,
                    text,
                    cv::Point(20, 30),
                    cv::FONT_HERSHEY_SIMPLEX,
                    0.8,
                    cv::Scalar(0, 0, 0),
                    2);

        cv::imshow("Dynamic Maze", frame);

        // 每一步延时，控制动画速度
        // 按 ESC 可以提前退出
        int key = cv::waitKey(120);
        if (key == 27) {
            break;
        }
    }

    // 最终结果停住
    cv::Mat finalFrame = baseMaze.clone();

    for (const auto& p : path) {
        int x1 = p.c * cellSize + cellSize / 4;
        int y1 = p.r * cellSize + cellSize / 4;
        int x2 = p.c * cellSize + 3 * cellSize / 4;
        int y2 = p.r * cellSize + 3 * cellSize / 4;

        cv::rectangle(finalFrame,
                      cv::Point(x1, y1),
                      cv::Point(x2, y2),
                      cv::Scalar(traceColor[0], traceColor[1], traceColor[2]),
                      -1);
    }

    cv::circle(finalFrame,
               cv::Point(exit.c * cellSize + cellSize / 2, exit.r * cellSize + cellSize / 2),
               cellSize / 3,
               cv::Scalar(playerColor[0], playerColor[1], playerColor[2]),
               -1);

    cv::putText(finalFrame,
                "Maze completed!",
                cv::Point(20, 30),
                cv::FONT_HERSHEY_SIMPLEX,
                0.8,
                cv::Scalar(0, 0, 255),
                2);

    cv::imshow("Dynamic Maze", finalFrame);

    // 保存最终结果
    cv::imwrite("dynamic_maze_result.png", finalFrame);

    cv::waitKey(0);
    return 0;
}