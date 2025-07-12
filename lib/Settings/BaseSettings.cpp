//
// Created by ckyiu on 7/11/2025.
//

#include <BaseSettings.h>

namespace Settings {
  /**
   * @brief Don't touch.
   */
  bool usbConnected = false;

  /**
   * @brief Save the settings to disk in JSON format.
   *
   * @return SaveToDiskResult
   */
  SaveToDiskResult BaseSettings::saveToDisk() {
    Serial1.printf("Saving %s settings to disk\n", this->getSettingsName());

#ifdef LOG_FREE_MEMORY
    Serial1.printf("Free memory before save to disk: heap %d kb, stack %d kb\n",
                   rp2040.getFreeHeap() / 1024, rp2040.getFreeStack() / 1024);
#endif
    { // Scope for JsonDocument
      JsonDocument doc;
      this->saveValuesToDocument(doc);
      doc.shrinkToFit();

#ifdef LOG_FREE_MEMORY
      Serial1.printf(
        "Free memory after building document: heap %d kb, stack %d kb\n",
        rp2040.getFreeHeap() / 1024, rp2040.getFreeStack() / 1024);
#endif

      { // Scope for file operations
        Serial1.println("Starting FatFS and opening file");
        if (!FatFS.begin()) {
          Serial1.println("Failed to init FatFS");
          return SaveToDiskResult::ERROR_FATFS_INIT_FAILED;
        }
        File file = FatFS.open(this->getSettingsFilePath(), "w");
        if (!file) {
          Serial1.printf("Failed to open %s for writing",
                         this->getSettingsFilePath());
          return SaveToDiskResult::ERROR_FILE_OPEN_FAILED;
        }

        Serial1.println("Serializing JSON to file");
#ifdef LOG_JSON_PARSED
        Serial1.println("JSON:");
        WriteLoggingStream loggingStream(file, Serial1);
        serializeJsonPretty(doc, loggingStream);
        Serial1.println("");
#else
        serializeJsonPretty(doc, file);
#endif

#ifdef LOG_FREE_MEMORY
        Serial1.printf(
          "Free memory after serialization: heap %d kb, stack %d kb\n",
          rp2040.getFreeHeap() / 1024, rp2040.getFreeStack() / 1024);
#endif

        Serial1.println("Closing file and stopping FatFS");
        file.close();
        FatFS.end();
      }
    }
#ifdef LOG_FREE_MEMORY
    Serial1.printf("Free memory after save to disk: heap %d kb, stack %d kb\n",
                   rp2040.getFreeHeap() / 1024, rp2040.getFreeStack() / 1024);
#endif
    Serial1.printf("%s settings saved to disk successfully\n",
                   this->getSettingsName());

    return SaveToDiskResult::OK;
  }

