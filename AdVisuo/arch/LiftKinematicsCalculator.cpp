#include "StdAfx.h"
#include "LiftKinematicsCalculator.h"

LiftKinematicsCalculator::~LiftKinematicsCalculator(void)
{
}
double LiftKinematicsCalculator::sign(double v)
{
	return v<0?-1:v>0?1:0;
}
double LiftKinematicsCalculator::getPosition(double t)
{
    double s = 0;

    t = t - T0;

    if (t < 0)
    {
        s = S0;
        return s;
    }

    if (t >= TT)
    {
        s = S0 + Dir * SM;
        return s;
    }

    for (int i = 0; i < phases; i++)
        if (t <= T[i])
        {
            if (i > 0)
                t = t - T[i - 1];

            i--;

            switch (P[i + 1])
            {
                case 1:
                    {
                        s = (1.0 / 6.0) * JM * t * t * t;
                    }
                    break;
                case 2:
                    {
                        s = S[i] + V[i] * t + 0.5 * AM * t * t;
                    }
                    break;
                case 3:
                    {
                        s = S[i] + V[i] * t + 0.5 * A[i] * t * t - (1.0 / 6.0) * JM * t * t * t;
                    }
                    break;
                case 4:
                    {
                        s = S[i] + VM * t;
                    }
                    break;
                case 5:
                    {
                        s = S[i] + V[i] * t + 0.5 * A[i] * t * t - (1.0 / 6.0) * JM * t * t * t;
                    }
                    break;
                case 6:
                    {
                        s = S[i] + V[i] * t + 0.5 * A[i] * t * t;
                    }
                    break;
                case 7:
                    {
                        s = S[i] + V[i] * t + 0.5 * A[i] * t * t + (1.0 / 6.0) * JM * t * t * t;
                    }
                    break;
            }

            s = S0 + Dir * s;

            break;
        }

    return s;
}


