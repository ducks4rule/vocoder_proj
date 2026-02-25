# Vocoder-TUI

A real-time vocoder with TUI for Arch Linux. Captures microphone input, processes it through FFT-based pitch shifting (phase vocoder using STFT), and outputs to speakers.

## Features

### Audio Processing
- Real-time microphone input via ALSA
- FFT-based pitch shifting using SMB PitchShift algorithm (STFT + phase vocoder)
- Log-frequency transformation for analysis
- Low-latency audio processing with configurable buffer sizes

### TUI (Terminal User Interface)
- Spectrum visualizer using ASCII/UTF-8 characters
- Real-time display of:
  - Current pitch shift value (semitones and frequency ratio)
  - Input level meter
  - Frequency spectrum
- Color-coded interface

### Keyboard Controls

#### Pitch Control (Homerow Keys)
| Key | Note | Frequency (Hz) | Semitones from C4 |
|-----|------|----------------|---------------------|
| `W` | C#4 | 277.18 | 1 |
| `A` | C4 | 261.63 | 0 |
| `S` | D4 | 293.66 | 2 |
| `E` | D#4 | 311.13 | 3 |
| `D` | E4 | 329.63 | 4 |
| `F` | F4 | 349.23 | 5 |
| `T` | F#4 | 369.99 | 6 |
| `G` | G4 | 392.00 | 7 |
| `Y` | G#4 | 415.30 | 8 |
| `H` | A4 | 440.00 | 9 |
| `U` | A#4 | 466.16 | 10 |
| `J` | B4 | 493.88 | 11 |
| `K` | C5 | 523.25 | 12 |

#### Control Keys
| Key | Function |
|-----|----------|
| `q` | Quit application |
| `+` / `-` | Adjust frequency ratio by ±0.1 |
| `=` / `_` | Fine tune frequency ratio by ±0.01 |
| `r` | Reset pitch to 1.0 (no shift) |
| `m` | Mute/unmute output |
| `h` | Show help |

### Technical Details

#### Dependencies
- `alsa-lib` - Linux audio I/O
- `ncurses` - Terminal user interface
- `fftw3` - Fast Fourier Transform (GPL)
- `cmake` - Build system
- `gcc` / `clang` - C++ compiler

#### Audio Parameters
- Sample Rate: 44100 Hz (configurable)
- FFT Size: 4096 samples
- Hop Size: 1024 samples (for 75% overlap)
- Window Function: Hann window

#### Algorithm
The vocoder uses the SMB PitchShift algorithm:
1. Apply Hann window to input frame
2. Compute FFT (STFT)
3. Convert to log-frequency scale
4. Apply pitch shift by scaling frequency bins
    - Adjust phase
5. Convert back to linear frequency
6. Compute inverse FFT
7. Overlap-add output frames

## Installation (Arch Linux)

```bash
# Install dependencies
sudo pacman -S alsa-lib ncurses cmake base-devel fftw

# Build from AUR (if available) or manually:
cd vocoder-tui
mkdir build && cd build
cmake ..
make
sudo make install
```

Or use the provided PKGBUILD:
```bash
makepkg -si
```

## Usage

```bash
vocoder-tui
```

The application will:
1. Initialize ALSA
2. Open default audio input device (microphone)
3. Start the TUI
4. Process microphone input in real-time

Press `q` to quit.

## TODO

### Phase 1: Project Setup
- [x] Create project directory structure (src/audio, src/dsp, src/ui)
- [x] Create CMakeLists.txt with build configuration
- [x] Create .gitignore
- [ ] Create stub source files for compilation test

### Phase 2: Audio I/O
- [ ] Implement ALSA audio capture (microphone input)
  - [ ] Initialize ALSA PCM device
  - [ ] Configure capture parameters (44100Hz, 16-bit, mono)
  - [ ] Implement ring buffer for audio data
  - [ ] Handle audio device errors and recovery
- [ ] Implement ALSA audio playback (speaker output)
  - [ ] Initialize playback PCM device
  - [ ] Configure playback parameters
  - [ ] Implement low-latency output buffer
  - [ ] Sync capture and playback streams

### Phase 3: DSP / Pitch Shifting
- [ ] Implement FFT wrapper (fftw3)
  - [ ] Create forward FFT function
  - [ ] Create inverse FFT function
  - [ ] Handle memory allocation and cleanup
- [ ] Implement SMB PitchShift algorithm
  - [ ] Implement Hann window function
  - [ ] Implement STFT (Short-Time Fourier Transform)
  - [ ] Implement frequency bin scaling for pitch shift
  - [ ] Implement phase vocoder for smooth pitch shifting
  - [ ] Implement overlap-add synthesis
- [ ] Implement log-frequency transformation

### Phase 4: TUI
- [ ] Implement ncurses TUI framework
  - [ ] Initialize ncurses (colors, keypad, cursor)
  - [ ] Create main render loop
  - [ ] Handle terminal resize
  - [ ] Cleanup on exit
- [ ] Implement spectrum visualizer
  - [ ] Convert FFT magnitude to dB scale
  - [ ] Map to ASCII/UTF-8 characters
  - [ ] Implement log-frequency scaling for display
- [ ] Implement UI components
  - [ ] Input level meter (VU meter)
  - [ ] Pitch shift display (semitones + ratio)
  - [ ] Frequency spectrum display
- [ ] Implement help overlay

### Phase 5: Controls
- [ ] Implement keyboard input handling
  - [ ] Homerow keys (A-K) for pitch selection
  - [ ] +/- for frequency ratio adjustment
  - [ ] =/_ for fine tune
  - [ ] r - reset pitch
  - [ ] m - mute/unmute
  - [ ] h - toggle help
  - [ ] q - quit

### Phase 6: Integration & Testing
- [ ] Integrate audio, DSP, and TUI components
- [ ] Test real-time audio processing latency
- [ ] Tune FFT size and hop size for performance
- [ ] Handle audio glitches and buffer underruns
- [ ] Test on target system (Arch Linux)
- [ ] Optimize for low CPU usage

### Phase 7: Polish
- [ ] Add error handling throughout
- [ ] Add configuration file support
- [ ] Add command-line arguments
- [ ] Create man page
- [ ] Create PKGBUILD for AUR
