# Orion Space Fuzz

![Format](https://img.shields.io/badge/Format-VST3%20%2F%20AU%20%2F%20Standalone-blue)
![C++](https://img.shields.io/badge/Language-C++17-00599C?logo=c%2B%2B)
![JUCE](https://img.shields.io/badge/Framework-JUCE%208-green)
![DSP](https://img.shields.io/badge/DSP-Real--Time_Audio-red)

**Orion Space Fuzz** is a real-time audio effect plugin (VST3/AU/Standalone) built with C++ and the JUCE framework. 

Designed as a sonic tribute to the legendary bass tone of Cliff Burton (specifically his spacey, aggressive, cello-like swells in tracks like *Orion*), this plugin combines **Non-linear Waveshaping (Fuzz)**, **Modulation (Chorus)**, and **Spatial Echo (Ring Buffer Delay)** into a single, cohesive DSP signal chain.

This project was developed to demonstrate core principles of Digital Signal Processing (DSP), memory-safe real-time C++ programming, and physical acoustic modeling.

---

## Parameters & UI Guide

The graphical user interface features a 2x3 grid of controls, allowing deep manipulation of the signal chain:

### Fuzz Stage
* **Fuzz Drive**: Controls the input gain fed into the non-linear waveshaper. Higher values aggressively square off the waveform, creating rich, harsh even and odd harmonics.

### Chorus Stage
* **Chorus Rate**: Adjusts the frequency of the Low-Frequency Oscillator (LFO) that modulates the delayed signal in the chorus block, determining the "speed" of the swirl.
* **Chorus Depth**: Controls the amplitude of the LFO, determining how wide the pitch modulation fluctuates.

### Delay Stage
* **Delay Time**: The time offset (in milliseconds) between the original signal and the echo.
* **Feedback**: The percentage of the delayed signal fed back into the input of the delay line. Higher values create nearly infinite, self-oscillating spatial echoes.
* **Delay Mix**: The linear crossfade between the dry signal (Fuzz + Chorus) and the wet signal (Delay).

---

## DSP Architecture & Mathematical Models

The plugin processes audio strictly in the high-priority real-time audio thread. All memory allocation occurs during the `prepareToPlay` phase, ensuring $O(1)$ time complexity per sample and zero audio dropouts.

### 1. The Fuzz Stage (Non-linear Waveshaping)
To simulate analog overdrive and fuzz, the input signal $x[n]$ is subjected to soft-clipping using a hyperbolic tangent function. To prevent the output volume from clipping the master bus when the drive is pushed to extreme limits, the signal is compensated by dividing it by the asymptotic limit of the drive factor.

**Mathematical Formula:**
$$y[n] = \frac{\tanh(\text{Drive} \cdot x[n])}{\tanh(\text{Drive})}$$

**C++ Implementation (`PluginProcessor.cpp`):**
```cpp
// channelData[sample] is the current audio sample x[n]
float inSample = channelData[sample];
channelData[sample] = std::tanh(inSample * drive) / std::tanh(drive);
```

### 2. The Chorus Stage
This stage utilizes the `juce::dsp::Chorus` module. It creates a thickening effect by mixing the original signal with one or more delayed copies. The delay time of these copies is continuously modulated by an LFO, causing slight, continuous pitch shifts (Doppler effect) that simulate multiple instruments playing simultaneously.

### 3. The Delay Stage (Ring Buffer & Recursion)
The delay line is implemented manually using a Circular Buffer (Ring Buffer) to avoid dynamic memory allocation. 

**A. Time to Sample Conversion:**
The delay time in milliseconds is converted to a discrete number of samples $D$ based on the host's sample rate $F_s$:
$$D = \text{DelayTime}_{ms} \times \frac{F_s}{1000}$$

**B. Pointer Wrap-around (Modulo Arithmetic):**
The read pointer $p_r$ looks back into the past relative to the current write pointer $p_w$. If it drops below zero, it wraps around to the end of the buffer of size $N$:
$$p_r = (p_w - D) \pmod N$$

**C. DSP Mixing and Recursive Feedback:**
The final output $y[n]$ is a linear interpolation between the Dry input $x[n]$ and the Wet delayed signal $d[n]$. Simultaneously, the current input plus the scaled delayed signal is written back into the buffer.
$$y[n] = x[n] \cdot (1 - \text{Mix}) + d[n] \cdot \text{Mix}$$
$$\text{Buffer}[p_w] = x[n] + (d[n] \cdot \text{Feedback})$$

**C++ Implementation (`PluginProcessor.cpp`):**
```cpp
// 1. Calculate Read Position
float delayInSamples = (delayTimeMS / 1000.0f) * sampleRate;
int readPosition = tempWritePos - (int)delayInSamples;
if (readPosition < 0) readPosition += delayBufferLength; // Wrap around

float delayedSample = delayData[readPosition]; // d[n]

// 2. Mix Dry and Wet
float outSample = (inSample * (1.0f - mix)) + (delayedSample * mix);

// 3. Recursive Feedback (Writing back to the buffer)
delayData[tempWritePos] = inSample + (delayedSample * feedback);
```

---

## Build Instructions

1. Clone the repository and navigate to the project folder.
2. Open `OrionSpaceFuzz.jucer` with **Projucer**.
3. Ensure `juce_dsp`, `juce_audio_basics`, `juce_audio_devices`, `juce_audio_formats`, `juce_audio_plugin_client`, `juce_audio_processors`, `juce_audio_utils`, `juce_core`, `juce_data_structures`, `juce_events`, and `juce_graphics` modules are properly linked in the Global Paths.
4. Export to your preferred IDE (Visual Studio 2022 / Xcode).
5. Build the project using the **Release** configuration for optimal DSP performance.
6. The resulting `.vst3` file can be loaded into any compatible DAW (e.g., Cubase, Reaper, Ableton Live).

## Author
REN Baiyi
