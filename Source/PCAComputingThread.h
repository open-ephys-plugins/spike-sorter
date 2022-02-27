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

#ifndef __PCACOMPUTINGTHREAD_H
#define __PCACOMPUTINGTHREAD_H

#include <ProcessorHeaders.h>

#include "PCAJob.h"

#include <algorithm>    // std::sort
#include <list>
#include <queue>
#include <atomic>

/** 

    Thread for managing PCA jobs

*/
class PCAComputingThread : juce::Thread
{
public:

    /** Constructor */
    PCAComputingThread();

    /** Computes PCA on waveforms */
    void run();

    /** Adds a job to the queue*/
    void addPCAjob(PCAJobPtr job);

private:
    PCAJobArray jobs;
	CriticalSection lock;
};


#endif // __PCACOMPUTINGTHREAD_H
