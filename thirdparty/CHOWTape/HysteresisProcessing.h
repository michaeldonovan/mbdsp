/**
 * By Jatin Chowdhury, licensed under GPLv3.0
 * https://github.com/jatinchowdhury18/AnalogTapeModel
 */
#ifndef HYSTERESISPROCESSING_H_INCLUDED
#define HYSTERESISPROCESSING_H_INCLUDED

#include <cmath>

#include "HysteresisOps.h"

enum SolverType
{
    RK2 = 0,
    RK4,
    NR4,
    NR8,
    STN,
    NUM_SOLVERS
};

/*
    Hysteresis processing for a model of an analog tape machine.
    For more information on the DSP happening here, see:
    https://ccrma.stanford.edu/~jatin/420/tape/TapeModel_DAFx.pdf
*/
class HysteresisProcessing
{
public:
    HysteresisProcessing() = default;
    HysteresisProcessing(HysteresisProcessing&&) noexcept = default;

    void reset()
    {
        M_n1 = 0.0;
        H_n1 = 0.0;
        H_d_n1 = 0.0;

        hpState.coth = 0.0;
        hpState.nearZero = false;
    }

    void setSampleRate(double newSR)
    {
        fs = newSR;
        T = 1.0 / fs;
        Talpha = T / 1.9;
    }

    void cook(double drive, double width, double sat, bool v1)
    {
        hpState.M_s = 0.5 + 1.5 * (1.0 - sat);
        hpState.a = hpState.M_s / (0.01 + 6.0 * drive);
        hpState.c = std::sqrt(1.0f - width) - 0.01;
        hpState.k = 0.47875;
        upperLim = 20.0;

        if(v1)
        {
            hpState.k = 27.0e3;
            hpState.c = 1.7e-1;
            hpState.M_s *= 50000.0;
            hpState.a = hpState.M_s / (0.01 + 40.0 * drive);
            upperLim = 100000.0;
        }

        hpState.nc = 1.0 - hpState.c;
        hpState.M_s_oa = hpState.M_s / hpState.a;
        hpState.M_s_oa_talpha = HysteresisOps::HysteresisState::alpha * hpState.M_s_oa;
        hpState.M_s_oa_tc = hpState.c * hpState.M_s_oa;
        hpState.M_s_oa_tc_talpha = HysteresisOps::HysteresisState::alpha * hpState.M_s_oa_tc;
        hpState.M_s_oaSq_tc_talpha = hpState.M_s_oa_tc_talpha / hpState.a;
        hpState.M_s_oaSq_tc_talphaSq =
            HysteresisOps::HysteresisState::alpha * hpState.M_s_oaSq_tc_talpha;
    }

    /* Process a single sample */
    template <SolverType solver, typename Float>
    inline Float process(Float H) noexcept
    {
        auto H_d = HysteresisOps::deriv(H, H_n1, H_d_n1, (Float) T);

        Float M;
        switch(solver)
        {
            case RK2:
                M = RK2Solver(H, H_d);
                break;
            case RK4:
                M = RK4Solver(H, H_d);
                break;
            case NR4:
                M = NRSolver<4>(H, H_d);
                break;
            case NR8:
                M = NRSolver<8>(H, H_d);
                break;

            default:
                M = 0.0;
        };

                // check for instability
#if HYSTERESIS_USE_SIMD
        auto notIllCondition = !(xsimd::isnan(M) || (M > upperLim));
        M = xsimd::select(notIllCondition, M, (Float) 0.0);
        H_d = xsimd::select(notIllCondition, H_d, (Float) 0.0);
#else
        bool illCondition = std::isnan(M) || M > upperLim;
        M = illCondition ? 0.0 : M;
        H_d = illCondition ? 0.0 : H_d;
#endif

        M_n1 = M;
        H_n1 = H;
        H_d_n1 = H_d;

        return M;
    }

private:
    // runge-kutta solvers
    template <typename Float>
    inline Float RK2Solver(Float H, Float H_d) noexcept
    {
        const Float k1 = HysteresisOps::hysteresisFunc(M_n1, H_n1, H_d_n1, hpState) * T;
        const Float k2 = HysteresisOps::hysteresisFunc(M_n1 + (k1 * 0.5), (H + H_n1) * 0.5,
                                                       (H_d + H_d_n1) * 0.5, hpState) *
                         T;

        return M_n1 + k2;
    }

    template <typename Float>
    inline Float RK4Solver(Float H, Float H_d) noexcept
    {
        const Float H_1_2 = (H + H_n1) * 0.5;
        const Float H_d_1_2 = (H_d + H_d_n1) * 0.5;

        const Float k1 = HysteresisOps::hysteresisFunc(M_n1, H_n1, H_d_n1, hpState) * T;
        const Float k2 =
            HysteresisOps::hysteresisFunc(M_n1 + (k1 * 0.5), H_1_2, H_d_1_2, hpState) * T;
        const Float k3 =
            HysteresisOps::hysteresisFunc(M_n1 + (k2 * 0.5), H_1_2, H_d_1_2, hpState) * T;
        const Float k4 = HysteresisOps::hysteresisFunc(M_n1 + k3, H, H_d, hpState) * T;

        constexpr double oneSixth = 1.0 / 6.0;
        constexpr double oneThird = 1.0 / 3.0;
        return M_n1 + k1 * oneSixth + k2 * oneThird + k3 * oneThird + k4 * oneSixth;
    }

    // newton-raphson solvers
    template <int nIterations, typename Float>
    inline Float NRSolver(Float H, Float H_d) noexcept
    {
        Float M = M_n1;
        const Float last_dMdt = HysteresisOps::hysteresisFunc(M_n1, H_n1, H_d_n1, hpState);

        Float dMdt;
        Float dMdtPrime;
        Float deltaNR;
        for(int n = 0; n < nIterations; ++n)
        {
            using namespace HysteresisOps;
            dMdt = hysteresisFunc(M, H, H_d, hpState);
            dMdtPrime = hysteresisFuncPrime(H_d, dMdt, hpState);
            deltaNR = (M - M_n1 - (Float) Talpha * (dMdt + last_dMdt)) /
                      (Float(1.0) - (Float) Talpha * dMdtPrime);
            M -= deltaNR;
        }

        return M;
    }

    // parameter values
    double fs = 48000.0;
    double T = 1.0 / fs;
    double Talpha = T / 1.9;
    double upperLim = 20.0;

    // state variables
#if HYSTERESIS_USE_SIMD
    xsimd::batch<double> M_n1 = 0.0;
    xsimd::batch<double> H_n1 = 0.0;
    xsimd::batch<double> H_d_n1 = 0.0;
#else
    double M_n1 = 0.0;
    double H_n1 = 0.0;
    double H_d_n1 = 0.0;
#endif

    HysteresisOps::HysteresisState hpState;
};

#endif
