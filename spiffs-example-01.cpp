#include "Arduino.h"
#include "driver/i2s.h"
#include "FS.h"
#include "SPIFFS.h"

// Konfigurasi pin
#define BCLK_PIN 15
#define LRC_PIN  16
#define DIN_PIN  7

// Sample rate dalam Hz
#define SAMPLE_RATE 44100

File audioFile;

 void printSPIFFSInfo() {
    const esp_partition_t* partition = esp_partition_find_first(
      ESP_PARTITION_TYPE_DATA,
      ESP_PARTITION_SUBTYPE_DATA_SPIFFS,
      NULL
    );

    if (partition == NULL) {
      Serial.println("[SPIFFS] Partisi SPIFFS tidak ditemukan!");
      return;
    }

    Serial.println("\n=== [INFO SPIFFS] ===");
    Serial.printf("Alamat partisi SPIFFS : 0x%x <-------------------\n", partition->address );
    Serial.printf("Ukuran partisi SPIFFS : %d bytes (%.2f MB)\n", partition->size, partition->size / (1024.0 * 1024));

    if (!SPIFFS.begin()) {
      Serial.println("[SPIFFS] Gagal mount SPIFFS.");
      return;
    }

    size_t total = SPIFFS.totalBytes();
    size_t used = SPIFFS.usedBytes();

    Serial.printf("Total ruang SPIFFS     : %d bytes (%.2f KB / %.2f MB)\n", total, total / 1024.0, total / (1024.0 * 1024));
    Serial.printf("Ruang terpakai         : %d bytes (%.2f KB / %.2f MB)\n", used, used / 1024.0, used / (1024.0 * 1024));
    Serial.printf("Ruang tersisa          : %d bytes (%.2f KB / %.2f MB)\n", total - used, (total - used) / 1024.0, (total - used) / (1024.0 * 1024));

    // List file
    Serial.println("\nDaftar File di SPIFFS:");
    File root = SPIFFS.open("/");
    File file = root.openNextFile();
    if (!file) {
      Serial.println("Tidak ada file di SPIFFS.");
    }
    while (file) {
      Serial.printf(" - %s \t(%d bytes)\n", file.name(), file.size());
      file = root.openNextFile();
    }
  }
    
void setup() {
  Serial.begin(115200);
  printSPIFFSInfo();    // Info SPIFFS
  delay(1000);
  Serial.println("Memulai pemutaran I2S...");

  // Inisialisasi SPIFFS
  if (!SPIFFS.begin(true)) {
    Serial.println("Inisialisasi SPIFFS gagal!");
    return;
  }

  // Buka file audio dari SPIFFS
  audioFile = SPIFFS.open("/output.raw", "r");
  if (!audioFile) {
    Serial.println("Gagal membuka file audio!");
    return;
  }

  // Konfigurasi I2S
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_I2S_MSB,
    .dma_buf_count = 8,
    .dma_buf_len = 1024,
    .use_apll = false,
    .tx_desc_auto_clear = true,
    .fixed_mclk = 0
  };

  i2s_pin_config_t pin_config = {
    .bck_io_num = BCLK_PIN,
    .ws_io_num = LRC_PIN,
    .data_out_num = DIN_PIN,
    .data_in_num = I2S_PIN_NO_CHANGE
  };

  i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_NUM_0, &pin_config);

  Serial.println("Memutar audio...");
}

void loop() {
  size_t bytes_written;

  // Membaca dari SPIFFS dan memutar audio mentah
  while (audioFile.available()) {
    uint8_t buffer[1024];  // Baca dalam blok 1024 byte
    int bytesRead = audioFile.read(buffer, sizeof(buffer));
    i2s_write(I2S_NUM_0, buffer, bytesRead, &bytes_written, portMAX_DELAY);
  }

  // Kembalikan pointer file ke awal untuk pengulangan
  audioFile.seek(0);

  Serial.println("Pemutaran selesai.");
  delay(5000); // Tunggu 5 detik sebelum mengulang
}
