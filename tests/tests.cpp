#include <catch2/catch_test_macros.hpp>

#include "DelayLine.hpp"
#include "Oversampler.hpp"
#include "TapTempo.hpp"

using namespace mbdsp;

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
