#include "network.hpp"
#include <iostream>
#include "translate.hpp"


using namespace Eigen;
using std::cout;
using std::endl; 
using std::vector;


Network::Network(const vector<int> &layerStructure, double learningRate, vector<int> activationFuncCodes) {
    _size = static_cast<int>(layerStructure.size());
    _layerStructure = layerStructure;
    _layers.push_back(Layer(layerStructure[0], 1, learningRate, activationFuncCodes[0]));
    for (int i = 1; i < _size; ++i) {
        _layers.push_back(Layer(layerStructure[i], layerStructure[i-1], learningRate, activationFuncCodes[i]));
    }
    _layers[0].setAsInputLayer();
    _layers[_size-1].setAsOutputLayer();
}


vector<double> Network::outputs(vector<double> &input) {
    VectorXd in = toEigenVec(input.size(), input);
    _outputs(in);
    return toSTDVec(_layers[_size-1].outputCache());
}


inline void Network::_outputs(const VectorXd &input) {
    _layers[0].outputs(input);
    for (int i = 1; i < _size; ++i) {
        _layers[i].outputs(_layers[i-1].outputCache());
    }
}


void Network::_update(const VectorXd &expected) {
    VectorXd res = expected;
    for (int i = _size-1; i > 0; --i) {
        res = _layers[i].calcDelta(res);
        _layers[i].update(_layers[i-1].outputCache());
    }
}


inline void Network::_train(size_t inputs_size, const vector<VectorXd> &inputs, const vector<VectorXd> &expecteds) {
    for (int i = 0; i < inputs_size; ++i) {
        _outputs(inputs[i]);
        _update(expecteds[i]);
    }
}


void Network::trainForEpoch(vector<vector<double>> &inputs, vector<vector<double>> &expecteds, int epoch) {
    vector<VectorXd> vinputs = toEigenVecs(inputs);
    vector<VectorXd> vexpecteds = toEigenVecs(expecteds);
    size_t size = vinputs.size();
    for (int i = 0; i < epoch; i++) {
        _train(size, vinputs, vexpecteds);
    }
}


NetworkInfo* Network::dump() {
    NetworkInfo* info = new NetworkInfo();
    for (int i = 1; i < _size; i++) {
        info->weights.emplace_back(_layers[i].getWeight());
        info->biases.emplace_back(_layers[i].getBias());
    }
    return info;
}


void Network::load(NetworkInfo* info) {
    for (int i = 1; i < _size; i++) {
        _layers[i].setWeight(info->weights[i-1]);
        _layers[i].setBias(info->biases[i-1]);
    }
}


void Network::printStatus() {
    for (int i = 1; i < _size; ++i) {
        cout << "\n<Layer" << i+1 << ">" << endl;
        _layers[i].printStatus();
    }
}
