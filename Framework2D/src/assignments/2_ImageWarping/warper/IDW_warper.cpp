#include "IDW_warper.h"
#include "Eigen/Dense"

#include "iostream"
#include "vector"

#define N 100

namespace USTC_CG
{
    std::pair<int, int> IDWWarper::warp(int x, int y) const
    {
        Eigen::Vector2d p(x, y), q(0, 0);
        for (int i = 0; i < point_num; i++){
            q += weight(p, i) * f(p, i);
        }
        return {q[0], q[1]};
    }

    void IDWWarper::update_IDW()
    {
        Eigen::Matrix2d A, B;
        for (int i = 0; i < point_num; i++){
            A.setZero();
            B.setZero();
            Eigen::Vector2d pi = m_point_p[i], qi = m_point_q[i];
            for (int j = 0; j < point_num; j++){
                if (j != i){
                    Eigen::Vector2d pj = m_point_p[j], qj = m_point_q[j];
                    double sigma = Sigma(pj, i) * N;
                    A += sigma * ((pj - pi) * (pj - pi).transpose());
                    B += sigma * ((qj - qi) * (pj - pi).transpose());
                }
            }
            if (B.isZero()){
                T.push_back(Eigen::Matrix2d::Identity());
            }
            else{
                T.push_back(A * B.inverse());
            }
        }
    }

    double IDWWarper::Sigma(Eigen::Vector2d p, int i) const
    {
        double result = 1 / (p - m_point_p[i]).squaredNorm();
        return result;
    }

    double IDWWarper::weight(Eigen::Vector2d p, int i) const
    {
        double result;
        if (p == m_point_p[i]){
            result = 1;
        }
        else{
            double denom = 0;
            for (int i = 0; i < point_num; i++){
                denom += Sigma(p, i);
            }
            result = Sigma(p, i) / denom;
        }
        return result;
    }

    Eigen::Vector2d IDWWarper::f(Eigen::Vector2d p, int i) const
    {
        Eigen::Vector2d result;
        result = m_point_q[i] + T[i] * (p - m_point_p[i]);
        return result;
    }
}  // namespace USTC_CG