//
// Inter.cpp -- Copyright (c) Microsoft Corporation. All rights reserved.
//
// Description:
//
//  Implementation of GainControl
//
#include <atlbase.h>
#include <atlcom.h>
#include <atlcoll.h>
#include <atlsync.h>
#include <mmreg.h>

#include <audioenginebaseapo.h>
#include <baseaudioprocessingobject.h>
#include <resource.h>

#include <float.h>

#include "InterAPO.h"

#include <EvnTrace.h>
#include "trace.h"
#include "Inter.tmh"

#pragma AVRT_CODE_BEGIN

void WriteSilence(
    _Out_writes_(u32FrameCount * u32SamplesPerFrame)
        FLOAT32 *pf32Frames,
    UINT32 u32FrameCount,
    UINT32 u32SamplesPerFrame )
{
    ZeroMemory(pf32Frames, sizeof(FLOAT32) * u32FrameCount * u32SamplesPerFrame);
}
#pragma AVRT_CODE_END

#pragma AVRT_CODE_BEGIN
void CopyFrames(
    _Out_writes_(u32FrameCount * u32SamplesPerFrame)
        FLOAT32 *pf32OutFrames,
    _In_reads_(u32FrameCount * u32SamplesPerFrame)
        const FLOAT32 *pf32InFrames,
    UINT32 u32FrameCount,
    UINT32 u32SamplesPerFrame )
{
    CopyMemory(pf32OutFrames, pf32InFrames, sizeof(FLOAT32) * u32FrameCount * u32SamplesPerFrame);
}
#pragma AVRT_CODE_END

#pragma AVRT_CODE_BEGIN
void GainControl(
    _Out_writes_(u32ValidFrameCount* u32SamplesPerFrame)
    FLOAT32* pf32OutputFrames,
    _In_reads_(u32ValidFrameCount* u32SamplesPerFrame)
    const FLOAT32* pf32InputFrames,
    UINT32       u32ValidFrameCount,
    UINT32       u32SamplesPerFrame,
    INT32        i32GainLevel)
{
    ASSERT_REALTIME();
    ATLASSERT(IS_VALID_TYPED_READ_POINTER(pf32InputFrames));
    ATLASSERT(IS_VALID_TYPED_READ_POINTER(pf32OutputFrames));

    FLOAT amplitude = (FLOAT)pow(10, i32GainLevel / 20.0);

    // Invariants:
    // 0 <= (*pu32InterIndex) < u32InterFrames
    // pf32OutputFrames[0 ... u32ValidFrameCount * u32SamplesPerFrame - 1] is writable
    // pf32InputFrames[0 ... u32ValidFrameCount * u32SamplesPerFrame - 1] is readable
    while (u32ValidFrameCount > 0)
    {
        // Process as many frames as possible from the input buffer
        UINT32 u32FramesToProcess = min(u32ValidFrameCount, u32SamplesPerFrame);
        for (UINT32 u32Frame = 0; u32Frame < u32FramesToProcess; u32Frame++)
        {
            // Process each sample in the frame
            for (UINT32 u32Sample = 0; u32Sample < u32SamplesPerFrame; u32Sample++)
            {
                // Apply the gain
                pf32OutputFrames[u32Frame * u32SamplesPerFrame + u32Sample] = pf32InputFrames[u32Frame * u32SamplesPerFrame + u32Sample] * amplitude;
            }
        }

        // Update the frame count
        u32ValidFrameCount -= u32FramesToProcess;
        pf32OutputFrames += u32FramesToProcess * u32SamplesPerFrame;
        pf32InputFrames += u32FramesToProcess * u32SamplesPerFrame;
    }
}
#pragma AVRT_CODE_END

#pragma AVRT_CODE_BEGIN
void Equalizer(
    _Out_writes_(u32ValidFrameCount* u32SamplesPerFrame)
    FLOAT32* pf32OutputFrames,
    _In_reads_(u32ValidFrameCount* u32SamplesPerFrame)
    const FLOAT32* pf32InputFrames,
    UINT32       u32ValidFrameCount,
    UINT32       u32SamplesPerFrame,
    BiQuadFilter *filter)
{
    ASSERT_REALTIME();
    ATLASSERT(IS_VALID_TYPED_READ_POINTER(pf32InputFrames));
    ATLASSERT(IS_VALID_TYPED_READ_POINTER(pf32OutputFrames));

    // Invariants:
    // 0 <= (*pu32InterIndex) < u32InterFrames
    // pf32OutputFrames[0 ... u32ValidFrameCount * u32SamplesPerFrame - 1] is writable
    // pf32InputFrames[0 ... u32ValidFrameCount * u32SamplesPerFrame - 1] is readable
    while (u32ValidFrameCount > 0)
    {
        // Process as many frames as possible from the input buffer
        UINT32 u32FramesToProcess = min(u32ValidFrameCount, u32SamplesPerFrame);
        for (UINT32 u32Frame = 0; u32Frame < u32FramesToProcess; u32Frame++)
        {
            // Process each sample in the frame
            for (UINT32 u32Sample = 0; u32Sample < u32SamplesPerFrame; u32Sample++)
            {
                // Apply the equalizer
                pf32OutputFrames[u32Frame * u32SamplesPerFrame + u32Sample] = filter->Process(pf32InputFrames[u32Frame * u32SamplesPerFrame + u32Sample]);
            }
        }

        // Update the frame count
        u32ValidFrameCount -= u32FramesToProcess;
        pf32OutputFrames += u32FramesToProcess * u32SamplesPerFrame;
        pf32InputFrames += u32FramesToProcess * u32SamplesPerFrame;
    }
}
#pragma AVRT_CODE_END
