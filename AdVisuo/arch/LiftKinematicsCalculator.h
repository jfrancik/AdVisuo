#pragma once
#include <math.h>

enum KinematicsProfile
{
    // /\_
    //    \/
    VReached = 0,
    // /\
    //   \/
    NoneReached = 1,
    //  _
    // / \
    //    \_/
    AReached = 2,
    //  _
    // / \_
    //     \_/
    VAReached = 3
};

class LiftKinematicsCalculator
{
public:
	~LiftKinematicsCalculator(void);

	//returns position at time t
	double getPosition(double t);
	double getStartTime(){return T0;}
	double getStopTime(){return T0+TT;}
	double getTransitTime(){return TT;}

	//s0 - startposition
	//t0 - starttime
	//s1 - endposition
	//v  - rated speed
	//a  - acceleration
	//j  - jerk	
	LiftKinematicsCalculator(double t0,double s0,  double s1,double v, double a, double j);

private:
	KinematicsProfile kinematicsProfile;

    double SM;

    double VM;

    double AM;

    double JM;

    double	A[7];

    double	V[7];

    double	J[7];

    double	S[7];

    double	T[7];

    int		P[7];

	int phases;

    double TT, VR, SVM, TVM, SA, VA, TA;
    
    double S0, T0, Dir;

	void recalculatePhases();

	double sign(double v);

	void Release();
};
