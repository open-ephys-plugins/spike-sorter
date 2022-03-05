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

#ifndef __PCAJOB_H
#define __PCAJOB_H

#include <ProcessorHeaders.h>

#include "Containers.h"

#include <algorithm>
#include <list>
#include <queue>
#include <atomic>

/** 
    
    Represents one job for analyzing an array of incoming spikes.

*/
class PCAjob : public ReferenceCountedObject
{
public:

    /** Constructor */
    PCAjob(SorterSpikeArray& _spikes, float* _pc1, float* _pc2,
           std::atomic<float>&,  std::atomic<float>&,  std::atomic<float>&,  std::atomic<float>&, std::atomic<bool>& _reportDone);

    /** Destructor */
    ~PCAjob();

    /** Computes covariance of the waveforms*/
    void computeCov();

    /** Computes the Singular Value Decomposition of the waveforms*/
    void computeSVD();

    float** cov;
    SorterSpikeArray spikes;
    float* pc1, *pc2;
    std::atomic<float>& pc1min, &pc2min, &pc1max, &pc2max;
    std::atomic<bool>& reportDone;

private:
    
    int svdcmp(float** a, int nRows, int nCols, float* w, float** v);
    float pythag(float a, float b);
    int dim;
};

typedef ReferenceCountedObjectPtr<PCAjob> PCAJobPtr;
typedef ReferenceCountedArray<PCAjob, CriticalSection> PCAJobArray;

#endif // __PCAJOB_H
