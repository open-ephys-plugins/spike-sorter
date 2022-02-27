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

#include <stdio.h>
#include <algorithm>

#include "PCAComputingThread.h"

void PCAComputingThread::addPCAjob(PCAJobPtr job)
{
	{
		ScopedLock critical(lock);
		jobs.add(job);
	}
	
    if (!isThreadRunning())
    {
        startThread();
    }
}

void PCAComputingThread::run()
{
    while (jobs.size() > 0)
    {
		lock.enter();
        PCAJobPtr J = jobs.removeAndReturn(0);
	if (J == nullptr) continue;
		lock.exit();
        // compute PCA
        // 1. Compute Covariance matrix
        // 2. Apply SVD on covariance matrix
        // 3. Extract the two principal components corresponding to the largest singular values

        J->computeCov();
        J->computeSVD();

        // 4. Report to the spike sorting electrode that PCA is finished
        J->reportDone = true;
    }
}


PCAComputingThread::PCAComputingThread() : Thread("PCA")
{

}
