#include <iostream>
#include <vector>

using namespace std;

// 动态规划函数：返回爬到 n 阶楼梯的方法数
int climbStairs(int n) {
    if (n <= 1) return 1;

    // dp[i] 表示爬到第 i 阶楼梯的方法数
    vector<int> dp(n + 1, 0);

    // 边界条件
    dp[0] = 1;  // 不爬也算一种方式
    dp[1] = 1;  // 爬 1 阶只有一种方式

    // 状态转移
    for (int i = 2; i <= n; i++) {
        dp[i] = dp[i - 1] + dp[i - 2];
    }

    return dp[n];
}

int main() {
    int n;
    cout << "请输入楼梯阶数 n: ";
    cin >> n;

    int ways = climbStairs(n);

    cout << "爬到第 " << n << " 阶楼梯的方法数为: " << ways << endl;

    return 0;
}