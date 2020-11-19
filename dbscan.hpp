#include <vector>

auto dbscan(const std::vector<std::pair<float, float>>& data, float eps, int min_pts) 
        -> std::vector<std::vector<size_t>>;