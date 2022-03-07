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

    /** Sets the size of the waveform (in samples) and re-set PCA calculation */
    void resizeWaveform(int numSamples);

    /** Tests whether a candidate spike belongs to one of the defined units*/
    bool sortSpike(SorterSpikePtr so, bool PCAfirst);

    /** Tests whether a candidate spike belongs to one of the available BoxUnits*/
    bool checkBoxUnits(SorterSpikePtr so);

    /** Tests whether a candidate spike belongs to one of the available PCAUnits*/
    bool checkPCAUnits(SorterSpikePtr so);

    /** Projects a spike waveform into PC space */
	void projectOnPrincipalComponents(SorterSpikePtr so);

    /** Gets the RGB color values for a unit */
    void getUnitColor(int unitId, uint8& R, uint8& G, uint8& B);
	
    /** Triggers re-calculation of PCs */
    void RePCA();

    /** Adds a new PCA unit*/
    void addPCAunit(PCAUnit unit);

    /** Adds a new unit with a single box at some default location */
    int addBoxUnit(int channel);

    /** Adds a new unit with a custom box */
    int addBoxUnit(int channel, Box B);

    /** Adds a new box to a unit at a default location */
    bool addBoxToUnit(int channel, int unitId);

    /** Adds a new custom box to a unit */
    bool addBoxToUnit(int channel, int unitId, Box B);

    /** Removes a box from a unit based on index */
    bool removeBoxFromUnit(int unitId, int boxIndex);

    /** Returns the number of boxes for a given unit*/
    int getNumBoxes(int unitId);

    /** Removes a unit by ID */
    bool removeUnit(int unitId);

    /** Removes all units from this sorter */
    void removeAllUnits();

    /** Copies the range values for the PC axes */
    void getPCArange(float& p1min, float& p2min, float& p1max, float& p2max);

    /** Sets the range values for the PC axes */
    void setPCArange(float p1min, float p2min, float p1max, float p2max);

    /** Sets bPCAJobFinished to false */
    void resetJobStatus();

    /** Returns true if calculation is finished*/
    bool isPCAfinished();

    /** Returns true if calculation is finished*/
    bool firstJobFinished();

    /** Returns a vector of all boxes for a BoxUnit */
    std::vector<Box> getUnitBoxes(int unitId);

    /** Returns a vector of all BoxUnits */
    std::vector<BoxUnit> getBoxUnits();

    /** Returns a vector of all PCAUnits */
    std::vector<PCAUnit> getPCAUnits();

    /** Sets the BoxUnits for this Sorter */
    void updateBoxUnits(std::vector<BoxUnit> _units);

    /** Sets the PCAUnits for this Sorter */
    void updatePCAUnits(std::vector<PCAUnit> _units);

    /** Generates the next global unit ID (across all Sorters) */
    static int generateUnitId();

    /** Generates the next available local unit ID (for this Sorter) */
    int generateLocalId();

    /** Re-generates IDs for all units */
    void generateNewIds();

    /** Selects a box for a particular unit */
    void setSelectedUnitAndBox(int unitId, int boxId);

    /** Returns the selected unit and box*/
    void getSelectedUnitAndBox(int& unitId, int& boxId);

    /** Saves sorting parameters for one electrode */
    void saveCustomParametersToXml(XmlElement* electrodeNode);

    /** Loads sorting parameters for one electrode*/
    void loadCustomParametersFromXml(XmlElement* electrodeNode);

private:

    CriticalSection mut;

    Electrode* electrode;

    PCAComputingThread* computingThread;

    SorterSpikeArray spikeBuffer;

    static int nextUnitId;

    std::vector<BoxUnit> boxUnits;
    std::vector<PCAUnit> pcaUnits;

    int numChannels, waveformLength;
    int selectedUnit, selectedBox;
    
    float* pc1, *pc2;
    std::atomic<float> pc1min, pc2min, pc1max, pc2max;
    
    int bufferSize,spikeBufferIndex;
    
    bool bPCAJobSubmitted,bPCAComputed, bRePCA, bPCAFirstJobFinished;
    std::atomic<bool> bPCAJobFinished;

};


#endif // __SPIKESORTBOXES_H
