  #include "Arduino.h"
  #include "FS.h"
  #include "SPIFFS.h"
  #include <esp_system.h>
  #include <esp_heap_caps.h>   

  // Deklarasi eksternal PSRAMSIZE jika tersedia
  extern size_t PSRAMSIZE;

  void printFlashInfo() {
    Serial.println("\n=== [INFO FLASH ESP32-S3] ===");
    Serial.println("Kapasitas Flash     : 16 MB (hardcoded)");
    Serial.println("Catatan: Kapasitas flash di-hardcode karena fungsi bawaan sudah deprecated.");
  }

  void printPSRAMInfo() {
    Serial.println("\n=== [INFO PSRAM ESP32-S3] ===");

  #if defined(BOARD_HAS_PSRAM)
    size_t psram_size = 0;

    // Coba ambil dari PSRAMSIZE jika tersedia
    #ifdef PSRAMSIZE
      psram_size = PSRAMSIZE;
    #else
      // Jika PSRAMSIZE tidak tersedia, coba hitung via heap caps
      psram_size = heap_caps_get_total_size(MALLOC_CAP_SPIRAM);
    #endif

    if (psram_size > 0) {
      Serial.printf("PSRAM Terdeteksi       : Ya\n");
      Serial.printf("Ukuran PSRAM             : %zu bytes (%.2f MB)\n", psram_size, psram_size / (1024.0 * 1024.0));
      Serial.printf("Heap PSRAM Tersedia      : %d bytes\n", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
      Serial.printf("Heap PSRAM Minimum bebas : %d bytes\n", heap_caps_get_minimum_free_size(MALLOC_CAP_SPIRAM));
    } else {
      Serial.println("PSRAM Tidak Terdeteksi atau Tidak Berfungsi");
    }
  #else
    Serial.println("\n[PSRAM] PSRAM tidak didukung/dinonaktifkan di konfigurasi.");
  #endif
  }

  void printHeapInfo() {
    Serial.println("\n=== [INFO HEAP MEMORI] ===");
    Serial.printf("Free Heap (Internal) : %d bytes\n", ESP.getFreeHeap());
    Serial.printf("Min Free Heap        : %d bytes\n", heap_caps_get_minimum_free_size(MALLOC_CAP_DEFAULT));
    Serial.printf("Largest Free Block   : %d bytes\n", heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT));

  #if defined(BOARD_HAS_PSRAM)
    Serial.printf("Free PSRAM             : %d bytes\n", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
    Serial.printf("Largest PSRAM Block    : %d bytes\n", heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM));
  #endif
  }

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
    delay(1000);

    Serial.println("\n\n██████████████████████████████████████");
    Serial.println("█ INFO SISTEM ESP32-S3               █");
    Serial.println("██████████████████████████████████████");

    printFlashInfo();     // Info Flash
    printPSRAMInfo();     // Info PSRAM
    printHeapInfo();      // Info Heap Memory
    printSPIFFSInfo();    // Info SPIFFS

    Serial.println("\n[INFO] Pemeriksaan selesai.\n");
  }

  void loop() {
    // Opsional: ulangi setiap X detik
    delay(10000);
  }
