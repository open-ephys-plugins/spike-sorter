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

#ifndef __SORTER_H
#define __SORTER_H

#include <ProcessorHeaders.h>

#include "Containers.h"

#include <algorithm>    // std::sort
#include <list>
#include <queue>
#include <atomic>

class PCAUnit;
class PCAComputingThread;
class Box;
class BoxUnit;
class Electrode;

/** 
    Sorts spikes from a single electrode (1-4 channels)

    Each Sorter can have an arbitrary number of Box units and PCA Units
*/
class Sorter
{
public:

    /** Constructor */
    Sorter(Electrode* electrode, PCAComputingThread* pcaThread);

    /** Destructor */
    ~Sorter();

    void resizeWaveform(int numSamples);

	void projectOnPrincipalComponents(SorterSpikePtr so);
	bool sortSpike(SorterSpikePtr so, bool PCAfirst);
    void RePCA();
    void addPCAunit(PCAUnit unit);
    int addBoxUnit(int channel);
    int addBoxUnit(int channel, Box B);

    void getPCArange(float& p1min,float& p2min, float& p1max,  float& p2max);
    void setPCArange(float p1min,float p2min, float p1max,  float p2max);
    void resetJobStatus();
    bool isPCAfinished();

    bool removeUnit(int unitID);

    void removeAllUnits();
    bool addBoxToUnit(int channel, int unitID);
    bool addBoxToUnit(int channel, int unitID, Box B);
    bool removeBoxFromUnit(int unitID, int boxIndex);
    int getNumBoxes(int unitID);
    std::vector<Box> getUnitBoxes(int unitID);
    std::vector<BoxUnit> getBoxUnits();
    std::vector<PCAUnit> getPCAUnits();

    void getUnitColor(int UnitID, uint8& R, uint8& G, uint8& B);
    void updateBoxUnits(std::vector<BoxUnit> _units);
    void updatePCAUnits(std::vector<PCAUnit> _units);
    int generateUnitID();
    int generateLocalID();
    void generateNewIDs();
    void setSelectedUnitAndBox(int unitID, int boxID);
    void getSelectedUnitAndBox(int& unitID, int& boxid);
    void saveCustomParametersToXml(XmlElement* electrodeNode);
    void loadCustomParametersFromXml(XmlElement* electrodeNode);
private:

    Electrode* electrode;

    static int nextUnitId;

    int numChannels, waveformLength;
    int selectedUnit, selectedBox;
    CriticalSection mut;
    std::vector<BoxUnit> boxUnits;
    std::vector<PCAUnit> pcaUnits;
    float* pc1, *pc2;
    std::atomic<float> pc1min, pc2min, pc1max, pc2max;
    SorterSpikeArray spikeBuffer;
    int bufferSize,spikeBufferIndex;
    PCAComputingThread* computingThread;
    bool bPCAJobSubmitted,bPCAcomputed,bRePCA;
    std::atomic<bool> bPCAjobFinished ;

};


#endif // __SPIKESORTBOXES_H
