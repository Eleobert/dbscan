#include "dbscan.hpp"
#include <iostream>
#include <string>
#include <system_error>
#include <vector>
#include <utility>
#include <fstream>
#include <charconv>
#include <cassert>
#include <tuple>
#include <cstring>


auto check_from_chars_error(std::errc err, const std::string_view& line, int line_counter)
{
    if(err == std::errc())
        return;
    
    if(err == std::errc::invalid_argument)
    {
        std::cerr << "Error: Invalid value \"" << line
            << "\" at line " << line_counter << "\n";
        std::exit(1);
    }

    if(err == std::errc::result_out_of_range)
    {
        std::cerr << "Error: Value \"" << line << "\"out of range at line " 
                  <<  line_counter << "\n";
        std::exit(1);
    }

}


auto push_values(std::vector<float>& store, const std::string_view& line, int line_counter)
{
    auto ptr = line.data();
    auto ec  = std::errc();
    auto n_pushed = 0;

    do
    {
        float value;
        auto [p, ec] =  std::from_chars(ptr, line.data() + line.size(), value);
        ptr = p + 1;
        check_from_chars_error(ec, line, line_counter);
        n_pushed++;
        store.push_back(value);

    }while(ptr < line.data() + line.size());

    return n_pushed;
}


auto read_values(const std::string& filename)
{
    std::ifstream file(filename);

    if(not file.good())
    {
        std::perror(filename.c_str());
        std::exit(2);
    }

    auto count = 0;

    auto points = std::vector<float>();
    auto dim    = 0;

    while(not file.eof())
    {
        count++;
        auto line = std::string();
        std::getline(file, line);

        if(not line.empty())
        {
            auto n_pushed = push_values(points, line, count);

            if(count != 1)
            {
                if(n_pushed != dim)
                {
                    std::cerr << "Inconsistent number of dimensions at line '" << count << "'\n";
                    std::exit(1);
                }
            }
            dim = n_pushed;
        }
    }

    return std::tuple(points, dim);
}


template<typename T>
auto to_num(const std::string& str)
{
    T value = 0;
    auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), value);

    if(ec != std::errc())
    {
        std::cerr << "Error converting value '" << str << "'\n";
        std::exit(1);
    }
    return value;
}


// noise will be labelled as 0
auto label(const std::vector<std::vector<size_t>>& clusters, size_t n)
{
    auto flat_clusters = std::vector<size_t>(n);

    for(size_t i = 0; i < clusters.size(); i++)
    {
        for(auto p: clusters[i])
        {
            flat_clusters[p] = i + 1;
        }
    }

    return flat_clusters;
}


auto dbscan2d(const std::span<const float>& data, float eps, int min_pts)
{
    auto points = std::vector<point2>(data.size() / 2);

    std::memcpy(points.data(), data.data(), sizeof(float) * data.size());

    auto clusters = dbscan(points, eps, min_pts);
    auto flat     = label (clusters, points.size());

    for(size_t i = 0; i < points.size(); i++)
    {
        std::cout << points[i].x << ',' << points[i].y << ','  << flat[i] << '\n';
    }
}


auto dbscan3d(const std::span<const float>& data, float eps, int min_pts)
{
    auto points = std::vector<point3>(data.size() / 3);

    std::memcpy(points.data(), data.data(), sizeof(float) * data.size());

    auto clusters = dbscan(points, eps, min_pts);
    auto flat     = label (clusters, points.size());

    for(size_t i = 0; i < points.size(); i++)
    {
        std::cout << points[i].x << ',' << points[i].y << ',' << points[i].z << ',' << flat[i] << '\n';
    }
}



int main(int argc, char** argv)
{
    if(argc != 4)
    {
        std::cerr << "usage: example <tsv file> <epsilon> <min points>\n";
        return 1;
    }

    auto epsilon  = to_num<float>(argv[2]);
    auto min_pts  = to_num<int>  (argv[3]);
    auto [values, dim] = read_values(argv[1]);

    if(dim == 2)
    {
        dbscan2d(values, epsilon, min_pts);
    }
    else if (dim == 3)
    {
        dbscan3d(values, epsilon, min_pts);
    }
}