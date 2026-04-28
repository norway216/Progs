#include <iostream>
#include <vector>

using namespace std;

// 定义矩阵类型
using Matrix = vector<vector<int>>;

// 矩阵乘法函数
Matrix multiplyMatrices(const Matrix& A, const Matrix& B) {
    int rowsA = A.size();
    int colsA = A[0].size();
    int rowsB = B.size();
    int colsB = B[0].size();

    // 检查矩阵是否可以相乘
    if (colsA != rowsB) {
        throw invalid_argument("Matrix dimensions do not match for multiplication.");
    }

    // 创建结果矩阵，初始化为 0
    Matrix result(rowsA, vector<int>(colsB, 0));

    // 执行矩阵乘法
    for (int i = 0; i < rowsA; ++i) {
        for (int j = 0; j < colsB; ++j) {
            for (int k = 0; k < colsA; ++k) {
                result[i][j] += A[i][k] * B[k][j];
            }
        }
    }

    return result;
}

// 打印矩阵
void printMatrix(const Matrix& matrix) {
    for (const auto& row : matrix) {
        for (const auto& elem : row) {
            cout << elem << " ";
        }
        cout << endl;
    }
}

int main() {
    // 示例：两个矩阵
    Matrix A = {
        {1, 2},
        {3, 4}
    };

    Matrix B = {
        {5, 6},
        {7, 8}
    };

    try {
        // 矩阵乘法
        Matrix result = multiplyMatrices(A, B);

        // 打印结果
        cout << "Result of matrix multiplication:" << endl;
        printMatrix(result);
    } catch (const exception& e) {
        cout << "Error: " << e.what() << endl;
    }

    return 0;
}