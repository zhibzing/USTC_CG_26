// HW2_TODO: Implement the RBFWarper class
#pragma once

#include "warper.h"
#include "Eigen/Dense"

#include <iostream>
#include <vector>

namespace USTC_CG
{
class RBFWarper : public Warper
{
   public:
    RBFWarper() = default;
    virtual ~RBFWarper() = default;
    // HW2_TODO: Implement the warp(...) function with RBF interpolation
    std::pair<int, int> warp(int x, int y) const override;

    // HW2_TODO: other functions or variables if you need
    // Compute alpha_i and r_i
    void update_RBF();

   private:
    Eigen::Vector2d R(Eigen::Vector2d p) const;
    double g(double d, int i) const;

   private:
    std::vector<Eigen::Vector2d> alpha;
    std::vector<double> r;
};
}  // namespace USTC_CG