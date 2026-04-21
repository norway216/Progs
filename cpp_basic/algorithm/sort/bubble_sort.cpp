#include <iostream>
#include <vector>
#include <functional>
#include <utility>

template<typename T, typename Compare = std::less<T>>
void bubbleSortHighPerf(std::vector<T>& arr, Compare comp = Compare{}) {
    if (arr.size() < 2) {
        return;
    }

    std::size_t n = arr.size();

    while (n > 1) {
        std::size_t lastSwap = 0;

        for (std::size_t i = 1; i < n; ++i) {
            if (comp(arr[i], arr[i - 1])) {
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
    std::vector<int> nums = {5, 2, 9, 1, 3};

    bubbleSortHighPerf(nums); // 升序

    for (auto x : nums) {
        std::cout << x << " ";
    }
    std::cout << '\n';

    bubbleSortHighPerf(nums, std::greater<int>{}); // 降序

    for (auto x : nums) {
        std::cout << x << " ";
    }
    std::cout << '\n';

    return 0;
}