# netconf-mqtt-bridge


````python
m = manager.connect_ssh("localhost", port=44555, username="Thomas", password="admin",allow_agent=False,hostkey_verify=False,look_for_keys=False)

n = xml_.to_ele('<get_schema/>')
n = xml_.to_ele('<set_color_green/>')
n = xml_.to_ele('<switch_off/>')

m.dispatch(n)
```
