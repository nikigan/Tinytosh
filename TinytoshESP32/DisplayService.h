#ifndef DISPLAY_SERVICE_H
#define DISPLAY_SERVICE_H

#include "structs.h"
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include "TimeService.h"

class DisplayService {
public:
    Adafruit_SSD1306 display;

    DisplayService(int width, int height, int reset_pin);
    void begin();
    
    void showOLEDStatus(std::initializer_list<String> lines, bool clear = true);
    void drawTimeScreen(const Config& config, String timeStr, String dateStr);
    void drawWeatherScreen(const Config& config, const WeatherData& data, const String& currentTime);
    void drawAQIScreen(const Config& config, const AirQualityData& data, const String& currentTime);
    void drawPcScreen(const PcStats& pcStats);
    void drawCryptoScreen(const CryptoData& data);
    void drawForecastScreen(const Config& config, const ForecastData& data);
    void drawPomodoroScreen(const PomodoroData& data);
    void drawNoData();
    void blinkScreen(int count, int on_ms, int off_ms);

    void drawScreen(int screenIndex, const Config& config, TimeService& timeService, const WeatherData& weather, const AirQualityData& aqi, const PcStats& pc, const CryptoData& crypto, const ForecastData& forecast, const PomodoroData& pomodoro);
    void animateTransition(int prevScreen, int nextScreen, const Config& config, TimeService& timeService, const WeatherData& weather, const AirQualityData& aqi, const PcStats& pc, const CryptoData& crypto, const ForecastData& forecast, const PomodoroData& pomodoro);

    bool isScreenEnabled(const Config& config, int screenIndex, const PomodoroData& pomodoro);

private:    
    uint8_t screenBufferOld[1024];
    uint8_t screenBufferNew[1024];

    int getNextAnimationEffect(uint16_t mask);
    void animateHorizontal(int prev, int next, const Config& c, TimeService& t, const WeatherData& w, const AirQualityData& a, const PcStats& p, const CryptoData& cr, const ForecastData& fc, const PomodoroData& pm);
    void animateVertical(int prev, int next, const Config& c, TimeService& t, const WeatherData& w, const AirQualityData& a, const PcStats& p, const CryptoData& cr, const ForecastData& fc, const PomodoroData& pm);
    void animateDissolve(int prev, int next, const Config& c, TimeService& t, const WeatherData& w, const AirQualityData& a, const PcStats& p, const CryptoData& cr, const ForecastData& fc, const PomodoroData& pm);
    void animateCurtain(int prev, int next, const Config& c, TimeService& t, const WeatherData& w, const AirQualityData& a, const PcStats& p, const CryptoData& cr, const ForecastData& fc, const PomodoroData& pm);
    void animateBlinds(int prev, int next, const Config& c, TimeService& t, const WeatherData& w, const AirQualityData& a, const PcStats& p, const CryptoData& cr, const ForecastData& fc, const PomodoroData& pm);

    String getWeatherDescription(int wmo_code);
    const unsigned char* getWeatherBitmap(int wmo_code, bool is_day);
    const unsigned char* getAQIBitmap(int val, bool is_eu);
};

#endif