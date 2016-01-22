#if !defined(BIGINT_CPP)
#include <stdint.h>
#include <assert.h>
typedef unsigned int uint;

#define UINT512_PARTS 16
// Little endian. First part is the least significant, (Length-1) is the most.
struct uint512 
{
    uint32_t Parts[UINT512_PARTS];
    
    uint512();
    uint512(int C);
    uint512(uint C);
    uint512(uint64_t C);
    uint512(uint32_t Parts[UINT512_PARTS]);
};

inline uint512 UINT512_MAX()
{ 
    uint512 Result;
    for(int32_t PartIndex = 0; PartIndex < UINT512_PARTS; ++PartIndex)
    { 
        Result.Parts[PartIndex] = 0xFFFFFFFF;
    }
    return Result;
}

inline uint512::uint512()
{
}

inline uint512::uint512(int C)
{
    ZeroMemory(this->Parts, sizeof(this->Parts));
    this->Parts[0] = C;
}

inline uint512::uint512(uint C)
{
    ZeroMemory(this->Parts, sizeof(this->Parts));
    this->Parts[0] = C;
}

inline uint512::uint512(uint64_t C)
{
    ZeroMemory(this->Parts, sizeof(this->Parts));
    this->Parts[0] = C & 0xFFFFFFFF;
    this->Parts[1] = (C >> 32) & 0xFFFFFFFF;
}

inline uint512::uint512(uint32_t Parts[16])
{
    CopyMemory(this->Parts, Parts, sizeof(Parts));
}

bool BigPartEqualsZero(uint512 A)
{
    return (A.Parts[1] | A.Parts[2] | A.Parts[3] | A.Parts[4] |
            A.Parts[5] | A.Parts[6] | A.Parts[7] | A.Parts[8] |
            A.Parts[9] | A.Parts[10] | A.Parts[11] | A.Parts[12] |
            A.Parts[13] | A.Parts[14] | A.Parts[15]) == 0;
}

bool Unsigned32AdditionOverflows(uint32_t A, uint32_t B)
{
    return UINT32_MAX - A < B;
}

inline uint512
operator+(uint512 A, uint512 B)
{
    uint512 Result = 0;
    bool OverflowedLastPart = false;
    for (int32_t PartIndex = 0; 
        PartIndex < UINT512_PARTS; 
        ++PartIndex)
    {
        Result.Parts[PartIndex] += A.Parts[PartIndex] + B.Parts[PartIndex];
        if (OverflowedLastPart) { Result.Parts[PartIndex] += 1; }

        if (Unsigned32AdditionOverflows(A.Parts[PartIndex], B.Parts[PartIndex]) ||
            (OverflowedLastPart && Unsigned32AdditionOverflows(A.Parts[PartIndex] + B.Parts[PartIndex], 1)))
        {
            OverflowedLastPart = true;
        }
        else
        {
            OverflowedLastPart = false;
        }
    }
    return Result;
}

inline bool
operator<(uint512 Left, uint Right)
{
    if (Left.Parts[0] >= Right) { return false; }
    if (!BigPartEqualsZero(Left)) { return false; }
    return true;
}

inline bool
operator<(uint512 Left, uint512 Right)
{
    for (int32_t PartIndex = UINT512_PARTS - 1; 
        PartIndex >= 0; 
        --PartIndex)
    {
        if (Left.Parts[PartIndex] < Right.Parts[PartIndex])
            return true;
        else if (Left.Parts[PartIndex] > Right.Parts[PartIndex])
            return false;
    }

    return false; // Equal
}

inline bool
operator==(uint512 Left, uint Right)
{
    if (!BigPartEqualsZero(Left)) { return false; }
    return Left.Parts[0] == Right;
}

inline bool
operator==(uint512 Left, uint512 Right)
{
    for (int32_t PartIndex = 0;
        PartIndex < UINT512_PARTS;
        ++PartIndex)
    {
        if (Left.Parts[PartIndex] != Right.Parts[PartIndex]) { return false; }
    }
    return true;
}

