# Arduino and DWM1001c communication

Arduino Uno Rev3 SPI communication with DWM1001c through DWM1001-DEV board

Getting the data written by the user is only possible if the tag has "bh connected" attribute
which can only (as of right now) be achieved by initially communicating with a bridge node on a
raspberry pi and only then the communication can be successful with other devices.
For this to work the tag has to be in a working network of nodes e.g. with atleast 3 anchors of whom one is
the initiator.

Source of information (except the workaround): https://decaforum.decawave.com/t/send-show-iot-data-in-uart-shell-mode/4440
    
Hypothesis: the RBPi gives somekind of signal through one of its GPIO pins to bridge node that initiates the
possibility of connecting to bh. Something like a broadcast message over the network. But the tag doesn't rely on
the bridge node after the connection has been made because it doesn't lose the attribute even if the bridge node is off, ofcourse
as long as the network is still up and running.
  
TODO: we have to check the output of the RBPi on boot, and what it sends to bridge node through interfaces (SPI, UART) 
and other GPIO pins (only one that is not a part of any interface). After that we could imitate the signal so the bridge 
node thinks it is on a RBPi.

Schematic: https://www.decawave.com/product/dwm1001-development-board/  RPI Header unit

P.S. The comments above applies only to backhaul tlv request, other (tested on a few, for example "panid get") requests 
seem to work perfectly fine without the workaround.

Some parts of the code are taken from https://decaforum.decawave.com/t/spi-communication-with-arduino/2652/2
