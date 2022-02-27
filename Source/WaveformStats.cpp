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

#include "WaveformStats.h"

// Running variance...
//Mk = Mk-1+ (xk - Mk-1)/k
//Sk = Sk-1 + (xk - Mk-1)*(xk - Mk).
//For 2 ≤ k ≤ n, the kth estimate of the variance is s2 = Sk/(k - 1).
WaveformStats::~WaveformStats()
{
}
WaveformStats::WaveformStats()
{
    numSamples = 0;
}

void WaveformStats::reset()
{
    numSamples = 0;
}


std::vector<double> WaveformStats::getMean(int index)
{
    std::vector<double> m;

    if (numSamples == 0)
    {
        return m;
    }

    int numSamplesInWaveForm = (int) WaveFormMean[0].size();
    m.resize(numSamplesInWaveForm);

    for (int k = 0; k < numSamplesInWaveForm; k++)
        m[k] = WaveFormMean[index][k];
    return m;
}

std::vector<double> WaveformStats::getStandardDeviation(int index)
{
    std::vector<double> WaveFormVar;

    if (numSamples == 0)
    {
        return WaveFormVar;
    }
    int numSamplesInWaveForm = (int) WaveFormMean[0].size();
    WaveFormVar.resize(numSamplesInWaveForm);

    for (int j = 0; j < numSamplesInWaveForm; j++)
    {
        if (numSamples - 1 == 0)
            WaveFormVar[j] = 0;
        else
            WaveFormVar[j] = sqrt(WaveFormSk[index][j] / (numSamples - 1));
    }
    return WaveFormVar;
}


void WaveformStats::resizeWaveform(int newlength)
{
    numSamples = 0; // this should ensure that update reallocates upon the next update.
}

void WaveformStats::update(SorterSpikePtr so)
{
    double ts = so->getTimestamp()/so->getChannel()->getSampleRate();
    if (numSamples == 0)
    {
        LastSpikeTime = ts;
    }
    else
    {
        //hist.update(1000.0 * (ts - LastSpikeTime));
        LastSpikeTime = ts;
    }

    newData = true;
	int nChannels = so->getChannel()->getNumChannels();
	int nSamples = so->getChannel()->getTotalSamples();
    if (numSamples == 0)
    {
        // allocate
        WaveFormMean.resize(nChannels);
        WaveFormSk.resize(nChannels);
        WaveFormMk.resize(nChannels);
        for (int k=0; k<nChannels; k++)
        {
            WaveFormMean[k].resize(nSamples);
            WaveFormSk[k].resize(nSamples);
            WaveFormMk[k].resize(nSamples);
        }

        for (int i = 0; i < nChannels; i++)
        {
            for (int j = 0; j < nSamples; j++)
            {
				WaveFormMean[i][j] = so->getData()[j + i*nSamples];
                WaveFormSk[i][j] = 0;
				WaveFormMk[i][j] = so->getData()[j + i*nSamples];
            }
        }
        numSamples += 1.0F;
        return;
    }
    // running mean
    for (int i = 0; i < nChannels; i++)
    {
        for (int j = 0; j < nSamples; j++)
        {
			WaveFormMean[i][j] = (numSamples * WaveFormMean[i][j] + so->getData()[j + i*nSamples]) / (numSamples + 1);
			WaveFormMk[i][j] += (so->getData()[j + i*nSamples] - WaveFormMk[i][j]) / numSamples;
			WaveFormSk[i][j] += (so->getData()[j + i*nSamples] - WaveFormMk[i][j]) * (so->getData()[j + i*nSamples] - WaveFormMk[i][j]);
        }
    }
    numSamples += 1.0F;
}


bool WaveformStats::queryNewData()
{
    if (newData == false)
        return false;
    newData = false;
    return true;
}
