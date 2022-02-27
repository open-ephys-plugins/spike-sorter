/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2013 Open Ephys

    ------------------------------------------------------------------

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef __CONTAINERS_H__
#define __CONTAINERS_H__

#include <ProcessorHeaders.h>

#ifndef MAX
#define MAX(x,y)((x)>(y))?(x):(y)
#endif

#ifndef MIN
#define MIN(x,y)((x)<(y))?(x):(y)
#endif

/** 
    Represents a point in 2D space
*/
class PointD
{
public:

    PointD();
    PointD(float x, float y);
    PointD(const PointD& P);
    const PointD operator+(const PointD& c1) const;
    PointD& operator+=(const PointD& rhs);
    PointD& operator-=(const PointD& rhs);

    const PointD operator-(const PointD& c1) const;
    const PointD operator*(const PointD& c1) const;

    float cross(PointD c) const;
    float X, Y;
};

/** 
    Holds data about an individual spike
*/
class SorterSpikeContainer : public ReferenceCountedObject
{
public:

    /** Constructor */
    SorterSpikeContainer(const SpikeChannel* channel, SpikePtr spike);

    /** Delete default constructor */
    SorterSpikeContainer() = delete;

    /** Return a pointer to the spike waveform data*/
    const float* getData() const;

    /** Return a pointer to the SpikeChannel object associated with this spike */
    const SpikeChannel* getChannel() const;

    /** Return the timestamp of this spike*/
    int64 getTimestamp() const;

    /** Spike color (RGB) */
    uint8 color[3];

    /** PC projections (X/Y)*/
    float pcProj[2];

    /** Sorted ID (> 0) */
    uint16 sortedId;

    float spikeDataBinToMicrovolts(int bin, int ch)
    {
        jassert(ch >= 0 && ch < chan->getNumChannels());
        jassert(bin >= 0 && bin <= chan->getTotalSamples());
        float v = getData()[bin + ch * chan->getTotalSamples()];
        return v;
    }

    float spikeDataIndexToMicrovolts(int index)
    {
        float v = getData()[index];
        return v;
    }

    float spikeTimeBinToMicrosecond(int bin, int ch = 0)
    {
        float spikeTimeSpan = 1.0f / chan->getSampleRate() * chan->getTotalSamples() * 1e6;
        return float(bin) / (chan->getTotalSamples() - 1) * spikeTimeSpan;
    }

    int microSecondsToSpikeTimeBin(float t, int ch = 0)
    {
        // Lets say we have 32 samples per wave form

        // t = 0 corresponds to the left most index.
        float spikeTimeSpan = (1.0f / chan->getSampleRate() * chan->getTotalSamples()) * 1e6;
        return MIN(chan->getTotalSamples() - 1, MAX(0, t / spikeTimeSpan * (chan->getTotalSamples() - 1)));
    }

private:
    int64 timestamp;
    HeapBlock<float> data;
    const SpikeChannel* chan;
};

typedef ReferenceCountedObjectPtr<SorterSpikeContainer> SorterSpikePtr;
typedef ReferenceCountedArray<SorterSpikeContainer, CriticalSection> SorterSpikeArray;




#endif  // __CONTAINERS_H__
