# ESP32-Deauthentication-Attack
Deauthentication attack using two ESP32 development boards. One board is set as the attacker, while the other is set as the access point.
The MAC address of the device being attacked is hardcoded into the program, see mac on line 82 in main.c.
To toggle between attack device and access point, see define.h.
<br>
# Program Considerations
Note that ESP-IDF does not allow the alteration of management frames. To bypass this, a sanity check must be redefined. See line 43 in main.c. One must compile the program with build flags -Wl, -zmuldefs. This is necessary for the redefinition of this sanity check.
