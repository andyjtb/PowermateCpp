//
//  PowermateController.hpp
//  PowerMate
//
//  Created by Andy Brown on 26/08/2016.
//  Copyright © 2016 Andy. All rights reserved.
//

#pragma once
#include "Libs/hidapi.h"

#include <thread>
#include <mutex>
#include <iostream>

class PowermateController
{
public:
  using PowermateCallback = std::function<void()>;
  using PowermateMoved = std::function<void(bool, int)>;
  
  struct State
  {
    bool buttonState{ false };
    
    int knobDisplacement{ 0 };
    
    int ledBrightness{ 0 };
    bool ledPulseEnabled { false };
    bool ledPulseDuringSleepEnabled{ false };
    
    int ledPulseSpeedFlags{ 0 };
    int ledPulseSpeed{ 0 };
  };
  
  struct HidInfo
  {
    std::wstring manufacturer;
    std::wstring product;
    std::wstring serialNumber;
  };
  
  PowermateController();
  ~PowermateController();
  
  void start();
  void startBlocking();
  
  void stop();
  
  void setPowermateConnectionChangedCallback(PowermateCallback connected) { connectionCallback = connected; }
  void setPowermateClickedHandler(PowermateCallback clicked) { clickedCallback = clicked; }
  void setPowermateMovedHandler (PowermateMoved moved) { movedCallback = moved; }
  
  void setLedBrightness(uint8_t brightness);
  
  void setPulsing (bool shouldPulse);
  
  const bool isConnected() const { return connectionStatus; }
private:
  enum Commands { StaticBrightness = 0x01, Pulsing = 0x03 };

  void detectorLoop();
  
  void readLoop();
  
  void parseHidInfo();
  
  State parseState(unsigned char* data, int bufferSize);
  void checkForClick(State& state);
  void checkForMoved(State& state);

  void handleConnect(hid_device_info* device);
  void handleDisconnect();
  
  std::thread powermateThread;
  
  PowermateCallback connectionCallback;
  PowermateCallback clickedCallback;
  PowermateMoved movedCallback;
  
  std::mutex detectorMutex;
  std::condition_variable detectorCond;

  bool shuttingDown{ false };
  
  //Powermate
  bool connectionStatus{ false };
  
  int vendorId{ 0x077d };
  int productId{ 0x0410 };
  
  //hidapi
  static const int MAX_STR{ 256 };
  unsigned char buf[65];
  wchar_t wstr[MAX_STR];
  hid_device* handle;
  
  //State
  bool isDown{ false };
  uint8_t ledBrightness{ 0 };
  
  HidInfo info;
};
