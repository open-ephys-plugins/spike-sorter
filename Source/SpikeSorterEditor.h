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

#ifndef __SPIKESORTEREDITOR_H_F0BD2DD9__
#define __SPIKESORTEREDITOR_H_F0BD2DD9__

#include <VisualizerEditorHeaders.h>

class Electrode;
class SpikeSorterCanvas;

/**

  User interface for the SpikeSorter processor.

  Allows the user to select an electrode to add box / PCA units to

  @see SpikeSorter

*/

class SpikeSorterEditor : public VisualizerEditor,
    public ComboBox::Listener
{
public:
    /** Constructor*/
    SpikeSorterEditor(GenericProcessor* parentNode);

    /** Destructor*/
    virtual ~SpikeSorterEditor() { }

    /** Creates the SpikeSorterCanvas */
    Visualizer* createNewCanvas();

    /** ComboBox::Listener callback*/
    void comboBoxChanged(ComboBox* comboBox);

    /** Selects the next available electrode */
    void nextElectrode();

    /** Selects the previous electrode */
    void previousElectrode();

    /** Called when selected stream is updated*/
    void SpikeSorterEditor::selectedStreamHasChanged() override;

private:

	ScopedPointer<ComboBox> electrodeList;

    Array<Electrode*> currentElectrodes;

    SpikeSorterCanvas* spikeSorterCanvas;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpikeSorterEditor);

};




#endif  // __SPIKESORTEREDITOR_H_F0BD2DD9__
