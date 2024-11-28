#include <Arduino.h>
#include <driver/i2s.h>

// I2S settings
#define I2S_WS  25  // Word Select (LRCK)
#define I2S_SD  33  // Serial Data (DOUT)
#define I2S_SCK 32  // Serial Clock (BCLK)
#define SAMPLE_RATE 16000
#define BUFFER_SIZE 512

void setup() {
  Serial.begin(115200);

  // I2S configuration
  i2s_config_t i2s_config = {
      .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX),
      .sample_rate = SAMPLE_RATE,
      .bits_per_sample = i2s_bits_per_sample_t(I2S_BITS_PER_SAMPLE_16BIT),
      .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT, // Mono
      .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_I2S),
      .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
      .dma_buf_count = 8,
      .dma_buf_len = BUFFER_SIZE
  };

  // I2S pin configuration
  i2s_pin_config_t pin_config = {
      .bck_io_num = I2S_SCK,
      .ws_io_num = I2S_WS,
      .data_out_num = -1, // Not used
      .data_in_num = I2S_SD
  };

  // Initialize I2S
  i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_NUM_0, &pin_config);
  i2s_zero_dma_buffer(I2S_NUM_0);
}

void loop() {
  int16_t audio_buffer[BUFFER_SIZE];
  size_t bytes_read;

  // Read audio samples from the I2S microphone
  i2s_read(I2S_NUM_0, audio_buffer, sizeof(audio_buffer), &bytes_read, portMAX_DELAY);

  // Send the audio data over Serial
  Serial.write((uint8_t *)audio_buffer, bytes_read);

  // Optional delay to avoid overwhelming the serial interface
  delay(10);
}
