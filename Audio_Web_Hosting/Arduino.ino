#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <Arduino.h>
#include <driver/i2s.h>

#define I2S_WS 25
#define I2S_SD 33
#define I2S_SCK 32
#define I2S_PORT I2S_NUM_0
#define I2S_SAMPLE_RATE   (16000)
#define I2S_SAMPLE_BITS   (16)
#define I2S_READ_LEN      (16 * 1024)
#define RECORD_TIME       (30) // Seconds
#define I2S_CHANNEL_NUM   (1)
#define FLASH_RECORD_SIZE (I2S_CHANNEL_NUM * I2S_SAMPLE_RATE * I2S_SAMPLE_BITS / 8 * RECORD_TIME)

File file;
const char filename[] = "/recording.wav";
const int headerSize = 44;

// Wi-Fi credentials
const char* ssid = "DNS";
const char* password = "01234567";

// Web server
AsyncWebServer server(80);

void setup() {
  Serial.begin(115200);

  // Initialize Wi-Fi connection
  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to Wi-Fi");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Initialize LITTLEFS and check if it is ready
  LITTLEFSInit();  // Initialize LITTLEFS
  i2sInit();       // Initialize I2S

  // Start the web server to serve the recording file
  server.on("/recording", HTTP_GET, [](AsyncWebServerRequest *request) {
  if (LittleFS.exists(filename)) {
    // Use "attachment" to trigger download in the browser
    request->send(LittleFS, filename, "application/octet-stream");
  } else {
    request->send(404, "text/plain", "File not found");
  }
  });

  // Start the server
  server.begin();

  // Start the recording task
  xTaskCreate(i2s_adc, "i2s_adc", 1024 * 4, NULL, 1, NULL);  // Increase stack size to 4KB for recording task
}

void loop() {
  // Main loop can be empty or used for additional tasks
}

void LITTLEFSInit() {
  if (!LittleFS.begin()) {
    Serial.println("LITTLEFS initialization failed! Formatting...");
    if (LittleFS.format()) {
      Serial.println("LITTLEFS formatted successfully.");
      if (!LittleFS.begin()) {
        Serial.println("Failed to initialize LITTLEFS after formatting!");
        while (1) yield();
      }
    } else {
      Serial.println("LITTLEFS format failed!");
      while (1) yield();
    }
  } else {
    Serial.println("LITTLEFS initialized successfully.");
  }

  // Prepare the file for writing
  LittleFS.remove(filename);  // Remove any existing file
  file = LittleFS.open(filename, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing!");
    return;
  }

  // Write WAV header to file
  byte header[headerSize];
  wavHeader(header, FLASH_RECORD_SIZE);
  file.write(header, headerSize);

  // List LITTLEFS contents
  listLITTLEFS();
}

void i2sInit() {
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = I2S_SAMPLE_RATE,
    .bits_per_sample = i2s_bits_per_sample_t(I2S_SAMPLE_BITS),
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
    .intr_alloc_flags = 0,
    .dma_buf_count = 64,
    .dma_buf_len = 1024,
    .use_apll = 1
  };

  i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);

  const i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_SCK,
    .ws_io_num = I2S_WS,
    .data_out_num = -1,
    .data_in_num = I2S_SD
  };

  i2s_set_pin(I2S_PORT, &pin_config);
}

void i2s_adc_data_scale(uint8_t *d_buff, uint8_t *s_buff, uint32_t len) {
  uint32_t j = 0;
  uint32_t dac_value = 0;
  for (int i = 0; i < len; i += 2) {
    dac_value = ((((uint16_t)(s_buff[i + 1] & 0xf) << 8) | ((s_buff[i + 0]))));
    d_buff[j++] = 0;
    d_buff[j++] = dac_value * 256 / 2048;
  }
}

