#pragma once

#include <iostream>
#include <chrono>

class TicToc {
public:
    void tic() {
        tic_time = std::chrono::high_resolution_clock::now();
    }

    double toc() {
        auto toc_time = std::chrono::high_resolution_clock::now();
        return (toc_time - tic_time).count() * 1E-9;
    }
private:
    std::chrono::high_resolution_clock::time_point tic_time;
};

