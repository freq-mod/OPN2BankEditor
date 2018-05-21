/*
 * OPN2 Bank Editor by Wohlstand, a free tool for music bank editing
 * Copyright (c) 2017-2018 Vitaly Novichkov <admin@wohlnet.ru>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef GENERATOR_H
#define GENERATOR_H

#include "chips/opn_chip_base.h"

#include "../bank.h"
#include <stdint.h>
#include <memory>
#include <mutex>

#include <QIODevice>
#include <QObject>

#define NUM_OF_CHANNELS         6
#define MAX_OPLGEN_BUFFER_SIZE  4096

struct OPN_Operator
{
    //! Raw register data
    uint8_t     data[7];
};

struct OPN_PatchSetup
{
    //! Operators prepared for sending to OPL chip emulator
    OPN_Operator    OPS[4];
    uint8_t         fbalg;
    uint8_t         lfosens;
    //! Fine tuning
    int8_t          finetune;
    //! Single note (for percussion instruments)
    uint8_t         tone;
};

struct GeneratorDebugInfo
{
    int32_t chan4op = -1;
    QString toStr();
};

class Generator
{
public:
    enum OPN_Chips
    {
        CHIP_Nuked = 0,
        CHIP_GENS,
        CHIP_MAME
    };
    Generator(uint32_t sampleRate, OPN_Chips initialChip);
    ~Generator();

    void initChip();
    void switchChip(OPN_Chips chipId);

    void generate(int16_t *frames, unsigned nframes);

    void NoteOn(uint32_t c, double hertz);
    void NoteOff(uint32_t c);
    void Touch_Real(uint32_t c, uint32_t volume);
    void Touch(uint32_t c, uint32_t volume);
    void Patch(uint32_t c);
    void Pan(uint32_t c, uint8_t value);
    void PlayNoteF(int noteID);
    void StopNoteF(int noteID);
    void PlayDrum(uint8_t drum, int noteID);

public:
    void Silence();
    void NoteOffAllChans();

    void PlayNote();
    void PlayMajorChord();
    void PlayMinorChord();
    void PlayAugmentedChord();
    void PlayDiminishedChord();
    void PlayMajor7Chord();
    void PlayMinor7Chord();
    void StopNote();

    void changePatch(const FmBank::Instrument &instrument, bool isDrum = false);
    void changeNote(int newnote);

    void changeLFO(bool enabled);
    void changeLFOfreq(int freq);

    const GeneratorDebugInfo &debugInfo() const
        { return m_debug; }

private:
    GeneratorDebugInfo m_debug;

private:
    void WriteReg(uint8_t port, uint16_t address, uint8_t byte);

    class NotesManager
    {
        struct Note
        {
            //! Currently pressed key. -1 means channel is free
            int note    = -1;
            //! Age in count of noteOn requests
            int age = 0;
        };
        //! Channels range, contains entries count equal to chip channels
        QVector<Note> channels;
        //! Round-Robin cycler. Looks for any free channel that is not busy. Otherwise, oldest busy note will be replaced
        uint8_t cycle = 0;
    public:
        NotesManager();
        ~NotesManager();
        void allocateChannels(int count);
        uint8_t noteOn(int note);
        int8_t  noteOff(int note);
        void clearNotes();
    } m_noteManager;

    int32_t     note;
    bool        m_isInstrumentLoaded = false;
    uint8_t     lfo_enable = 0x00;
    uint8_t     lfo_freq   = 0x00;
    uint8_t     lfo_reg    = 0x00;

    uint8_t     testDrum;
    OPN_PatchSetup m_patch;

    uint32_t    m_rate = 44100;

    std::unique_ptr<OPNChipBase> chip;

    //! index of operators pair, cached, needed by Touch()
    uint16_t    m_ins[NUM_OF_CHANNELS];
    //! value poked to B0, cached, needed by NoteOff)(
    uint8_t     m_pit[NUM_OF_CHANNELS];
    //! LFO and panning value cached
    uint8_t     m_pan_lfo[NUM_OF_CHANNELS];
};

#endif // GENERATOR_H
