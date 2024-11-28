import serial
import wave
import numpy as np

# Serial port settings
SERIAL_PORT = "COM4"  # Update this to match your port
BAUD_RATE = 115200
TIMEOUT = 1

# File paths
OUTPUT_TEXT_FILE = "audio_data_26_11.txt"
OUTPUT_WAV_FILE = "audio_data_26_11.wav"

# Audio settings
SAMPLE_RATE = 16000  # Match with ESP32
BUFFER_SIZE = 512  # Must match ESP32 buffer size

def receive_audio_data():
    try:
        print(f"Connecting to {SERIAL_PORT} at {BAUD_RATE} baud...")
        with serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=TIMEOUT) as ser:
            audio_samples = []
            print("Receiving audio data... Press Ctrl+C to stop.")

            while True:
                try:
                    # Read BUFFER_SIZE * 2 bytes (16-bit samples)
                    raw_data = ser.read(BUFFER_SIZE * 2)
                    if len(raw_data) == 0:
                        continue

                    # Ensure buffer size is divisible by 2
                    if len(raw_data) % 2 != 0:
                        print("Warning: Received incomplete sample, skipping.")
                        continue

                    # Convert to 16-bit integers
                    samples = np.frombuffer(raw_data, dtype=np.int16)
                    audio_samples.extend(samples)
                    print(f"Received {len(samples)} samples... Total: {len(audio_samples)} samples")

                except KeyboardInterrupt:
                    print("Audio capture stopped by user.")
                    break

        # Check if any data was captured
        if len(audio_samples) == 0:
            print("No audio data received. Exiting.")
            return

        # Save to text file
        print(f"Saving raw audio data to {OUTPUT_TEXT_FILE}...")
        with open(OUTPUT_TEXT_FILE, "w") as f:
            for sample in audio_samples:
                f.write(f"{sample}\n")
        print(f"Raw audio data saved to {OUTPUT_TEXT_FILE}.")

        # Calculate the effective sample rate based on the number of samples and duration
        captured_duration = len(audio_samples) / SAMPLE_RATE  # Time in seconds
        effective_sample_rate = len(audio_samples) / captured_duration

        print(f"Captured duration: {captured_duration:.2f} seconds")
        print(f"Effective sample rate: {effective_sample_rate:.2f} Hz")

        # Save to WAV file
        print(f"Saving audio data to {OUTPUT_WAV_FILE}...")
        with wave.open(OUTPUT_WAV_FILE, "w") as wav_file:
            wav_file.setnchannels(1)  # Mono
            wav_file.setsampwidth(2)  # 16-bit
            wav_file.setframerate(int(effective_sample_rate))  # Use calculated sample rate
            wav_file.writeframes(np.array(audio_samples, dtype=np.int16).tobytes())
        print(f"Audio data saved to {OUTPUT_WAV_FILE}.")
        print("Process completed successfully.")

    except Exception as e:
        print(f"An error occurred: {e}")

if __name__ == "__main__":
    receive_audio_data()
