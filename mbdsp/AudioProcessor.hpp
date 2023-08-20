#ifndef AUDIOPROCESSOR_HPP
#define AUDIOPROCESSOR_HPP

namespace mbdsp
{

template <class T = float>
class AudioProcessor
{
public:
    using sample_type = T;
    virtual ~AudioProcessor() = default;

    virtual sample_type Process(sample_type in) = 0;

    inline virtual void ProcessBlock(sample_type* in, size_t size)
    {
        for(size_t i = 0; i < size; ++i) { in[i] = Process(in[i]); }
    }
};

}  // namespace mbdsp

#endif  // AUDIOPROCESSOR_HPP