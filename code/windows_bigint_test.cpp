#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "bigint.cpp"

#define ArrayCount(x) (sizeof(x) / sizeof((x)[0]))

static uint512
Random512(uint32_t PartCount)
{
    uint512 Result;
    for (uint PartIndex = 0;
        PartIndex < min(UINT512_PARTS, PartCount);
        ++PartIndex)
    {
        //TODO(chronister): rand() is not cryptographically suitable, also not always 16 bit
        Result.Parts[PartIndex] = rand() | rand() << 16;
    }
    return Result;
}

static uint32_t
LogBase10(uint512 N)
{
    uint32_t Result = 0;
    while (N > 0)
    {
        N = N / 10;
        ++Result;
    }
    return Result;
}

static uint32_t
LogBase2(uint512 N)
{
    assert(N != 0);

    uint32_t Result = 0;
    while (N > 0)
    {
        N = N >> 1;
        ++Result;
    }
    return Result - 1;
}

static uint512
TwoPowN(uint32_t N)
{
    uint512 Result = 1;
    while (N > 0)
    {
        Result = Result << 1;
        --N;
    }
    return Result;
}

static uint512
SquareRootSeedEstimate(uint512 N)
{
    uint32_t n = LogBase2(N);
    uint512 Approximation = TwoPowN(n / 2);
    return Approximation;
}

static uint512
SquareRootBabylonian(uint512 N, uint512 Seed, uint IterationCount)
{
    uint512 Result = Seed;
    for (uint I = 0; I < IterationCount; ++I)
    {
        Result = (Result + (N / Result)) >> 1; // /2 TODO: make this optimization in the division function!
    }
    return Result;
}

void
PrintUInt512(uint512 N, bool NewLine = false)
{
    char Buf[160];
    ZeroMemory(Buf, 160);
    uint32_t Length = LogBase10(N);
    for (int32_t I = Length - 1; I >= 0; --I)
    {
        uint512 R = N % 10;
        Buf[I] = (char)('0' + R.Parts[0]);
        N = N / 10;
    }
    if (NewLine)
        printf("%.*s\n", Length, Buf);
    else 
        printf("%.*s", Length, Buf);
}

// PRIMALITY TESTS:
// If prime: returns 1;
// If not prime: returns smallest prime factor
// If not a number that can be prime: returns 0

static const int SmallPrimes[] = {
    5,7,11,13,17,19,23,29,31,37,41,43,47,53,59,61,67,71
    ,73,79,83,89,97,101,103,107,109,113,127,131,137,139,149,151,157,163,167,173
    ,179,181,191,193,197,199,211,223,227,229,233,239,241,251,257,263,269,271,277,281
    ,283,293,307,311,313,317,331,337,347,349,353,359,367,373,379,383,389,397,401,409
    ,419,421,431,433,439,443,449,457,461,463,467,479,487,491,499,503,509,521,523,541
    ,547,557,563,569,571,577,587,593,599,601,607,613,617,619,631,641,643,647,653,659
    ,661,673,677,683,691,701,709,719,727,733,739,743,751,757,761,769,773,787,797,809
    ,811,821,823,827,829,839,853,857,859,863,877,881,883,887,907,911,919,929,937,941
    ,947,953,967,971,977,983,991,997,1009,1013,1019,1021,1031,1033,1039,1049,1051,1061,1063,1069
    ,1087,1091,1093,1097,1103,1109,1117,1123,1129,1151,1153,1163,1171,1181,1187,1193,1201,1213,1217,1223
    ,1229,1231,1237,1249,1259,1277,1279,1283,1289,1291,1297,1301,1303,1307,1319,1321,1327,1361,1367,1373
    ,1381,1399,1409,1423,1427,1429,1433,1439,1447,1451,1453,1459,1471,1481,1483,1487,1489,1493,1499,1511
    ,1523,1531,1543,1549,1553,1559,1567,1571,1579,1583,1597,1601,1607,1609,1613,1619,1621,1627,1637,1657
    ,1663,1667,1669,1693,1697,1699,1709,1721,1723,1733,1741,1747,1753,1759,1777,1783,1787,1789,1801,1811
    ,1823,1831,1847,1861,1867,1871,1873,1877,1879,1889,1901,1907,1913,1931,1933,1949,1951,1973,1979,1987
    ,1993,1997,1999,2003,2011,2017,2027,2029,2039,2053,2063,2069,2081,2083,2087,2089,2099,2111,2113,2129
    ,2131,2137,2141,2143,2153,2161,2179,2203,2207,2213,2221,2237,2239,2243,2251,2267,2269,2273,2281,2287
    ,2293,2297,2309,2311,2333,2339,2341,2347,2351,2357,2371,2377,2381,2383,2389,2393,2399,2411,2417,2423
    ,2437,2441,2447,2459,2467,2473,2477,2503,2521,2531,2539,2543,2549,2551,2557,2579,2591,2593,2609,2617
    ,2621,2633,2647,2657,2659,2663,2671,2677,2683,2687,2689,2693,2699,2707,2711,2713,2719,2729,2731,2741
    ,2749,2753,2767,2777,2789,2791,2797,2801,2803,2819,2833,2837,2843,2851,2857,2861,2879,2887,2897,2903
    ,2909,2917,2927,2939,2953,2957,2963,2969,2971,2999,3001,3011,3019,3023,3037,3041,3049,3061,3067,3079
    ,3083,3089,3109,3119,3121,3137,3163,3167,3169,3181,3187,3191,3203,3209,3217,3221,3229,3251,3253,3257
    ,3259,3271,3299,3301,3307,3313,3319,3323,3329,3331,3343,3347,3359,3361,3371,3373,3389,3391,3407,3413
    ,3433,3449,3457,3461,3463,3467,3469,3491,3499,3511,3517,3527,3529,3533,3539,3541,3547,3557,3559,3571 };

