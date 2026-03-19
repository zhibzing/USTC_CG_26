#pragma once

#include "warper.h"
#include "dlib/dnn.h"
#include "dlib/statistics.h"
#include "dlib/matrix.h"

#include <iostream>
#include <vector>

namespace USTC_CG
{
class NeuralWarper : public Warper
{
   public:
    NeuralWarper() = default;
    virtual ~NeuralWarper() = default;
    // Implement the warp(...) function by learning
    std::pair<int, int> warp(int x, int y) const override;

    void update_neural();
   
   private:
    using warping_net = dlib::loss_mean_squared_multioutput<    // loss mean squared
                        dlib::fc<2,                             // output layer: 1 dim
                        dlib::relu<dlib::fc<10,                 // hidden layer 2: 10 dim + ReLU activation
                        dlib::relu<dlib::fc<10,                 // hidden layer 1: 10 dim + ReLU activation
                        dlib::input<dlib::matrix<float>>        // input layer: 1 dim
                        >>>>>>;
    
    void normalize();
    void train_network();

   private:
    std::vector<dlib::matrix<float>> test_input, test_output;
    dlib::vector_normalizer<dlib::matrix<float>> input_normalizer;
    dlib::vector_normalizer<dlib::matrix<float>> output_normalizer;
    dlib::matrix<float> output_means, output_stds;
    mutable warping_net net;
};
}  // namespace USTC_CG