  /**
   * @brief Load the settings from disk in JSON format.
   *
   * @return LoadFromDiskResult
   */
  LoadFromDiskResult BaseSettings::loadFromDisk() {
    Serial1.printf("Loading %s settings from disk\n", this->getSettingsName());

#ifdef LOG_FREE_MEMORY
    Serial1.printf(
      "Free memory before load from disk: heap %d kb, stack %d kb\n",
      rp2040.getFreeHeap() / 1024, rp2040.getFreeStack() / 1024);
#endif
    {
      Serial1.println("Starting FatFS and opening file");
      if (!FatFS.begin()) {
        Serial1.println("Failed to init FatFS");
        return LoadFromDiskResult::ERROR_FATFS_INIT_FAILED;
      }
      File file = FatFS.open(this->getSettingsFilePath(), "r");
      if (!file) {
        Serial1.printf("Failed to open %s for reading",
                       this->getSettingsFilePath());
        return LoadFromDiskResult::ERROR_FILE_OPEN_FAILED;
      }

      {
        Serial1.println("Deserializing JSON from file");
        JsonDocument doc;
#ifdef LOG_JSON_PARSED
        Serial1.println("JSON:");
        ReadLoggingStream loggingStream(file, Serial1);
        DeserializationError error = deserializeJson(doc, loggingStream);
        Serial1.println("");
#else
        DeserializationError error = deserializeJson(doc, file);
#endif
        Serial1.println("Closing file and stopping FatFS");
        file.close();
        FatFS.end();

        if (error) {
          Serial1.printf("Failed to deserialize JSON: %s\n", error.c_str());
          switch (error.code()) {
            case DeserializationError::TooDeep: {
              Serial1.println("JSON is too deep, please check the file");
              return LoadFromDiskResult::ERROR_JSON_PARSE_TOO_DEEP;
            }
            case DeserializationError::NoMemory: {
              Serial1.println("JSON parsing failed due to insufficient memory");
              return LoadFromDiskResult::ERROR_JSON_PARSE_NO_MEMORY;
            }
            case DeserializationError::InvalidInput: {
              Serial1.println("JSON parsing failed due to invalid input");
              return LoadFromDiskResult::ERROR_JSON_PARSE_INVALID_INPUT;
            }
            case DeserializationError::IncompleteInput: {
              Serial1.println("JSON parsing failed due to incomplete input");
              return LoadFromDiskResult::ERROR_JSON_PARSE_INCOMPLETE_INPUT;
            }
            case DeserializationError::EmptyInput: {
              Serial1.println("JSON parsing failed due to empty input");
              return LoadFromDiskResult::ERROR_JSON_PARSE_EMPTY_INPUT;
            }
            case DeserializationError::Ok:
            default: {
              Serial1.println("Unknown error");
              return LoadFromDiskResult::ERROR_JSON_PARSE_UNKNOWN_ERROR;
            }
          }
        }

#ifdef LOG_FREE_MEMORY
        Serial1.printf(
          "Free memory after deserialization: heap %d kb, stack %d kb\n",
          rp2040.getFreeHeap() / 1024, rp2040.getFreeStack() / 1024);
#endif

        this->lastValidationResult = this->validateSettings(doc);
        if (this->lastValidationResult == 0) {
          Serial1.printf("%s settings validation passed\n",
                         this->getSettingsName());
          this->loadValuesFromDocument(doc);
        } else {
          Serial1.printf(
            "Validation failed for %s settings after loading from disk\n",
            this->getSettingsName());
          return LoadFromDiskResult::ERROR_VALIDATION_FAILED;
        }
      }
    }
#ifdef LOG_FREE_MEMORY
    Serial1.printf(
      "Free memory after load from disk: heap %d kb, stack %d kb\n",
      rp2040.getFreeHeap() / 1024, rp2040.getFreeStack() / 1024);
#endif
    Serial1.printf("%s settings loaded from disk successfully\n",
                   this->getSettingsName());

    return LoadFromDiskResult::OK;
  }

  /**
   * @brief Start exposing the FatFS filesystem to the computer as a USB.
   *
   * This will allow the user to modify the settings file on the USB drive.
   */
  void BaseSettings::fatFSUSBBegin() {
    Serial1.println("Exposing FatFS to USB");
    FatFSUSB.onUnplug([](uint32_t i) {
      usbConnected = false;
      Serial1.println("USB unplugged");
    });
    FatFSUSB.onPlug([](uint32_t i) {
      usbConnected = true;
      Serial1.println("USB plugged in");
    });
    FatFSUSB.driveReady([](uint32_t i) { return true; });
    FatFSUSB.begin();
    usbConnected = true;
    Serial1.println("FatFSUSB started");
  }

  /**
   * @brief Stop exposing the FatFS filesystem to the computer as a USB.
   *
   * This will stop the user from modifying the settings file on the USB drive.
   */
  void BaseSettings::fatFSUSBEnd() {
    FatFSUSB.end();
    usbConnected = false;
    Serial1.println("FatFSUSB stopped");
  }
} // Settings