LiftKinematicsCalculator::LiftKinematicsCalculator(double t0, double s0, double s1,double v, double a, double j)
{
	VM = v;
    AM = a;
    JM = j;
    S0 = s0;
    T0 = t0;
    double s = s1 - s0;
    Dir = sign(s);
    SM = fabs(s);
    //SA distance to reach AM      
    //VA velocity at AM            
    //TA time to reach AM           
    //SVM distance to reach VM at AM
    //TVM time to reach SVM        

    SA = 2.0 * pow(AM, 3.0) / pow(JM, 2.0);
    VA = sqrt(AM * SA / 2.0);
    TA = 4.0 * AM / JM;
    SVM = (pow(VM, 2.0) / AM) + (AM * VM / JM);
    TVM = SVM / VM + VM / AM + AM / JM;

    double F;

    //If VM is not reached by thetime AM reached
    if (VA >= VM)
    {
        //Calculate SVM and TVM
        SVM = sqrt((4.0 / JM) * (pow(VM, 3.0)));
        TVM = pow(((32.0 * SVM) / JM), (1.0 / 3.0));

        if (SM > SVM)
        {
            //From SVM 
            TT = TVM + (SM - SVM) / VM;
            VR = VM;
            kinematicsProfile = VReached;
        }
        else
        {
            // Up to SVM 
            F = pow(((pow(JM, 2.0)) * SM / 2.0), (1.0 / 3.0));
            TT = pow(((32.0 * SM) / JM), (1.0 / 3.0));
            VR = sqrt((F * SM) / 2.0);
            kinematicsProfile = NoneReached;
        }
    }
    else
        if (SM > SA)
        {
            //From AM    
            VR = -(pow(AM, 2.0) / (2.0 * JM)) + sqrt(pow((pow(AM, 2.0) / (2.0 * JM)), 2.0) + SM * AM);

            if (VR > VM)
            {
                //At VM  
                TT = TVM + (SM - SVM) / VM;
                VR = VM;
				kinematicsProfile = VAReached;
            }
            else
            {
                kinematicsProfile = AReached;
                TT = AM / JM + sqrt(pow((AM / JM), 2.0) + 4.0 * SM / AM);
            }
        }
        else
        {
            //Up to AM  
            F = pow(((pow(JM, 2.0)) * SM / 2.0), (1.0 / 3.0));
            TT = pow(((32.0 * SM) / JM), (1.0 / 3.0));
            VR = sqrt((F * SM) / 2.0);
            kinematicsProfile = NoneReached;
        }

	recalculatePhases();
}
void LiftKinematicsCalculator::recalculatePhases()
{
    double constVTravelTime = 0;

    double constATravelTime = 0;

    double accTime = 0;

    phases = 0;

    switch (kinematicsProfile)
    {

        case NoneReached:
            constVTravelTime = 0;
            constATravelTime = 0;
            accTime = TT / 4.0;
            phases = 4;
            break;
        case AReached:
            constVTravelTime = 0;
            constATravelTime = (TT - TA) / 2.0;
            accTime = TA / 4.0;
            phases = 6;
            break;
        case VReached:
            constVTravelTime = (TT - TVM);
            constATravelTime = 0;
            accTime = TVM / 4.0;
            phases = 5;
            break;
        case VAReached:
            constVTravelTime = TT - TVM;
            constATravelTime = (TVM - TA) / 2.0;
            accTime = TA / 4.0;
            phases = 7;
            break;
    }

    int i = 0;
    //phase 1
    double t = accTime;
    J[i] = JM;
    A[i] = JM * t;
    V[i] = 0.5 * JM * t * t;
    S[i] = (1.0 / 6.0) * JM * t * t * t;
    P[i] = 1;
    T[i] = t;
    i++;
    //phase 2
    if (constATravelTime > 0)
    {
        t = constATravelTime;
        J[i] = 0;
        A[i] = AM;
        V[i] = V[i - 1] + AM * t;
        S[i] = S[i - 1] + V[i - 1] * t + 0.5 * AM * t * t;
        P[i] = 2;
        T[i] = T[i - 1] + t;
        i++;
    }

    //phase 3

    t = accTime;
    J[i] = -JM;
    A[i] = A[i - 1] - JM * t;
    V[i] = V[i - 1] + A[i - 1] * t - 0.5 * JM * t * t;
    S[i] = S[i - 1] + V[i - 1] * t + 0.5 * A[i - 1] * t * t - (1.0 / 6.0) * JM * t * t * t;
    P[i] = 3;
    T[i] = T[i - 1] + t;
    i++;

    //phase 4
    if (constVTravelTime > 0)
    {
        t = constVTravelTime;
        J[i] = 0;
        A[i] = 0;
        V[i] = VM;
        S[i] = S[i - 1] + VM * t;
        P[i] = 4;
        T[i] = T[i - 1] + t;
        i++;
    }
    //phase 5
    t = accTime;
    J[i] = -JM;
    A[i] = A[i - 1] - JM * t;
    V[i] = V[i - 1] - 0.5 * JM * t * t;
    S[i] = S[i - 1] + V[i - 1] * t + 0.5 * A[i - 1] * t * t - (1.0 / 6.0) * JM * t * t * t;
    P[i] = 5;
    T[i] = T[i - 1] + t;
    i++;
    //phase 6
    if (constATravelTime > 0)
    {
        t = constATravelTime;
        J[i] = 0;
        A[i] = A[i - 1];
        V[i] = V[i - 1] - AM * t;
        S[i] = S[i - 1] + V[i - 1] * t + 0.5 * A[i - 1] * t * t;
        P[i] = 6;
        T[i] = T[i - 1] + t;
        i++;
    }
    //phase 7
    t = accTime;
    J[i] = JM;
    A[i] = A[i - 1] + JM * t;
    V[i] = V[i - 1] + A[i - 1] * t + 0.5 * JM * t * t;
    S[i] = S[i - 1] + V[i - 1] * t + 0.5 * A[i - 1] * t * t + (1.0 / 6.0) * JM * t * t * t;
    P[i] = 7;
    T[i] = T[i - 1] + t;
}
