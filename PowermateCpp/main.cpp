//
//  main.cpp
//  PowerMate
//
//  Created by Andy Brown on 26/08/2016.
//  Copyright © 2016 Andy. All rights reserved.
//

#include <iostream>
#include <fstream>
#include <vector>
#include <string>

#include "PowermateController.hpp"

int main(int argc, const char * argv[])
{
  std::cout << "Powermate Controller v0." << 1 << std::endl;
  std::cout << "Awaiting Powermate..." << std::endl;
  
  PowermateController powermate;
 
  auto powermateClicked = [&powermate]()
  {
      std::cout << "Click\n";
  };

  powermate.setPowermateClickedHandler(powermateClicked);
  
  auto powermateConnected = [&]()
  {
    if (powermate.isConnected())
      std::cout << "Connected\n";
    else
      std::cout << "Disconnected\n";
  };

  powermate.setPowermateConnectionChangedCallback(powermateConnected);

  powermate.setPowermateMovedHandler([](bool isAntiClockwise, int ticks)
  {
      std::string direction = isAntiClockwise ? "Anti Clockwise" : "Clockwise";
      
      std::cout << "Moved: " << ticks << direction << "\n";
  });
 
  powermate.start();
  
  powermate.setPulsing(true);
  
  int a = 0;
  std::cin>> a;
  
  return 0;
}
