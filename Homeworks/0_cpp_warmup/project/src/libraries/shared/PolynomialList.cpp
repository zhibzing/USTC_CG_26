#include "PolynomialList.h"

#include <iostream>
#include <iterator>
#include <fstream>
#include <sstream>

using namespace std;

PolynomialList::PolynomialList(const PolynomialList& other) {
    m_Polynomial.clear();
    m_Polynomial.insert(m_Polynomial.begin(), other.m_Polynomial.begin(), other.m_Polynomial.end());
    compress();
}

PolynomialList::PolynomialList(const string& file) {
    m_Polynomial.clear();
    if(!ReadFromFile(file)){
        throw std::runtime_error("Cannot read from file");
    }
    compress();
}

PolynomialList::PolynomialList(const double* cof, const int* deg, int n) {
    m_Polynomial.clear();
    for(int i = 0; i < n; i++){
        m_Polynomial.push_back(Term(deg[i], cof[i]));
    }
    compress();
}

PolynomialList::PolynomialList(const vector<int>& deg, const vector<double>& cof) {
    m_Polynomial.clear();
    int n = deg.size();
    for(int i = 0; i < n; i++){
        m_Polynomial.push_back(Term(deg[i], cof[i]));
    }
    compress();
}

double PolynomialList::coff(int i) const {
    if(i < 0 || i >= m_Polynomial.size()){
        throw std::out_of_range("Intex out of range");
    }
    auto it = m_Polynomial.begin();
    std::advance(it, i);
    return it->cof; // you should return a correct value
}

double& PolynomialList::coff(int i) {
    if(i < 0 || i >= m_Polynomial.size()){
        throw std::out_of_range("Intex out of range");
    }
    auto it = m_Polynomial.begin();
    std::advance(it, i);
    return it->cof; // you should return a correct value
}

void PolynomialList::compress() {
    m_Polynomial.sort([](const Term& a, const Term& b){
        return a.deg > b.deg;
    });

    auto it = m_Polynomial.begin();
    while(it != m_Polynomial.end()){
        auto next = it;
        next++;

        if(next != m_Polynomial.end() && next->deg == it->deg){
            it->cof += next->cof;
            m_Polynomial.erase(next);
        }
        else{
            it++;
        }
    }
}

PolynomialList PolynomialList::operator+(const PolynomialList& right) const {
    PolynomialList t;
    t.m_Polynomial.insert(t.m_Polynomial.end(), m_Polynomial.begin(), m_Polynomial.end());
    t.m_Polynomial.insert(t.m_Polynomial.end(), right.m_Polynomial.begin(), right.m_Polynomial.end());
    t.compress();
    return t; // you should return a correct value
}

PolynomialList PolynomialList::operator-(const PolynomialList& right) const {
    PolynomialList t;
    size_t originalSize = m_Polynomial.size();
    t.m_Polynomial.insert(t.m_Polynomial.end(), m_Polynomial.begin(), m_Polynomial.end());
    t.m_Polynomial.insert(t.m_Polynomial.end(), right.m_Polynomial.begin(), right.m_Polynomial.end());
    auto it = t.m_Polynomial.begin();
    std::advance(it, originalSize);
    for(; it != t.m_Polynomial.end(); it++){
        it->cof *= -1;
    }
    t.compress();
    return t; // you should return a correct value
}

PolynomialList PolynomialList::operator*(const PolynomialList& right) const {
    PolynomialList t;
    for(auto it = m_Polynomial.begin(); it != m_Polynomial.end(); it++){
        for(auto right_it = right.m_Polynomial.begin(); right_it != right.m_Polynomial.end(); right_it++){
            int deg = it->deg + right_it->deg;
            double cof = it->cof * right_it->cof;
            t.m_Polynomial.push_back(Term(deg, cof));
        }
    }
    t.compress();
    return t; // you should return a correct value
}

PolynomialList& PolynomialList::operator=(const PolynomialList& right) {
    if(this != &right){
        m_Polynomial.clear();
        m_Polynomial.insert(m_Polynomial.begin(), right.m_Polynomial.begin(), right.m_Polynomial.end());
    }
    return *this;
}

void PolynomialList::Print() const {
    for(auto it = m_Polynomial.begin(); it != m_Polynomial.end(); it++){
        if(it != m_Polynomial.begin() && it->cof > 0){
            std::cout<<"+";
        }
        if(it->cof != 0){
            std::cout<<it->cof<<"x^"<<it->deg;
        }
    }
    std::cout<<std::endl<<"-------------------------"<<std::endl;
}

bool PolynomialList::ReadFromFile(const string& file) {
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
        int m_deg;
        double m_cof;
        std::stringstream ss(line);
        ss>>m_deg>>m_cof;
        m_Polynomial.push_back(Term(m_deg, m_cof));
    }
    return true; // you should return a correct value
}

PolynomialList::Term& PolynomialList::AddOneTerm(const Term& term) {
    m_Polynomial.push_back(term);
    compress();
    for(auto it = m_Polynomial.begin(); it != m_Polynomial.end(); it++){
        if(it->deg == term.deg){
            return *it;
        }
    }
    throw std::runtime_error("Cannot find term with deg:" + term.deg);
}
