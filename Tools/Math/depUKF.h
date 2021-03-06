#ifndef depUKF_H
#define depUKF_H

#include "Matrix.h"
//\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
//#######################################################################################################
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// This is the old depricated UKF class, you do not want to use it. The new one is much better, trust me.
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//#######################################################################################################
/////////////////////////////////////////////////////////////////////////////////////////////////////////
class depUKF
{
public:
    depUKF();
    depUKF(unsigned int numStates);
    depUKF(const depUKF& source);
    ~depUKF();
    void CalculateSigmaWeights(float kappa = 1.0f);
    Matrix GenerateSigmaPoints() const;
    Matrix CalculateMeanFromSigmas(const Matrix& sigmaPoints) const;
    Matrix CalculateCovarianceFromSigmas(const Matrix& sigmaPoints, const Matrix& mean) const;
    Matrix PerformTimeUpdate(const Matrix& updateSigmaPoints);
    void setMean(const Matrix& newMean) {m_mean = newMean;};
    void setCovariance(const Matrix& newCovariance) {m_covariance = newCovariance;};
    double getMean(int stateId) const;
    double calculateSd(int stateId) const;
    bool setState(Matrix mean, Matrix covariance);
    bool  timeUpdate(const Matrix& updatedSigmaPoints, const Matrix& processNoise);
    bool measurementUpdate(const Matrix& measurement, const Matrix& measurementNoise, const Matrix& predictedMeasurementSigmas, const Matrix& stateEstimateSigmas);

protected:
   unsigned int m_numStates;
   Matrix m_mean;
   Matrix m_covariance;
   Matrix m_sigmaWeights;
   Matrix m_sqrtSigmaWeights;
   float m_kappa;
};

#endif // depUKF_H
