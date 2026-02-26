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

## Implementation Notes

### Audio Level Meters
The input and output level meters display audio levels in dB scale (-60 to 0 dB):
- **Buffer initialization**: Audio buffers are zeroed with `memset()` each iteration to handle partial ALSA reads
- **dB calculation**: Only calculated when audio is successfully captured (`captured > 0`)
- **Smoothing**: Exponential moving average (0.3 factor) applied in TUI render to reduce flicker

### Logging
- Uses singleton Logger with thread-safe mutex
- Log levels: DEBUG, INFO, ERROR
- Output to file `/tmp/vocoder-tui.log` with timestamps
- Errors also printed to stderr
- Macros: `LOG_DEBUG()`, `LOG_INFO()`, `LOG_ERROR()`

## TODO

### Phase 1: Project Setup
- [x] Create project directory structure (src/audio, src/dsp, src/ui)
- [x] Create CMakeLists.txt with build configuration
- [x] Create .gitignore
- [x] Create stub source files for compilation test

### Phase 2: Audio I/O
- [x] Implement ALSA audio capture (microphone input)
  - [x] Initialize ALSA PCM device
  - [x] Configure capture parameters (44100Hz, float32, mono)
  - [x] Handle partial reads with buffer zeroing
  - [x] Handle audio device errors and recovery
- [x] Implement ALSA audio playback (speaker output)
  - [x] Initialize playback PCM device
  - [x] Configure playback parameters
  - [x] Handle EAGAIN and buffer underruns

### Phase 2.5: Audio Features
- [x] Add mute/unmute output
- [x] Add output volume control (0.0 - 1.0)
- [ ] Add playback delay (monitoring delay)
  - [ ] Implement ring buffer for delay
  - [ ] Make delay configurable (0-500ms)
- [ ] Add device selection
  - [ ] List available audio devices
  - [ ] Allow selecting capture device
  - [ ] Allow selecting playback device

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
- [x] Implement ncurses TUI framework
  - [x] Initialize ncurses (colors, keypad, cursor)
  - [x] Create main render loop
  - [ ] Handle terminal resize
  - [x] Cleanup on exit
- [ ] Implement spectrum visualizer
  - [ ] Convert FFT magnitude to dB scale
  - [ ] Map to ASCII/UTF-8 characters
  - [ ] Implement log-frequency scaling for display
- [x] Implement UI components
  - [x] Input level meter (VU meter)
  - [x] Master volume indicator
  - [ ] Move mute sign to avoid overlap with master meter
  - [ ] Pitch shift display (semitones + ratio)
  - [ ] Frequency spectrum display
  - [x] Mute status indicator
  - [x] Volume level indicator
  - [ ] Delay amount indicator
  - [ ] Current device names (capture/playback)
- [ ] Implement device selection UI
  - [ ] List available devices
  - [ ] Allow selecting capture device
  - [ ] Allow selecting playback device
- [ ] Implement help overlay

### Phase 5: Controls
- [x] Implement keyboard input handling (basic q to quit)
- [ ] Implement full keyboard controls
  - [ ] Homerow keys (A-K) for pitch selection
  - [x] +/- for frequency ratio adjustment
  - [x] =/_ for fine tune
  - [x] r - reset pitch
  - [x] m - mute/unmute microphone
  - [ ] v - cycle volume preset
  - [x] [ - decrease volume
  - [x] ] - increase volume
  - [ ] d - adjust delay
  - [ ] < - decrease delay
  - [ ] > - increase delay
  - [ ] i - show input devices
  - [ ] o - show/list output devices
  - [ ] 1-9 - select input device by number
  - [ ] Shift+1-9 - select output device by number
  - [ ] h - toggle help
  - [x] q - quit

### Phase 6: Integration & Testing
- [x] Integrate audio, DSP, and TUI components
- [ ] Test real-time audio processing latency
- [ ] Tune FFT size and hop size for performance
- [x] Handle audio glitches and buffer underruns
- [ ] Test on target system (Arch Linux)
- [ ] Optimize for low CPU usage

### Phase 7: Polish
- [x] Add error handling throughout
- [ ] Add configuration file support
- [ ] Add command-line arguments
- [ ] Create man page
- [ ] Create PKGBUILD for AUR
- [ ] Implement playback delay (monitoring delay)

### Future Design Considerations
- [ ] Implement delay option for monitoring
- [ ] Consider switching to callback-based audio (async) for lower latency
  - Current: loop-based (polling) - simpler, easier to debug
  - Future: callback-based - lower CPU, lower latency, more complex
  - Would require restructuring main loop and adding thread synchronization
