#pragma once

#include "warper.h"
#include "Eigen/Dense"
#include "annoy/annoylib.h"
#include "annoy/kissrandom.h"

#include <iostream>
#include <vector>

namespace USTC_CG
{
class HolefillWarper : public Warper
{
   public:
    HolefillWarper() = default;
    virtual ~HolefillWarper() = default;

    std::pair<int, int> warp(int x, int y) const {return {x, y};};
    // Fill the hole
    void Holefill(Image& image);
    // Update filled points and hole points
    void update_holefill(const Image& image, const Eigen::MatrixXi mask);

   private:
    int n_trees = 50;
    int n_neighbors = 5;
    float radio = 3;
    float epsilon = 0.001;

    Annoy::AnnoyIndex<int, float, Annoy::Euclidean, Annoy::Kiss64Random, 
                        Annoy::AnnoyIndexSingleThreadedBuildPolicy> index{2};
    std::vector<Eigen::Vector2d> known_pixel;
    std::vector<Eigen::Vector2d> hole_pixel;

   private:
    // Set index
    void buildIndex();
    // Find the nearest point of (x, y)
    std::pair<std::vector<int>, std::vector<float>> 
    findNearest(int x, int y, int k) const;
    // Compute the color
    std::vector<unsigned char> 
    interpolate(std::vector<int> neighbors, 
        std::vector<float> dists, const Image& image) const;
    // Compute the weight of each point
    std::vector<float> compute_weight(std::vector<float> dists) const;
};
}  // namespace USTC_CG