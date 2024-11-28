#include <Arduino.h>
#include <driver/i2s.h>

// I2S settings
#define SAMPLE_RATE 16000            // INMP441 minimum supported sample rate
#define TARGET_RATE 16000            // Target sample rate for the test
#define I2S_BUFFER_SIZE 512         // Buffer size for I2S

// Pin configurations (update according to your board)
#define I2S_WS  25                  // Word Select (L/R clock)
#define I2S_SD  33                  // Serial Data
#define I2S_SCK 32                  // Bit Clock

void setup() {
  Serial.begin(115200); // Debug output

  // Configure I2S
  i2s_config_t i2s_config = {
      .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX), // Master receive mode
      .sample_rate = SAMPLE_RATE,                       // Sampling rate
      .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,     // 16-bit samples
      .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,      // Mono channel
      .communication_format = I2S_COMM_FORMAT_I2S_MSB,  // Standard I2S
      .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,         // Interrupt level
      .dma_buf_count = 8,                               // Number of DMA buffers
      .dma_buf_len = I2S_BUFFER_SIZE,                   // DMA buffer length
      .use_apll = false,                                // Disable APLL
      .tx_desc_auto_clear = false,                      // Auto-clear TX
      .fixed_mclk = 0                                   // No fixed MCLK
  };

  i2s_pin_config_t pin_config = {
      .bck_io_num = I2S_SCK,
      .ws_io_num = I2S_WS,
      .data_out_num = I2S_PIN_NO_CHANGE,
      .data_in_num = I2S_SD
  };

  // Install and configure I2S driver
  i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_NUM_0, &pin_config);

  Serial.println("I2S Initialized");
}

void loop() {
  static unsigned long start_time = millis();
  static int sample_count = 0;
  int16_t i2s_buffer[I2S_BUFFER_SIZE];
  size_t bytes_read;

  // Read data from I2S
  i2s_read(I2S_NUM_0, (char *)i2s_buffer, I2S_BUFFER_SIZE * sizeof(int16_t), &bytes_read, portMAX_DELAY);

  // Downsample to TARGET_RATE
  static int downsample_counter = 0;
  for (int i = 0; i < bytes_read / 2; i++) { // Each sample is 2 bytes
    downsample_counter++;
    if (downsample_counter >= SAMPLE_RATE / TARGET_RATE) {
      downsample_counter = 0;
      sample_count++;
    }
  }

  // Measure elapsed time
  unsigned long elapsed_time = millis() - start_time;
  if (elapsed_time >= 60000) { // 1-second interval
    Serial.printf("Captured samples in last minute: %d\n", sample_count);
    sample_count = 0;
    start_time = millis();
  }
}
