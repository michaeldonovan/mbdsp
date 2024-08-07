#include <cmath>
#include <limits>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "mbdsp/Controls/Controls.hpp"
#include "mbdsp/DelayLine.hpp"
#include "mbdsp/Oversampler.hpp"
#include "mbdsp/TapTempo.hpp"
#include "mbdsp/Utils.hpp"

using namespace mbdsp;
using namespace Catch::Matchers;

TEST_CASE("fast_pow2", "[utils]")
{
    for(float f = 0; f < 10; f += .1) { REQUIRE_THAT(fastpow2(f), WithinRel(std::pow(2, f), .01)); }
}

TEST_CASE("fast_pow10", "[utils]")
{
    for(float f = 0; f < 10; f += .1)
    {
        REQUIRE_THAT(fast_pow10(f), WithinRel(std::pow(10, f), .01));
    }
}

TEST_CASE("log2_approx", "[utils]")
{
    for(float f = 0; f < 10; f += .1)
    {
        REQUIRE_THAT(log2_approx(f), WithinRel(std::log2f(f), .01));
    }
}

TEST_CASE("powf_approx", "[utils]")
{
    for(float f = 0; f < 10; f += .01)
    {
        for(int n = 0; n < 10; n += 1)
        {
            REQUIRE_THAT(powf_approx(f, n), WithinRel(std::pow(f, n), .05));
        }
    }
}

TEST_CASE("db_to_amp", "[utils]")
{
    for(float db = -48; db < 48; db += .5)
    {
        REQUIRE_THAT(db_to_amp(db), WithinRel(std::pow(10.f, db / 20.f), .01));
    }
}

TEST_CASE("amp_to_db", "[utils]")
{
    // we lose accuracy near zero
    for(float amp = .015; amp < 10; amp += .1)
    {
        REQUIRE_THAT(amp_to_db(amp), WithinRel(std::log10(amp) * 20.f, .05));
    }
}

TEST_CASE("TapTempo returns correct beat length", "[taptempo]")
{
    TapTempo<int> tap;
    tap.Init(10);
    REQUIRE(tap.Tap(1) == 0);
    for(auto i : {3, 5, 7, 9, 11}) { REQUIRE(tap.Tap(i) == 2); }
    REQUIRE(tap.GetBeatLength() == 2);
}

TEST_CASE("TapTempo rejects taps over max length", "[taptempo]")
{
    TapTempo<int> tap;
    tap.Init(10);
    REQUIRE(tap.Tap(2) == 0);
    REQUIRE(tap.Tap(3) == 1);

    // over max length, should reset
    REQUIRE(tap.Tap(15) == 0);
    REQUIRE(tap.GetBeatLength() == 0);
    REQUIRE(tap.Tap(16) == 1);
}

TEST_CASE("Oversampler calls processor correct number of times", "[oversampler]")
{
    Oversampler<float> os;
    os.Init();
    size_t n_calls = 0;
    const auto processor = [&n_calls](float in) {
        n_calls++;
        return in;
    };
    const auto out = os.Process(0, processor);
    REQUIRE(n_calls == 2);
}

TEST_CASE("Delay line returns sample at correct delay", "[delayline]")
{
    const std::vector<float> in_buff = {0, 1, 2, 3, 4, 5};
    DelayLine<float> dl;
    dl.Init(in_buff.size());

    for(auto x : in_buff) { dl.Write(x); }

    REQUIRE(dl.Read(1) == 5);
    REQUIRE(dl.Read(2) == 4);
    REQUIRE(dl.Read(4) == 2);

    for(size_t i = 0; i < 100; ++i)
    {
        dl.Write(i);
        REQUIRE(dl.Read(1) == i);
    }
}

TEST_CASE("Delay line interpolates between samples", "[delayline]")
{
    const std::vector<float> in_buff = {0, 1, 2, 3, 4, 5};
    DelayLine<float> dl;
    dl.Init(in_buff.size());
    dl.Write(1);
    dl.Write(2);
    const auto read = dl.Read(1.5);
    REQUIRE(read > 1);
    REQUIRE(read < 2);
}

TEST_CASE("RemapExp", "[controls]")
{
    auto val = 0.f;
    auto input = [&val]() {
        return val;
    };

    auto ctrl = Control<float>(input).Remap<Remap::Exponential<float, 0.f, 100.f, 2.f>>();
    for(val = 0; val < 1; val += .1f) { REQUIRE_THAT(ctrl(), WithinRel(val * val * 100, .01f)); }
}

TEST_CASE("PitchControl", "[controls]")
{
    float v_oct;
    auto v_oct_fn = [&v_oct]() {
        return v_oct;
    };
    auto coarse = []() {
        return 0.f;
    };

    constexpr float f_base = 440.f;
    constexpr float v_max = 10.f;

    auto ctrl = PitchControl<float, f_base, f_base>(v_oct_fn, coarse);

    // a4
    v_oct = 0 / v_max;
    REQUIRE(ctrl() == 440);

    // a5
    v_oct = 1 / v_max;
    REQUIRE(ctrl() == 880);

    // a6
    v_oct = 2 / v_max;
    REQUIRE(ctrl() == 1760);

    // a7
    v_oct = 3 / v_max;
    REQUIRE(ctrl() == 3520);
}

TEST_CASE("PitchControlFine", "[controls]")
{
    float v_oct;
    auto v_oct_fn = [&v_oct] {
        return v_oct;
    };
    auto coarse = [] {
        return 0.f;
    };

    float fine;
    auto fine_fn = [&fine] {
        return fine;
    };

    constexpr float f_base = 440.f;
    constexpr float v_max = 10.f;
    constexpr float fine_semis = 12.f;

    auto ctrl = PitchControl<float, f_base, f_base, fine_semis, v_max>(v_oct_fn, coarse, fine_fn);

    // Bb4x
    v_oct = 0 / v_max;
    fine = 1 / fine_semis;
    REQUIRE_THAT(ctrl(), WithinRel(466.1638f, .01f));

    // c6
    v_oct = 1 / v_max;
    fine = 2 / fine_semis;
    REQUIRE_THAT(ctrl(), WithinRel(1046.502f, .01f));

    // a7
    v_oct = 2 / v_max;
    fine = 12 / fine_semis;
    REQUIRE_THAT(ctrl(), WithinRel(3520.f, .01f));
}