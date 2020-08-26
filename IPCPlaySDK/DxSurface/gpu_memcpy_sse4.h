/*
 *      Copyright (C) 2011-2015 Hendrik Leppkes
 *      http://www.1f0.de
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 *  Taken from the QuickSync decoder by Eric Gur
 */
#pragma once 
#include <emmintrin.h>
#include <smmintrin.h>
#if _MSC_VER < 1600
#define nullptr	NULL
#endif

// gpu_memcpy is a memcpy style function that copied data very fast from a
// GPU tiled memory (write back)
// Performance tip: page offset (12 lsb) of both addresses should be different
//  optimally use a 2K offset between them.
inline void* gpu_memcpy(void* d, const void* s, size_t size)
{
    static const size_t regsInLoop = sizeof(size_t) * 2; // 8 or 16

    if (d == nullptr || s == nullptr) return nullptr;

    // If memory is not aligned, use memcpy
    bool isAligned = (((size_t)(s) | (size_t)(d)) & 0xF) == 0;
    if (!isAligned)
    {
        return memcpy(d, s, size);
    }

    __m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7;
#ifdef _M_X64
    __m128i xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15;
#endif

    size_t reminder = size & (regsInLoop * sizeof(xmm0) - 1); // Copy 128 or 256 bytes every loop
    size_t end = 0;

    __m128i* pTrg = (__m128i*)d;
    __m128i* pTrgEnd = pTrg + ((size - reminder) >> 4);
    __m128i* pSrc = (__m128i*)s;
    
    // Make sure source is synced - doesn't hurt if not needed.
    _mm_sfence();

    while (pTrg < pTrgEnd)
    {
        // _mm_stream_load_si128 emits the Streaming SIMD Extensions 4 (SSE4.1) instruction MOVNTDQA
        // Fastest method for copying GPU RAM. Available since Penryn (45nm Core 2 Duo/Quad)
        xmm0  = _mm_stream_load_si128(pSrc);
        xmm1  = _mm_stream_load_si128(pSrc + 1);
        xmm2  = _mm_stream_load_si128(pSrc + 2);
        xmm3  = _mm_stream_load_si128(pSrc + 3);
        xmm4  = _mm_stream_load_si128(pSrc + 4);
        xmm5  = _mm_stream_load_si128(pSrc + 5);
        xmm6  = _mm_stream_load_si128(pSrc + 6);
        xmm7  = _mm_stream_load_si128(pSrc + 7);
#ifdef _M_X64 // Use all 16 xmm registers
        xmm8  = _mm_stream_load_si128(pSrc + 8);
        xmm9  = _mm_stream_load_si128(pSrc + 9);
        xmm10 = _mm_stream_load_si128(pSrc + 10);
        xmm11 = _mm_stream_load_si128(pSrc + 11);
        xmm12 = _mm_stream_load_si128(pSrc + 12);
        xmm13 = _mm_stream_load_si128(pSrc + 13);
        xmm14 = _mm_stream_load_si128(pSrc + 14);
        xmm15 = _mm_stream_load_si128(pSrc + 15);
#endif
        pSrc += regsInLoop;
        // _mm_store_si128 emit the SSE2 intruction MOVDQA (aligned store)
        _mm_store_si128(pTrg     , xmm0);
        _mm_store_si128(pTrg +  1, xmm1);
        _mm_store_si128(pTrg +  2, xmm2);
        _mm_store_si128(pTrg +  3, xmm3);
        _mm_store_si128(pTrg +  4, xmm4);
        _mm_store_si128(pTrg +  5, xmm5);
        _mm_store_si128(pTrg +  6, xmm6);
        _mm_store_si128(pTrg +  7, xmm7);
#ifdef _M_X64 // Use all 16 xmm registers
        _mm_store_si128(pTrg +  8, xmm8);
        _mm_store_si128(pTrg +  9, xmm9);
        _mm_store_si128(pTrg + 10, xmm10);
        _mm_store_si128(pTrg + 11, xmm11);
        _mm_store_si128(pTrg + 12, xmm12);
        _mm_store_si128(pTrg + 13, xmm13);
        _mm_store_si128(pTrg + 14, xmm14);
        _mm_store_si128(pTrg + 15, xmm15);
#endif
        pTrg += regsInLoop;
    }

    // Copy in 16 byte steps
    if (reminder >= 16)
    {
        size = reminder;
        reminder = size & 15;
        end = size >> 4;
        for (size_t i = 0; i < end; ++i)
        {
            pTrg[i] = _mm_stream_load_si128(pSrc + i);
        }
    }

    // Copy last bytes - shouldn't happen as strides are modulu 16
    if (reminder)
    {
        __m128i temp = _mm_stream_load_si128(pSrc + end);

        char* ps = (char*)(&temp);
        char* pt = (char*)(pTrg + end);

        for (size_t i = 0; i < reminder; ++i)
        {
            pt[i] = ps[i];
        }
    }

    return d;
}

#ifndef WIN64

inline void  memcpyMMX(void *Dest, void *Src, size_t nBytes)
{
	__asm
	{
		mov     esi, dword ptr[Src]
			mov     edi, dword ptr[Dest]
			mov     ecx, nBytes
			shr     ecx, 6                      // nBytes / 64
			cmp     ecx, 0
			je      Myloop
			align 8
		CopyLoop:
		movq    mm0, qword ptr[esi]
			movq    mm1, qword ptr[esi + 8 * 1]
			movq    mm2, qword ptr[esi + 8 * 2]
			movq    mm3, qword ptr[esi + 8 * 3]
			movq    mm4, qword ptr[esi + 8 * 4]
			movq    mm5, qword ptr[esi + 8 * 5]
			movq    mm6, qword ptr[esi + 8 * 6]
			movq    mm7, qword ptr[esi + 8 * 7]
			movq    qword ptr[edi], mm0
			movq    qword ptr[edi + 8 * 1], mm1
			movq    qword ptr[edi + 8 * 2], mm2
			movq    qword ptr[edi + 8 * 3], mm3
			movq    qword ptr[edi + 8 * 4], mm4
			movq    qword ptr[edi + 8 * 5], mm5
			movq    qword ptr[edi + 8 * 6], mm6
			movq    qword ptr[edi + 8 * 7], mm7
			add     esi, 64
			add     edi, 64
			loop CopyLoop
			//		emms
			align 8
		Myloop:
		mov     ecx, nBytes
			and     ecx, 63
			cmp     ecx, 0
			je EndCopyLoop
			; align 8
			; CopyLoop2:
		mov dl, char ptr[esi]
		mov  char ptr[edi], dl
			;        inc esi
			;        inc edi
			;        dec ecx
			;        jne CopyLoop2
			rep movsb
		EndCopyLoop :
	}
}

#endif