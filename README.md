# ATEM Tally
This project is a tally light for a BlackMagic ATEM production switcher. This uses a NodeMCU and NodeRed server to connect to the ATEM using [blackmagic-atem-nodered](https://www.npmjs.com/package/blackmagic-atem-nodered).

## Features
* Preview/Live tally updates
* Fail safe detection. Detects if Wifi has diconnected or connection has been lost to the server
* ME ignoring. Can be configured to ignore certain MEs if required or show tallys based on all/multiple MEs
* Custom colours
* Stage/user leds
* Uses network broadcast so defining ip addresses is not needed, they just have to be on the same network
* Show blue feature which shows a blue light to the stage to show where the camera is

## Download
[Click here to download the latest release](https://github.com/Kardinia-Church/ATEM-Tally/releases)

# How to use
There are 2 major parts that are needed to make this work, this is the tally(s) it's self, and the server which talks directly to the ATEM
## Server Side
The server is required to talk to the ATEM. The server then sends out messages to the tally(s) which will show the state.
### Dependencies 
The following dependencies are required:
* [NodeRed](https://nodered.org/docs/getting-started/local) it's best to use a machine as a server. NodeRed is great and can do heaps more too!
* [blackmagic-atem-nodered](https://www.npmjs.com/package/blackmagic-atem-nodered) install via the NodeRed Palette installer found in the side menu
### Set it up
* Go into the side menu, press import and paste the following:
```
[{"id":"23e2e7f9.028308","type":"atem-atem","z":"a9a9c4f6.2cf508","name":"Auditorium ATEM","network":"c8cac07b.eda62","outputMode":"supported","sendInitialData":"yes","sendStatusUpdates":"yes","x":630,"y":160,"wires":[["3a4c4f60.d4063"]]},{"id":"3a4c4f60.d4063","type":"function","z":"a9a9c4f6.2cf508","name":"Tally Lights v2","func":"if(msg.payload.cmd == \"inputProperty\"){\n    var output = {\n        \"payload\": new Buffer(0),\n        \"check\": 0\n    }\n    \n    var i = 0;\n    for(var key in msg.payload.data.inputs) {\n        if(key <= 20) {\n            var currentBuffer = Buffer(5);\n            var input = msg.payload.data.inputs[key];\n            currentBuffer.writeUInt16BE(input.id, 0);\n            currentBuffer[2] = input.tallys.programTally.state ? 0x01 : 0x00;\n            currentBuffer[3] = input.tallys.previewTally.state ? 0x01 : 0x00;\n            currentBuffer[4] = (input.tallys.downstreamKeyerTallyFill.state || input.tallys.downstreamKeyerTallyKey.state ||input.tallys.upstreamKeyerTallyFill.state || input.tallys.upstreamKeyerTallyKey.state) ? 0x01 : 0x00;\n            output.payload = Buffer.concat([output.payload, currentBuffer]);\n            for(var j = 2; j < 5; j++){output.check += currentBuffer[j] * input.id;}\n        }\n    }\n    return [output, undefined];\n}\n\nif(msg.payload.cmd == \"programInput\" || msg.payload.cmd == \"previewInput\" || msg.payload.cmd == \"downstreamKeyer\" || msg.payload.cmd == \"upstreamKeyer\") {\n    return [undefined, {\n        \"payload\": {\n            \"cmd\": \"inputProperty\",\n            \"data\": {}\n        }\n    }];\n}","outputs":2,"noerr":0,"x":820,"y":160,"wires":[["2f3da66d.f6c6aa"],["23e2e7f9.028308"]],"outputLabels":["To Tally","ATEM"]},{"id":"2f3da66d.f6c6aa","type":"udp out","z":"a9a9c4f6.2cf508","name":"Broadcast","addr":"192.168.0.255","iface":"","port":"5654","ipv":"udp4","outport":"","base64":false,"multicast":"broad","x":980,"y":120,"wires":[]},{"id":"d849d39a.98167","type":"udp in","z":"a9a9c4f6.2cf508","name":"Incoming from TallyLights","iface":"","port":"8000","ipv":"udp4","multicast":"false","group":"","datatype":"utf8","x":210,"y":160,"wires":[["9f43d8d0.d06978"]]},{"id":"9f43d8d0.d06978","type":"function","z":"a9a9c4f6.2cf508","name":"Data request tallys","func":"if(msg.payload == \"dataRequest\") {\n    var msg1 = {\n        \"payload\": {\n            \"cmd\": \"inputProperty\",\n            \"data\": {}\n        }\n    }\n    return msg1;\n}","outputs":1,"noerr":0,"x":430,"y":160,"wires":[["23e2e7f9.028308"]]},{"id":"c8cac07b.eda62","type":"atem-network","z":"","name":"Auditorium ATEM","ipAddress":"192.168.0.1"}]
```
* Update the ATEM information to follow with your ATEM settings and press Deploy to save the changes.

## Tallys
### Required Hardware
The following hardware is required to build a single tally
* 1x - [NodeMCU V3 ESP8266](https://components101.com/development-boards/nodemcu-esp8266-pinout-features-and-datasheet)
* 2x - PL9823 5mm Pixel RGB LED
### 3D Case
* If you wish to use the 3D printed case provided print the bottom.stl and top.stl files found in the [3Ds folder](https://github.com/Kardinia-Church/ATEM-Tally/tree/subscription-based/3Ds)
### Wiring it up
![Circuit Diagram](/images/circuitDiagram.png)
* Note the first LED in the chain is the user LED
### Programming the NodeMCU
* In order to program the NodeMCU you either need to download [Visual Studio Code](https://code.visualstudio.com/) and install the [PlatformIO](https://platformio.org/install/ide?install=vscode) extension which is used to compile the project
* Download the source code from GitHub [here!](https://github.com/Kardinia-Church/ATEM-Tally/releases)
* Open Platform IO home in Visual Studio Code and open the project found in the ``Source`` folder
* Open the ```settings.h``` file found in the ```src``` folder. This contains the settings that can be configured
* At the minimum the ```ssid``` and ```password``` must be set to your wifi network. This must be the same network as the server!
The following variables can also be set:
* ```inputID``` - The input id
* ```ssid``` - The wifi SSID to connect to
* ```password``` - The wifi password
* ```ignoredMEs``` - What MEs to ignore. By default this is ME 2, 3, 4 (So ME1 will be the tally ME).
* ```previewEnabled``` - Should preview be enabled?
* ```programEnabled``` - Should program be enabled?
* ```keyerDSEnabled``` - Should the DS keyer be enabled?
* ```keyerUSEnabled``` - Should the US keyer be enabled
* ```showBlue``` - Should a blue led appear on the stage led to indicate location?
* ```liveColor``` - What colour should live be?
* ```keyerColor``` - What colour should live on a keyer be?
* ```standbyColor``` - What colour should preview/standby be?
* ```blueColor``` - What colour should the indicator be?
* ```offColor``` - What colour should off be?
* Finally upload the code to the NodeMCU using the arrow that points to the right on the bottom bar
If successful the NodeMCU should either be blank or display a tally state. If it is showing blue or another colour see the error codes below.

## Errors
### Colours
* Flashing Blue - Connecting to Wifi
* Flashing blue then flashing another color (version color) - This means it timed out connecting to Wifi and has rebooted. Check your Wifi settings / Wifi
* Solid blue - This means the tally is attempting to "subscribe" to the server if this stays on then there is an issue connecting to the server. Check if the tally is on the same network and if the server is running correctly

## See Also
### [ATEM-CCU-2-BT](https://github.com/Kardinia-Church/ATEM-CCU-2-BT)
Another tally that also acts as a CCU controller for a Blackmagic Camera over bluetooth allowing for CCU control of cameras using the ATEM control panel
### [blackmagic-atem-nodered](https://www.npmjs.com/package/blackmagic-atem-nodered)
A NodeRed interface to the Blackmagic ATEM allowing for most controls and feedback from a ATEM switcher
