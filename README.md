# ESPWebConfig
Arduino web based configuration library for ESP8266 chip.

Configure the device in a web inteface. After first boot it will act as an access point,
so you don't need ssid and password hardcoded in your program.

![Connect to AP to configure](examples/BasicESPWebConfig/esp_ap.png)
![Connect to AP to configure](examples/BasicESPWebConfig/esp_cfg2.png)

After config and reboot, you can read parameter values and use them in your program.
![Connect to AP to configure](examples/BasicESPWebConfig/esp_done.png)


The wifi stuff above is always there, but you can add extra parameters during config,
that you can use during normal execution.

TODO: Implement reset button.
TODO: Prettier.
