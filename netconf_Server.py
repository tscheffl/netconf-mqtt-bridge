# -*- coding: utf-8 -*-#
#
# February 23 2017, Thomas Scheffler <scheffler@beuth-hochschule.de>
#
# Copyright (c) 2017
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
from __future__ import absolute_import, division, unicode_literals, print_function, nested_scopes
import getpass
import logging
from lxml import etree
#from netconf import client as netconf_client
from netconf import server as netconf_server
from ncclient.xml_ import new_ele, sub_ele
#from netconf.error import RPCError
import paho.mqtt.client as mqtt
from parse_json import generate_yang

logger = logging.getLogger(__name__)
nc_server = None
NC_PORT = 44555 #None
NC_DEBUG = False


test ='''
{
    "dummy":
    {
        "description": "blabla_3",
        "output": "0"
    }		
}'''

#test =
'''
{
  "rpc": {
    "set_color_green": {
      "description": "RPC call that sets the LIFX-Led Color to green",
      "mqtt-command": "GREEN"
    },
    "switch_off": {
      "description": "Switches the LIFX-Led off",
      "mqtt-command": "OFF"
    }
  }
}
'''
yangModel = ""
command_dict = {}

class NetconfMethods (netconf_server.NetconfMethods):

    #build the config
    nc_config = new_ele('config')
    configuration = sub_ele(nc_config, 'configuration')
    system = sub_ele(configuration, 'system')
    location = sub_ele(system, 'location')
    sub_ele(location, 'building').text = "Main Campus, A"
    sub_ele(location, 'floor').text = "5"
    sub_ele(location, 'rack').text = "27"
    #<netconf-state xmlns="urn:ietf:params:xml:ns:yang:ietf-netconf-monitoring">
    #<schemas/>
    #</netconf-state>
    
    @classmethod    
    def rpc_get (cls, unused_session, rpc, *unused_params):
        #return etree.Element("ok")
        return cls.nc_config
        
    def rpc_hubble (self, unused_session, rpc, *unused_params):
        return etree.Element("okidoki")
        
    def rpc_get_schema(self, unused_session, rpc, *unused_params):
        root = etree.Element("data", xmlns="urn:ietf:params:xml:ns:yang:ietf-netconf-monitoring")
        root.text = etree.CDATA(yangModel)
        print (command_dict)
        return root
        #return etree.Element("okiki")
        #return generate_yang()

def setup_module (unused_module):
    global nc_server

    yangModel,command_dict = generate_yang(test)

    logging.basicConfig(level=logging.DEBUG)

    if nc_server is not None:
        logger.error("XXX Called setup_module multiple times")
    else:
        sctrl = netconf_server.SSHUserPassController(username=getpass.getuser(),
                                             password="admin")
        nc_server = netconf_server.NetconfSSHServer(server_ctl=sctrl,
                                            server_methods=NetconfMethods(),
                                            port=NC_PORT,
                                            host_key="tests/host_key",
                                            debug=NC_DEBUG)


### Generate Netconf Methods
def build_Netconf_Methods(method_Name, mqtt_Parameter):
    logger.info("build_Netconf_Methods called: " + method_Name + mqtt_Parameter)
    ld = {}
    string = 'self.extra = "Hello"'
    
    exec("""
def rpc_%s(self, unused_session, rpc, *unused_params):
    %s
    print(self.extra)
    mqtt_client.publish("LIFX", "%s")
    return etree.Element("%s")
    """ % (method_Name, string, mqtt_Parameter, method_Name), None, ld)
    print('locals got: {}'.format(ld))
    for name, value in ld.items():
        setattr(NetconfMethods, name, value)
    
    #attach RPC to configuration
    rpc = sub_ele(NetconfMethods.configuration,"""rpc_%s""" % (method_Name))


###  MQTT functions
def mqtt_connect(mqtt_client, userdata, flags, rc):
    print("Connected with result code " + str(rc))
    mqtt_client.subscribe("yang/config/#")

def mqtt_message(mqtt_client, userdata, msg):
    global test
    global yangModel, command_dict
    logger.info(msg.topic + " " + str(msg.payload))
    a = (msg.topic.split("/"))
    if a[0] == "yang" and a[1] == "config":
        print ("Split", a[0], a[1])
        test = msg.payload
        yangModel,command_dict = generate_yang(test)
        for i in command_dict:
            print (i, command_dict[i])
            build_Netconf_Methods(i, command_dict[i])

mqtt_client = mqtt.Client()
mqtt_client.on_connect = mqtt_connect
mqtt_client.on_message = mqtt_message

### Run....
mqtt_client.connect("localhost", 1883, 60)

#mqtt_client.loop_forever()
mqtt_client.loop_start()

print ("#\n#\n#\n#\n")

query = """
<get><filter>
<devices xmlns="http://tail-f.com/ns/ncs">
<global-settings/>
</devices></filter></get>
"""
query1 = "<hubble/>"

setup_module(None)

logger.info("Connecting to 127.0.0.1 port %d", nc_server.port)
#session = netconf_client.NetconfSSHSession("127.0.0.1",
#                                    username=getpass.getuser(),
#                                    password="admin",
#                                    port=nc_server.port,
#                                    debug=NC_DEBUG)
#session.send_rpc(query)


def rpc_zack (self, unused_session, rpc, *unused_params):
    return etree.Element("blue")

NetconfMethods.rpc_zack = rpc_zack
# del NetconfMethods.rpc_zack


#calling functions from string
#getattr(session,'send_rpc')("<get/>")

#session.send_rpc("<BELL/>")