inline bool
operator!=(uint512 Left, uint Right) { return !(Right == Left); }
inline bool
operator>(uint512 Left, uint Right) { return Right < Left; }
inline bool
operator<=(uint512 Left, uint Right) { return !(Left > Right); }
inline bool
operator>=(uint512 Left, uint Right) { return !(Left < Right); }

inline bool
operator!=(uint512 Left, uint512 Right) { return !(Right == Left); }
inline bool
operator>(uint512 Left, uint512 Right) { return Right < Left; }
inline bool
operator<=(uint512 Left, uint512 Right) { return !(Left > Right); }
inline bool
operator>=(uint512 Left, uint512 Right) { return !(Left < Right); }

inline uint512
operator<<(uint512 A, uint8_t Shift)
{
    uint512 Result;
    assert(Shift <= 32); //TODO
    for (int32_t PartIndex = UINT512_PARTS - 1;
        PartIndex >= 0;
        --PartIndex)
    {
        Result.Parts[PartIndex] = A.Parts[PartIndex] << Shift;
        if (PartIndex != 0)
        {
            Result.Parts[PartIndex] |= A.Parts[PartIndex - 1] >> (32 - Shift);
        }
    }
    return Result;
}

inline uint512
operator>>(uint512 A, uint8_t Shift)
{
    assert(Shift <= 32); //TODO
    uint512 Result;
    for (int32_t PartIndex = 0;
        PartIndex < UINT512_PARTS;
        ++PartIndex)
    {
        Result.Parts[PartIndex] = A.Parts[PartIndex] >> Shift;
        if (PartIndex != UINT512_PARTS)
        {
            Result.Parts[PartIndex] |= A.Parts[PartIndex + 1] << (32 - Shift);
        }
    }
    return Result;
}

inline uint512
operator*(uint512 A, uint512 B)
{
    uint512 Result = 0;
    while (A >= 1)
    {
        if ((A.Parts[0] & 1) == 1) Result = Result + B;
        A = A >> 1;
        B = B << 1;
    }
    return Result;
}

inline uint512
operator-(uint512 A)
{
    uint512 Result;
    for (int PartIndex = 0;
        PartIndex < UINT512_PARTS;
        ++PartIndex)
    {
        Result.Parts[PartIndex] = ~A.Parts[PartIndex];
    }
    return Result + 1;
}

inline uint512
operator-(uint512 Left, uint512 Right)
{
    return Left + -Right;
}

inline bool
GetBitAt(uint512 A, uint32_t Place)
{
    assert(Place < 512); // Zero-indexed
    int Part = Place / 32;
    int PartPlace = Place - Part;
    return (A.Parts[Part] >> PartPlace) & 1;
}

inline void
SetBitAt(uint512* A, uint32_t Place, bool Value)
{
    assert(Place < 512); // Zero-indexed
    int Part = Place / 32;
    int PartPlace = Place - Part;
    int BitValue = (A->Parts[Part] >> PartPlace) & 1;
    if (Value) { BitValue = !BitValue; }
    // If XOR'd with itself, will be zero (false)
    // If XOR'd with its negation, will be one (true)
    A->Parts[Part] ^= BitValue << PartPlace;
}

struct uint512_divison_result
{
    uint512 Quotient;
    uint512 Remainder;
};

// Egyptian division algorithm for computing A / B
inline uint512_divison_result
UInt512Division(uint512 A, uint512 B)
{
    assert(B != 0);
    if (A < B) { 
        uint512_divison_result Result = { 0, A };
        return Result;
    }

    uint512 C = B;
    while (A - C >= C) C = C + C; // Largest doubling of B
    uint512 N = 1;
    A = A - C;

    while (C != B)
    {
        C = C >> 1; // Half(C);
		N = N + N;
        if (C <= A) { A = A - C; N = N + 1; }
    }

    uint512_divison_result Result = { N, A };
    return Result;
}

inline uint512
operator/(uint512 N, uint512 D)
{
    uint512_divison_result Result = UInt512Division(N, D);
    return Result.Quotient;
}

inline uint512
operator%(uint512 N, uint512 D)
{
    uint512_divison_result Result = UInt512Division(N, D);
    return Result.Remainder;
}

#define BIGINT_CPP
#endif
