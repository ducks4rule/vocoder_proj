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

- [ ] Create project structure (CMakeLists.txt, src directories)
- [ ] Implement ALSA audio capture and playback
- [ ] Implement FFT wrapper using fftw3
- [ ] Implement SMB pitch shift algorithm
- [ ] Implement TUI with spectrum visualizer
- [ ] Implement keyboard controls
- [ ] Test and verify audio processing works
- [ ] Build and test on target system
