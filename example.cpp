#include <cstddef>
#include <cstdio>
#include <iostream>
#include <string>
#include <vector>
#include <utility>
#include <fstream>
#include <charconv>
#include <cassert>


auto dbscan(const std::vector<std::pair<float, float>>& data, float eps, int min_pts) -> std::vector<std::vector<size_t>>;


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


auto read_pair(const std::string_view& line, int line_counter)
{
    auto res = std::pair<double, double>();

    auto [ptr1, ec1] = std::from_chars(&line[0], &line[line.size()], res.first);
    // make sure there is space between the numbers
    ec1 = (ptr1 == &line[line.size()] || *ptr1 != ',')? std::errc::invalid_argument: ec1;
    ptr1++;
    check_from_chars_error(ec1, line, line_counter);


    auto [ptr2, ec2] = std::from_chars(ptr1, &line[line.size()], res.second);
    check_from_chars_error(ec2, line, line_counter);

    return res;
}


auto read_points(const std::string& filename)
{
    std::ifstream file(filename);

    if(not file.good())
    {
        std::perror(filename.c_str());
        std::exit(2);
    }

    auto count = 0;

    auto points = std::vector<std::pair<float, float>>();

    while(not file.eof())
    {
        count++;
        auto line = std::string();
        std::getline(file, line);

        if(not line.empty())
        {
            auto pair = read_pair(line, count);
            points.push_back(pair);
        }
    }

    return points;
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
auto flatten(const std::vector<std::vector<size_t>>& clusters, size_t n)
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



int main(int argc, char** argv)
{
    if(argc != 4)
    {
        std::cerr << "usage: example <tsv file> <epsilon> <min points>\n";
        return 1;
    }

    auto epsilon  = to_num<float>(argv[2]);
    auto min_pts  = to_num<int>  (argv[3]);
    auto points   = read_points(argv[1]);
    
    auto clusters = dbscan(points, epsilon, min_pts);
    auto flat     = flatten(clusters, points.size());

    for(size_t i = 0; i < points.size(); i++)
    {
        std::cout << points[i].first << ',' << points[i].second << ',' << flat[i] << '\n';
    }
}