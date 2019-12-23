#include <benchmark/benchmark.h>
#include <memory>

using BYTE = unsigned char;

static void SIMDMemCopy(void * __restrict _Dest, const void * __restrict _Source, size_t NumQuadwords)
{
    __m128i * __restrict Dest = (__m128i * __restrict)_Dest;
    const __m128i * __restrict Source = (const __m128i * __restrict)_Source;

    // Discover how many quadwords precede a cache line boundary.  Copy them separately.
    size_t InitialQuadwordCount = (4 - ((size_t)Source >> 4) & 3) & 3;
    if(InitialQuadwordCount > NumQuadwords)
        InitialQuadwordCount = NumQuadwords;

    switch(InitialQuadwordCount)
    {
    case 3: _mm_stream_si128(Dest + 2, _mm_load_si128(Source + 2));     // Fall through
    case 2: _mm_stream_si128(Dest + 1, _mm_load_si128(Source + 1));     // Fall through
    case 1: _mm_stream_si128(Dest + 0, _mm_load_si128(Source + 0));     // Fall through
    default:
        break;
    }

    if(NumQuadwords == InitialQuadwordCount)
        return;

    Dest += InitialQuadwordCount;
    Source += InitialQuadwordCount;
    NumQuadwords -= InitialQuadwordCount;

    size_t CacheLines = NumQuadwords >> 2;

    switch(CacheLines)
    {
    default:
    case 10: _mm_prefetch((char *)(Source + 36), _MM_HINT_NTA);    // Fall through
    case 9:  _mm_prefetch((char *)(Source + 32), _MM_HINT_NTA);    // Fall through
    case 8:  _mm_prefetch((char *)(Source + 28), _MM_HINT_NTA);    // Fall through
    case 7:  _mm_prefetch((char *)(Source + 24), _MM_HINT_NTA);    // Fall through
    case 6:  _mm_prefetch((char *)(Source + 20), _MM_HINT_NTA);    // Fall through
    case 5:  _mm_prefetch((char *)(Source + 16), _MM_HINT_NTA);    // Fall through
    case 4:  _mm_prefetch((char *)(Source + 12), _MM_HINT_NTA);    // Fall through
    case 3:  _mm_prefetch((char *)(Source + 8), _MM_HINT_NTA);    // Fall through
    case 2:  _mm_prefetch((char *)(Source + 4), _MM_HINT_NTA);    // Fall through
    case 1:  _mm_prefetch((char *)(Source + 0), _MM_HINT_NTA);    // Fall through

        // Do four quadwords per loop to minimize stalls.
        for(size_t i = CacheLines; i > 0; --i)
        {
            // If this is a large copy, start prefetching future cache lines.  This also prefetches the
            // trailing quadwords that are not part of a whole cache line.
            if(i >= 10)
                _mm_prefetch((char *)(Source + 40), _MM_HINT_NTA);

            _mm_stream_si128(Dest + 0, _mm_load_si128(Source + 0));
            _mm_stream_si128(Dest + 1, _mm_load_si128(Source + 1));
            _mm_stream_si128(Dest + 2, _mm_load_si128(Source + 2));
            _mm_stream_si128(Dest + 3, _mm_load_si128(Source + 3));

            Dest += 4;
            Source += 4;
        }

    case 0:    // No whole cache lines to read
        break;
    }

    // Copy the remaining quadwords
    switch(NumQuadwords & 3)
    {
    case 3: _mm_stream_si128(Dest + 2, _mm_load_si128(Source + 2));     // Fall through
    case 2: _mm_stream_si128(Dest + 1, _mm_load_si128(Source + 1));     // Fall through
    case 1: _mm_stream_si128(Dest + 0, _mm_load_si128(Source + 0));     // Fall through
    default:
        break;
    }

    _mm_sfence();
}

static void BM_SIMDMemcpy(benchmark::State & state) {
    int64_t size = state.range(0);

    std::unique_ptr<BYTE[]> srcPtr = std::make_unique<BYTE[]>(size);
    std::unique_ptr<BYTE[]> dstPtr = std::make_unique<BYTE[]>(size);

    memset(srcPtr.get(), '#', size);

    BYTE * src = srcPtr.get();
    BYTE * dst = dstPtr.get();

    int64_t numquads = size >> 4;

    for(auto _ : state) {
        SIMDMemCopy(dst, src, numquads);
    }

    state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(size));
}

static void BM_UseCommonMemcpy(benchmark::State & state) {
    int64_t size = state.range(0);

    std::unique_ptr<BYTE[]> srcPtr = std::make_unique<BYTE[]>(size);
    std::unique_ptr<BYTE[]> dstPtr = std::make_unique<BYTE[]>(size);

    memset(srcPtr.get(), '#', size);

    BYTE * src = srcPtr.get();
    BYTE * dst = dstPtr.get();

    for(auto _ : state) {
        memcpy(dst, src, size);
    }

    state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(size));
}

BENCHMARK(BM_UseCommonMemcpy)
        ->Arg(0x40)
        ->Arg(0x80)
        ->Arg(0x100)
        ->Arg(0x200)
        ->Arg(0x400)
        ->Arg(0x2000)
        ->Arg(0x4000)
        ->Arg(0x10000)
        ->Arg(0x100000)
        ->Arg(0x1000000);

BENCHMARK(BM_SIMDMemcpy)
        ->Arg(0x40)
        ->Arg(0x80)
        ->Arg(0x100)
        ->Arg(0x200)
        ->Arg(0x400)
        ->Arg(0x2000)
        ->Arg(0x4000)
        ->Arg(0x10000)
        ->Arg(0x100000)
        ->Arg(0x1000000);

BENCHMARK_MAIN();

