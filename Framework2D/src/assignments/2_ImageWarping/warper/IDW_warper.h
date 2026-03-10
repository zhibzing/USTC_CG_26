// HW2_TODO: Implement the IDWWarper class
#pragma once

#include "warper.h"
#include "Eigen/Dense"

#include <iostream>
#include <vector>

namespace USTC_CG
{
class IDWWarper : public Warper
{
   public:
    IDWWarper() = default;
    virtual ~IDWWarper() = default;
    // HW2_TODO: Implement the warp(...) function with IDW interpolation
    std::pair<int, int> warp(int x, int y) const override;

    // HW2_TODO: other functions or variables if you need
    // Compute T_i
    void update_IDW();

   private:
    double Sigma(Eigen::Vector2d p, int i) const;
    double weight(Eigen::Vector2d p, int i) const;
    Eigen::Vector2d f(Eigen::Vector2d p, int i) const;

   private:
    std::vector<Eigen::Matrix2d> T;
};
}  // namespace USTC_CG