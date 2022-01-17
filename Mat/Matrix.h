#ifndef __MATRIX_H__
#define __MATRIX_H__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#define TINY			1.0e-20
#include <assert.h>
#include <stdio.h>
#include "Mat.h"
class CMatrix  
{
public:
	CMatrix();
	CMatrix(int nRows, int nCols);
	virtual ~CMatrix();
	void Inversed(void);
	double *operator[](int i){	return m_pData +i*m_nx;}
	const double *operator[](int i) const { return m_pData +i*m_nx;}
	int LUDecomposition(double *A, int nx, int *indx);
	int SingularValueDecomposition(CMatrix &A, double *W, CMatrix &V);
	double CombineParam(double a, double b);
	double Square(double a);
	double SignfollowParam(double a, double b);
	void DebugInfo(const char *pFormat);
	double * m_pData;
	int		m_nx, m_ny;
};
bool DLTtoH(float pRefXYList[][2], float pDebugXYList[][2], int nNum, CMatrix &H);
#endif 
