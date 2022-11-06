# ESPWebConfig

Arduino web based configuration library for the ESP8266 chip.

Wifi parameters (ssid and password) are configured in a web interface. When configured, the device automatically connects to wifi, that way you do not need ssid and password in your program.

- At first boot it will act as a Wifi access point
- Connect to the access point with a phone or laptop
- Go to the web page 192.168.4.1
- Fill in the SSID and password in the web form and submit it
- After reboot the device will connect to the network

## Example: BasicESPWebConfig

The most basic example, only configures wifi in setup.

```cpp
ESPWebConfig espConfig;

setup() {
  // Read config and set up wifi.
  // If no config (first boot), this function will block, set up an access point and serve config web UI.
  espConfig.setup();

  // Configuration is read and wifi connected.
}
```

![Connect to AP to configure](examples/BasicESPWebConfig/esp_ap.png)
![Connect to AP to configure](examples/BasicESPWebConfig/esp_cfg2.png)

In this example a wifi client is making a call and printing the result to Serial output.
```
Configuration done! 192.168.1.90
Test wifi connection
Google responded with code 301
```

## Example CustomESPWebConfig

Besides wifi, this example configures four other parameters. The values of these extra parameters are accessible from your normal code. The values are always of type "const char *".

### Create custom variables

Add them as an array of strings to the constructor:

```cpp
const char* GREETING_KEY = "Greeting*";
const char* NAME_KEY = "Name";
const char* DATE_KEY = "Date*|date";
const char* CHECK_KEY = "Check|checkbox";

ESPWebConfig espConfig("configpass", "Message", {GREETING_KEY, NAME_KEY, DATE_KEY, CHECK_KEY});
```

### Get the value of a parameter

After config you can read the variables with the same key you defined in the setup:

```cpp
const char* greeting = espConfig.getParameter(GREETING_KEY);
```

### HTML formatting

Apart from the parameter name you can also set "mandatory" and "html form input type".

To mark a parameter as mandatory, simply add an asterix in the end.

To set input type use the name|type notation, where name will be the label in the html form and type will be used as the input type. There is no check that the type is valid or useful, it will just set type="type" in the HTML-code. Simple types like date, password and url works great.
Other typwes like "radio" doesn't make sense, it requires more stuff in the html code to work.

![Connect to AP to configure](examples/CustomESPWebConfig/config_custom.png)

![Connect to AP to configure](examples/CustomESPWebConfig/custom.png)
