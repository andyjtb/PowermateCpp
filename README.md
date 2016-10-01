# PowermateCpp
A cross platform library for the Griffin Powermate controller

This library heavily utilises the [hidapi](https://github.com/signal11/hidapi) library by **signal11**

# Usage

Included in the Libs folder are precompiled versions of hidapi for both Mac and Windows

These are just to speed up the setup, you are probably better off compiling it for your own usage

## Mac 
The hidapi library requires IOKit and CoreFoundation frameworks

## Basic Usage

Include the "PowermateController.hpp" header file

`#include "PowermateController.hpp"`

Create a PowermateController object and register your callback functions for Powermate actions. These callbacks are function objects, so can be lambdas, if preferred:

`
  PowermateController powermate;

  powermate.setPowermateMovedHandler([](bool isAntiClockwise, int ticks)
  {
      std::string direction = isAntiClockwise ? "Anti Clockwise" : "Clockwise";
      
      std::cout << "Moved: " << ticks << direction << "\n";
  });
`

These callbacks will happen on the powermate thread so guard data access appropriately

Then start the controller:

`powermate.start();`


Finally, decide on your logic and enjoy