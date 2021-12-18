#include "Matrix.h"
CMatrix::CMatrix()
{
	m_pData = NULL;
	m_nx = m_ny = 0;
}
CMatrix::CMatrix(int ny, int nx)
{
	m_pData = NULL;
	if (m_pData)
	{
		delete[]m_pData;
	}
	m_nx = nx;
	m_ny = ny;
	m_pData = new double[ny * nx];
}
CMatrix::~CMatrix()
{
	if (m_pData)
	{
		delete []m_pData;
	}
}
void CMatrix::Inversed(void)
{
	int *indx = new int [m_ny];
	double *lineCol = new double[m_ny];
	double *matrixLU = new double[m_ny*m_nx];
	LUDecomposition(m_pData, m_nx, indx);
	for (int j = 0; j < m_nx; j++)
	{
		for (int i = 0; i < m_ny; i++)
			lineCol[i] = 0;
		lineCol[j] = 1;
		int index = -1;
		for (int ii = 0; ii < m_nx; ii++)
		{
			int ip = indx[ii];
			double sum = lineCol[ip];
			lineCol[ip] = lineCol[ii];
			if (index >= 0)
				for (int jj = index; jj <= ii - 1; jj++)
					sum -= m_pData[ii*m_nx + jj] * lineCol[jj];
			else
				if (sum) index = ii;
			lineCol[ii] = sum;
		}
		for (int ii = m_nx - 1; ii >= 0; ii--)
		{
			double sum = lineCol[ii];
			for (int jj = ii + 1; jj < m_nx; jj++)
				sum -= m_pData[ii*m_nx + jj] * lineCol[jj];
			lineCol[ii] = sum / m_pData[ii*m_nx + ii];
		}
		for (int i = 0; i < m_ny; i++)
			matrixLU[i*m_nx + j] = lineCol[i];
	}
	for (int j = 0; j < m_nx; j++)
	{
		for (int i = 0; i < m_ny; i++)
		{
			m_pData[i*m_nx + j]=matrixLU[i*m_nx + j];
		}
	}
	delete []matrixLU;
	delete []lineCol;
	delete []indx;
}
int CMatrix::LUDecomposition(double *A,int nx, int *indx)
{
	int i, j, k, imax;
	double big, dum, sum, temp;
	double *vv = new double[nx];
	for (i = 0; i < nx; i++)
	{	
		big = 0.;
		for (j = 0; j < nx; j++)
			if ((temp = fabs(A[i*m_nx + j])) > big) big = temp;
		if (big == 0.0)
		{
			return 1;
		}
		vv[i] = 1. / big;
	}
	for (j = 0; j < nx; j++)
	{	
		for (i = 0; i < j; i++)
		{
			sum = A[i*m_nx + j];
			for (k = 0; k < i; k++)
				sum -= A[i*m_nx + k] * A[k*m_nx + j];
			A[i*m_nx + j] =sum;
		}
		big = 0.;	
		for (i = j; i < nx; i++)
		{
			sum = A[i*m_nx + j];
			for (k = 0; k < j; k++)
				sum -= A[i*m_nx + k] * A[k*m_nx + j];
			A[i*m_nx + j] = sum;
			if ((dum = vv[i] * fabs(sum)) >= big)
			{
				big = dum;
				imax = i;
			}
		}
		if (j != imax)
		{	
			for (k = 0; k < nx; k++)
			{	
				dum = A[imax*m_nx + k];
				A[imax*m_nx + k] = A[j*m_nx + k];
				A[j*m_nx + k] = dum;
			}
			vv[imax] = vv[j];
		}
		indx[j] = imax;
		if (A[j*m_nx + j] == 0.0) A[j*m_nx + j] = TINY;
		if (j != nx)
		{
			dum = 1. / A[j*m_nx + j];
			for (i = j + 1; i < nx; i++) A[i*m_nx + j] *= dum;
		}
	}
	delete[]vv;
	return 0;
}
int CMatrix::SingularValueDecomposition(CMatrix &A, double *W, CMatrix &V)
{
	int i, j, k, l, flag, its, jj, nm;
	double anorm, c, f, g, h, s, scale, x, y, z;
	int m = A.m_ny;
	int n = A.m_nx;
	double *rv1=new double[n];
	g = scale = anorm = 0.0;		
	for (i = 0; i < n; i++)
	{
		l = i + 1;
		rv1[i] = scale * g;
		g = s = scale = 0.0;
		if (i < m)
		{
			for (k = i; k < m; k++)
			{
				scale += fabs(A[k][i]);
			}
			if (scale)
			{
				for (k = i; k < m; k++)
				{
					A[k][i] /= scale;
					s += A[k][i] * A[k][i];
				}
				f = A[i][i];
				g = -SignfollowParam(sqrt(s), f);
				h = f * g - s;
				A[i][i] = f - g;
				for (j = l; j < n; j++)
				{
					for (s = 0.0, k = i; k < m; k++)
						s += A[k][i] * A[k][j];
					f = s / h;
					for (k = i; k < m; k++)
						A[k][j] += f * A[k][i];
				}
				for (k = i; k < m; k++)
					A[k][i] *= scale;
			}
		}
		W[i] = scale * g;
		g = s = scale = 0.0;
		if (i < m && i != n - 1)
		{
			for (k = l; k < n; k++)
			{
				scale += fabs(A[i][k]);
			}
			if (scale)
			{
				for (k = l; k < n; k++)
				{
					A[i][k] /= scale;
					s += A[i][k] * A[i][k];
				}
				f = A[i][l];
				g = -SignfollowParam(sqrt(s), f);
				h = f * g - s;
				A[i][l] = f - g;
				for (k = l; k < n; k++)
					rv1[k] = A[i][k] / h;
				for (j = l; j < m; j++)
				{
					for (s = 0.0, k = l; k < n; k++)
						s += A[j][k] * A[i][k];
					for (k = l; k < n; k++)
						A[j][k] += s * rv1[k];
				}
				for (k = l; k < n; k++)
					A[i][k] *= scale;
			}
		}
		if (anorm<(fabs(W[i]) + fabs(rv1[i])))
		{
			anorm = (fabs(W[i]) + fabs(rv1[i]));
		}
	}
	for (i = n - 1; i >= 0; i--)
	{		
		if (i < n - 1)
		{
			if (g)
			{
				for (j = l; j < n; j++)		
					V[j][i] = (A[i][j] / A[i][l]) / g;
				for (j = l; j < n; j++)
				{
					for (s = 0.0, k = l; k < n; k++)
						s += A[i][k] * V[k][j];
					for (k = l; k < n; k++) V[k][j] += s * V[k][i];
				}
			}
			for (j = l; j < n; j++)
				V[i][j] = V[j][i] = 0.0;
		}
		V[i][i] = 1.0;
		g = rv1[i];
		l = i;
	}
	for (i = (m < n ? m : n) - 1; i >= 0; i--)
	{		
		l = i + 1;
		g = W[i];
		for (j = l; j < n; j++)
			A[i][j] = 0.0;
		if (g)
		{
			g = 1.0 / g;
			for (j = l; j < n; j++)
			{
				for (s = 0.0, k = l; k < m; k++)
					s += A[k][i] * A[k][j];
				f = (s / A[i][i]) * g;
				for (k = i; k < m; k++)
					A[k][j] += f * A[k][i];
			}
			for (j = i; j < m; j++)
				A[j][i] *= g;
		}
		else for (j = i; j < m; j++)
			A[j][i] = 0.0;
		A[i][i]++;
	}
	for (k = n - 1; k >= 0; k--)
	{		
		for (its = 1; its <= 30; its++)
		{		
			flag = 1;
			for (l = k; l >= 0; l--)
			{		
				nm = l - 1;				
				if ((double)(fabs(rv1[l]) + anorm) == anorm)
				{
					flag = 0;
					break;
				}
				if ((double)(fabs(W[nm]) + anorm) == anorm)
					break;
			}
			if (flag)
			{
				c = 0.0;					
				s = 1.0;
				for (i = l; i <= k; i++)
				{
					f = s * rv1[i];
					rv1[i] = c * rv1[i];
					if ((double)(fabs(f) + anorm) == anorm)
						break;
					g = W[i];
					h = CombineParam(f, g);
					W[i] = h;
					h = 1.0 / h;
					c = g * h;
					s = -f * h;
					for (j = 0; j < m; j++)
					{
						y = A[j][nm];
						z = A[j][i];
						A[j][nm] = y * c + z * s;
						A[j][i] = z * c - y * s;
					}
				}
			}
			z = W[k];
			if (l == k)
			{			
				if (z < 0.0)
				{		
					W[k] = -z;
					for (j = 0; j < n; j++)
						V[j][k] = -V[j][k];
				}
				break;
			}
			if (its == 30)
			{
				return 2;
			}
			x = W[l];					
			nm = k - 1;
			y = W[nm];
			g = rv1[nm];
			h = rv1[k];
			f = ((y - z) * (y + z) + (g - h) * (g + h)) / (2.0 * h * y);
			g = CombineParam(f, 1.0);
			f = ((x - z) * (x + z) + h * ((y / (f + SignfollowParam(g, f))) - h)) / x;
			c = s = 1.0;				
			for (j = l; j <= nm; j++)
			{
				i = j + 1;
				g = rv1[i];
				y = W[i];
				h = s * g;
				g = c * g;
				z = CombineParam(f, h);
				rv1[j] = z;
				c = f / z;
				s = h / z;
				f = x * c + g * s;
				g = g * c - x * s;
				h = y * s;
				y *= c;
				for (jj = 0; jj < n; jj++)
				{
					x = V[jj][j];
					z = V[jj][i];
					V[jj][j] = x * c + z * s;
					V[jj][i] = z * c - x * s;
				}
				z = CombineParam(f, h);
				W[j] = z;				
				if (z)
				{
					z = 1.0 / z;
					c = f * z;
					s = h * z;
				}
				f = c * g + s * y;
				x = c * y - s * g;
				for (jj = 0; jj < m; jj++)
				{
					y = A[jj][j];
					z = A[jj][i];
					A[jj][j] = y * c + z * s;
					A[jj][i] = z * c - y * s;
				}
			}
			rv1[l] = 0.0;
			rv1[k] = f;
			W[k] = x;
		}
	}
	delete []rv1;
	return 0;
}
double CMatrix::CombineParam(double a, double b)
{
	double absa = fabs(a);
	double absb = fabs(b);
	if (absa > absb) 
		return absa * sqrt(1.0 + Square(absb / absa));
	else 
		return (absb == 0.0 ? 0.0 : absb * sqrt(1.0 + Square(absa / absb)));
}
double CMatrix::Square(double a)
{
	return (a == 0.0 ? 0.0 : a * a);
}
double CMatrix::SignfollowParam(double a, double b)
{
	return (b >= 0.0 ? fabs(a) : -fabs(a));
}
void CMatrix::DebugInfo(const char *pFormat)
{
	int i, j;
	for (i = 0; i < m_ny; i++)
	{
		for (j = 0; j < m_nx; j++)
		{
			printf(pFormat, m_pData[i*m_nx + j]);
		}
		printf("\n");
	}
}
bool DLTtoH(float pRefXYList[][2], float pDebugXYList[][2], int nNum, CMatrix &H)
{
	const float csqrt2 = sqrt(2);
	float AvgX1, AvgY1, AvgX2, AvgY2;
	if (nNum < 4)
	{
		printf("At least four points are required\n ");
		return false;
	}
	//***********get average  x y *******************
	AvgX1 = AvgY1 = AvgX2 = AvgY2 = 0.0;
	for (int n = 0; n < nNum; n++)
	{
		AvgX1 += pRefXYList[n][0];
		AvgY1 += pRefXYList[n][1];
		AvgX2 += pDebugXYList[n][0];
		AvgY2 += pDebugXYList[n][1];
	}
	AvgX1 /= nNum;
	AvgY1 /= nNum;
	AvgX2 /= nNum;
	AvgY2 /= nNum;
	//***********get ref debug DiffSquareAvg   *******************
	float RefDiffSquareAvg, DebugDiffSquareAvg;
	RefDiffSquareAvg = DebugDiffSquareAvg = 0.0;
	for (int n = 0; n < nNum; n++)
	{
		float fX1 = pRefXYList[n][0] - AvgX1;
		float fY1 = pRefXYList[n][1] - AvgY1;
		float fX2 = pDebugXYList[n][0] - AvgX2;
		float fY2 = pDebugXYList[n][1] - AvgY2;
		RefDiffSquareAvg += (float)sqrt(fX1 * fX1 + fY1 * fY1);
		DebugDiffSquareAvg += (float)sqrt(fX2 * fX2 + fY2 * fY2);
	}
	RefDiffSquareAvg /= nNum;
	DebugDiffSquareAvg /= nNum;
	if (RefDiffSquareAvg == 0 || DebugDiffSquareAvg == 0)
	{
		return false;
	}
	//***********init T1 T2 matrix   *******************
	CMatrix T1(3, 3);
	CMatrix T2(3, 3);
	T1[0][0] = csqrt2 / RefDiffSquareAvg;		T1[0][1] = 0.0;								T1[0][2] = -AvgX1 * csqrt2 / RefDiffSquareAvg;
	T1[1][0] = 0.0;								T1[1][1] = csqrt2 / RefDiffSquareAvg;		T1[1][2] = -AvgY1 * csqrt2 / RefDiffSquareAvg;
	T1[2][0] = 0.0;								T1[2][1] = 0.0;								T1[2][2] = 1.0;
	T1.Inversed();
	T2[0][0] = csqrt2 / DebugDiffSquareAvg;	T2[0][1] = 0.0;								T2[0][2] = -AvgX2 * csqrt2 / DebugDiffSquareAvg;
	T2[1][0] = 0.0;								T2[1][1] = csqrt2 / DebugDiffSquareAvg;	T2[1][2] = -AvgY2 * csqrt2 / DebugDiffSquareAvg;
	T2[2][0] = 0.0;								T2[2][1] = 0.0;								T2[2][2] = 1.0;
	//***********init T1 T2 matrix   *******************
	const int len = 9;
	CMatrix RelativeError(2 * nNum, len);
	for (int n = 0; n < nNum; n++)
	{
		float dX1 = pRefXYList[n][0] - AvgX1;
		float dY1 = pRefXYList[n][1] - AvgY1;
		float dX2 = pDebugXYList[n][0] - AvgX2;
		float dY2 = pDebugXYList[n][1] - AvgY2;
		dX1 = (dX1 * csqrt2) / RefDiffSquareAvg;
		dY1 = (dY1 * csqrt2) / RefDiffSquareAvg;
		dX2 = (dX2 * csqrt2) / DebugDiffSquareAvg;
		dY2 = (dY2 * csqrt2) / DebugDiffSquareAvg;
		RelativeError[n * 2][0] = RelativeError[n * 2][1] = RelativeError[n * 2][2] = 0;
		RelativeError[n * 2][3] = -dX2;
		RelativeError[n * 2][4] = -dY2;
		RelativeError[n * 2][5] = -1.0;
		RelativeError[n * 2][6] = dY1 * dX2;
		RelativeError[n * 2][7] = dY1 * dY2;
		RelativeError[n * 2][8] = dY1;
		RelativeError[n * 2 + 1][0] = dX2;
		RelativeError[n * 2 + 1][1] = dY2;
		RelativeError[n * 2 + 1][2] = 1.0;
		RelativeError[n * 2 + 1][3] = RelativeError[n * 2 + 1][4] = RelativeError[n * 2 + 1][5] = 0;
		RelativeError[n * 2 + 1][6] = -dX1 * dX2;
		RelativeError[n * 2 + 1][7] = -dX1 * dY2;
		RelativeError[n * 2 + 1][8] = -dX1;
	}
	double W[len];
	CMatrix V(len, len);
	RelativeError.SingularValueDecomposition(RelativeError, W, V);
	//*********** Get the minimum value of each row *******************
	int mini = 0;
	float fMinW = W[0];
	for (int i = 1; i < len; i++)
	{
		if (W[i] < fMinW)
		{
			fMinW = W[i];
			mini = i;
		}
	}
	H[0][0] = V[0][mini];
	H[0][1] = V[1][mini];
	H[0][2] = V[2][mini];
	H[1][0] = V[3][mini];
	H[1][1] = V[4][mini];
	H[1][2] = V[5][mini];
	H[2][0] = V[6][mini];
	H[2][1] = V[7][mini];
	H[2][2] = V[8][mini];
	//	H = T1*H*T2;
	//***********  Get final H *******************
	double TMPH[3][3];
	for (int ii = 0; ii < 3; ii++)
	{
		for (int jj = 0; jj < 3; jj++)
		{
			double sum = 0.0;
			for (int k = 0; k < 3; k++)
				sum += T1[ii][k] * H[k][jj];
			TMPH[ii][jj] = sum;
		}
	}
	for (int ii = 0; ii < 3; ii++)
	{
		for (int jj = 0; jj < 3; jj++)
		{
			double sum = 0.0;
			for (int k = 0; k < 3; k++)
				sum += TMPH[ii][k] * T2[k][jj];
			H[ii][jj] = sum;
		}
	}
	//*********** 	Normalized matrix *******************
	if (H[2][2] != 0)
	{
		double H22 = H[2][2];
		H[0][0] /= H22;
		H[0][1] /= H22;
		H[0][2] /= H22;
		H[1][0] /= H22;
		H[1][1] /= H22;
		H[1][2] /= H22;
		H[2][0] /= H22;
		H[2][1] /= H22;
		H[2][2] /= H22;
	}
	return true;
}

