import wave
import numpy as np

# File paths
WAV_FILE = 'queen_present.wav'
OUTPUT_FILE = 'fft_magnitude.txt'
FFT_SIZE = 1024

def process_wav_fft():
    try:
        print(f"Opening WAV file: {WAV_FILE}...")
        with wave.open(WAV_FILE, 'rb') as wav_file:
            n_channels = wav_file.getnchannels()
            samp_width = wav_file.getsampwidth()
            frame_rate = wav_file.getframerate()
            n_frames = wav_file.getnframes()

            print(f"WAV file details: Channels={n_channels}, Sample Width={samp_width}, Frame Rate={frame_rate}, Frames={n_frames}")

            if n_channels != 1 or samp_width != 2:
                raise ValueError("The .wav file must be 16-bit mono PCM format.")

            wav_data = wav_file.readframes(n_frames)
            print("WAV file loaded.")
            pcm_data = np.frombuffer(wav_data, dtype=np.int16)

            # Apply windowing (e.g., Hann window)
            hann_window = np.hanning(FFT_SIZE)
            windowed_data = pcm_data[:FFT_SIZE] * hann_window

            # Perform FFT on the windowed data
            fft_output = np.fft.rfft(windowed_data, n=FFT_SIZE)
            fft_magnitude = np.abs(fft_output)

            print(f"Saving FFT data to {OUTPUT_FILE}...")
            with open(OUTPUT_FILE, 'w') as f:
                for value in fft_magnitude:
                    f.write(f"{value}\n")
            print(f"FFT data saved to {OUTPUT_FILE}.")

    except Exception as e:
        print(f"An error occurred: {e}")

if __name__ == '__main__':
    process_wav_fft()
