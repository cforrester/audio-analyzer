# Real-Time Audio Spectrum Analyzer & Beat Detector

A Dockerized C++ service and web client for real-time audio spectrum analysis, beat detection (BPM), and interactive visualization. Supports both live audio streams and uploaded files.

## Features

* Live stream input via URL (Internet radio)
* File upload input
* Real-time FFT-based spectrum analysis
* Onset/beat detection and BPM estimation
* WebSocket broadcasting of JSON-encoded data
* Interactive web UI with:

  * Spectrum display (HTML5 Canvas)
  * BPM readout
  * Controls for stream URL, file upload, FFT parameters, filter toggles

## Prerequisites

* Ubuntu VPS (tested on 22.04)
* Docker & Docker Compose installed

## Installation

```bash
git clone https://github.com/<your-username>/audio-analyzer.git
cd audio-analyzer
docker-compose up --build -d
```

## Usage

Open your browser at `http://<VPS_IP>:8080/`, then:

1. Enter a stream URL or choose an audio file to upload.
2. Click **Start**.
3. View real-time spectrum visualization and BPM readout.

**Test streams:**

* `https://ice6.somafm.com/groovesalad-128-mp3`
* `https://live.amperwave.net/direct/ppm-jazz24mp3-ibc1`

## Configuration

* **Ports:** Modify `docker-compose.yml` to change host port mappings.
* **SSL:** If you need HTTPS and secure WebSockets (`wss://`), configure Nginx with Let's Encrypt in your Docker setup.

## Feature Roadmap

### v1.0 (Current)

* Core FFT and BPM detection
* WebSocket server and basic web UI
* Dockerized C++ backend
* Live stream and file upload support

### v1.1

* UI controls for:

  * FFT window size and hop size adjustment
  * Low-pass / high-pass filter toggles
* Robust error handling and reconnection logic
* Comprehensive unit tests for core routines

### v1.2

* Data export: download spectrum & BPM data as CSV or JSON
* Chroma feature extraction (key detection)
* UI enhancements: responsive design, light/dark theme support

### v2.0

* Machine learning integration:

  * Audio embeddings
  * Genre classification model
* Progressive Web App (PWA) support for mobile devices
* Performance monitoring dashboard (CPU, memory usage)

## Testing

Run unit tests for FFT and BPM routines:

```bash
docker-compose run --rm app bash -lc "cd build && ctest --output-on-failure"
```

## License

This project is licensed under the MIT License. See [LICENSE](LICENSE) for details.
