#include "Holefill.h"
#include "Eigen/Dense"
#include "annoy/annoylib.h"
#include "annoy/kissrandom.h"

#include <iostream>
#include <vector>

namespace USTC_CG
{
    void HolefillWarper::Holefill(Image& image)
    {
        for (size_t i = 0; i < hole_pixel.size(); i++){
            int x = (int)hole_pixel[i].x();
            int y = (int)hole_pixel[i].y();

            std::vector<unsigned char> color;

            auto[neighbors, dists] = findNearest(x, y, n_neighbors);
            color = interpolate(neighbors, dists, image);
            image.set_pixel(x, y, color);
        }
    }

    void HolefillWarper::update_holefill(const Image& image, Eigen::MatrixXi mask)
    {
        int width = image.width();
        int height = image.height();
        for (int j = 0; j < height; j++){
            for (int i = 0; i < width; i++){
                if (mask(j, i) == 0){
                    hole_pixel.push_back(Eigen::Vector2d(i, j));
                }
                else{
                    known_pixel.push_back(Eigen::Vector2d(i, j));
                }
            }
        }
        buildIndex();
    }

    void HolefillWarper::buildIndex()
    {
        if (known_pixel.empty())
            return;

        for (int i = 0; i < known_pixel.size(); i++){
            const auto& p = known_pixel[i];
            float pixel[2] = {(float)p.x(), (float)p.y()};
            index.add_item(i, pixel);
        }

        index.build(n_trees);
    }

    std::pair<std::vector<int>, std::vector<float>> 
    HolefillWarper::findNearest(int x, int y, int k) const
    {
        float query[2] = {(float)x, (float)y};
        std::vector<int> neighbors;
        std::vector<float> dists;

        index.get_nns_by_vector(query, k, -1, &neighbors, &dists);

        return {neighbors, dists};
    }

    std::vector<unsigned char> 
    HolefillWarper::interpolate(std::vector<int> neighbors, 
        std::vector<float> dists, const Image& image) const
    {
        std::vector<unsigned char> color(3, 0);
        std::vector<float> weight = compute_weight(dists);

        for (int i = 0; i < n_neighbors; i++){
            Eigen::Vector2d point = known_pixel[neighbors[i]];
            std::vector<unsigned char> pixel = image.get_pixel(point.x(), point.y());
            for (int k = 0; k < 3; k++){
                color[k] += weight[i] * pixel[k];
            }
        }

        return color;
    }

    std::vector<float> 
    HolefillWarper::compute_weight(std::vector<float> dists) const
    {
        std::vector<float> weight;
        float total_weight = 0;
        for (int i = 0; i < dists.size(); i++){
            if (dists[i] > radio){
                weight.push_back(0);
            }
            else{
                weight.push_back(1 / (dists[i] + epsilon));
            }
            total_weight += weight[i];
        }
        if (total_weight != 0){
            for (int i = 0; i < dists.size(); i++){
                weight[i] /= total_weight;
            }
        }
        return weight;
    }
}  // namespace USTC_CG