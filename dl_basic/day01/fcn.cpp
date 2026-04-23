#include <iostream>
#include <vector>
#include <Eigen/Dense>

using namespace Eigen;
using namespace std;

// 激活函数：Sigmoid
MatrixXd sigmoid(const MatrixXd& z) {
    return 1.0 / (1.0 + (-z.array()).exp());
}

// Sigmoid 的导数
MatrixXd sigmoid_derivative(const MatrixXd& a) {
    return a.array() * (1.0 - a.array());
}

// 输出层激活：Softmax (数值稳定性版本)
MatrixXd softmax(const MatrixXd& z) {
    MatrixXd exp_z = (z.rowwise() - z.colwise().maxCoeff()).array().exp();
    VectorXd sum_exp = exp_z.colwise().sum();
    return exp_z.array().rowwise() / sum_exp.transpose().array();
}

class NeuralNetwork {
public:
    // 构造函数：初始化网络结构 (例如: {784, 128, 10})
    NeuralNetwork(const vector<int>& layers, double learning_rate = 0.01) 
        : layers(layers), lr(learning_rate) {
        
        for (size_t i = 0; i < layers.size() - 1; ++i) {
            // Xavier/Glorot 初始化防止梯度消失
            double limit = sqrt(6.0 / (layers[i] + layers[i+1]));
            weights.push_back(MatrixXd::Random(layers[i+1], layers[i]) * limit);
            biases.push_back(VectorXd::Zero(layers[i+1]));
        }
    }

    // 前向传播
    MatrixXd forward(const MatrixXd& input) {
        activations.clear();
        zs.clear();
        
        activations.push_back(input);
        MatrixXd curr_a = input;

        for (size_t i = 0; i < weights.size(); ++i) {
            MatrixXd z = (weights[i] * curr_a).colwise() + biases[i];
            zs.push_back(z);
            
            // 最后一层使用 Softmax，其余使用 Sigmoid
            if (i == weights.size() - 1)
                curr_a = softmax(z);
            else
                curr_a = sigmoid(z);
                
            activations.push_back(curr_a);
        }
        return curr_a;
    }

    // 反向传播与参数更新
    void backward(const MatrixXd& target) {
        int L = weights.size();
        MatrixXd output = activations.back();
        
        // 计算输出层误差 (Cross-Entropy + Softmax 的导数简化为 output - target)
        MatrixXd delta = output - target;

        for (int i = L - 1; i >= 0; --i) {
            // 计算梯度
            MatrixXd dW = (delta * activations[i].transpose()) / delta.cols();
            VectorXd db = delta.rowwise().mean();

            // 如果不是第一层，计算下一层的 delta
            if (i > 0) {
                delta = (weights[i].transpose() * delta).array() * sigmoid_derivative(activations[i]).array();
            }

            // SGD 更新参数
            weights[i] -= lr * dW;
            biases[i] -= lr * db;
        }
    }

    void train(const MatrixXd& x, const MatrixXd& y) {
        forward(x);
        backward(y);
    }

private:
    vector<int> layers;
    double lr;
    vector<MatrixXd> weights;
    vector<VectorXd> biases;
    vector<MatrixXd> activations; // 存储每一层的输出 (a)
    vector<MatrixXd> zs;          // 存储每一层的线性变换结果 (z = Wa + b)
};

int main() {
    // 示例：输入层2，隐层3，输出层2
    NeuralNetwork nn({2, 3, 2}, 0.1);

    // 模拟数据 (Batch size = 1)
    MatrixXd x(2, 1);
    x << 0.5, -0.2;
    
    MatrixXd y(2, 1);
    y << 0, 1; // 独热编码标签

    cout << "训练前预测值:\n" << nn.forward(x) << endl;

    // 简单循环训练
    for(int i = 0; i < 1000; ++i) {
        nn.train(x, y);
    }

    cout << "1000次迭代后预测值:\n" << nn.forward(x) << endl;

    return 0;
}