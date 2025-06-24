
#include <cstdio>
#include <cstdint>
#include <vector>
#include <deque>
#include <thread>
#include <mutex>
#include <cmath>

#include <fftw3.h>
#include <aubio/aubio.h>

#include "crow_all.h"
#include "json.hpp"

using json = nlohmann::json;

// Store active WebSocket connections
std::mutex g_clients_mutex;
std::vector<crow::websocket::connection*> g_clients;

int main() {
    crow::SimpleApp app;

    CROW_ROUTE(app, "/")([] {
        return "OK";
    });

    // WebSocket on /ws — note we pass &app here
    auto& ws = CROW_ROUTE(app, "/ws")
        .websocket(&app)
        .onopen([](crow::websocket::connection& conn){
            std::lock_guard<std::mutex> lk(g_clients_mutex);
            g_clients.push_back(&conn);
        })
        .onclose([](crow::websocket::connection& conn, const std::string&, uint16_t){
            std::lock_guard<std::mutex> lk(g_clients_mutex);
            auto it = std::find(g_clients.begin(), g_clients.end(), &conn);
            if(it != g_clients.end()) g_clients.erase(it);
        })
        .onmessage([](crow::websocket::connection& conn, const std::string& msg, bool){
            // We’ll handle “start_stream” below
            auto cmd = json::parse(msg);
            if(cmd["cmd"] == "start_stream") {
                std::string url = cmd["url"];
                std::thread([url](){
                  // 1) Launch FFmpeg to emit 16-bit mono PCM @44.1 kHz
                  std::string ffcmd = "ffmpeg -i \"" + url +
                    "\" -f s16le -ac 1 -ar 44100 -hide_banner -loglevel error -";
                  FILE* ff = popen(ffcmd.c_str(), "r");
                  if(!ff) return;

                  // 2) Prepare aubio tempo tracker
                  uint_t winSize = 2048, hopSize = 512, sampleRate = 44100;
                  aubio_tempo_t* tempo = new_aubio_tempo("default", winSize, hopSize, sampleRate);
                  fvec_t* in_fvec = new_fvec(hopSize);
                  fvec_t* out_fvec = new_fvec(1);

                  // 3) Prepare FFTW
                  std::vector<double> fft_in(winSize), fft_out(winSize);
                  auto plan = fftw_plan_r2r_1d(winSize, fft_in.data(), fft_out.data(),
                                              FFTW_R2HC, FFTW_MEASURE);

                  // 4) Sliding-window buffer
                  std::deque<double> window;

                  // 5) Main loop: read hopSize samples per iteration
                  int16_t rawbuf[hopSize];
                  while(fread(rawbuf, sizeof(int16_t), hopSize, ff) == hopSize) {
                      // Convert and feed aubio
                      for(uint_t i=0;i<hopSize;i++){
                          float s = rawbuf[i]/32768.0f;
                          fvec_set_sample(in_fvec, s, i);
                      }
                      aubio_tempo_do(tempo, in_fvec, out_fvec);

                      // Append to window for FFT
                      for(int i=0;i<hopSize;i++)
                          window.push_back(rawbuf[i]/32768.0);

                      // When we have enough for one FFT
                      if(window.size() >= winSize) {
                          // Copy last winSize samples into fft_in
                          for(int i=0;i<winSize;i++)
                              fft_in[i] = window[i];
                          // Discard the first hopSize samples
                          for(int i=0;i<hopSize;i++)
                              window.pop_front();

                          // Execute FFT
                          fftw_execute(plan);

                          // Build magnitude spectrum (first winSize/2 bins)
                          std::vector<double> spectrum(winSize/2);
                          for(int i=0;i<winSize/2;i++){
                              double re = fft_out[i];
                              double im = (i==0||i==winSize/2)? 0 : fft_out[winSize-i];
                              spectrum[i] = std::sqrt(re*re + im*im);
                          }

                          // Check for beat
                          bool is_beat = fvec_get_sample(out_fvec,0) != 0;
                          double bpm = aubio_tempo_get_bpm(tempo);

                          // Build JSON message
                          json msg;
                          msg["type"] = "data";
                          msg["spectrum"] = spectrum;
                          if(is_beat) msg["bpm"] = bpm;

                          auto text = msg.dump();

                          // Broadcast to all connected clients
                          std::lock_guard<std::mutex> lk(g_clients_mutex);
                          for(auto *c : g_clients)
                              c->send_text(text);
                      }
                  }

                  // 6) Cleanup
                  pclose(ff);
                  del_aubio_tempo(tempo);
                  del_fvec(in_fvec);
                  del_fvec(out_fvec);
                  fftw_destroy_plan(plan);
                                  }).detach();
                              }
        });

    app.port(8080).multithreaded().run();
}
