#include <procedural_tree.hpp>
#include <skinning.hpp>

#include <chrono>

#define N 300

int main() {
    int i = 0;
    std::chrono::time_point<std::chrono::system_clock> start, end;
    start = std::chrono::system_clock::now();

    while (i < N) {
        auto sk = ptree::CreateSkeleton(15);

        std::vector<ptree::Vertex> vertices;
        std::vector<uint32_t> indices;
        ptree::Skin_GO(15, sk, vertices, indices, false, 1.0f, nullptr);
        i++;
    }

    end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end - start;

    std::cout << "elapsed time: " << elapsed_seconds.count() << " average " << elapsed_seconds.count() / N << " per iteration" << std::endl;

    return 0;
}