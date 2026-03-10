#include "RBF_warper.h"
#include "Eigen/Dense"

#include <iostream>
#include <vector>
#include <cmath>

namespace USTC_CG
{
    std::pair<int, int> RBFWarper::warp(int x, int y) const
    {
        Eigen::Vector2d p(x, y), q(0, 0);
        for (int i = 0; i < point_num; i++){
            double d = (p - m_point_p[i]).norm();
            q += alpha[i] * g(d, i);
        }
        q += p;
        return {q[0], q[1]};
    }

    void RBFWarper::update_RBF()
    {
        for (int i = 0; i < point_num; i++){
            double min = INFINITY;
            for (int j = 0; j < point_num; j++){
                if (j != i){
                    double d = (m_point_p[i] - m_point_p[j]).norm();
                    if (min > d){
                        min = d;
                    }
                }
            }
            r.push_back(min);
        }

        Eigen::MatrixXd G(point_num, point_num);
        Eigen::MatrixXd t(point_num, 2);
        Eigen::MatrixXd a(point_num, 2);
        for (int j = 0; j < point_num; j++){
            for (int i = 0; i < point_num; i++){
                double d = (m_point_p[j] - m_point_p[i]).norm();
                G(j, i) = g(d, i);
            }
            t(j, 0) = m_point_q[j](0) - m_point_p[j](0);
            t(j, 1) = m_point_q[j](1) - m_point_p[j](1);
        }
        a = G.inverse() * t;
        for (int i = 0; i < point_num; i++){
            Eigen::Vector2d alpha_i(a(i, 0), a(i, 1));
            alpha.push_back(alpha_i);
        }
    }

    Eigen::Vector2d RBFWarper::R(Eigen::Vector2d p) const
    {
        Eigen::Vector2d result = Eigen::Vector2d::Zero();
        for (int i = 0; i < point_num; i++){
            double d = (p - m_point_p[i]).norm();
            result += alpha[i] * g(d, i);
        }
        return result;
    }

    double RBFWarper::g(double d, int i) const
    {
        double result;
        result = pow(pow(d, 2) + pow(r[i], 2), -0.5);
        return result;
    }
}  // namespace USTC_CG