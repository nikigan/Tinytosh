#include <HardwareSerial.h>
#include "PcMonitorService.h"


void PcMonitorService::handleSerial(PcStats &stats, PomodoroData &pomodoro) {
    // 1. Process incoming Serial data
    while (Serial.available()) {
        char incomingChar = Serial.read();
        if (incomingChar == '\n' || incomingChar == '\r') {
            if (bufferIndex > 0) {
                serialBuffer[bufferIndex] = '\0';
                parseJson(serialBuffer, stats, pomodoro);
            }
            bufferIndex = 0;
        } else if (bufferIndex < 256 - 1) {
            if (incomingChar > 31) serialBuffer[bufferIndex++] = incomingChar;
        }
    }

    // 2. Check for Timeout (Heartbeat)
    if (millis() - lastDataTimestamp > DATA_TIMEOUT_MS) {
        stats.cpu_percent = 0;
        stats.net_down_kb = 0;
        stats.mem_percent = 0;
        stats.disk_percent = 0;
    }
}

void PcMonitorService::parseJson(const char* jsonString, PcStats &stats, PomodoroData &pomodoro) {
    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, jsonString);

    if (error) {
        return;
    }

    // Handle screen switch command
    if (doc.containsKey("screen_cmd")) {
        const char* cmd = doc["screen_cmd"];
        if (strcmp(cmd, "next") == 0) {
            screenSwitchRequested = true;
        }
        if (!doc.containsKey("cpu_percent")) return;
    }

    // Handle pomodoro commands
    if (doc.containsKey("pomo_cmd")) {
        const char* cmd = doc["pomo_cmd"];
        if (strcmp(cmd, "start") == 0) {
            pomodoro.active = true;
            pomodoro.is_work = true;
            pomodoro.start_millis = millis();
            pomodoro.work_ms = (doc["work_min"] | 45) * 60UL * 1000;
            pomodoro.break_ms = (doc["break_min"] | 5) * 60UL * 1000;
            pomodoro.phase_duration_ms = pomodoro.work_ms;
        } else if (strcmp(cmd, "stop") == 0) {
            pomodoro.active = false;
        }
        // If this JSON only has pomo_cmd, skip stats parsing
        if (!doc.containsKey("cpu_percent")) return;
    }

    lastDataTimestamp = millis();

    stats.cpu_percent = doc["cpu_percent"] | 0.0;
    stats.net_down_kb = doc["net_down_kb"] | 0.0;
    stats.mem_percent = doc["mem_percent"] | 0.0;
    stats.disk_percent = doc["disk_percent"] | 0.0;
}

const PcStats& PcMonitorService::getStats() const {
    return currentStats;
}