void i2s_adc(void *arg) {
  int i2s_read_len = I2S_READ_LEN;
  int flash_wr_size = 0;
  size_t bytes_read;

  char *i2s_read_buff = (char *)calloc(i2s_read_len, sizeof(char));
  uint8_t *flash_write_buff = (uint8_t *)calloc(i2s_read_len, sizeof(char));

  // Start recording
  Serial.println(" *** Recording Start *** ");
  while (flash_wr_size < FLASH_RECORD_SIZE) {
    i2s_read(I2S_PORT, (void *)i2s_read_buff, i2s_read_len, &bytes_read, portMAX_DELAY);
    i2s_adc_data_scale(flash_write_buff, (uint8_t *)i2s_read_buff, i2s_read_len);
    file.write((const byte *)flash_write_buff, i2s_read_len);
    flash_wr_size += i2s_read_len;
    ets_printf("Sound recording %u%%\n", flash_wr_size * 100 / FLASH_RECORD_SIZE);
    ets_printf("Never Used Stack Size: %u\n", uxTaskGetStackHighWaterMark(NULL));
  }
  
  // Ensure file is fully written and closed
  file.flush();
  file.close();

  free(i2s_read_buff);
  free(flash_write_buff);

  listLITTLEFS();  // List files after recording ends
  vTaskDelete(NULL);
}

void wavHeader(byte *header, int wavSize) {
    // 'RIFF' header
    header[0] = 'R'; header[1] = 'I'; header[2] = 'F'; header[3] = 'F';
    unsigned int fileSize = wavSize + 36;
    header[4] = (byte)(fileSize & 0xFF);
    header[5] = (byte)((fileSize >> 8) & 0xFF);
    header[6] = (byte)((fileSize >> 16) & 0xFF);
    header[7] = (byte)((fileSize >> 24) & 0xFF);

    // 'WAVE' format
    header[8] = 'W'; header[9] = 'A'; header[10] = 'V'; header[11] = 'E';

    // 'fmt ' subchunk
    header[12] = 'f'; header[13] = 'm'; header[14] = 't'; header[15] = ' ';
    header[16] = 16; header[17] = 0;  // Subchunk size (16 for PCM)
    header[18] = 1; header[19] = 0;   // PCM format
    header[20] = 1; header[21] = 0;   // Mono audio (1 channel)
    header[22] = (byte)(I2S_SAMPLE_RATE & 0xFF);
    header[23] = (byte)((I2S_SAMPLE_RATE >> 8) & 0xFF);
    header[24] = (byte)((I2S_SAMPLE_RATE >> 16) & 0xFF);
    header[25] = (byte)((I2S_SAMPLE_RATE >> 24) & 0xFF);
    header[26] = (byte)((I2S_SAMPLE_RATE * I2S_SAMPLE_BITS / 8) & 0xFF);
    header[27] = (byte)((I2S_SAMPLE_RATE * I2S_SAMPLE_BITS / 8 >> 8) & 0xFF);
    header[28] = 2; header[29] = 0;   // 16 bits per sample
    header[30] = 16; header[31] = 0;  // Bits per sample

    // 'data' subchunk
    header[32] = 'd'; header[33] = 'a'; header[34] = 't'; header[35] = 'a';
    header[36] = (byte)(wavSize & 0xFF);
    header[37] = (byte)((wavSize >> 8) & 0xFF);
    header[38] = (byte)((wavSize >> 16) & 0xFF);
    header[39] = (byte)((wavSize >> 24) & 0xFF);
}

void listLITTLEFS(void) {
  Serial.println(F("\r\nListing LITTLEFS files:"));
  static const char line[] PROGMEM = "=================================================";

  Serial.println(FPSTR(line));
  Serial.println(F("  File name                              Size"));
  Serial.println(FPSTR(line));

  fs::File root = LittleFS.open("/");
  if (!root) {
    Serial.println(F("Failed to open directory"));
    return;
  }
  if (!root.isDirectory()) {
    Serial.println(F("Not a directory"));
    return;
  }

  fs::File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      Serial.print("DIR : ");
      Serial.print(file.name());
    } else {
      String fileName = file.name();
      Serial.print("  " + fileName);
      int spaces = 33 - fileName.length();
      if (spaces < 1) spaces = 1;
      while (spaces--) Serial.print(" ");
      String fileSize = String(file.size());
      spaces = 10 - fileSize.length();
      if (spaces < 1) spaces = 1;
      while (spaces--) Serial.print(" ");
      Serial.println(fileSize + " bytes");
    }
    file = root.openNextFile();
  }

  Serial.println(FPSTR(line));
  Serial.println();
}