static uint512
PrimalityTestEarlyOut(uint512 N)
{
    if (BigPartEqualsZero(N)) 
    {
        if (N.Parts[0] <= 1) { return 0; }
        else if (N.Parts[0] <= 3) { return N.Parts[0]; }
    }
    else if ((N.Parts[0] & 1) == 0) { return 2; }
    else if (N % 3 == 0) { return 3; }
    for (uint PrimeIndex = 0; PrimeIndex < ArrayCount(SmallPrimes); ++PrimeIndex)
    {
        if (N % SmallPrimes[PrimeIndex] == 0) { return SmallPrimes[PrimeIndex]; }
    }
    return 1;
}

// TODO(chronister): Unless this can be optimized better, DO NOT USE! Takes way too long for anything whose prime factor
// is larger than...like...100k
static uint512
PrimalityTestBruteForce(uint512 N)
{
    // Early out
    uint512 EarlyOutResult = PrimalityTestEarlyOut(N);
    if (EarlyOutResult != 1) { return EarlyOutResult; }

    uint512 SquareRootSeed = SquareRootSeedEstimate(N);
    // NOTE(chronister): This will always be an overestimate due to the way the Babylonian method works
    uint512 SquareRootIsh = SquareRootBabylonian(N, SquareRootSeed, 4);

    for (uint512 I = 6; I < SquareRootIsh; I = I + 6)
    {
        uint512 A = I - 1;
        if (N % A == 0) { return A; }
        uint512 B = I + 1;
        if (N % B == 0) { return B; }
    }

    return 1;
}

static uint512
PrimalityTestRandom(uint512 N, uint32_t k)
{
    // Early out
    uint512 EarlyOutResult = PrimalityTestEarlyOut(N);
    if (EarlyOutResult != 1) { return EarlyOutResult; }

    for (uint32_t i = 0; i < k; ++i)
    {
        uint512 PossibleWitness = Random512(16/2);
        if (N % PossibleWitness == 0) { return PossibleWitness; }
    }
    return 1;
}

#define TIME_START(var) LARGE_INTEGER var##_start; QueryPerformanceCounter(&var##_start)
#define TIME_END(var) LARGE_INTEGER var##_end; QueryPerformanceCounter(&var##_end); \
    printf("\nElapsed (" #var ") : %dms\n", ((var##_end.QuadPart - var##_start.QuadPart) * 1000) / Freq);

static uint64_t Freq;

int main(int ArgCount, char* ArgValues[])
{
    srand(GetTickCount());

    LARGE_INTEGER Freql;
    QueryPerformanceFrequency(&Freql);
    Freq = Freql.QuadPart;
    
    uint512 N;
    uint512 Factor = 1;
    do
    {
        N = Random512(16);
        printf("N = ");
        PrintUInt512(N);
        TIME_START(PrimeTest);
        // This is really, really slow!
        Factor = PrimalityTestRandom(N, 10000000);
        if (Factor == 1) printf("Prime? YES");
        else {
            printf(" Prime? NO: ");
            PrintUInt512(Factor, true);
        }
        TIME_END(PrimeTest);

    } while (Factor != 1);

}

