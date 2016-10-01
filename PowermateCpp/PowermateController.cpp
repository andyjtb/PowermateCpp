//
//  PowermateController.cpp
//  Powermate
//
//  Created by Andy Brown on 26/08/2016.
//  Copyright © 2016 Andy. All rights reserved.
//

#include "PowermateController.hpp"

PowermateController::PowermateController()
{
}

PowermateController::~PowermateController()
{
  stop();
}

void PowermateController::start()
{  
  shuttingDown = false;
  
  powermateThread = std::thread([this]()
  {
    detectorLoop();
  });
}

void PowermateController::startBlocking()
{
  start();
  powermateThread.join();
}

void PowermateController::stop()
{
  shuttingDown = true;
  
  if (powermateThread.joinable())
  {
    detectorCond.notify_all();
    powermateThread.join();
  }
}

void PowermateController::detectorLoop()
{
  while (!shuttingDown)
  {
    //Enumerate connected devices
    hid_device_info* info = nullptr;
    
    while ((info = hid_enumerate(vendorId, productId)))
    {
      handleConnect(info);
    }
    
    //Wait
    {
      std::unique_lock<std::mutex> lock(detectorMutex);
      
      std::cv_status status = detectorCond.wait_for(lock, std::chrono::seconds(2));
      
      if (status == std::cv_status::no_timeout)
      {
        //Thread has been signalled to exit
        return;
      }
    }
  }
}

void PowermateController::setLedBrightness(uint8_t brightness)
{
  if (brightness >= 0 && brightness <= 255)
  {
    unsigned char buffer[9];
    buffer[0] = 0;
    buffer[1] = 0x41;
    buffer[2] = 1;
    buffer[3] = Commands::StaticBrightness;
    buffer[4] = 0;
    buffer[5] = brightness;
    buffer[6] = 0;
    buffer[7] = 0;
    buffer[8] = 0;
    
    if (handle)
      hid_send_feature_report(handle, buffer, 9);
  }
}

void PowermateController::setPulsing(bool shouldPulse)
{
  unsigned char buffer[9];
  buffer[0] = 0x00;
  buffer[1] = 0x41;
  buffer[2] = 0x01;
  buffer[3] = Commands::Pulsing;
  buffer[4] = 0x00;
  buffer[5] = shouldPulse ? 0x01 : 0x00;
  buffer[6] = 0x00;
  buffer[7] = 0x00;
  buffer[8] = 0x00;
  
  hid_send_feature_report(handle, buffer, 9);
}

void PowermateController::readLoop()
{
  int res = 0;
  
  // Read requested state
  do
  {
    res = hid_read(handle, buf, 65);
    if (res == -1)
    {
      handleDisconnect();
      return;
    }
    
    State currentState = parseState(buf, res);
    checkForClick(currentState);
    
    checkForMoved(currentState);
    
    //    Byte 0: Button
    //    Byte 1: Knob displacement
    //    Byte 2: 0
    //    Byte 3: LED brightness
    //    Byte 4: LED status (pulsing or constant brightness, pulse while sleep)
    //    Byte 5: LED multiplier (for pulsing LED)
    
// Print out the returned buffer.
//    printf("Buffer: ");
//    for (int i = 0; i < res; i++)
//      printf("%d,", buf[i]);
//    printf("\n");
    
  } while (res != -1);
  
}

void PowermateController::parseHidInfo()
{
  int res = hid_get_manufacturer_string(handle, wstr, MAX_STR);
  
  if (res == 0)
    info.manufacturer = wstr;
  
  res = hid_get_product_string(handle, wstr, MAX_STR);
  
  if (res == 0)
    info.product = wstr;
  
  res = hid_get_serial_number_string(handle, wstr, MAX_STR);
  
  if (res == 0)
    info.serialNumber = wstr;
}

PowermateController::State PowermateController::parseState(unsigned char* data, int bufferSize)
{
  State state;
  
  if (data && bufferSize > 0)
  {
    state.buttonState = data[0] == 0;
    state.knobDisplacement = data[1] < 128 ? data[1] : -256 + data[1];
    state.ledBrightness = data[2];
    state.ledPulseEnabled = (data[4] & 0x01) == 0x01;
    state.ledPulseDuringSleepEnabled = (data[4] & 0x04) == 0x04;
    state.ledPulseSpeedFlags = (data[4] & 0x30) >> 4;
    state.ledPulseSpeed = 0;
    
    switch (state.ledPulseSpeedFlags)
    {
      case 0: state.ledPulseSpeed = -data[5]; break;
      case 1: state.ledPulseSpeed = 0; break;
      case 2: state.ledPulseSpeed = data[5]; break;
    }
  }
  
  return state;
}

void PowermateController::checkForClick(State& state)
{
  if (!state.buttonState)
    isDown = true;
  else if (state.buttonState && isDown)
  {
    //Power mate was previously down and is now up, count as a click
    isDown = false;
    
    if (clickedCallback)
      clickedCallback();
  }
}

void PowermateController::checkForMoved(PowermateController::State& state)
{
  int wheelDelta = state.knobDisplacement;
  
  if (wheelDelta != 0 && movedCallback)
    movedCallback(wheelDelta > 0, std::abs(wheelDelta));
}

void PowermateController::handleConnect (hid_device_info* device)
{
  if (device)
  {
    //Device connected
    int res;
    
    // Open the device using the VID, PID,
    // and optionally the Serial number.
    handle = hid_open(vendorId, productId, NULL);
    
    if (handle)
    {
      connectionStatus = true;
      
      // Send an Output report to toggle the LED (cmd 0x80)
      buf[0] = 0; // First byte is report number
      buf[1] = 0x80;
      res = hid_write(handle, buf, 65);
      
      parseHidInfo();
      
      if (connectionCallback)
        connectionCallback();

      readLoop();
    }
  }
}

void PowermateController::handleDisconnect ()
{
  hid_close(handle);
  
  connectionStatus = false;
  
  if (connectionCallback)
    connectionCallback();
}