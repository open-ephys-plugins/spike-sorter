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

    /** Default constructor */
    PointD();

    /** Constructor with X/Y location*/
    PointD(float x, float y);

    /** Copy constructor */
    PointD(const PointD& P);

    /** Overload "+" operator */
    const PointD operator+(const PointD& c1) const;

    /** Overload "+= operator */
    PointD& operator+=(const PointD& rhs);

    /** Overload "-=" operator */
    PointD& operator-=(const PointD& rhs);

    /** Overload "-" operator */
    const PointD operator-(const PointD& c1) const;

    /** Overload "*" operator */
    const PointD operator*(const PointD& c1) const;

    /** Cross-product with another point */
    float cross(PointD c) const;

    /** X and Y coordinates*/
    float X, Y;
};

/** 
    Holds data about an individual spike
*/
class SorterSpikeContainer : public ReferenceCountedObject
{
public:

    /** Constructor */
    SorterSpikeContainer(const SpikeChannel* channel, uint16 sortedId, int64 timestamp, const float* data);

    /** Delete default constructor */
    SorterSpikeContainer() = delete;

    /** Return a pointer to the spike waveform data*/
    const float* getData() const;

    /** Return a pointer to the SpikeChannel object associated with this spike */
    const SpikeChannel* getChannel() const;

    /** Return the timestamp of this spike*/
    int64 getTimestamp() const;

    /** Returns the minimum value of this spike's waveform on a particular channel*/
    float getMinimum(int chan = 0);

    /** Returns the maximum value of this spike's waveform on a particular channel*/
    float getMaximum(int chan = 0);

    /** Check that the minimum is below all thresholds */
    bool checkThresholds(Array<float> thresholds);

    /** Spike color (RGB) */
    uint8 color[3];

    /** PC projections (X/Y)*/
    float pcProj[2];

    /** Sorted ID (> 0) */
    uint16 sortedId;

    /** Helper function to find the microvolts value at a given bin for one channel*/
    float spikeDataBinToMicrovolts(int bin, int ch)
    {
        jassert(ch >= 0 && ch < chan->getNumChannels());
        jassert(bin >= 0 && bin <= chan->getTotalSamples());

        float v = getData()[bin + ch * chan->getTotalSamples()];

        return v;
    }

    /** Helper function to find the microvolts value at a given index*/
    float spikeDataIndexToMicrovolts(int index)
    {
        float v = getData()[index];
        return v;
    }

    /** Helper function to find the microsecond value at a given bin for one channel*/
    float spikeTimeBinToMicrosecond(int bin, int ch = 0)
    {
        float spikeTimeSpan = 1.0f / chan->getSampleRate() * chan->getTotalSamples() * 1e6;
        return float(bin) / (chan->getTotalSamples() - 1) * spikeTimeSpan;
    }

    /** Helper function to convert from microseconds to a time bin*/
    int microSecondsToSpikeTimeBin(float t, int ch = 0)
    {
        // t = 0 corresponds to the left-most index.
        float spikeTimeSpan = (1.0f / chan->getSampleRate() * chan->getTotalSamples()) * 1e6;
        return MIN(chan->getTotalSamples() - 1, MAX(0, t / spikeTimeSpan * (chan->getTotalSamples() - 1)));
    }

private:
    int64 timestamp;
    HeapBlock<float> data;
    const SpikeChannel* chan;
};

/** Reference-counted object pointer to a spike container*/
typedef ReferenceCountedObjectPtr<SorterSpikeContainer> SorterSpikePtr;

/** Reference-counted array of spike containers*/
typedef ReferenceCountedArray<SorterSpikeContainer, CriticalSection> SorterSpikeArray;


#endif  // __CONTAINERS_H__
