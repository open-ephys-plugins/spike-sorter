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

private:
    int64 timestamp;
    HeapBlock<float> data;
    const SpikeChannel* chan;
};

typedef ReferenceCountedObjectPtr<SorterSpikeContainer> SorterSpikePtr;
typedef ReferenceCountedArray<SorterSpikeContainer, CriticalSection> SorterSpikeArray;




#endif  // __CONTAINERS_H__
