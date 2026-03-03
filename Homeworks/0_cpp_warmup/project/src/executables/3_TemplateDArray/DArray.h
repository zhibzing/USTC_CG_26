#pragma once

#include <iostream>
#define N 10

// interfaces of Dynamic Array class DArray
template <class DataType>
class DArray {
public:
	DArray(); // default constructor
	DArray(int nSize, DataType dValue = 0); // set an array with default values
	DArray(const DArray& arr); // copy constructor
	~DArray(); // deconstructor

	void Print() const; // print the elements of the array

	int GetSize() const; // get the size of the array
	void SetSize(int nSize); // set the size of the array

	const DataType& GetAt(int nIndex) const; // get an element at an index
	void SetAt(int nIndex, DataType dValue); // set the value of an element

	DataType& operator[](int nIndex); // overload operator '[]'
	const DataType& operator[](int nIndex) const; // overload operator '[]'

	void PushBack(DataType dValue); // add a new element at the end of the array
	void DeleteAt(int nIndex); // delete an element at some index
	void InsertAt(int nIndex, DataType dValue); // insert a new element at some index

	DArray& operator = (const DArray& arr); //overload operator '='

private:
	DataType* m_pData; // the pointer to the array memory
	int m_nSize; // the size of the array
	int m_nMax;

private:
	void Init(); // initilize the array
	void Free(); // free the array
	void Reserve(int nSize); // allocate enough memory
};

// default constructor
template <class DataType>
DArray<DataType>::DArray() {
	Init();
}

// set an array with default values
template <class DataType>
DArray<DataType>::DArray(int nSize, DataType dValue) {
	if(nSize < 0){
		m_nSize = 0;
	}
	else{
		m_nSize = nSize;
	}
	if(nSize >= N){
		m_nMax = 2 * nSize;
		m_pData = new DataType[m_nMax]();
	}
	else{
		m_nMax = N;
		m_pData = new DataType[N]();
	}
}

template <class DataType>
DArray<DataType>::DArray(const DArray& arr) {
	m_nMax = arr.m_nMax;
	m_nSize = arr.m_nSize;
	m_pData = new DataType[m_nMax]();
	for(int i = 0; i < m_nSize; i++){
		m_pData[i] = arr.m_pData[i];
	}
}

// deconstructor
template <class DataType>
DArray<DataType>::~DArray() {
	Free();
}

// display the elements of the array
template <class DataType>
void DArray<DataType>::Print() const {
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
template <class DataType>
void DArray<DataType>::Init() {
	m_nMax = N;
	m_nSize = 0;
	m_pData = new DataType[N]();
}

// free the array
template <class DataType>
void DArray<DataType>::Free() {
	delete[] m_pData;
}

// get the size of the array
template <class DataType>
int DArray<DataType>::GetSize() const {
	return m_nSize; // you should return a correct value
}

// set the size of the array
template <class DataType>
void DArray<DataType>::SetSize(int nSize) {
	if(nSize <= 0){
		Init();
	}
	else{
		if(nSize > m_nMax){
			m_nMax = 2 * nSize;
			DataType* new_pData = new DataType[m_nMax];
			for(int i = 0; i < m_nSize; i++){
				new_pData[i] = m_pData[i];
			}
			delete[] m_pData;
			m_pData = new_pData;
		}
		for(int i = nSize; i < m_nSize; i++){
			m_pData[i] = 0;
		}
		m_nSize = nSize;
	}
}

// get an element at an index
template <class DataType>
const DataType& DArray<DataType>::GetAt(int nIndex) const {
	if(nIndex < 0 || nIndex >= m_nSize){
		throw std::out_of_range("Index out of range");	
	}
	return m_pData[nIndex]; // you should return a correct value
}

// set the value of an element 
template <class DataType>
void DArray<DataType>::SetAt(int nIndex, DataType dValue) {
	if(nIndex < 0 || nIndex >= m_nSize){
		throw std::out_of_range("Index out of range");	
	}
	m_pData[nIndex] = dValue;
}

// overload operator '[]'
template <class DataType>
const DataType& DArray<DataType>::operator[](int nIndex) const {
	if(nIndex < 0 || nIndex >= m_nSize){
		throw std::out_of_range("Index out of range");	
	}
	return m_pData[nIndex]; // you should return a correct value
}

// add a new element at the end of the array
template <class DataType>
void DArray<DataType>::PushBack(DataType dValue) {
	m_pData[m_nSize] = dValue;
	m_nSize++;
	if(m_nSize == m_nMax){
		m_nMax = 2 * m_nMax;
		DataType* new_pData = new DataType[m_nMax]();
		for(int i = 0; i < m_nSize; i++){
			new_pData[i] = m_pData[i];
		}
		delete[] m_pData;
		m_pData = new_pData;
	}
}

// delete an element at some index
template <class DataType>
void DArray<DataType>::DeleteAt(int nIndex) {
	if(nIndex < 0 || nIndex >= m_nSize){
		throw std::out_of_range("Index out of range");	
	}
	for(int i = nIndex; i < m_nSize; i++){
		m_pData[i] = m_pData[i + 1];
	}
	m_nSize--;
}

// insert a new element at some index
template <class DataType>
void DArray<DataType>::InsertAt(int nIndex, DataType dValue) {
	if(nIndex < 0 || (nIndex >= m_nSize && m_nSize > 0) || (nIndex > m_nSize && m_nSize == 0)){
		throw std::out_of_range("Index out of range");
	}
	for(int i = m_nSize; i > nIndex; i--){
		m_pData[i] = m_pData[i - 1];
	}
	m_pData[nIndex] = dValue;
	m_nSize++;
	if(m_nSize == m_nMax){
		m_nMax = 2 * m_nMax;
		DataType* new_pData = new DataType[m_nMax]();
		for(int i = 0; i < m_nSize; i++){
			new_pData[i] = m_pData[i];
		}
		delete[] m_pData;
		m_pData = new_pData;
	}
}

// overload operator '='
template <class DataType>
DArray<DataType>& DArray<DataType>::operator = (const DArray& arr) {
	if(this == &arr){
		return *this;
	}
	m_nMax = arr.m_nMax;
	m_nSize = arr.m_nSize;
	m_pData = new DataType[m_nMax]();
	for(int i = 0; i < m_nSize; i++){
		m_pData[i] = arr.m_pData[i];
	}
	return *this;
}
