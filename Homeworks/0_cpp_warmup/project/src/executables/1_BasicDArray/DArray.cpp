// implementation of class DArray
#include "DArray.h"
#include <iostream>

// default constructor
DArray::DArray() {
	Init();
}

// set an array with default values
DArray::DArray(int nSize, double dValue) {
	if(nSize > 0){
		m_pData = new double[nSize];
		m_nSize = nSize;
		for(int i = 0; i < m_nSize; i++){
			m_pData[i] = dValue;
		}
	}
	else{
		m_pData = nullptr;
		m_nSize = 0;
	}
}

DArray::DArray(const DArray& arr) {
	m_nSize = arr.m_nSize;
	if(arr.m_nSize > 0){
		m_pData = new double[m_nSize];
		for(int i = 0; i < m_nSize; i++){
			m_pData[i] = arr.m_pData[i];
		}
	}
	else{
		m_pData = nullptr;
	}
}

// deconstructor
DArray::~DArray() {
	Free();
}

// display the elements of the array
void DArray::Print() const {
	if(m_nSize == 0){
		std::cout<<"The array is NULL."<<std::endl;
	}
	else{
		for(int i = 0; i < m_nSize; i++){
			std::cout<<i<<":"<<m_pData[i]<<std::endl;
		}
	}
	std::cout<<"----------------"<<std::endl;
}

// initilize the array
void DArray::Init() {
	m_pData = nullptr;
	m_nSize = 0;
}

// free the array
void DArray::Free() {
	if(m_pData != nullptr){
		delete[] m_pData;
		m_pData = nullptr;
	}
	m_nSize = 0;
}

// get the size of the array
int DArray::GetSize() const {
	return m_nSize; // you should return a correct value
}

// set the size of the array
void DArray::SetSize(int nSize) {
	if(nSize <= 0){
		Free();
	}
	else{
		int copy_nSize = (m_nSize < nSize) ? m_nSize : nSize;
		double* new_pData = new double[nSize]();
		for(int i = 0; i < copy_nSize; i++){
			new_pData[i] = m_pData[i];
		}
		delete[] m_pData;
		m_pData = new_pData;
		m_nSize = nSize;
	}
}

// get an element at an index
const double& DArray::GetAt(int nIndex) const {
	if(nIndex < 0 || nIndex >= m_nSize){
		throw std::out_of_range("Index out of range");	
	}
	return m_pData[nIndex]; // you should return a correct value
}

// set the value of an element 
void DArray::SetAt(int nIndex, double dValue) {
	if(nIndex < 0 || nIndex >= m_nSize){
		throw std::out_of_range("Index out of range");	
	}
	m_pData[nIndex] = dValue;
}

// overload operator '[]'
const double& DArray::operator[](int nIndex) const {
	if(nIndex < 0 || nIndex >= m_nSize){
		throw std::out_of_range("Index out of range");	
	}
	return m_pData[nIndex]; // you should return a correct value
}

// add a new element at the end of the array
void DArray::PushBack(double dValue) {
	double* new_pData = new double[m_nSize + 1];
	for(int i = 0; i < m_nSize; i++){
		new_pData[i] = m_pData[i];
	}
	new_pData[m_nSize] = dValue;
	delete[] m_pData;
	m_pData = new_pData;
	m_nSize++;
}

// delete an element at some index
void DArray::DeleteAt(int nIndex) {
	if(nIndex < 0 || nIndex >= m_nSize){
		throw std::out_of_range("Index out of range");	
	}
	if(m_nSize == 1){
		Free();
		return;
	}
	double* new_pData = new double[m_nSize - 1];
	for(int i = 0; i < nIndex; i++){
		new_pData[i] = m_pData[i];
	}
	for(int i = nIndex; i < m_nSize - 1; i++){
		new_pData[i] = m_pData[i + 1];
	}
	delete[] m_pData;
	m_pData = new_pData;
	m_nSize--;
}

// insert a new element at some index
void DArray::InsertAt(int nIndex, double dValue) {
	if(m_nSize == 0){
		if(nIndex != 0){
			throw std::out_of_range("Index out of range");
		}
		m_pData = new double[1];
		m_pData[0] = dValue;
		m_nSize = 1;
	}
	else{
		if(nIndex < 0 || nIndex >= m_nSize){
			throw std::out_of_range("Index out of range");	
		}
		double* new_pData = new double[m_nSize + 1];
		for(int i = 0; i < nIndex; i++){
			new_pData[i] = m_pData[i];
		}
		new_pData[nIndex] = dValue;
		for(int i = nIndex + 1; i < m_nSize + 1; i++){
			new_pData[i] = m_pData[i - 1];
		}
		delete[] m_pData;
		m_pData = new_pData;
		m_nSize++;
	}
}

// overload operator '='
DArray& DArray::operator = (const DArray& arr) {
	if(this == &arr){
		return *this;
	}
	Free();
	m_nSize = arr.m_nSize;
	if(arr.m_nSize > 0){
		m_pData = new double[m_nSize];
		for(int i = 0; i < m_nSize; i++){
			m_pData[i] = arr.m_pData[i];
		}
	}
	else{
		m_pData = nullptr;
	}
	return *this;
}
