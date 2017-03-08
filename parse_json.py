from six.moves import StringIO

import pyang
import pyang.plugin
from pyang.statements import Statement

import netconf.util as ncutil
from lxml import etree
from netconf import nsmap_update, NSMAP, qmap
from ncclient.xml_ import new_ele, sub_ele

from optparse import OptionParser

import json
from collections import OrderedDict
from pprint import pprint

class DummyRepository(pyang.Repository):
	"""Dummy implementation of abstract :class:`pyang.Repository`
	   for :class:`pyang.Context` instantiations
	"""

	def get_modules_and_revisions(self, ctx):
		"""Just a must-have dummy method, returning empty ``tuple``.
		Modules are directly given to pyang output plugins

		"""
		return ()

#http://www.jsoneditoronline.org
#test =
'''
{
  "rpc": {
	"set_color": {
	  "description": "test",
	  "input": {
		"leaf": {
		  "color": {
			"type": "string"
		  }
		}
	  }
	},
	"switch_off": {
	  "description": "Switches the LIFX-Led off"
	}
  }
}'''

test ='''
{
  "rpc": {
	"set_color": {
	  "description": "test",
	  "input": "string"
	},
	"switch_off": {
	  "description": "Switches the LIFX-Led off"
	}
  }
}
'''


count = 0
level_memory = 0
	
def parse_dict(v, module):
	print "##############"
	huff2 = []
	count2 = 0
	if isinstance(v, dict):
		for k, v2 in v.items():
			print "# K:", k
			if k == 'rpc':
				for k2, v3 in v2.items():
					print "#### K2:", k2, count2
					huff2.append(Statement(None, None , None, 'rpc',k2))
					module.substmts.append(huff2[count2])
					count2 += 1
					level = count2-1
					for k3,v4 in v3.items():
						print "###### K3:", k3,v4, count2
						if not isinstance(v4, dict):
							huff2.append(Statement(None, None , None, k3,v4))
							huff2[level].substmts.append(huff2[count2])
							count2 += 1
						else:
							#key, value = v4.popitem()
							#print '>>>',key, value
							huff2.append(Statement(None, None , None, k3, None))
							huff2[level].substmts.append(huff2[count2])
							count2 += 1
							for k4, v5 in v4.items():
								print "########x K4:", k4,v5, count2
								huff2.append(Statement(None, None , None, k4,v5))
								huff2[count2-1].substmts.append(huff2[count2])
								count2 += 1
								




def print_dict(v, module, prefix='', level=0, huff = []):
	#huff.append(Statement(None, None , None, 'container','mqtt-netconf-bridge'))
	#module.substmts.append(huff[count])
	global count, level_memory
	
	if isinstance(v, dict):
		for k, v2 in v.items():
			p2 = "{}['{}']".format(prefix, k)
			print "\nDict: --- ", level, count
			print k, v2 
			huff.append(Statement(None, None , None, 'container',k))
			if count is 0:
				module.substmts.append(huff[level])
				level_memory[level] = count
			elif count is not 0 and level is 0:
				huff[level_memory[level]].substmts.append(huff[count])
				#level_memory[level] = count
				
			elif k is 'rpc':
				level = 0	
				module.substmts.append(huff[count])
			
			else:
				print 'D:', level, repr(huff[1])
				huff[level_memory[level-1]].substmts.append(huff[count])
				#huff[level-1].substmts.append(huff[count])
				level_memory[level] = count
				
			count += 1
			#if count > level:
			#	level = count-1
			print_dict(v2, module, p2, level = level + 1, huff = huff)
			
	elif isinstance(v, list):
		for i, v2 in enumerate(v):
			p2 = "{}[{}]".format(prefix, i)
			print "List: ---", level
			print i, v2
#
			huff.append(Statement(None, None , None, 'list',i))
#			huff[level-1].substmts.append(huff[level])
			huff[level_memory[level-1]].substmts.append(huff[count])
			count += 1
			print_dict(v2, module, p2, level = level + 1, huff = huff)
	
	else:
		print('{} = {}'.format(prefix, repr(v)))
		huff.append(Statement(None, None , None, 'leaf',v))
		#huff[level-1].substmts.append(huff[count])
		huff[level_memory[level-1]].substmts.append(huff[count])
		level_memory[level] = count
		count += 1


def generate_yang(test):
	global count, level_memory
	'''
	Generate a YANG-in-XML Tree
	- print the YANG Tree as string with SerialIO
	- build a new root-Element, called <data> with 'xmlns' Attribute
	- attach the stringified CDATA to the new Element
	- print the XML
	'''
	
	#python-modeled.netconf/modeled/netconf/yang/__init__.py
	module1 = Statement(None, None, None, 'module', 'mqtt-netconf-bridge')
	
	my_namespace = "http://ipv6lab.beuth-hochschule.de/mqtt-netconf-bridge"
	my_prefix = "mnb"
	
	namespace = Statement(None, module1, None, 'namespace', my_namespace)
	module1.substmts.append(namespace)
	
	prefix = Statement(None, module1, None, 'prefix', my_prefix)
	module1.substmts.append(prefix)
	
	#http://stackoverflow.com/questions/10844064/items-in-json-object-are-out-of-order-using-json-dumps
	data = json.loads(test, object_pairs_hook=OrderedDict)
	count = 0
	level_memory = {}
	#print_dict(data, module1, count)
	parse_dict(data, module1)
	
	#revision = str(datetime.now())
	#revision = Statement(None, module, None, 'revision', revision)
	#module.substmts.append(revision)
	
	#https://github.com/mbj4668/pyang/blob/master/pyang/plugin.py
	#https://github.com/modeled/modeled.netconf/blob/master/modeled/netconf/yang/container.py
	
	
	"""Serialize YANG container to the given output `format`.
			"""
	# output stream for pyang output plugin
	stream = StringIO()
	
	# gets filled with all availabe pyang output format plugins
	PYANG_PLUGINS = {}
	
	# register and initialise pyang plugin
	pyang.plugin.init([])
	for plugin in pyang.plugin.plugins:
		plugin.add_output_format(PYANG_PLUGINS)
	del plugin
	
	#for name in PYANG_PLUGINS:
	#    print(name)
	#...
	#dsdl
	#depend
	#name
	#omni
	#yin
	#tree
	#jstree
	#capability
	#yang
	#uml
	#jtox
	#jsonxsl
	#sample-xml-skeleton
	plugin = PYANG_PLUGINS['yang']
	
	# register plugin options according to pyang script
	optparser = OptionParser()
	plugin.add_opts(optparser)
	
	# pyang plugins also need a pyang.Context
	ctx = pyang.Context(DummyRepository())
	
	# which offers plugin-specific options (just take defaults)
	ctx.opts = optparser.parse_args([])[0]
	
	# ready to serialize
	plugin.emit(ctx, [module1], stream)
	
	# and return the resulting data
	stream.seek(0)
	yang = stream.getvalue()
	
	print '\nAusgabe: '
	print(stream.read())
	print ""
	#return stream.read()
	
	#l = etree.tostring(root,  pretty_print=True)
	root = etree.Element("data", xmlns="urn:ietf:params:xml:ns:yang:ietf-netconf-monitoring")
	root.text = etree.CDATA(yang)
	#print  etree.tostring(root,  pretty_print=True)
	#return  etree.tostring(root,  pretty_print=True)
	return  root
	
if __name__ == "__main__":
	result = generate_yang(test)
	print 'hhhuuu'
	print result
