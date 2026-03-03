#include "PolynomialMap.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <cassert>
#include <cmath>

using namespace std;

PolynomialMap::PolynomialMap(const PolynomialMap& other) {
	m_Polynomial.clear();
    m_Polynomial.insert(other.m_Polynomial.begin(), other.m_Polynomial.end());
    compress();
}

PolynomialMap::PolynomialMap(const string& file) {
	m_Polynomial.clear();
    if(!ReadFromFile(file)){
        throw std::runtime_error("Cannot read from file");
    }
    compress();
}

PolynomialMap::PolynomialMap(const double* cof, const int* deg, int n) {
	m_Polynomial.clear();
	for(int i = 0; i < n; i++){
        m_Polynomial[deg[i]] += cof[i];
    }
    compress();
}

PolynomialMap::PolynomialMap(const vector<int>& deg, const vector<double>& cof) {
	m_Polynomial.clear();
	assert(deg.size() == cof.size());
	int n = deg.size();
    for(int i = 0; i < n; i++){
        m_Polynomial[deg[i]] += cof[i];
    }
    compress();
}

double PolynomialMap::coff(int i) const {
	auto it = m_Polynomial.find(i);
    return it != m_Polynomial.end() ? it->second : 0.0; // you should return a correct value
}

double& PolynomialMap::coff(int i) {
	return m_Polynomial[i]; // you should return a correct value
}

void PolynomialMap::compress() {
	auto it = m_Polynomial.begin();
	while(it != m_Polynomial.end()){
		if(it->second == 0){
			it = m_Polynomial.erase(it);
		}
		else{
			it++;
		}
	}
}

PolynomialMap PolynomialMap::operator+(const PolynomialMap& right) const {
	PolynomialMap t;
	for(auto it = m_Polynomial.begin(); it != m_Polynomial.end(); it++){
		t.m_Polynomial[it->first] += it->second;
	}
	for(auto it = right.m_Polynomial.begin(); it != right.m_Polynomial.end(); it++){
		t.m_Polynomial[it->first] += it->second;
	}
	t.compress();
	return t; // you should return a correct value
}

PolynomialMap PolynomialMap::operator-(const PolynomialMap& right) const {
	PolynomialMap t;
	for(auto it = m_Polynomial.begin(); it != m_Polynomial.end(); it++){
		t.m_Polynomial[it->first] += it->second;
	}
	for(auto it = right.m_Polynomial.begin(); it != right.m_Polynomial.end(); it++){
		t.m_Polynomial[it->first] += -it->second;
	}
	t.compress();
	return t; // you should return a correct value
}

PolynomialMap PolynomialMap::operator*(const PolynomialMap& right) const {
	PolynomialMap t;
    for(auto it = m_Polynomial.begin(); it != m_Polynomial.end(); it++){
        for(auto right_it = right.m_Polynomial.begin(); right_it != right.m_Polynomial.end(); right_it++){
            int deg = it->first + right_it->first;
            double cof = it->second * right_it->second;
            t.m_Polynomial[deg] += cof;
        }
    }
    t.compress();
	return t; // you should return a correct value
}

PolynomialMap& PolynomialMap::operator=(const PolynomialMap& right) {
	if(this != &right){
        m_Polynomial.clear();
        m_Polynomial.insert(right.m_Polynomial.begin(), right.m_Polynomial.end());
    }
	return *this;
}

void PolynomialMap::Print() const {
	for(auto it = m_Polynomial.rbegin(); it != m_Polynomial.rend(); it++){
        if(it != m_Polynomial.rbegin() && it->second > 0){
            std::cout<<"+";
        }
        if(it->second != 0){
            std::cout<<it->second<<"x^"<<it->first;
        }
    }
    std::cout<<std::endl<<"-------------------------"<<std::endl;
}

bool PolynomialMap::ReadFromFile(const string& file) {
	std::ifstream data(file);
    if(!data.is_open()){
        throw std::runtime_error("Cannot open " + file);
        return false;
    }

    char flag = 'F';
    int term_num = 0;
    std::string line;
    if(!std::getline(data, line)){
        throw std::runtime_error("Cannot get the first line");
        return false;
    }

    std::stringstream firstline(line);
    firstline>>flag>>term_num;
    if(!(flag == 'P')){
        throw std::invalid_argument("First letter must be 'P'");
        return false;
    }
    for(int i = 0; i < term_num; i++){
        if(!std::getline(data, line)){
            throw std::runtime_error("Cannot get the " + std::to_string(i + 2) + "th line");
            return false;
        }
        int deg;
        double cof;
        std::stringstream ss(line);
        ss>>deg>>cof;
        m_Polynomial[deg] += cof;
    }
    return true; // you should return a correct value
}
