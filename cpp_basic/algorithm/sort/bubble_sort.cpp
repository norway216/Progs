#include <iostream>
#include <vector>
#include <utility>  // std::swap

template<typename T>
void bubbleSortOptimized(std::vector<T>& arr) {
    if (arr.size() < 2) {
        return;
    }

    std::size_t n = arr.size();

    while (n > 1) {
        std::size_t lastSwap = 0;

        for (std::size_t i = 1; i < n; ++i) {
            if (arr[i - 1] > arr[i]) {
                std::swap(arr[i - 1], arr[i]);
                lastSwap = i;
            }
        }

        if (lastSwap == 0) {
            break;
        }

        n = lastSwap;
    }
}

int main() {
    std::vector<int> nums = {5, 1, 4, 2, 8, 3, 6};

    bubbleSortOptimized(nums);

    for (const auto& x : nums) {
        std::cout << x << " ";
    }
    std::cout << std::endl;

    return 0;
}