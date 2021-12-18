#include <math.h>
#include "MathFunction.h"
int CLogFUNTransform::m_nLog2=2048;
int CLogFUNTransform::m_nLinearBits=12;
int CLogFUNTransform::m_nLogMax=12*2048;
int CLogFUNTransform::m_nLinearMax=4095;
int CLogFUNTransform::m_nYList[14]={22528,22707,22876,23036,23187,23331,23469,23600,23726,23847,23963,24181,24385,24576};
int CLogFUNTransform::m_nDYList[13]={179,169,160,151,144,138,131,126,121,116,218,204,191}; 
int CLogFUNTransform::m_nXList[17]={2048,2140,2234,2333,2435,2544,2657,2774,2897,3025,3159,3298,3447,3598,3758,3924,4096};
int CLogFUNTransform::m_nDXList[16]={92,94,99,102,109,113,117,123,128,134,139,149,151,160,166,172};
void CLogFUNTransform::BuildYList()
{
	int i, x, step;
	double fLog2=(double) m_nLog2/log(2.0);
	m_nLogMax=m_nLinearBits*m_nLog2;
	m_nYList[13]=m_nLogMax;
	m_nYList[0]=m_nLogMax-m_nLog2;
	x=1<<(m_nLinearBits-1);
	step=1<<(m_nLinearBits-5);
	for(i=1; i<=10; i++)
	{
		x+=step;
		m_nYList[i]=(int) (log((double) x)*fLog2+0.5);
	}
	step*=2;
	for(; i<13; i++)
	{
		x+=step;
		m_nYList[i]=(int) (log((double) x)*fLog2+0.5);
	}
	for(i=0; i<13; i++)
	{
		m_nDYList[i]=m_nYList[i+1]-m_nYList[i];
	}
}
void CLogFUNTransform::BuildXList()
{
	int i;
	m_nXList[0]=1<<(m_nLinearBits-1);
	m_nXList[16]=1<<m_nLinearBits;
	m_nLinearMax=(1<<m_nLinearBits)-1;
	int nStep=m_nXList[0];
	for(i=1; i<16; i++)
	{
		int nY=m_nYList[0]+i*m_nLog2/16;
		for(int j=1; j<14; j++)
		{
			if(nY<=m_nYList[j])
			{
				int fx=m_nXList[0]*16;
				if(j<=10)
				{
					fx+=nStep*(j-1);
					fx+=(nY-m_nYList[j-1])*nStep/(m_nYList[j]-m_nYList[j-1]);
				}
				else
				{
					fx+=nStep*(2*(j-11))+nStep*10;
					fx+=(nY-m_nYList[j-1])*2*nStep/(m_nYList[j]-m_nYList[j-1]);
				}
				m_nXList[i]=(fx+8)/16;
				break;
			}
		}
	}
	for(i=0; i<16; i++)
	{
		m_nDXList[i]=m_nXList[i+1]-m_nXList[i];
	}
}
void CLogFUNTransform::SetLinearBits(int nNewBits)
{
	if(nNewBits!=m_nLinearBits)
	{
		int i;
		m_nYList[0]=18432+(nNewBits-10)*m_nLog2;
		m_nYList[1]=18611+(nNewBits-10)*m_nLog2;
		m_nYList[2]=18780+(nNewBits-10)*m_nLog2;
		m_nYList[3]=18940+(nNewBits-10)*m_nLog2;
		m_nYList[4]=19091+(nNewBits-10)*m_nLog2;
		m_nYList[5]=19235+(nNewBits-10)*m_nLog2;
		m_nYList[6]=19373+(nNewBits-10)*m_nLog2;
		m_nYList[7]=19504+(nNewBits-10)*m_nLog2;
		m_nYList[8]=19630+(nNewBits-10)*m_nLog2;
		m_nYList[9]=19751+(nNewBits-10)*m_nLog2;
		m_nYList[10]=19867+(nNewBits-10)*m_nLog2;
		m_nYList[11]=20085+(nNewBits-10)*m_nLog2;
		m_nYList[12]=20289+(nNewBits-10)*m_nLog2;
		m_nYList[13]=20480+(nNewBits-10)*m_nLog2;
		for(i=0; i<13; i++)
		{
			m_nDYList[i]=m_nYList[i+1]-m_nYList[i];
		}
		m_nLogMax = nNewBits*m_nLog2;
		m_nLinearMax = (1<<nNewBits)-1;
		m_nLinearBits = nNewBits;
		BuildXList();
	}
}
int CLogFUNTransform::Linear2Log(int nX)
{
	int i, j, nY, dx, dy;
	int C=0;
	int bit=0;
	int mask=1<<(m_nLinearBits-1);
	while((nX&mask)==0&&mask>0)
	{
		mask>>=1;
		bit++;
	}
	nX=nX+1;
	if(bit>0)
	{
		nX=nX<<bit;
		C+=bit*m_nLog2;
	}
	//nX>512
	mask=(1<<(m_nLinearBits-1))-1;
	nX=nX&mask;
	if(nX==0)	
	{
		C-=m_nLog2;
	}

	int shift=m_nLinearBits-5;
	mask=(1<<shift)-1;
	i=nX>>shift;
	dx=nX&mask;
	if(i<=9)	
	{
		nY=m_nYList[i]-C;
		dy=m_nDYList[i];
		dx=(dx*dy)>>shift;
	}
	else
	{
		i-=10;
		j=i&1;
		dx=((j<<shift)|dx);
		i>>=1;
		nY=m_nYList[i+10]-C;
		dy=m_nDYList[i+10];
		dx=(dx*dy)>>(shift+1);
	}
	nY+=dx;
	if(nY<0)nY=0;	if(nY>m_nLogMax)nY=m_nLogMax;
	return nY;
}
int CLogFUNTransform::Log2Linear(int nY)
{
	int i, nX;
	int S=0;
	int seg=m_nLog2/16;
	int mask1=m_nLog2-1;
	int mask2=seg-1;
	int half=m_nLog2/32;
	if(nY<0)
	{
		nY=-nY;
		S=nY/m_nLog2;
		nY=(nY&mask1);
		if(nY>0)
		{
			S++;
			nY=m_nLog2-nY;
		}
		S=-S;
	}
	else
	{
		S=nY/m_nLog2;
		nY=(nY&mask1);
		if(nY==0)
		{
			S--;
			nY=m_nLog2;
		}
	}
	S=m_nLinearBits-1-S;
	i=nY/seg;
	nY=(nY&mask2);
	if(S>0)
	{
		nX=m_nXList[i];
		nY=nY*m_nDXList[i];
		nX=nX+nY/seg;		
		half=1<<(S-1);
		nX=(nX+half)>>S;
	}
	else
	{
		S=-S;
		nX=m_nXList[i]<<S;
		nY=nY*m_nDXList[i];
		nX=nX+((nY<<S)+half)/seg;
	}
	return nX;
}
int CLogFUNTransform::ComputeLogRatio(int nGain, int nShift)
{
	while(nGain>(1<<m_nLinearBits))
	{
		nGain>>=1;
		nShift--;
	}
	return Linear2Log(nGain-1)-nShift*m_nLog2;
}
int CLogFUNTransform::ComputeLinearRatio(int nGain, int nShift)
{
	return Log2Linear(nGain+nShift*m_nLog2);
}

