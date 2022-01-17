#ifndef __MATH_FUNCTION_H_
#define __MATH_FUNCTION_H_
class CLogFUNTransform
{
protected:
	static int m_nYList[14];
	static int m_nDYList[13];
	static int m_nXList[17];
	static int m_nDXList[16];
	static void BuildYList();
	static void BuildXList();
public:
	static int m_nLog2;
	static int m_nLinearBits;
	static int m_nLogMax;
	static int m_nLinearMax;
	static void SetLinearBits(int nNewBits);
	static int Linear2Log(int nX);
	static int Log2Linear(int nY);
	static int ComputeLogRatio(int nGain, int nShift);
	static int ComputeLinearRatio(int nGain, int nShift);
};
#endif