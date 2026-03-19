#include "Neural_warper.h"
#include "dlib/dnn.h"
#include "dlib/matrix.h"

#include <iostream>
#include <vector>

namespace USTC_CG
{
    std::pair<int, int> NeuralWarper::warp(int x, int y) const
    {
        dlib::matrix<float> input(2, 1), output(2, 1);
        input(0, 0) = x;
        input(1, 0) = y;
        input = input_normalizer(input);
        output = net(input);
        output(0, 0) = output(0, 0) / output_stds(0, 0) + output_means(0, 0);
        output(1, 0) = output(1, 0) / output_stds(1, 0) + output_means(1, 0);

        return {(int)output(0, 0), (int)output(1, 0)};
    }

    void NeuralWarper::update_neural()
    {
        dlib::matrix<float> input(2, 1);
        dlib::matrix<float> output(2, 1);

        for (int i = 0; i < point_num; i++){
            input(0, 0) = m_point_p[i](0);
            input(1, 0) = m_point_p[i](1);
            output(0, 0) = m_point_q[i](0);
            output(1, 0) = m_point_q[i](1);

            test_input.push_back(input);
            test_output.push_back(output);
        }

        normalize();
        train_network();
    }

    void NeuralWarper::normalize()
    {
        input_normalizer.train(test_input);
        for (auto& x : test_input) x = input_normalizer(x);
        output_normalizer.train(test_output);
        output_means = output_normalizer.means();
        output_stds = output_normalizer.std_devs();
        for (auto& x : test_output) x = output_normalizer(x);
    }

    void NeuralWarper::train_network()
    {
        dlib::adam solver(0.002, 0.9, 0.999);
        dlib::dnn_trainer<warping_net, dlib::adam> trainer(net, solver);
        
        trainer.set_learning_rate(0.001);
        trainer.set_min_learning_rate(1e-6);
        trainer.set_mini_batch_size(128);
        trainer.set_learning_rate_shrink_factor(0.1);
        trainer.set_iterations_without_progress_threshold(500);
        trainer.be_verbose();
        std::cout << "Starting training..." << std::endl;
        trainer.train(test_input, test_output);
    }
}  // namespace USTC_CG