# ESPWebConfig

Arduino web based configuration library for the ESP8266 chip.

Configure the wifi of the device in a web interface, that way you do not need ssid and password in your program.

- At first boot it will act as a Wifi access point
- Connect to the access point with a phone or laptop.
- Go to the web page 192.168.4.1.
- Fill in the SSID and password in the web form and submit it.
- After reboot the device will connect to the network.

## Example: BasicESPWebConfig

The most basic example, only configures wifi in setup.

```cpp
ESPWebConfig espConfig;

setup() {
  // Read config and set up wifi.
  // If no config (first boot), this function will block, set up an access point and serve config web UI.
  espConfig.setup();

  // Configuration is read and wifi connected
  // ...
}
```

![Connect to AP to configure](examples/BasicESPWebConfig/esp_ap.png)
![Connect to AP to configure](examples/BasicESPWebConfig/esp_cfg2.png)

After config the device will connect to the wifi you configured.

In this example a wifi client is making a call and printing the result to Serial output.
```
Configuration done! 192.168.1.90
Test wifi connection
Google responded with code 301
```

## Example CustomESPWebConfig

The wifi configuration is always there, but you can also add extra parameters during config. The values of these extra parameters are accessible from your normal code. The values are always of type "char *" but you can of course cast them to whatever you need.

### Create custom variables

Add them as an array of strings to the constructor:

```cpp
const char* GREETING_KEY = "Greeting*";
const char* NAME_KEY = "Name";
const char* DATE_KEY = "Date*|date";
const char* CHECK_KEY = "Check|checkbox";

String parameters[] = {GREETING_KEY, NAME_KEY, DATE_KEY, CHECK_KEY};
ESPWebConfig espConfig("configpass", parameters, 4);
```

### Get the value of a parameter

After config you can read the variables with the same key you defined in the setup:

```cpp
char* greeting = espConfig.getParameter(GREETING_KEY);
```

### Web sugar

Apart from the name you can also set input type and mark a parameter as required in the web UI.

To set a parameter as required, simply add an asterix in the end.

To set input type use the naame|type notation, where name will be shown in the web UI and type will be used as type for that html input. There is no check that the type is valid or usefull, it will just set type="type" in the HTML-code. Simple types like date, password and url works great.
Other typwes like "radio" button doesn't make sense, it requires more stuff in the html code to work.

![Connect to AP to configure](examples/CustomESPWebConfig/config_custom.png)

![Connect to AP to configure](examples/CustomESPWebConfig/custom.png)
