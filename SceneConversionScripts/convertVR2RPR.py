# Copyright 2020 Advanced Micro Devices, Inc
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#    http://www.apache.org/licenses/LICENSE-2.0
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Vray to RadeonProRender Converter

import time
import os
import math
import traceback

import maya.mel as mel
import maya.cmds as cmds

VR2RPR_CONVERTER_VERSION = "1.6.3"

MAX_RAY_DEPTH = None

# log functions

def write_converted_property_log(rpr_name, rs_name, rpr_attr, rs_attr):

	try:
		file_path = cmds.file(q=True, sceneName=True) + ".log"
		with open(file_path, 'a') as f:
			f.write(u"    property {}.{} is converted to {}.{}   \r\n".format(rs_name, rs_attr, rpr_name, rpr_attr).encode('utf-8'))
	except:
		pass


def write_own_property_log(text):

	try:
		file_path = cmds.file(q=True, sceneName=True) + ".log"
		with open(file_path, 'a') as f:
			f.write("    {}   \r\n".format(text))
	except:
		pass


def start_log(rs, rpr):

	try:
		text  = u"Found node: \r\n    name: {} \r\n".format(rs).encode('utf-8')
		text += "type: {} \r\n".format(cmds.objectType(rs))
		text += u"Converting to: \r\n    name: {} \r\n".format(rpr).encode('utf-8')
		text += "type: {} \r\n".format(cmds.objectType(rpr))
		text += "Conversion details: \r\n"

		file_path = cmds.file(q=True, sceneName=True) + ".log"
		with open(file_path, 'a') as f:
			f.write(text)
	except:
		pass
	


def end_log(rs):

	try:
		text  = u"Conversion of {} is finished.\n\n \r\n".format(rs).encode('utf-8')

		file_path = cmds.file(q=True, sceneName=True) + ".log"
		with open(file_path, 'a') as f:
			f.write(text)
	except:
		pass
		

# additional fucntions

def copyProperty(rpr_name, conv_name, rpr_attr, conv_attr):

	# full name of attribute
	conv_field = conv_name + "." + conv_attr
	rpr_field = rpr_name + "." + rpr_attr
	vr_type = type(getProperty(conv_name, conv_attr))
	rpr_type = type(getProperty(rpr_name, rpr_attr))

	try:
		listConnections = cmds.listConnections(conv_field)
		# connection convert
		if listConnections and cmds.objectType(listConnections[0]) != "transform":
			obj, channel = cmds.connectionInfo(conv_field, sourceFromDestination=True).split('.')
			source_name, source_attr = convertMaterial(obj, channel).split('.')
			connectProperty(source_name, source_attr, rpr_name, rpr_attr)
			
		# complex color conversion for each channel (RGB/XYZ/HSV)
		elif not listConnections and rpr_type == vr_type == tuple:

			# change attr for channel conversion in some cases
			if cmds.objectType(conv_name) == 'VRayMtl' and conv_attr == 'color':
				conv_attr = "diffuseColor"
			elif cmds.objectType(conv_name) == 'VRayCarPaintMtl' and conv_attr == 'color':
				conv_attr = "base_color"
			elif cmds.objectType(conv_name) in ('VRayLightRectShape', 'VRayLightSphereShape', 'VRayLightMesh', 'VRayLightIESShape') and conv_attr == 'lightColor':
				conv_attr = "color"
			conv_field = conv_name + "." + conv_attr

			# RGB 
			if cmds.objExists(conv_field + "R") and cmds.objExists(rpr_field + "R"):
				copyProperty(rpr_name, conv_name, rpr_attr + "R", conv_attr + "R")
				copyProperty(rpr_name, conv_name, rpr_attr + "G", conv_attr + "G")
				copyProperty(rpr_name, conv_name, rpr_attr + "B", conv_attr + "B")
			elif cmds.objExists(conv_field + "R") and cmds.objExists(rpr_field + "X"):
				copyProperty(rpr_name, conv_name, rpr_attr + "X", conv_attr + "R")
				copyProperty(rpr_name, conv_name, rpr_attr + "Y", conv_attr + "G")
				copyProperty(rpr_name, conv_name, rpr_attr + "Z", conv_attr + "B")
			elif cmds.objExists(conv_field + "R") and cmds.objExists(rpr_field + "H"):
				copyProperty(rpr_name, conv_name, rpr_attr + "H", conv_attr + "R")
				copyProperty(rpr_name, conv_name, rpr_attr + "S", conv_attr + "G")
				copyProperty(rpr_name, conv_name, rpr_attr + "V", conv_attr + "B")
			# XYZ 
			elif cmds.objExists(conv_field + "X") and cmds.objExists(rpr_field + "R"):
				copyProperty(rpr_name, conv_name, rpr_attr + "R", conv_attr + "X")
				copyProperty(rpr_name, conv_name, rpr_attr + "G", conv_attr + "Y")
				copyProperty(rpr_name, conv_name, rpr_attr + "B", conv_attr + "Z")
			elif cmds.objExists(conv_field + "X") and cmds.objExists(rpr_field + "X"):
				copyProperty(rpr_name, conv_name, rpr_attr + "X", conv_attr + "X")
				copyProperty(rpr_name, conv_name, rpr_attr + "Y", conv_attr + "Y")
				copyProperty(rpr_name, conv_name, rpr_attr + "Z", conv_attr + "Z")
			elif cmds.objExists(conv_field + "X") and cmds.objExists(rpr_field + "H"):
				copyProperty(rpr_name, conv_name, rpr_attr + "H", conv_attr + "X")
				copyProperty(rpr_name, conv_name, rpr_attr + "S", conv_attr + "Y")
				copyProperty(rpr_name, conv_name, rpr_attr + "V", conv_attr + "Z")
			# HSV 
			elif cmds.objExists(conv_field + "H") and cmds.objExists(rpr_field + "R"):
				copyProperty(rpr_name, conv_name, rpr_attr + "R", conv_attr + "H")
				copyProperty(rpr_name, conv_name, rpr_attr + "G", conv_attr + "S")
				copyProperty(rpr_name, conv_name, rpr_attr + "B", conv_attr + "V")
			elif cmds.objExists(conv_field + "H") and cmds.objExists(rpr_field + "X"):
				copyProperty(rpr_name, conv_name, rpr_attr + "X", conv_attr + "H")
				copyProperty(rpr_name, conv_name, rpr_attr + "Y", conv_attr + "S")
				copyProperty(rpr_name, conv_name, rpr_attr + "Z", conv_attr + "V")
			elif cmds.objExists(conv_field + "H") and cmds.objExists(rpr_field + "H"):
				copyProperty(rpr_name, conv_name, rpr_attr + "H", conv_attr + "H")
				copyProperty(rpr_name, conv_name, rpr_attr + "S", conv_attr + "S")
				copyProperty(rpr_name, conv_name, rpr_attr + "V", conv_attr + "V")
			else:
				print("[ERROR] Failed to find right variant for {}.{} conversion".format(conv_name, conv_attr))

		# field conversion
		else:
			if vr_type == rpr_type or vr_type == unicode:
				setProperty(rpr_name, rpr_attr, getProperty(conv_name, conv_attr))
			elif vr_type == tuple and rpr_type == float:
				if cmds.objExists(conv_field + "R"):
					conv_attr += "R"
				elif cmds.objExists(conv_field + "X"):
					conv_attr += "X"
				elif cmds.objExists(conv_field + "H"):
					conv_attr += "H"
				setProperty(rpr_name, rpr_attr, getProperty(conv_name, conv_attr))
			elif vr_type == float and rpr_type == tuple:
				if cmds.objExists(rpr_field + "R"):
					rpr_attr1 = rpr_attr + "R"
					rpr_attr2 = rpr_attr + "G"
					rpr_attr3 = rpr_attr + "B"
				elif cmds.objExists(rpr_field + "X"):
					rpr_attr1 = rpr_attr + "X"
					rpr_attr2 = rpr_attr + "Y"
					rpr_attr3 = rpr_attr + "Z"
				elif cmds.objExists(conv_field + "H"):
					rpr_attr1 = rpr_attr + "H"
					rpr_attr2 = rpr_attr + "S"
					rpr_attr3 = rpr_attr + "V"
				setProperty(rpr_name, rpr_attr1, getProperty(conv_name, conv_attr))
				setProperty(rpr_name, rpr_attr2, getProperty(conv_name, conv_attr))
				setProperty(rpr_name, rpr_attr3, getProperty(conv_name, conv_attr))

			write_converted_property_log(rpr_name, conv_name, rpr_attr, conv_attr)
	except Exception as ex:
		traceback.print_exc()
		print(u"[ERROR] Failed to copy parameters from {} to {}".format(conv_field, rpr_field).encode('utf-8'))
		write_own_property_log(u"[ERROR] Failed to copy parameters from {} to {}".format(conv_field, rpr_field).encode('utf-8'))


def setProperty(rpr_name, rpr_attr, value):

	# full name of attribute
	rpr_field = rpr_name + "." + rpr_attr

	try:
		# break existed connection
		if not mapDoesNotExist(rpr_name, rpr_attr):
			source = cmds.connectionInfo(rpr_field, sourceFromDestination=True)
			cmds.disconnectAttr(source, rpr_field)

		if type(value) == tuple:
			cmds.setAttr(rpr_field, value[0], value[1], value[2])
		elif type(value) == str or type(value) == unicode:
			cmds.setAttr(rpr_field, value, type="string")
		else:
			cmds.setAttr(rpr_field, value)
		write_own_property_log(u"Set value {} to {}.".format(value, rpr_field).encode('utf-8'))
	except Exception as ex:
		traceback.print_exc()
		print(u"[ERROR] Set value {} to {} is failed. Check the values and their boundaries. ".format(value, rpr_field).encode('utf-8'))
		write_own_property_log(u"[ERROR] Set value {} to {} is failed. Check the values and their boundaries. ".format(value, rpr_field).encode('utf-8'))


def getProperty(material, attr, size=False):

	# full name of attribute
	field = material + "." + attr
	try:

		if size:
			value = cmds.getAttr(field, size=True)
		else:
			value = cmds.getAttr(field)
			# used for color. it has [(),(),()] structure.
			if type(value) == list:
				value = value[0]
	except Exception as ex:
		print(u"[ERROR] Failed to get information about {} field in {} node.".format(attr, material).encode('utf-8'))
		write_own_property_log(u"[ERROR] Failed to get information about {} field in {} node.".format(attr, material).encode('utf-8'))
		return

	return value


def mapDoesNotExist(rs_name, rs_attr):

	# full name of attribute
	rs_field = rs_name + "." + rs_attr

	try:
		if cmds.listConnections(rs_field):
			return 0
		elif cmds.objExists(rs_field + "R"):
			if cmds.listConnections(rs_field + "R") or cmds.listConnections(rs_field + "G") or cmds.listConnections(rs_field + "B"):
				return 0
		elif cmds.objExists(rs_field + "X"):
			if cmds.listConnections(rs_field + "X") or cmds.listConnections(rs_field + "Y") or cmds.listConnections(rs_field + "Z"):
				return 0
		elif cmds.objExists(rs_field + "H"):
			if cmds.listConnections(rs_field + "H") or cmds.listConnections(rs_field + "S")	or cmds.listConnections(rs_field + "V"):
				return 0
	except Exception as ex:
		traceback.print_exc()
		print(u"[ERROR] There is no {} field in this node. Check the field and try again. ".format(rs_field).encode('utf-8'))
		write_own_property_log(u"[ERROR] There is no {} field in this node. Check the field and try again. ".format(rs_field).encode('utf-8'))
		return

	return 1


def connectProperty(source_name, source_attr, rpr_name, rpr_attr):

	# full name of attribute
	source = source_name + "." + source_attr
	rpr_field = rpr_name + "." + rpr_attr

	try:
		source_type = type(getProperty(source_name, source_attr))
		dest_type = type(getProperty(rpr_name, rpr_attr))

		if rpr_attr in ("surfaceShader", "volumeShader"):
			cmds.connectAttr(source, rpr_field, force=True)

		elif cmds.objExists(source_name + ".outAlpha") and cmds.objExists(source_name + ".outColor"):

			if cmds.objectType(source_name) == "file":
				setProperty(source_name, "ignoreColorSpaceFileRules", 1)

			if source_type == dest_type:
				cmds.connectAttr(source, rpr_field, force=True)
			elif source_type == tuple and dest_type == float:
				source = source_name + ".outAlpha"
				cmds.connectAttr(source, rpr_field, force=True)
			elif source_type == float and dest_type == tuple:
				source = source_name + ".outColor"
				cmds.connectAttr(source, rpr_field, force=True)

		else:
			if source_type == dest_type:
				cmds.connectAttr(source, rpr_field, force=True)
			elif source_type == tuple and dest_type == float:
				if cmds.objExists(source + "R"):
					source += "R"
				elif cmds.objExists(source + "X"):
					source += "X"
				elif cmds.objExists(source + "X"):
					source += "H"
				cmds.connectAttr(source, rpr_field, force=True)
			elif source_type == float and dest_type == tuple:
				if cmds.objExists(rpr_field + "R"):
					rpr_field1 = rpr_field + "R"
					rpr_field2 = rpr_field + "G"
					rpr_field3 = rpr_field + "B"
				elif cmds.objExists(rpr_field + "X"):
					rpr_field1 = rpr_field + "X"
					rpr_field2 = rpr_field + "Y"
					rpr_field3 = rpr_field + "Z"
				elif cmds.objExists(rpr_field + "H"):
					rpr_field1 = rpr_field + "H"
					rpr_field2 = rpr_field + "S"
					rpr_field3 = rpr_field + "V"
				cmds.connectAttr(source, rpr_field1, force=True)
				cmds.connectAttr(source, rpr_field2, force=True)
				cmds.connectAttr(source, rpr_field3, force=True)

		write_own_property_log(u"Created connection from {} to {}.".format(source, rpr_field).encode('utf-8'))
	except Exception as ex:
		traceback.print_exc()
		print(u"[ERROR] Connection {} to {} is failed.".format(source, rpr_field).encode('utf-8'))
		write_own_property_log(u"[ERROR] Connection {} to {} is failed.".format(source, rpr_field).encode('utf-8'))


def invertValue(rpr_name, conv_name, rpr_attr, conv_attr):
	connection = cmds.listConnections(conv_name + "." + conv_attr)
	if connection and cmds.objectType(connection[0]) == "reverse":
		if mapDoesNotExist(connection[0], "input"):
			setProperty(rpr_name, rpr_attr, getProperty(connection[0], "input"))
		else:
			if cmds.listConnections(connection[0] + ".input"):
				copyProperty(rpr_name, connection[0],  rpr_attr, "input")
			elif cmds.listConnections(connection[0] + ".inputX"):
				copyProperty(rpr_name, connection[0],  rpr_attr, "inputX")
			elif cmds.listConnections(connection[0] + ".inputY"):
				copyProperty(rpr_name, connection[0],  rpr_attr, "inputY")
			elif cmds.listConnections(connection[0] + ".inputZ"):
				copyProperty(rpr_name, connection[0],  rpr_attr, "inputZ")
	elif connection:
		reverse_arith = cmds.shadingNode("RPRArithmetic", asUtility=True)
		reverse_arith = cmds.rename(reverse_arith, "Reverse_arithmetic")
		setProperty(reverse_arith, "operation", 1)
		setProperty(reverse_arith, "inputA", (1, 1, 1))
		copyProperty(reverse_arith, conv_name, "inputB", conv_attr)
		connectProperty(reverse_arith, "out", rpr_name, rpr_attr)
	else:
		conv_value = getProperty(conv_name, conv_attr)
		if type(conv_value) == float:
			setProperty(rpr_name, rpr_attr, 1 - conv_value)
		elif type(conv_value) == tuple:
			setProperty(rpr_name, rpr_attr, (1 - conv_value[0], 1 - conv_value[1], 1 - conv_value[2]))

	

def convertbump2d(conv_name, source):

	if cmds.objExists(conv_name + "_rpr"):
		rpr = conv_name + "_rpr"
	else:
		bump_type = getProperty(conv_name, "bumpInterp")
		if not bump_type:
			rpr = cmds.shadingNode("RPRBump", asUtility=True)
			rpr = cmds.rename(rpr, conv_name + "_rpr")
		else:
			rpr = cmds.shadingNode("RPRNormal", asUtility=True)
			rpr = cmds.rename(rpr, conv_name + "_rpr")

		# Logging to file
		start_log(conv_name, rpr)

		# Fields conversion
		copyProperty(rpr, conv_name, "color", "bumpValue")
		copyProperty(rpr, conv_name, "strength", "bumpDepth")

		# Logging to file
		end_log(conv_name)

	conversion_map = {
		"outNormal": "out",
		"outNormalX": "outX",
		"outNormalY": "outY",
		"outNormalZ": "outZ"
	}

	rpr += "." + conversion_map[source]
	return rpr


def convertBlendColors(conv_name, source):

	if cmds.objExists(conv_name + "_rpr"):
		rpr = conv_name + "_rpr"
	else:
		rpr = cmds.shadingNode("RPRBlendValue", asUtility=True)
		rpr = cmds.rename(rpr, conv_name + "_rpr")

		# Logging to file
		start_log(conv_name, rpr)

		# Fields conversion
		copyProperty(rpr, conv_name, "inputA", "color1")
		copyProperty(rpr, conv_name, "inputB", "color2")
		copyProperty(rpr, conv_name, "weight", "blender")

		# Logging to file
		end_log(conv_name)

	conversion_map = {
		"output": "out",
		"outputR": "outR",
		"outputG": "outG",
		"outputB": "outB"
	}

	rpr += "." + conversion_map[source]
	return rpr


def convertLuminance(conv_name, source):

	if cmds.objExists(conv_name + "_rpr"):
		rpr = conv_name + "_rpr"
	else:
		rpr = cmds.shadingNode("RPRArithmetic", asUtility=True)
		rpr = cmds.rename(rpr, conv_name + "_rpr")

		# Logging to file
		start_log(conv_name, rpr)

		# Fields conversion
		copyProperty(rpr, conv_name, "inputA", "value")
		setProperty(rpr, "inputB", (0, 0, 0))
		setProperty(rpr, "operation", 19)

		# Logging to file
		end_log(conv_name)

	conversion_map = {
		"outValue": "outX"
	}

	rpr += "." + conversion_map[source]
	return rpr


def convertColorComposite(conv_name, source):

	operation = getProperty(conv_name, "operation")
	if operation == 2:
		if cmds.objExists(conv_name + "_rpr"):
			rpr = conv_name + "_rpr"
		else:
			rpr = cmds.shadingNode("RPRBlendValue", asUtility=True)
			rpr = cmds.rename(rpr, conv_name + "_rpr")

			# Logging to file
			start_log(conv_name, rpr)

			# Fields conversion
			copyProperty(rpr, conv_name, "inputA", "alphaA")
			copyProperty(rpr, conv_name, "inputB", "alphaB")
			copyProperty(rpr, conv_name, "weight", "factor")
			

			# Logging to file
			end_log(conv_name)

		conversion_map = {
			"outAlpha": "outR"
		}

		rpr += "." + conversion_map[source]
		return rpr

	else:

		if cmds.objExists(conv_name + "_rpr"):
			rpr = conv_name + "_rpr"
		else:
			rpr = cmds.shadingNode("RPRArithmetic", asUtility=True)
			rpr = cmds.rename(rpr, conv_name + "_rpr")

			# Logging to file
			start_log(conv_name, rpr)

			# Fields conversion
			if operation in (0, 4, 5):
				setProperty(rpr, "operation", 0)
				if source == "outAlpha":
					copyProperty(rpr, conv_name, "inputA", "alphaA")
					copyProperty(rpr, conv_name, "inputB", "alphaB")
				else:
					copyProperty(rpr, conv_name, "inputA", "colorA")
					copyProperty(rpr, conv_name, "inputB", "colorB")
			elif operation == 1:
				if source == "outAlpha":
					if mapDoesNotExist(conv_name, "alphaA") and mapDoesNotExist(conv_name, "alphaB"):
						alphaA = getProperty(conv_name, alphaA)
						alphaB = getProperty(conv_name, alphaB)
						if alphaA > alphaB:
							copyProperty(rpr, conv_name, "inputA", "alphaA")
							copyProperty(rpr, conv_name, "inputB", "alphaB")
						else:
							copyProperty(rpr, conv_name, "inputA", "alphaB")
							copyProperty(rpr, conv_name, "inputB", "alphaA")
					elif mapDoesNotExist(conv_name, "alphaA"):
						copyProperty(rpr, conv_name, "inputA", "alphaA")
						copyProperty(rpr, conv_name, "inputB", "alphaB")
					elif mapDoesNotExist(conv_name, "alphaB"):
						copyProperty(rpr, conv_name, "inputA", "alphaB")
						copyProperty(rpr, conv_name, "inputB", "alphaA")
					else:
						copyProperty(rpr, conv_name, "inputA", "alphaA")
						copyProperty(rpr, conv_name, "inputB", "alphaB")
				else:
					if mapDoesNotExist(conv_name, "colorA") and mapDoesNotExist(conv_name, "colorB"):
						colorA = getProperty(conv_name, alphaA)
						colorB = getProperty(conv_name, colorB)
						if colorA[0] > colorB[0] or colorA[1] > colorB[1] or colorA[2] > colorB[2]:
							copyProperty(rpr, conv_name, "inputA", "colorA")
							copyProperty(rpr, conv_name, "inputB", "colorB")
						else:
							copyProperty(rpr, conv_name, "inputA", "colorB")
							copyProperty(rpr, conv_name, "inputB", "colorA")
					elif mapDoesNotExist(conv_name, "colorA"):
						copyProperty(rpr, conv_name, "inputA", "colorA")
						copyProperty(rpr, conv_name, "inputB", "colorB")
					elif mapDoesNotExist(conv_name, "colorB"):
						copyProperty(rpr, conv_name, "inputA", "colorB")
						copyProperty(rpr, conv_name, "inputB", "colorA")
					else:
						copyProperty(rpr, conv_name, "inputA", "colorA")
						copyProperty(rpr, conv_name, "inputB", "colorB")
			elif operation == 3:
				setProperty(rpr, "operation", 2)
				if source == "outAlpha":
					copyProperty(rpr, conv_name, "inputA", "alphaA")
					copyProperty(rpr, conv_name, "inputB", "alphaB")
				else:
					copyProperty(rpr, conv_name, "inputA", "colorA")
					copyProperty(rpr, conv_name, "inputB", "colorB")
			elif operation == 6:
				setProperty(rpr, "operation", 1)
				if source == "outAlpha":
					if mapDoesNotExist(conv_name, "alphaA"):
						copyProperty(rpr, conv_name, "inputB", "alphaA")
						copyProperty(rpr, conv_name, "inputA", "alphaB")
					else:
						copyProperty(rpr, conv_name, "inputA", "alphaA")
						copyProperty(rpr, conv_name, "inputB", "alphaB")
				else:
					if mapDoesNotExist(conv_name, "alphaA"):
						copyProperty(rpr, conv_name, "inputB", "colorA")
						copyProperty(rpr, conv_name, "inputA", "colorB")
					else:
						copyProperty(rpr, conv_name, "inputA", "colorA")
						copyProperty(rpr, conv_name, "inputB", "colorB")
			elif operation == 7:
				setProperty(rpr, "operation", 25)
				if source == "outAlpha":
					copyProperty(rpr, conv_name, "inputA", "alphaB")
					copyProperty(rpr, conv_name, "inputB", "alphaA")
				else:
					copyProperty(rpr, conv_name, "inputA", "colorB")
					copyProperty(rpr, conv_name, "inputB", "colorA")
			elif operation == 8:
				setProperty(rpr, "operation", 20)
				if source == "outAlpha":
					copyProperty(rpr, conv_name, "inputA", "alphaA")
					copyProperty(rpr, conv_name, "inputB", "alphaB")
				else:
					copyProperty(rpr, conv_name, "inputA", "colorA")
					copyProperty(rpr, conv_name, "inputB", "colorB")


			# Logging to file
			end_log(conv_name)

		conversion_map = {
			"outAlpha": "outX",
			"outColor": "out",
			"outColorR": "outX",
			"outColorG": "outY",
			"outColorB": "outZ"
		}

		rpr += "." + conversion_map[source]
		return rpr


def convertReverse(conv_name, source):

	if cmds.objExists(conv_name + "_rpr"):
		rpr = conv_name + "_rpr"
	else:
		rpr = cmds.shadingNode("RPRArithmetic", asUtility=True)
		rpr = cmds.rename(rpr, conv_name + "_rpr")

		# Logging to file
		start_log(conv_name, rpr)

		# Fields conversion
		setProperty(rpr, "inputA", (1, 1, 1))
		copyProperty(rpr, conv_name, "inputB", "input")
		setProperty(rpr, "operation", 1)

		# Logging to file
		end_log(conv_name)

	conversion_map = {
		"output": "out",
		"outputX": "outX",
		"outputY": "outY",
		"outputZ": "outZ"
	}

	rpr += "." + conversion_map[source]
	return rpr


def convertPreMultiply(conv_name, source):

	if cmds.objExists(conv_name + "_rpr"):
		rpr = conv_name + "_rpr"
	else:
		rpr = cmds.shadingNode("RPRArithmetic", asUtility=True)
		rpr = cmds.rename(rpr, conv_name + "_rpr")

		# Logging to file
		start_log(conv_name, rpr)

		# Fields conversion
		copyProperty(rpr, conv_name, "inputA", "inColor")
		alpha = getProperty(conv_name, "inAlpha")
		setProperty(rpr, "inputB", (alpha, alpha, alpha))
		setProperty(rpr, "operation", 2)

		# Logging to file
		end_log(conv_name)

	conversion_map = {
		"outAlpha": "outX",
		"outColor": "out",
		"outColorR": "outX",
		"outColorG": "outY",
		"outColorB": "outZ"
	}

	rpr += "." + conversion_map[source]
	return rpr


def convertVectorProduct(conv_name, source):

	operation = getProperty(conv_name, "operation")
	if operation in (1, 2):
		if cmds.objExists(conv_name + "_rpr"):
			rpr = conv_name + "_rpr"
		else:
			rpr = cmds.shadingNode("RPRArithmetic", asUtility=True)
			rpr = cmds.rename(rpr, conv_name + "_rpr")

			# Logging to file
			start_log(conv_name, rpr)

			# Fields conversion
			if operation == 1:
				setProperty(rpr, "operation", 11)
			elif operation == 2:
				setProperty(rpr, "operation", 12)

			copyProperty(rpr, conv_name, "inputA", "input1")
			copyProperty(rpr, conv_name, "inputB", "input2")

			# Logging to file
			end_log(conv_name)

		conversion_map = {
			"output": "out",
			"outputX": "outX",
			"outputY": "outY",
			"outputZ": "outZ"
		}

		rpr += "." + conversion_map[source]
		return rpr
	else:
		conv_name += "." + source
		return conv_name


def convertChannels(conv_name, source):

	if "outColor" in source:

		if cmds.objExists(conv_name + "_color_rpr"):
			rpr = conv_name + "_color_rpr"
		else:

			rpr = cmds.shadingNode("RPRArithmetic", asUtility=True)
			rpr = cmds.rename(rpr, conv_name + "_color_rpr")

			# Logging to file
			start_log(conv_name, rpr)

			# Fields conversion
			copyProperty(rpr, conv_name, "inputA", "inColor")

			# Logging to file
			end_log(conv_name)

		conversion_map = {
			"outColor": "out",
			"outColorR": "outX",
			"outColorG": "outY",
			"outColorB": "outZ"
		}

		rpr += "." + conversion_map[source]
		return rpr

	elif "outAlpha" in source:

		if cmds.objExists(conv_name + "_alpha_rpr"):
			rpr = conv_name + "_alpha_rpr"
		else:

			rpr = cmds.shadingNode("RPRArithmetic", asUtility=True)
			rpr = cmds.rename(rpr, conv_name + "_alpha_rpr")

			# Logging to file
			start_log(conv_name, rpr)

			# Fields conversion
			copyProperty(rpr, conv_name, "inputA", "inAlpha")

			# Logging to file
			end_log(conv_name)

		conversion_map = {
			"outAlpha": "outX"
		}

		rpr += "." + conversion_map[source]
		return rpr


def convertmultiplyDivide(conv_name, source):

	if cmds.objExists(conv_name + "_rpr"):
		rpr = conv_name + "_rpr"
	else:
		rpr = cmds.shadingNode("RPRArithmetic", asUtility=True)
		rpr = cmds.rename(rpr, conv_name + "_rpr")

		# Logging to file
		start_log(conv_name, rpr)

		# Fields conversion
		operation = getProperty(conv_name, "operation")
		operation_map = {
			1: 2,
			2: 3,
			3: 15
		}
		setProperty(rpr, "operation", operation_map[operation])
		copyProperty(rpr, conv_name, "inputA", "input1")
		copyProperty(rpr, conv_name, "inputB", "input2")
		
		# Logging to file
		end_log(conv_name)

	conversion_map = {
		"output": "out",
		"outputX": "outX",
		"outputY": "outY",
		"outputZ": "outZ"
	}

	rpr += "." + conversion_map[source]
	return rpr


# standard utilities
def convertStandardNode(vrayMaterial, source):

	not_converted_list = ("materialInfo", "defaultShaderList", "shadingEngine", "place2dTexture")
	try:
		for attr in cmds.listAttr(vrayMaterial):
			connection = cmds.listConnections(vrayMaterial + "." + attr)
			if connection:
				if cmds.objectType(connection[0]) not in not_converted_list and attr not in (source, "message"):
					obj, channel = cmds.connectionInfo(vrayMaterial + "." + attr, sourceFromDestination=True).split('.')
					source_name, source_attr = convertMaterial(obj, channel).split('.')
					connectProperty(source_name, source_attr, vrayMaterial, attr)
	except:
		pass

	return vrayMaterial + "." + source


# unsupported utilities
def convertUnsupportedNode(vrayMaterial, source, postfix="_UNSUPPORTED_NODE"):

	if cmds.objExists(vrayMaterial + postfix):
		rpr = vrayMaterial + postfix
	else:
		rpr = cmds.shadingNode("RPRArithmetic", asUtility=True)
		rpr = cmds.rename(rpr, vrayMaterial + postfix)

		# Logging to file
		start_log(vrayMaterial, rpr)

		# 2 connection save
		try:
			setProperty(rpr, "operation", 0)
			unsupported_connections = 0
			for attr in cmds.listAttr(vrayMaterial):
				connection = cmds.listConnections(vrayMaterial + "." + attr)
				if connection:
					if cmds.objectType(connection[0]) not in ("materialInfo", "defaultShaderList", "shadingEngine") and attr not in (source, "message"):
						if unsupported_connections < 2:
							obj, channel = cmds.connectionInfo(vrayMaterial + "." + attr, sourceFromDestination=True).split('.')
							source_name, source_attr = convertMaterial(obj, channel).split('.')
							valueType = type(getProperty(vrayMaterial, attr))
							if valueType == tuple:
								if unsupported_connections < 1:
									connectProperty(source_name, source_attr, rpr, "inputA")
								else:
									connectProperty(source_name, source_attr, rpr, "inputB")
							else:
								if unsupported_connections < 1:
									connectProperty(source_name, source_attr, rpr, "inputAX")
								else:
									connectProperty(source_name, source_attr, rpr, "inputBX")
							unsupported_connections += 1
		except Exception as ex:
			traceback.print_exc()

		# Logging to file
		end_log(vrayMaterial)

	sourceType = type(getProperty(vrayMaterial, source))
	if sourceType == tuple:
		rpr += ".out"
	else:
		rpr += ".outX"

	return rpr


def convertVRayTemperature(vr, source):

	if cmds.objExists(vr + "_rpr"):
		rpr = vr + "_rpr"
	else:
		rpr = cmds.shadingNode("RPRArithmetic", asUtility=True)
		rpr = cmds.rename(rpr, vr + "_rpr")

		# Logging to file
		start_log(vr, rpr)

		# Fields conversion
		setProperty(rpr, 'operation', 0)
		setProperty(rpr, 'inputB', (0, 0, 0))
		colorMode = getProperty(vr, 'colorMode')
		if colorMode:
			setProperty(rpr, 'inputA', convertTemperature(getProperty(vr, 'temperature')))
		else:
			copyProperty(rpr, vr, "inputA", "color")
		

		# Logging to file
		end_log(vr)

	conversion_map = {
		"color": "out",
		"colorR": "outX",
		"colorG": "outY",
		"colorB": "outZ",
		"temperature": "out",
		"rgbMultiplier": "out",
		"gammaCorrection": "out",
		"alpha": "out",
		"red": "outX",
		"green": "outY",
		"blue": "outZ"
	}

	rpr += "." + conversion_map[source]
	return rpr


def convertVRayFresnel(vr, source):

	if cmds.objExists(vr + "_rpr"):
		rpr = vr + "_rpr"
	else:
		rpr = cmds.shadingNode("RPRBlendValue", asUtility=True)
		rpr = cmds.rename(rpr, vr + "_rpr")

		# Logging to file
		start_log(vr, rpr)

		# Fields conversion
		colorConstantFront = cmds.shadingNode("colorConstant", asUtility=True)
		copyProperty(colorConstantFront, vr, 'inColor', 'frontColor')

		colorConstantSide = cmds.shadingNode("colorConstant", asUtility=True)
		copyProperty(colorConstantSide, vr, 'inColor', 'sideColor')

		RPRFresnel = cmds.shadingNode("RPRFresnel", asUtility=True)
		copyProperty(RPRFresnel, vr, 'ior', 'IOR')

		connectProperty(colorConstantFront, 'outColor', rpr, 'inputA')
		connectProperty(colorConstantSide, 'outColor', rpr, 'inputB')
		connectProperty(RPRFresnel, 'out', rpr, 'weight')

		# Logging to file
		end_log(vr)

	conversion_map = {
		"outColor": "out",
		"outColorR": "outR",
		"outColorG": "outG",
		"outColorB": "outB"
	}

	rpr += "." + conversion_map[source]
	return rpr


def convertVRayLayeredTex(vr, source):

	if cmds.objExists(vr + "_rpr"):
		rpr = vr + "_rpr"
	else:
		rpr = cmds.shadingNode("RPRBlendValue", asUtility=True)
		
		# Logging to file
		start_log(vr, rpr)

		# check used layers 
		def checkLayerEnabled(index):
			if getProperty(vr, "layers[{}].enabled".format(index)):
				if getProperty(vr, "layers[{}].tex".format(index)) != [(0.5, 0.5, 0.5)] or getProperty(vr, "layers[{}].mask".format(index)) != [(1, 1, 1)] \
					or getProperty(vr, "layers[{}].blendMode".format(index)) or getProperty(vr, "layers[{}].opacity".format(index)) < 1:
					return True
			return False


		# convert vray layer mask
		def convertMask(index):
			# special case. Invert mask
			invertValue = False
			if layer_size > 1 and index == layers_idxs[0]:
				invertValue = True

			# mask hasn't map. Convert with formula
			if mapDoesNotExist(vr, "layers[{}].mask".format(index)):
				# convert color to weight
				mask_color = getProperty(vr, "layers[{}].mask".format(index))
				if mask_color == (1, 1, 1):
					mask_weight = 0
				else:
					mask_weight = mask_color[0] * 0.3 + mask_color[1] * 0.59 + mask_color[2] * 0.11
				# opacity hasn't map
				if mapDoesNotExist(vr, "layers[{}].opacity".format(index)):
					opacity = getProperty(vr, "layers[{}].opacity".format(index))
					if opacity < 1 and invertValue:
						setProperty(rpr, "weight", 1 - mask_weight * opacity)
					elif opacity < 1:
						setProperty(rpr, "weight", mask_weight * opacity)
					else:
						setProperty(rpr, "weight", mask_weight)
				# opacity is map
				else:
					mult_opacity = cmds.shadingNode("RPRArithmetic", asUtility=True)
					mult_opacity = cmds.rename(lum_mult_opacity, 'layer_{}_mult_opacity'.format(index))
					setProperty(mult_opacity, "operation", 2)
					setProperty(mult_opacity, "inputAX", mask_weight)
					copyProperty(mult_opacity, vr, "inputBX", "layers[{}].opacity".format(index))

					if invertValue:
						invert_mask = cmds.shadingNode("RPRArithmetic", asUtility=True)
						invert_mask = cmds.rename(invert_mask, 'invert_mask')
						setProperty(invert_mask, "operation", 1)
						setProperty(invert_mask, "inputAX", 1)
						connectProperty(mult_opacity, "outX", invert_mask, "inputBX")
						connectProperty(invert_mask, "outX", rpr, "weight")
					else:
						connectProperty(mult_opacity, "outX", rpr, "weight")

			# mask has map. Convert with luminance node
			else:
				# opacity is used
				if not mapDoesNotExist(vr, "layers[{}].opacity".format(index)) or getProperty(vr, "layers[{}].opacity".format(index)) < 1:
					
					mask_mult_opacity = cmds.shadingNode("RPRArithmetic", asUtility=True)
					mask_mult_opacity = cmds.rename(mask_mult_opacity, 'layer_{}_mask_mult_opacity'.format(index))
					setProperty(mask_mult_opacity, "operation", 2)
					copyProperty(mask_mult_opacity, vr, "inputA", "layers[{}].mask".format(index))
					copyProperty(mask_mult_opacity, vr, "inputBX", "layers[{}].opacity".format(index))

					if invertValue:
						invert_mask = cmds.shadingNode("RPRArithmetic", asUtility=True)
						invert_mask = cmds.rename(invert_mask, 'invert_mask')
						setProperty(invert_mask, "operation", 1)
						setProperty(invert_mask, "inputA", (1, 1, 1))
						connectProperty(mask_mult_opacity, "out", invert_mask, "inputB")
						connectProperty(invert_mask, "outX", rpr, "weight")

					else:
						connectProperty(mask_mult_opacity, "outX", rpr, "weight")
				# no opacity
				else:
					if invertValue:
						invert_mask = cmds.shadingNode("RPRArithmetic", asUtility=True)
						invert_mask = cmds.rename(invert_mask, 'invert_mask')
						setProperty(invert_mask, "operation", 1)
						setProperty(invert_mask, "inputA", (1, 1, 1))
						copyProperty(invert_mask, vr, "inputA", "layers[{}].mask".format(index))
						connectProperty(invert_mask, "outX", rpr, "weight")
					else:
						luminance = cmds.shadingNode("luminance", asUtility=True)
						luminance = cmds.rename(luminance, 'layer_{}_mask'.format(index))
						copyProperty(luminance, vr, "value", "layers[{}].mask".format(index))
						connectProperty(luminance, "outValue", rpr, "weight")

		# get count of layers
		layer_size = getProperty(vr, "layers", size=True)

		layers_idxs = []
		current_layer_index = 0
		while len(layers_idxs) < layer_size and current_layer_index < 20:
			if checkLayerEnabled(current_layer_index):
				layers_idxs.append(current_layer_index)
			current_layer_index += 1
			if current_layer_index == 20:
				print("[ERROR] Script doesn't support layers with index > 20!")

		# convert 1st layer		
		rpr = cmds.rename(rpr, "layer_{}_blend".format(layers_idxs[0]))
		copyProperty(rpr, vr, "inputA", "layers[{}].tex".format(layers_idxs[0]))
		convertMask(layers_idxs[0])

		if layer_size > 1:
			copyProperty(rpr, vr, "inputB", "layers[{}].tex".format(layers_idxs[1]))

		for idx in layers_idxs[1:]:

			old_rpr = rpr
			rpr = cmds.shadingNode("RPRBlendValue", asUtility=True)
			rpr = cmds.rename(rpr, "layers_{}_blend".format(idx))

			blendMode = getProperty(vr, "layers[{}].blendMode".format(idx))
			if blendMode:
				arith = cmds.shadingNode("RPRArithmetic", asUtility=True)
				arith = cmds.rename(arith, 'layers_{}_arith'.format(idx))

				def convertUsingOneArith():
					copyProperty(arith, vr, "inputA", "layers[{}].tex".format(idx))
					connectProperty(old_rpr, "out", arith, "inputB")
					connectProperty(arith, "out", rpr, "inputA")

				if blendMode in (1, 13): # average & spotlite blend
					setProperty(arith, "operation", 0)
					copyProperty(arith, vr, "inputA", "layers[{}].tex".format(idx))
					connectProperty(old_rpr, "out", arith, "inputB")

					arith_avg = cmds.shadingNode("RPRArithmetic", asUtility=True)
					setProperty(arith_avg, "operation", 3)
					connectProperty(arith, "out", arith_avg, "inputA")
					setProperty(arith_avg, "inputB", (2, 2, 2))
					connectProperty(arith_avg, "out", rpr, "inputA")

				elif blendMode in (2, 11): # add & linear dodge
					setProperty(arith, "operation", 0)
					convertUsingOneArith()

				elif blendMode == 3: # sub
					setProperty(arith, "operation", 1)
					convertUsingOneArith()

				elif blendMode == 4: # darken
					setProperty(arith, "operation", 21)
					convertUsingOneArith()

				elif blendMode in (5, 12): # mult & spotlite
					setProperty(arith, "operation", 2)
					convertUsingOneArith()

				elif blendMode == 6: # color burn 
					setProperty(arith, "operation", 2)
					setProperty(arith, "inputA", (1, 1, 1))
					connectProperty(old_rpr, "out", arith, "inputB")

					arith_div = cmds.shadingNode("RPRArithmetic", asUtility=True)
					setProperty(arith_div, "operation", 3)
					connectProperty(arith, "out", arith_div, "inputA")
					copyProperty(arith_div, vr, "inputB", "layers[{}].tex".format(idx))
					
					arith_invert = cmds.shadingNode("RPRArithmetic", asUtility=True)
					setProperty(arith_invert, "operation", 1)
					setProperty(arith_invert, "inputA", (1, 1, 1))
					connectProperty(arith_div, "out", arith_invert, "inputB")

					connectProperty(arith_invert, "out", rpr, "inputA")

				elif blendMode == 7: # linear burn
					setProperty(arith, "operation", 0)
					copyProperty(arith, vr, "inputA", "layers[{}].tex".format(idx))
					connectProperty(old_rpr, "out", arith, "inputB")

					arith_minus_one = cmds.shadingNode("RPRArithmetic", asUtility=True)
					setProperty(arith_invert, "operation", 1)
					connectProperty(arith, "out", arith_minus_one, "inputA")
					setProperty(arith_minus_one, "inputA", (1, 1, 1))

					connectProperty(arith_minus_one, "out", rpr, "inputA")

				elif blendMode == 8: # lighten
					setProperty(arith, "operation", 22)
					convertUsingOneArith()

				elif blendMode == 9: # screen
					setProperty(arith, "operation", 1)
					setProperty(arith, "inputA", (1, 1, 1))
					copyProperty(arith, vr, "inputB", "layers[{}].tex".format(idx))

					arith_invert = cmds.shadingNode("RPRArithmetic", asUtility=True)
					setProperty(arith_invert, "operation", 1)
					setProperty(arith_invert, "inputA", (1, 1, 1))
					connectProperty(old_rpr, "out", arith_invert, "inputB")

					arith_mult = cmds.shadingNode("RPRArithmetic", asUtility=True)
					setProperty(arith_mult, "operation", 2)
					connectProperty(arith, "out", arith_mult, "inputA")
					connectProperty(arith_invert, "out", arith_mult, "inputB")

					arith_invert_mult = cmds.shadingNode("RPRArithmetic", asUtility=True)
					setProperty(arith_invert_mult, "operation", 1)
					setProperty(arith_invert_mult, "inputA", (1, 1, 1))
					connectProperty(arith_invert, "out", arith_invert_mult, "inputB")

					connectProperty(arith_invert_mult, "out", rpr, "inputA")

				elif blendMode == 9: # color dodge
					setProperty(arith, "operation", 1)
					setProperty(arith, "inputA", (1, 1, 1))
					copyProperty(arith, vr, "inputB", "layers[{}].tex".format(idx))

					arith_div = cmds.shadingNode("RPRArithmetic", asUtility=True)
					setProperty(arith_div, "operation", 3)
					connectProperty(old_rpr, "out", arith_div, "inputA")
					connectProperty(arith, "out", arith_div, "inputB")

					connectProperty(arith_div, "out", rpr, "inputA")

				else:
					arith = cmds.rename(arith, 'layers_{}_UNSUPPORTED_BLEND_MODE'.format(idx))
					setProperty(arith, "operation", 0)

				
			else:
				copyProperty(rpr, vr, "inputA", "layers[{}].tex".format(idx))

			connectProperty(old_rpr, "out", rpr, "inputB")
			convertMask(idx)		

		# Logging to file
		end_log(vr)

	conversion_map = {
		"outColor": "out",
		"outColorR": "outR",
		"outColorG": "outG",
		"outColorB": "outB"
	}

	rpr += "." + conversion_map[source]
	return rpr


def convertVRayMultiSubTex(vr, source):

	if cmds.objExists(vr + "_rpr"):
		rpr = vr + "_rpr"
	else:

		if getProperty(vr, "multiSubType") == 30:

			rpr = cmds.shadingNode("RPRBlendValue", asUtility=True)

			# Logging to file
			start_log(vr, rpr)

			# check used layers 
			def checkSubTexEnabled(index):
				if getProperty(vr, "subTexList[{}].subTexListUsed".format(index)):
					if getProperty(vr, "subTexList[{}].subTexListID".format(index)) != 1 or getProperty(vr, "subTexList[{}].subTexListTex".format(index)) != [(0.5, 0.5, 0.5)]:
						return True
				return False

			# get count of layers
			subText_size = getProperty(vr, "subTexList", size=True)
			
			idGenTex = getProperty(vr, "idGenTex")

			subText_idxs = []
			current_subText_index = 0
			noIdGenTexInTexture = True
			while len(subText_idxs) < subText_size and current_subText_index < 20:
				if checkSubTexEnabled(current_subText_index):
					subText_idxs.append(current_subText_index)
				if getProperty(vr, "subTexList[{}].subTexListID".format(current_subText_index)) == idGenTex:
					noIdGenTexInTexture = False
				current_subText_index += 1
				if current_subText_index == 20:
					print("[ERROR] Script doesn't support layers with index > 20!")

			# default texture conversion
			copyProperty(rpr, vr, "inputA", "defTexture")
			if noIdGenTexInTexture:
				setProperty(rpr, "weight", 0)

			first_texture = True
			second_texture = True
			for idx in subText_idxs:
				if first_texture:
					copyProperty(rpr, vr, "inputB", "subTexList[{}].subTexListTex".format(idx))
					if getProperty(vr, "subTexList[{}].subTexListID".format(idx)) == idGenTex:
						setProperty(rpr, "weight", 1)
					first_texture = False
				else:
					prev_rpr = rpr
					rpr = cmds.shadingNode("RPRBlendValue", asUtility=True)
					rpr = cmds.rename(rpr, "texture_{}_blend".format(idx))
					connectProperty(prev_rpr, 'out', rpr, 'inputA')
					copyProperty(rpr, vr, "inputB", "subTexList[{}].subTexListTex".format(idx))
					if getProperty(vr, "subTexList[{}].subTexListID".format(idx)) == idGenTex:
						setProperty(rpr, "weight", 1)
					else:
						setProperty(rpr, "weight", 0)

			# Logging to file
			end_log(vr)

		else:
			rpr = cmds.shadingNode("RPRArithmetic", asUtility=True)
			rpr = cmds.rename(rpr, vr + "_UNSUPPORTED_NODE")
			copyProperty(rpr, vr, "inputA", "defTexture")


	conversion_map = {
		"outColor": "out",
		"outColorR": "outR",
		"outColorG": "outG",
		"outColorB": "outB"
	}

	rpr += "." + conversion_map[source]
	return rpr


def duplicateFileNode(node):
	file = cmds.duplicate(node)[0]
	place2dTexture = cmds.listConnections(node, type="place2dTexture")[0]
	texture = cmds.duplicate(place2dTexture)[0]
	cmds.connectAttr(texture + ".coverage", file + ".coverage", f=True)
	cmds.connectAttr(texture + ".translateFrame", file + ".translateFrame", f=True)
	cmds.connectAttr(texture + ".rotateFrame", file + ".rotateFrame", f=True)
	cmds.connectAttr(texture + ".mirrorU", file + ".mirrorU", f=True)
	cmds.connectAttr(texture + ".mirrorV", file + ".mirrorV", f=True)
	cmds.connectAttr(texture + ".stagger", file + ".stagger", f=True)
	cmds.connectAttr(texture + ".wrapU", file + ".wrapU", f=True)
	cmds.connectAttr(texture + ".wrapV", file + ".wrapV", f=True)
	cmds.connectAttr(texture + ".repeatUV", file + ".repeatUV", f=True)
	cmds.connectAttr(texture + ".offset", file + ".offset", f=True)
	cmds.connectAttr(texture + ".rotateUV", file + ".rotateUV", f=True)
	cmds.connectAttr(texture + ".noiseUV", file + ".noiseUV", f=True)
	cmds.connectAttr(texture + ".vertexUvTwo", file + ".vertexUvTwo" , f=True)
	cmds.connectAttr(texture + ".vertexUvThree", file + ".vertexUvThree", f=True)
	cmds.connectAttr(texture + ".vertexCameraOne", file + ".vertexCameraOne", f=True)
	cmds.connectAttr(texture + ".outUV", file + ".uv", f=True)
	cmds.connectAttr(texture + ".outUvFilterSize", file + ".uvFilterSize")
	cmds.connectAttr(texture + ".vertexUvOne", file + ".vertexUvOne")

	return file, place2dTexture


def convertVRayTriplanar(vr, source):

	if cmds.objExists(vr + "_rpr"):
		rpr = vr + "_rpr"
	else:
		rpr = cmds.shadingNode("projection", asUtility=True)
		rpr = cmds.rename(rpr, vr + "_rpr")

		# Logging to file
		start_log(vr, rpr)

		# Fields conversion
		setProperty(rpr, 'projType', 6)
		file_node, place2dTexture = None, None
		textureX = cmds.listConnections(vr + ".textureX")
		if textureX and cmds.objectType(textureX[0]) == "file":
			file_node, place2dTexture = duplicateFileNode(textureX[0])
			connectProperty(file_node, "outColor", rpr, "image")
		else:
			copyProperty(rpr, vr, 'image', 'textureX')

		if file_node:
			setProperty(place2dTexture, 'repeatU', getProperty(vr, "scale") * 20 * getProperty(place2dTexture, "repeatU"))
			setProperty(place2dTexture, 'repeatV', getProperty(vr, "scale") * 20 * getProperty(place2dTexture, "repeatV"))
		else:
			rpr = cmds.rename(rpr, vr + "_UNSUPPORTED_NODE")

		# Logging to file
		end_log(vr)

	rpr += ".outColor"
	return rpr


def convertVRayInverseExposure(vr, source):

	if cmds.objExists(vr + "_rpr"):
			rpr = vr + "_rpr"
	else:

		vrayCameraPhysicalOn = False
		cameras = cmds.ls(type="camera")
		for cam in cameras:
			if "vrayCameraPhysicalOn" in cmds.listAttr(cam):
				if getProperty(cam, "vrayCameraPhysicalOn"):
					vrayCameraPhysicalOn = True

		if vrayCameraPhysicalOn:

			rpr = cmds.shadingNode("RPRArithmetic", asUtility=True)
			rpr = cmds.rename(rpr, vr + "_rpr")

			# Logging to file
			start_log(vr, rpr)

			f_number_power_by_2 = cmds.shadingNode("RPRArithmetic", asUtility=True)
			f_number_power_by_2 = cmds.rename(f_number_power_by_2, "f_number_power_by_2")
			setProperty(f_number_power_by_2, "operation", 15)
			copyProperty(f_number_power_by_2, vr, "inputAX", "fNumber")
			setProperty(f_number_power_by_2, "inputBX", 2)

			shutter_mult_fnumber = cmds.shadingNode("RPRArithmetic", asUtility=True)
			shutter_mult_fnumber = cmds.rename(shutter_mult_fnumber, "shutter_mult_fnumber")
			setProperty(shutter_mult_fnumber, "operation", 2)
			connectProperty(f_number_power_by_2, "outX", shutter_mult_fnumber, "inputAX")
			copyProperty(shutter_mult_fnumber, vr, "inputBX", "shutterSpeed")

			mult_x_120 = cmds.shadingNode("RPRArithmetic", asUtility=True)
			mult_x_120 = cmds.rename(mult_x_120, "mult_x_120")
			setProperty(mult_x_120, "operation", 2)
			connectProperty(shutter_mult_fnumber, "outX", mult_x_120, "inputAX")
			setProperty(mult_x_120, "inputBX", 120)

			div_by_iso_1 = cmds.shadingNode("RPRArithmetic", asUtility=True)
			div_by_iso_1 = cmds.rename(div_by_iso_1, "div_by_iso_1")
			setProperty(div_by_iso_1, "operation", 3)
			connectProperty(mult_x_120, "outX", div_by_iso_1, "inputAX")
			copyProperty(div_by_iso_1, vr, "inputBX", "iso")

			one_div_by = cmds.shadingNode("RPRArithmetic", asUtility=True)
			one_div_by = cmds.rename(one_div_by, "one_div_by")
			setProperty(one_div_by, "operation", 3)
			setProperty(one_div_by, "inputAX", 1)
			connectProperty(div_by_iso_1, "outX", one_div_by, "inputBX")

			fnumber_div_shutter = cmds.shadingNode("RPRArithmetic", asUtility=True)
			fnumber_div_shutter = cmds.rename(fnumber_div_shutter, "fnumber_div_shutter")
			setProperty(fnumber_div_shutter, "operation", 3)
			connectProperty(f_number_power_by_2, "outX", fnumber_div_shutter, "inputAX")
			copyProperty(fnumber_div_shutter, vr, "inputBX", "shutterSpeed")

			add_x_250 = cmds.shadingNode("RPRArithmetic", asUtility=True)
			add_x_250 = cmds.rename(add_x_250, "add_x_250")
			setProperty(add_x_250, "operation", 0)
			connectProperty(fnumber_div_shutter, "outX", add_x_250, "inputAX")
			setProperty(add_x_250, "inputBX", 250)

			sub_by_iso = cmds.shadingNode("RPRArithmetic", asUtility=True)
			sub_by_iso = cmds.rename(sub_by_iso, "sub_by_iso")
			setProperty(sub_by_iso, "operation", 1)
			connectProperty(add_x_250, "outX", sub_by_iso, "inputAX")
			copyProperty(sub_by_iso, vr, "inputBX", "iso")

			mult_divs = cmds.shadingNode("RPRArithmetic", asUtility=True)
			mult_divs = cmds.rename(mult_divs, "mult_divs")
			setProperty(mult_divs, "operation", 2)
			connectProperty(one_div_by, "outX", mult_divs, "inputAX")
			connectProperty(sub_by_iso, "outX", mult_divs, "inputBX")

			setProperty(rpr, "operation", 2)
			connectProperty(mult_divs, "outX", rpr, "inputAX")
			connectProperty(mult_divs, "outX", rpr, "inputAY")
			connectProperty(mult_divs, "outX", rpr, "inputAZ")
			copyProperty(rpr, vr, "inputB", "inTexture")
			
			# Logging to file
			end_log(vr)

		else:
			rpr = cmds.shadingNode("RPRArithmetic", asUtility=True)
			rpr = cmds.rename(rpr, vr + "_UNSUPPORTED_NODE")
			copyProperty(rpr, vr, "inputA", "inTexture")


	conversion_map = {
		"outColor": "out",
		"outColorR": "outR",
		"outColorG": "outG",
		"outColorB": "outB",
		"inTexture": "out",
		"inTextureR": "outR",
		"inTextureG": "outG",
		"inTextureB": "outB"
	}

	rpr += "." + conversion_map[source]
	return rpr


def convertVRayVertexColors(vr, source):

	if cmds.objExists(vr + "_rpr"):
		rpr = vr + "_rpr"
	else:
		rpr = cmds.shadingNode("RPRLookup", asUtility=True)
		rpr = cmds.rename(rpr, vr + "_rpr")

		# Logging to file
		start_log(vr, rpr)

		# Fields conversion
		setProperty(rpr, 'type', 256)
		
		# Logging to file
		end_log(vr)

	rpr += ".out"
	return rpr


def convertVRayUserScalar(vr, source):

	if cmds.objExists(vr + "_rpr"):
		rpr = vr + "_rpr"
	else:
		rpr = cmds.shadingNode("floatConstant", asUtility=True)
		rpr = cmds.rename(rpr, vr + "_rpr")

		# Logging to file
		start_log(vr, rpr)

		# Fields conversion
		copyProperty(rpr, vr, "inFloat", "defaultValue")
		
		# Logging to file
		end_log(vr)

	conversion_map = {
		"outAlpha": "outFloat"
	}

	rpr += "." + conversion_map[source]
	return rpr


def convertVRayUserInteger(vr, source):

	if cmds.objExists(vr + "_rpr"):
		rpr = vr + "_rpr"
	else:
		rpr = cmds.shadingNode("floatConstant", asUtility=True)
		rpr = cmds.rename(rpr, vr + "_rpr")

		# Logging to file
		start_log(vr, rpr)

		# Fields conversion
		copyProperty(rpr, vr, "inFloat", "defaultValue")
		
		# Logging to file
		end_log(vr)

	conversion_map = {
		"outInt": "outFloat"
	}

	rpr += "." + conversion_map[source]
	return rpr


def convertVRayUserColor(vr, source):

	if cmds.objExists(vr + "_rpr"):
		rpr = vr + "_rpr"
	else:
		rpr = cmds.shadingNode("colorConstant", asUtility=True)
		rpr = cmds.rename(rpr, vr + "_rpr")

		# Logging to file
		start_log(vr, rpr)

		# Fields conversion
		copyProperty(rpr, vr, "inColor", "color")
		
		# Logging to file
		end_log(vr)


	rpr += "." + source
	return rpr


def convertVRayDirt(vr, source):

	if cmds.objExists(vr + "_rpr"):
		rpr = vr + "_rpr"
	else:
		rpr = cmds.shadingNode("RPRAmbientOcclusion", asUtility=True)
		rpr = cmds.rename(rpr, vr + "_rpr")

		# Logging to file
		start_log(vr, rpr)

		# Fields conversion
		copyProperty(rpr, vr, 'occludedColor', 'blackColor')
		copyProperty(rpr, vr, 'unoccludedColor', 'whiteColor')

		if getProperty(vr, "falloff") > 0:
			setProperty(rpr, "radius", 0.02076 * (getProperty(vr, "radius") / getProperty(vr, "falloff")) ** 0.89408)
		else:
			setProperty(rpr, "radius", 0.02076 * getProperty(vr, "radius") ** 0.89408)

		if getProperty(vr, "invertNormal"):
			setProperty(rpr, "side", 1)

		# Logging to file
		end_log(vr)

	rpr += ".output"
	return rpr


# Create default uber material for unsupported material
def convertUnsupportedMaterial(vrayMaterial, source):

	assigned = checkAssign(vrayMaterial)
	
	if cmds.objExists(vrayMaterial + "_rpr"):
		rprMaterial = vrayMaterial + "_rpr"
	else:
		# Creating new Uber material
		rprMaterial = cmds.shadingNode("RPRUberMaterial", asShader=True)
		rprMaterial = cmds.rename(rprMaterial, (vrayMaterial + "_UNSUPPORTED_MATERIAL"))

		# Check shading engine in vrayMaterial
		if assigned:
			sg = rprMaterial + "SG"
			cmds.sets(renderable=True, noSurfaceShader=True, empty=True, name=sg)
			connectProperty(rprMaterial, "outColor", sg, "surfaceShader")

		# Logging to file
		start_log(vrayMaterial, rprMaterial)

		# set green color
		setProperty(rprMaterial, "diffuseColor", (0, 1, 0))

		end_log(vrayMaterial)

	if source:
		rprMaterial += "." + source
	return rprMaterial


######################## 
##  VRayMtl
########################

def convertVRayMtl(vrMaterial, source):

	assigned = checkAssign(vrMaterial)
	
	if cmds.objExists(vrMaterial + "_rpr"):
		rprMaterial = vrMaterial + "_rpr"
	else:
		# Creating new Uber material
		rprMaterial = cmds.shadingNode("RPRUberMaterial", asShader=True)
		rprMaterial = cmds.rename(rprMaterial, vrMaterial + "_rpr")

		# Check shading engine in vrMaterial
		if assigned:
			sg = rprMaterial + "SG"
			cmds.sets(renderable=True, noSurfaceShader=True, empty=True, name=sg)
			connectProperty(rprMaterial, "outColor", sg, "surfaceShader")
			
		# Enable properties, which are default in VRay
		defaultEnable(rprMaterial, vrMaterial, "diffuse", "diffuseColorAmount", "color")
		defaultEnable(rprMaterial, vrMaterial, "reflections", "reflectionColorAmount", "reflectionColor")
		defaultEnable(rprMaterial, vrMaterial, "refraction", "refractionColorAmount", "refractionColor")
		
		# Logging to file
		start_log(vrMaterial, rprMaterial)

		# Basic parameters
		copyProperty(rprMaterial, vrMaterial, "diffuseColor", "color")
		copyProperty(rprMaterial, vrMaterial, "diffuseWeight", "diffuseColorAmount")
		copyProperty(rprMaterial, vrMaterial, "diffuseRoughness", "roughnessAmount")
		
		if getProperty(vrMaterial, 'illumColor') != (0, 0, 0) or not mapDoesNotExist(vrMaterial, 'illumColor'):
			setProperty(rprMaterial, 'emissive', 1)
			copyProperty(rprMaterial, vrMaterial, "emissiveColor", "illumColor")

		# opacity
		opacity_color = getProperty(vrMaterial, "opacityMap")
		if opacity_color[0] < 1 or opacity_color[1] < 1 or opacity_color[2] < 1:
			if mapDoesNotExist(vrMaterial, "opacityMap"):
				transparency = 1 - max(opacity_color)
				setProperty(rprMaterial, "transparencyLevel", transparency)
			else:
				invertValue(rprMaterial, vrMaterial, "transparencyLevel", "opacityMap")
			setProperty(rprMaterial, "transparencyEnable", 1)

		# reflection
		copyProperty(rprMaterial, vrMaterial, "reflectColor", "reflectionColor")
		copyProperty(rprMaterial, vrMaterial, "reflectWeight", "reflectionColorAmount")

		metalness = getProperty(vrMaterial, 'metalness')
		if metalness:
			setProperty(rprMaterial, 'reflectMetalMaterial', 1)
			copyProperty(rprMaterial, vrMaterial, 'reflectMetalness', 'metalness')
			copyProperty(rprMaterial, vrMaterial, 'reflectColor', 'color')

		useRoughness = getProperty(vrMaterial, 'useRoughness')
		if useRoughness:
			copyProperty(rprMaterial, vrMaterial, "reflectRoughness", "reflectionGlossiness")
		else:
			invertValue(rprMaterial, vrMaterial, 'reflectRoughness', 'reflectionGlossiness')

		if getProperty(vrMaterial, 'lockFresnelIORToRefractionIOR'):
			copyProperty(rprMaterial, vrMaterial, "reflectIOR", "refractionIOR")
		else:
			fresnelIOR = getProperty(vrMaterial, 'fresnelIOR')

			if fresnelIOR < 0.1 or fresnelIOR > 10:
				setProperty(rprMaterial, 'reflectMetalMaterial', 1) 
				setProperty(rprMaterial, 'reflectMetalness', 1)

			if mapDoesNotExist(vrMaterial, 'fresnelIOR') and fresnelIOR > 10:
				setProperty(rprMaterial, "reflectIOR", 10)
			else:
				copyProperty(rprMaterial, vrMaterial, "reflectIOR", "fresnelIOR")

		copyProperty(rprMaterial, vrMaterial, "reflectAnisotropy", "anisotropy")
		anisotropyDerivation = getProperty(vrMaterial, 'anisotropyDerivation')
		if anisotropyDerivation:
			copyProperty(rprMaterial, vrMaterial, 'reflectAnisotropyRotation', 'anisotropyUVWGen')
		else:
			anisotropyRotation = getProperty(vrMaterial, 'anisotropyRotation')
			anisotropyRotation_mod = math.modf(anisotropyRotation)[0]
			anisotropyAxis = getProperty(vrMaterial, 'anisotropyAxis')
			if anisotropyAxis == 0:
				rprAnisotropyRotation = -0.403 * (anisotropyRotation_mod ** 3) + 0.5714 * (anisotropyRotation_mod ** 2) -1.086 * anisotropyRotation_mod - 0.1993
			elif anisotropyAxis == 1:
				rprAnisotropyRotation = -1 * anisotropyRotation_mod
			elif anisotropyAxis == 2:
				rprAnisotropyRotation = -1.5873 * anisotropyRotation_mod ** 3 + 2.4603 * anisotropyRotation_mod ** 2 - 1.373 * anisotropyRotation_mod + 0.7
				setProperty(rprMaterial, 'reflectAnisotropyRotation', rprAnisotropyRotation)

		global MAX_RAY_DEPTH

		reflectionsMaxDepth = getProperty(vrMaterial, 'reflectionsMaxDepth')
		if MAX_RAY_DEPTH and reflectionsMaxDepth < MAX_RAY_DEPTH:
			MAX_RAY_DEPTH = reflectionsMaxDepth

		refractionsMaxDepth = getProperty(vrMaterial, 'refractionsMaxDepth')
		if MAX_RAY_DEPTH and refractionsMaxDepth < MAX_RAY_DEPTH:
			MAX_RAY_DEPTH = refractionsMaxDepth

		copyProperty(rprMaterial, vrMaterial, 'refractColor', 'fogColor')
		refractColor = getProperty(vrMaterial, 'refractionColor')
		rpr_refr_weight = getProperty(vrMaterial, 'refractionColorAmount') * (0.3 * refractColor[0] + 0.59 * refractColor[1] + 0.11 * refractColor[2])
		setProperty(rprMaterial, 'refractWeight', rpr_refr_weight)
		invertValue(rprMaterial, vrMaterial, 'refractRoughness', 'refractionGlossiness')
		copyProperty(rprMaterial, vrMaterial, 'refractIor', 'refractionIOR')

		if getProperty(vrMaterial, 'refractionIOR') == 1:
			setProperty(rprMaterial, 'refractThinSurface', 1)

		sssOn = getProperty(vrMaterial, 'sssOn')
		if sssOn:
			setProperty(rprMaterial, 'sssEnable', 1)

		# bump
		if not mapDoesNotExist(vrMaterial, 'bumpMap') and source != 'blend_bump':
			bumpMapType = getProperty(vrMaterial, 'bumpMapType')
			if bumpMapType in (0, 1):
				if bumpMapType == 0:
					rpr_node = cmds.shadingNode("RPRBump", asUtility=True)
				elif bumpMapType == 1:
					rpr_node = cmds.shadingNode("RPRNormal", asUtility=True)
				copyProperty(rpr_node, vrMaterial, 'color', 'bumpMap')
				copyProperty(rpr_node, vrMaterial, 'strength', 'bumpMult')
				setProperty(rprMaterial, 'normalMapEnable', 1)
				connectProperty(rpr_node, 'out', rprMaterial, 'normalMap')

		end_log(vrMaterial)

	if source:
		rprMaterial += "." + source
	return rprMaterial


######################## 
##  VRayAlSurface
########################

def convertVRayAlSurface(vrMaterial, source):

	assigned = checkAssign(vrMaterial)
	
	if cmds.objExists(vrMaterial + "_rpr"):
		rprMaterial = vrMaterial + "_rpr"
	else:
		# Creating new Uber material
		rprMaterial = cmds.shadingNode("RPRUberMaterial", asShader=True)
		rprMaterial = cmds.rename(rprMaterial, vrMaterial + "_rpr")

		# Check shading engine in vrMaterial
		if assigned:
			sg = rprMaterial + "SG"
			cmds.sets(renderable=True, noSurfaceShader=True, empty=True, name=sg)
			connectProperty(rprMaterial, "outColor", sg, "surfaceShader")
		
		# Logging to file
		start_log(vrMaterial, rprMaterial)

		# Enable properties, which are default in VRay
		defaultEnable(rprMaterial, vrMaterial, "diffuse", "diffuseStrength", "diffuse")
		defaultEnable(rprMaterial, vrMaterial, "reflections", "reflect1Strength", "reflect1")

		# opacity
		opacity = getProperty(vrMaterial, "opacity")
		if opacity < 1:
			if mapDoesNotExist(vrMaterial, "opacity"):
				transparency = 1 - opacity
				setProperty(rprMaterial, "transparencyLevel", transparency)
			else:
				invertValue(rprMaterial, vrMaterial, "transparencyLevel", "opacity")
			setProperty(rprMaterial, "transparencyEnable", 1)

		# diffuse parameters
		copyProperty(rprMaterial, vrMaterial, "diffuseColor", "diffuse")
		copyProperty(rprMaterial, vrMaterial, "diffuseWeight", "diffuseStrength")

		if not mapDoesNotExist(vrMaterial, 'diffuseBumpMap'):
			diffuseBumpType = getProperty(vrMaterial, 'diffuseBumpType')
			if diffuseBumpType in (0, 1):
				if diffuseBumpType == 0:
					rpr_node = cmds.shadingNode("RPRBump", asUtility=True)
				elif diffuseBumpType == 1:
					rpr_node = cmds.shadingNode("RPRNormal", asUtility=True)
				copyProperty(rpr_node, vrMaterial, 'color', 'diffuseBumpMap')
				copyProperty(rpr_node, vrMaterial, 'strength', 'diffuseBumpAmount')
				setProperty(rprMaterial, 'useShaderNormal', 0)
				connectProperty(rpr_node, 'out', rprMaterial, 'diffuseNormal')

		# refl
		copyProperty(rprMaterial, vrMaterial, "reflectColor", "reflect1")
		copyProperty(rprMaterial, vrMaterial, "reflectWeight", "reflect1Strength")
		copyProperty(rprMaterial, vrMaterial, "reflectRoughness", "reflect1Roughness")
		copyProperty(rprMaterial, vrMaterial, "reflectIOR", "reflect1IOR")

		if not mapDoesNotExist(vrMaterial, 'reflect1BumpMap'):
			reflect1BumpType = getProperty(vrMaterial, 'reflect1BumpType')
			if reflect1BumpType in (0, 1):
				if reflect1BumpType == 0:
					rpr_node = cmds.shadingNode("RPRBump", asUtility=True)
				elif reflect1BumpType == 1:
					rpr_node = cmds.shadingNode("RPRNormal", asUtility=True)
				copyProperty(rpr_node, vrMaterial, 'color', 'reflect1BumpMap')
				copyProperty(rpr_node, vrMaterial, 'strength', 'reflect1BumpAmount')
				setProperty(rprMaterial, 'reflectUseShaderNormal', 0)
				connectProperty(rpr_node, 'out', rprMaterial, 'reflectNormal')

		# sss
		copyProperty(rprMaterial, vrMaterial, 'sssWeight', 'sssMix')
		if getProperty(vrMaterial, 'sssMix') > 0:
			setProperty(rprMaterial, 'sssEnable', 1)

		# sss1
		sss1_rad_x_den_scale = cmds.shadingNode("RPRArithmetic", asUtility=True)
		sss1_rad_x_den_scale = cmds.rename(sss1_rad_x_den_scale, 'sss1_rad_x_den_scale')
		setProperty(sss1_rad_x_den_scale, 'operation', 2)
		copyProperty(sss1_rad_x_den_scale, vrMaterial, 'inputA', 'sss1Radius')
		copyProperty(sss1_rad_x_den_scale, vrMaterial, 'inputB', 'sssDensityScale')

		sss1_rad_x_weight = cmds.shadingNode("RPRArithmetic", asUtility=True)
		sss1_rad_x_weight = cmds.rename(sss1_rad_x_weight, 'sss1_rad_x_weight')
		setProperty(sss1_rad_x_weight, 'operation', 2)
		connectProperty(sss1_rad_x_den_scale, 'out', sss1_rad_x_weight, 'inputA')
		copyProperty(sss1_rad_x_weight, vrMaterial, 'inputB', 'sss1Weight')

		sss1_rad_x_color = cmds.shadingNode("RPRArithmetic", asUtility=True)
		sss1_rad_x_color = cmds.rename(sss1_rad_x_color, 'sss1_rad_x_color')
		setProperty(sss1_rad_x_color, 'operation', 2)
		connectProperty(sss1_rad_x_weight, 'out', sss1_rad_x_color, 'inputA')
		copyProperty(sss1_rad_x_color, vrMaterial, 'inputB', 'sss1Color')

		# sss2

		sss2_rad_x_den_scale = cmds.shadingNode("RPRArithmetic", asUtility=True)
		sss2_rad_x_den_scale = cmds.rename(sss2_rad_x_den_scale, 'sss2_rad_x_den_scale')
		setProperty(sss2_rad_x_den_scale, 'operation', 2)
		copyProperty(sss2_rad_x_den_scale, vrMaterial, 'inputA', 'sss2Radius')
		copyProperty(sss2_rad_x_den_scale, vrMaterial, 'inputB', 'sssDensityScale')

		sss2_rad_x_weight = cmds.shadingNode("RPRArithmetic", asUtility=True)
		sss2_rad_x_weight = cmds.rename(sss2_rad_x_weight, 'sss2_rad_x_weight')
		setProperty(sss2_rad_x_weight, 'operation', 2)
		connectProperty(sss2_rad_x_den_scale, 'out', sss2_rad_x_weight, 'inputA')
		copyProperty(sss2_rad_x_weight, vrMaterial, 'inputB', 'sss2Weight')

		sss2_rad_x_color = cmds.shadingNode("RPRArithmetic", asUtility=True)
		sss2_rad_x_color = cmds.rename(sss2_rad_x_color, 'sss2_rad_x_color')
		setProperty(sss2_rad_x_color, 'operation', 2)
		connectProperty(sss2_rad_x_weight, 'out', sss2_rad_x_color, 'inputA')
		copyProperty(sss2_rad_x_color, vrMaterial, 'inputB', 'sss2Color')

		# sss3

		sss3_rad_x_den_scale = cmds.shadingNode("RPRArithmetic", asUtility=True)
		sss3_rad_x_den_scale = cmds.rename(sss3_rad_x_den_scale, 'sss3_rad_x_den_scale')
		setProperty(sss3_rad_x_den_scale, 'operation', 2)
		copyProperty(sss3_rad_x_den_scale, vrMaterial, 'inputA', 'sss3Radius')
		copyProperty(sss3_rad_x_den_scale, vrMaterial, 'inputB', 'sssDensityScale')

		sss3_rad_x_weight = cmds.shadingNode("RPRArithmetic", asUtility=True)
		sss3_rad_x_weight = cmds.rename(sss3_rad_x_weight, 'sss3_rad_x_weight')
		setProperty(sss3_rad_x_weight, 'operation', 2)
		connectProperty(sss3_rad_x_den_scale, 'out', sss2_rad_x_weight, 'inputA')
		copyProperty(sss3_rad_x_weight, vrMaterial, 'inputB', 'sss3Weight')

		sss3_rad_x_color = cmds.shadingNode("RPRArithmetic", asUtility=True)
		sss3_rad_x_color = cmds.rename(sss3_rad_x_color, 'sss3_rad_x_color')
		setProperty(sss3_rad_x_color, 'operation', 2)
		connectProperty(sss3_rad_x_weight, 'out', sss3_rad_x_color, 'inputA')
		copyProperty(sss3_rad_x_color, vrMaterial, 'inputB', 'sss3Color')

		# sss radius
		mix_sss1_sss2 = cmds.shadingNode("RPRArithmetic", asUtility=True)
		mix_sss1_sss2 = cmds.rename(mix_sss1_sss2, 'mix_sss1_color_and_sss2_color')
		setProperty(mix_sss1_sss2, 'operation', 20)
		connectProperty(sss1_rad_x_color, 'out', mix_sss1_sss2, 'inputA')
		connectProperty(sss2_rad_x_color, 'out', mix_sss1_sss2, 'inputB')

		mix_sss3 = cmds.shadingNode("RPRArithmetic", asUtility=True)
		mix_sss3 = cmds.rename(mix_sss3, 'mix_sss3_color')
		setProperty(mix_sss3, 'operation', 20)
		connectProperty(mix_sss1_sss2, 'out', mix_sss3, 'inputA')
		connectProperty(sss3_rad_x_color, 'out', mix_sss3, 'inputB')
		connectProperty(mix_sss3, 'out', rprMaterial, 'subsurfaceRadius0')

		# get radius weight
		sss_radius_dict = {'sss1Radius': getProperty(vrMaterial, 'sss1Radius'), 'sss2Radius': getProperty(vrMaterial, 'sss2Radius'), 'sss3Radius': getProperty(vrMaterial, 'sss3Radius')}
		highest_radius = max(sss_radius_dict, key=sss_radius_dict.get)
		sss_radius_dict.pop(highest_radius)
		middle_radius = max(sss_radius_dict, key=sss_radius_dict.get)
		lowest_radius = min(sss_radius_dict, key=sss_radius_dict.get)
		lowest_radius_weight = 1 / (getProperty(vrMaterial, lowest_radius) * 100 / getProperty(vrMaterial, highest_radius))
		middle_radius_weight = 1 / (getProperty(vrMaterial, middle_radius) * 100 / getProperty(vrMaterial, highest_radius))

		# volume and backscattering color
		low_and_mid_with_low_weight = cmds.shadingNode("RPRBlendValue", asUtility=True)
		low_and_mid_with_low_weight = cmds.rename(low_and_mid_with_low_weight, 'low_and_mid_with_low_weight')
		connectProperty(sss1_rad_x_color, 'out', low_and_mid_with_low_weight, 'inputA')
		connectProperty(sss3_rad_x_color, 'out', low_and_mid_with_low_weight, 'inputB')
		setProperty(low_and_mid_with_low_weight, 'weight', lowest_radius_weight)

		high_and_mid_with_low_weight = cmds.shadingNode("RPRBlendValue", asUtility=True)
		high_and_mid_with_low_weight = cmds.rename(high_and_mid_with_low_weight, 'high_and_mid_with_low_weight')
		connectProperty(low_and_mid_with_low_weight, 'out', high_and_mid_with_low_weight, 'inputA')
		connectProperty(sss2_rad_x_color, 'out', high_and_mid_with_low_weight, 'inputB')
		setProperty(high_and_mid_with_low_weight, 'weight', middle_radius_weight)

		setProperty(rprMaterial, 'separateBackscatterColor', 1)
		connectProperty(high_and_mid_with_low_weight, 'out', rprMaterial, 'backscatteringColor')
		connectProperty(high_and_mid_with_low_weight, 'out', rprMaterial, 'volumeScatter')

		# bump
		if not mapDoesNotExist(vrMaterial, 'bumpMap'):
			bumpType = getProperty(vrMaterial, 'bumpType')
			if bumpType in (0, 1):
				if bumpType == 0:
					rpr_node = cmds.shadingNode("RPRBump", asUtility=True)
				elif bumpType == 1:
					rpr_node = cmds.shadingNode("RPRNormal", asUtility=True)
				copyProperty(rpr_node, vrMaterial, 'color', 'bumpMap')
				copyProperty(rpr_node, vrMaterial, 'strength', 'bumpAmount')
				setProperty(rprMaterial, 'normalMapEnable', 1)
				connectProperty(rpr_node, 'out', rprMaterial, 'normalMap')



		end_log(vrMaterial)

	if source:
		rprMaterial += "." + source
	return rprMaterial


######################## 
##  VRayCarPaintMtl
########################

def convertVRayCarPaintMtl(vrMaterial, source):

	assigned = checkAssign(vrMaterial)
	
	if cmds.objExists(vrMaterial + "_rpr"):
		rprMaterial = vrMaterial + "_rpr"
	else:
		# Creating new Uber material
		rprMaterial = cmds.shadingNode("RPRUberMaterial", asShader=True)
		rprMaterial = cmds.rename(rprMaterial, vrMaterial + "_rpr")

		# Check shading engine in vrMaterial
		if assigned:
			sg = rprMaterial + "SG"
			cmds.sets(renderable=True, noSurfaceShader=True, empty=True, name=sg)
			connectProperty(rprMaterial, "outColor", sg, "surfaceShader")
		
		# Logging to file
		start_log(vrMaterial, rprMaterial)

		defaultEnable(rprMaterial, vrMaterial, "clearCoat", "coat_strength", "coat_color")		

		# diffuse parameters
		copyProperty(rprMaterial, vrMaterial, "diffuseColor", "color")

		# refl 
		setProperty(rprMaterial, 'reflections', 1)
		copyProperty(rprMaterial, vrMaterial, "reflectColor", "color")
		setProperty(rprMaterial, 'reflectMetalMaterial', 1)
		copyProperty(rprMaterial, vrMaterial, 'reflectMetalness', 'base_reflection')
		invertValue(rprMaterial, vrMaterial, 'reflectRoughness', 'base_glossiness')

		if not mapDoesNotExist(vrMaterial, 'base_bumpMap'):
			base_bumpMapType = getProperty(vrMaterial, 'base_bumpMapType')
			if base_bumpMapType in (0, 1):
				if base_bumpMapType == 0:
					rpr_node = cmds.shadingNode("RPRBump", asUtility=True)
				elif base_bumpMapType == 1:
					rpr_node = cmds.shadingNode("RPRNormal", asUtility=True)
				copyProperty(rpr_node, vrMaterial, 'color', 'base_bumpMap')
				copyProperty(rpr_node, vrMaterial, 'strength', 'base_bumpMult')
				setProperty(rprMaterial, 'normalMapEnable', 1)
				connectProperty(rpr_node, 'out', rprMaterial, 'normalMap')

		if not getProperty(vrMaterial, 'base_trace_reflections'):
			setProperty(rprMaterial, 'reflectMetalness', 0)

		# coat
		if getProperty(vrMaterial, 'coat_trace_reflections'):

			copyProperty(rprMaterial, vrMaterial, 'coatWeight', 'coat_strength')
			copyProperty(rprMaterial, vrMaterial, 'coatColor', 'coat_color') 
			invertValue(rprMaterial, vrMaterial, 'coatRoughness', 'coat_glossiness')

			if not mapDoesNotExist(vrMaterial, 'coat_bumpMap'):
				coat_bumpMapType = getProperty(vrMaterial, 'coat_bumpMapType')
				if coat_bumpMapType in (0, 1):
					if coat_bumpMapType == 0:
						rpr_node = cmds.shadingNode("RPRBump", asUtility=True)
					elif coat_bumpMapType == 1:
						rpr_node = cmds.shadingNode("RPRNormal", asUtility=True)
					copyProperty(rpr_node, vrMaterial, 'color', 'coat_bumpMap')
					copyProperty(rpr_node, vrMaterial, 'strength', 'coat_bumpMult')
					setProperty(rprMaterial, 'coatUseShaderNormal', 0)
					connectProperty(rpr_node, 'out', rprMaterial, 'normalMap')

		else:
			setProperty(rprMaterial, 'clearCoat', 0)

		if getProperty(vrMaterial, 'coat_trace_reflections') and getProperty(vrMaterial, 'coat_strength') > 0:
			blend_material = cmds.shadingNode("RPRBlendMaterial", asShader=True)
			blend_sg = blend_material + "SG"
			cmds.sets(renderable=True, noSurfaceShader=True, empty=True, name=blend_sg)
			connectProperty(blend_material, "outColor", blend_sg, "surfaceShader")

			metal_uber = cmds.shadingNode("RPRUberMaterial", asShader=True)
			setProperty(metal_uber, 'reflections', 1)
			setProperty(metal_uber, 'reflectRoughness', 0)
			setProperty(metal_uber, 'reflectMetalMaterial', 1)
			setProperty(metal_uber, 'reflectMetalness', 1)

			connectProperty(rprMaterial, 'outColor', blend_material, 'color0')
			connectProperty(metal_uber, 'outColor', blend_material, 'color1')
			copyProperty(blend_material, vrMaterial, 'weight', 'coat_strength')

			rprMaterial = blend_material


		end_log(vrMaterial)

	if source:
		rprMaterial += "." + source
	return rprMaterial


######################## 
##  VRayLightMtl
########################

def convertVRayLightMtl(vrMaterial, source):

	assigned = checkAssign(vrMaterial)
	
	if cmds.objExists(vrMaterial + "_rpr"):
		rprMaterial = vrMaterial + "_rpr"
	else:
		# Creating new Uber material
		rprMaterial = cmds.shadingNode("RPRUberMaterial", asShader=True)
		rprMaterial = cmds.rename(rprMaterial, vrMaterial + "_rpr")

		# Check shading engine in vrMaterial
		if assigned:
			sg = rprMaterial + "SG"
			cmds.sets(renderable=True, noSurfaceShader=True, empty=True, name=sg)
			connectProperty(rprMaterial, "outColor", sg, "surfaceShader")
		
		# Logging to file
		start_log(vrMaterial, rprMaterial)

		setProperty(rprMaterial, 'diffuse', 0)

		setProperty(rprMaterial, 'emissive', 1)
		colorMode = getProperty(vrMaterial, 'colorMode')
		if colorMode:
			setProperty(rprMaterial, 'emissiveColor', convertTemperature(getProperty(vrMaterial, 'temperature')))
		else:
			copyProperty(rprMaterial, vrMaterial, 'emissiveColor', 'color')
		copyProperty(rprMaterial, vrMaterial, 'emissiveIntensity', 'colorMultiplier')

		if getProperty(vrMaterial, 'emitOnBackSide'):
			setProperty(rprMaterial, 'emissiveDoubleSided', 1)

		# opacity
		opacity_color = getProperty(vrMaterial, "opacity")
		if opacity_color[0] < 1 or opacity_color[1] < 1 or opacity_color[2] < 1:
			if mapDoesNotExist(vrMaterial, "opacity"):
				transparency = 1 - max(opacity_color)
				setProperty(rprMaterial, "transparencyLevel", transparency)
			else:
				invertValue(rprMaterial, vrMaterial, "transparencyLevel", "opacity")
			setProperty(rprMaterial, "transparencyEnable", 1)

		if getProperty(vrMaterial, 'multiplyColorByOpacity'):
			mult_color_by_opacity = cmds.shadingNode("RPRArithmetic", asUtility=True)
			mult_color_by_opacity = cmds.rename(mult_color_by_opacity, 'mult_color_by_opacity')
			setProperty(mult_color_by_opacity, 'operation', 2)
			copyProperty(mult_color_by_opacity, rprMaterial, 'inputA', 'emissiveColor')
			copyProperty(mult_color_by_opacity, rprMaterial, 'inputB', 'transparencyLevel')
			connectProperty(mult_color_by_opacity, 'out', rprMaterial, 'emissiveColor')

		end_log(vrMaterial)

	if source:
		rprMaterial += "." + source
	return rprMaterial


######################## 
##  VRayToonMtl
########################

def convertVRayToonMtl(vrMaterial, source):

	assigned = checkAssign(vrMaterial)
	
	if cmds.objExists(vrMaterial + "_rpr"):
		rprMaterial = vrMaterial + "_rpr"
	else:
		# Creating new Uber material
		rprMaterial = cmds.shadingNode("RPRUberMaterial", asShader=True)
		rprMaterial = cmds.rename(rprMaterial, vrMaterial + "_rpr")

		# Check shading engine in vrMaterial
		if assigned:
			sg = rprMaterial + "SG"
			cmds.sets(renderable=True, noSurfaceShader=True, empty=True, name=sg)
			connectProperty(rprMaterial, "outColor", sg, "surfaceShader")
		
		# Logging to file
		start_log(vrMaterial, rprMaterial)

		defaultEnable(rprMaterial, vrMaterial, "diffuse", "diffuseColorAmount", "diffuseColor")		
		defaultEnable(rprMaterial, vrMaterial, "reflections", "reflectionColorAmount", "reflectionColor")		

		copyProperty(rprMaterial, vrMaterial, "diffuseColor", "diffuseColor")
		copyProperty(rprMaterial, vrMaterial, "diffuseWeight", "diffuseColorAmount")

		copyProperty(rprMaterial, vrMaterial, "reflectColor", "reflectionColor")
		copyProperty(rprMaterial, vrMaterial, "reflectWeight", "reflectionColorAmount")
		invertValue(rprMaterial, vrMaterial, "reflectRoughness", "reflectionGlossiness")

		copyProperty(rprMaterial, vrMaterial, 'refractColor', 'fogColor')
		refractColor = getProperty(vrMaterial, 'refractionColor')
		rpr_refr_weight = getProperty(vrMaterial, 'refractionColorAmount') * (0.3 * refractColor[0] + 0.59 * refractColor[1] + 0.11 * refractColor[2])
		setProperty(rprMaterial, 'refractWeight', rpr_refr_weight)
		invertValue(rprMaterial, vrMaterial, 'refractRoughness', 'refractionGlossiness')
		copyProperty(rprMaterial, vrMaterial, 'refractIor', 'refractionIOR')

		if getProperty(vrMaterial, 'refractionIOR') == 1:
			setProperty(rprMaterial, 'refractThinSurface', 1)

		if getProperty(vrMaterial, 'illumColor') != (0, 0, 0) or not mapDoesNotExist(vrMaterial, 'illumColor'):
			setProperty(rprMaterial, 'emissive', 1)
			copyProperty(rprMaterial, vrMaterial, "emissiveColor", "illumColor")

		if getProperty(vrMaterial, 'opacityMap') != (1, 1, 1):
			if mapDoesNotExist(vrMaterial, "opacityMap"):
				transparency = 1 - max(getProperty(vrMaterial, "opacityMap"))
				setProperty(rprMaterial, "transparencyLevel", transparency)
			else:
				invertValue(rprMaterial, vrMaterial, "transparencyLevel", "opacityMap")
			setProperty(rprMaterial, "transparencyEnable", 1)

		if not mapDoesNotExist(vrMaterial, 'bumpMap'):
			base_bumpMapType = getProperty(vrMaterial, 'bumpMapType')
			if base_bumpMapType in (0, 1):
				if base_bumpMapType == 0:
					rpr_node = cmds.shadingNode("RPRBump", asUtility=True)
				elif base_bumpMapType == 1:
					rpr_node = cmds.shadingNode("RPRNormal", asUtility=True)
				copyProperty(rpr_node, vrMaterial, 'color', 'bumpMap')
				copyProperty(rpr_node, vrMaterial, 'strength', 'bumpMult')
				setProperty(rprMaterial, 'normalMapEnable', 1)
				connectProperty(rpr_node, 'out', rprMaterial, 'normalMap')

		end_log(vrMaterial)

	if source:
		rprMaterial += "." + source
	return rprMaterial


######################## 
##  VRayMtlWrapper
########################

def convertVRayMtlWrapper(vrMaterial, source):

	assigned = checkAssign(vrMaterial)
	
	if cmds.objExists(vrMaterial + "_rpr"):
		rprMaterial = vrMaterial + "_rpr"
	else:
		# Creating new Uber material
		baseMaterial = cmds.listConnections(vrMaterial + ".baseMaterial")
		if baseMaterial:
			rprMaterial = convertMaterial(baseMaterial[0], "")
			rprMaterial = cmds.rename(rprMaterial, vrMaterial + "_rpr")
		else:
			rprMaterial = cmds.shadingNode("RPRUberMaterial", asShader=True)
			rprMaterial = cmds.rename(rprMaterial, vrMaterial + "_rpr")

		# Check shading engine in vrMaterial
		if assigned:
			sg = rprMaterial + "SG"
			cmds.sets(renderable=True, noSurfaceShader=True, empty=True, name=sg)
			connectProperty(rprMaterial, "outColor", sg, "surfaceShader")
		
		if cmds.objectType(rprMaterial) == "RPRUberMaterial":
			if getProperty(vrMaterial, "receiveCaustics"):
				setProperty(rprMaterial, "refractAllowCaustics", 1)
			if getProperty(vrMaterial, "reflectionAmount") > 0:
				setProperty(rprMaterial, "reflections", 1)
				copyProperty(rprMaterial, vrMaterial, "reflectWeight", "reflectionAmount")
			if getProperty(vrMaterial, "refractionAmount") > 0:
				setProperty(rprMaterial, "refraction", 1)
				copyProperty(rprMaterial, vrMaterial, "refractWeight", "reflectionAmount")

	if source:
		rprMaterial += "." + source
	return rprMaterial


######################## 
##  VRayHairNextMtl
########################

def convertVRayHairNextMtl(vrMaterial, source):

	assigned = checkAssign(vrMaterial)
	
	if cmds.objExists(vrMaterial + "_rpr"):
		rprMaterial = vrMaterial + "_rpr"
	else:
		# Creating new Uber material
		rprMaterial = cmds.shadingNode("RPRUberMaterial", asShader=True)
		rprMaterial = cmds.rename(rprMaterial, vrMaterial + "_rpr")

		# Check shading engine in vrMaterial
		if assigned:
			sg = rprMaterial + "SG"
			cmds.sets(renderable=True, noSurfaceShader=True, empty=True, name=sg)
			connectProperty(rprMaterial, "outColor", sg, "surfaceShader")
		
		# Logging to file
		start_log(vrMaterial, rprMaterial)

		setProperty(rprMaterial, 'reflections', 1)
		setProperty(rprMaterial, 'clearCoat', 1)

		setProperty(rprMaterial, 'reflectAnisotropy', 1)
		setProperty(rprMaterial, 'reflectAnisotropyRotation', 0.35)
		setProperty(rprMaterial, 'reflectIOR', 10)

		# transparency
		transparency = getProperty(vrMaterial, "transparency")
		if transparency < 1:
			if mapDoesNotExist(vrMaterial, "transparency"):
				transparency = 1 - transparency
				setProperty(rprMaterial, "transparencyLevel", transparency)
			else:
				invertValue(rprMaterial, vrMaterial, "transparencyLevel", "transparency")
			setProperty(rprMaterial, "transparencyEnable", 1)

		copyProperty(rprMaterial, vrMaterial, 'diffuseColor', 'diffuse_color')
		copyProperty(rprMaterial, vrMaterial, 'diffuseWeight', 'diffuse_amount')
		invertValue(rprMaterial, vrMaterial, 'coatRoughness', 'primary_glossiness_boost')
		invertValue(rprMaterial, vrMaterial, 'reflectRoughness', 'glossiness')

		arith1 = cmds.shadingNode("RPRArithmetic", asUtility=True)
		setProperty(arith1, 'operation', 2)
		copyProperty(arith1, vrMaterial, 'inputA', 'dye_color')
		invertValue(arith1, vrMaterial, 'inputB', 'melanin')

		arith2 = cmds.shadingNode('RPRArithmetic', asUtility=True)
		setProperty(arith2, 'operation', 2)
		connectProperty(arith1, 'out', arith2, 'inputA')
		setProperty(arith2, 'inputB', (0.3, 0.3, 0.3))

		arith3 = cmds.shadingNode('RPRArithmetic', asUtility=True)
		setProperty(arith3, 'operation', 2)
		connectProperty(arith2, 'out', arith3, 'inputA')
		copyProperty(arith3, vrMaterial, 'inputB', 'secondary_tint')
		connectProperty(arith3, 'out', rprMaterial, 'reflectColor')

		end_log(vrMaterial)

	if source:
		rprMaterial += "." + source
	return rprMaterial


######################## 
##  VRayFastSSS2
########################

def convertVRayFastSSS2(vrMaterial, source):

	assigned = checkAssign(vrMaterial)
	
	if cmds.objExists(vrMaterial + "_rpr"):
		rprMaterial = vrMaterial + "_rpr"
	else:
		# Creating new Uber material
		rprMaterial = cmds.shadingNode("RPRUberMaterial", asShader=True)
		rprMaterial = cmds.rename(rprMaterial, vrMaterial + "_rpr")

		# Check shading engine in vrMaterial
		if assigned:
			sg = rprMaterial + "SG"
			cmds.sets(renderable=True, noSurfaceShader=True, empty=True, name=sg)
			connectProperty(rprMaterial, "outColor", sg, "surfaceShader")
		
		# Logging to file
		start_log(vrMaterial, rprMaterial)

		setProperty(rprMaterial, 'reflections', True)
		copyProperty(rprMaterial, vrMaterial, 'reflectIOR', 'ior')
		copyProperty(rprMaterial, vrMaterial, 'reflectColor', 'reflection')
		copyProperty(rprMaterial, vrMaterial, 'reflectWeight', 'reflectionAmount')
		invertValue(rprMaterial, vrMaterial, 'reflectRoughness', 'glossiness')

		diffuse_color = cmds.shadingNode("RPRArithmetic", asUtility=True)
		diffuse_color = cmds.rename(diffuse_color, "diffuse_color_mult_overall")
		setProperty(diffuse_color, 'operation', 2)
		copyProperty(diffuse_color, vrMaterial, 'inputA', 'diffuseTex')
		copyProperty(diffuse_color, vrMaterial, 'inputB', 'overallTex')
		connectProperty(diffuse_color, 'out', rprMaterial, 'diffuseColor')

		copyProperty(rprMaterial, vrMaterial, 'diffuseWeight', 'diffuseAmount')
		setProperty(rprMaterial, 'sssEnable', True)
		setProperty(rprMaterial, 'separateBackscatterColor', True)
		if getProperty(vrMaterial, "scale") > 0:
			setProperty(rprMaterial, "backscatteringWeight", 1)

		if getProperty(vrMaterial, 'colorMode'):
			backscattering_color_mult = cmds.shadingNode("RPRArithmetic", asUtility=True)
			backscattering_color_mult = cmds.rename(backscattering_color_mult, "backscattering_color_mult_overall")
			setProperty(backscattering_color_mult, 'operation', 2)
			copyProperty(backscattering_color_mult, vrMaterial, 'inputA', 'subsurfaceColor')
			copyProperty(backscattering_color_mult, vrMaterial, 'inputB', 'overallTex')
			connectProperty(backscattering_color_mult, 'out', rprMaterial, 'volumeScatter')

		else:
			backscattering_color_mult = cmds.shadingNode("RPRArithmetic", asUtility=True)
			backscattering_color_mult = cmds.rename(backscattering_color_mult, "backscattering_color_mult_overall")
			setProperty(backscattering_color_mult, 'operation', 2)
			copyProperty(backscattering_color_mult, vrMaterial, 'inputA', 'subsurfaceColor')
			copyProperty(backscattering_color_mult, vrMaterial, 'inputB', 'overallTex')

			backscattering_color_mult_1_33 = cmds.shadingNode("RPRArithmetic", asUtility=True)
			backscattering_color_mult_1_33 = cmds.rename(backscattering_color_mult_1_33, "backscattering_color_mult_1_33")
			setProperty(backscattering_color_mult_1_33, 'operation', 2)
			connectProperty(backscattering_color_mult, "out", backscattering_color_mult_1_33, 'inputA')
			setProperty(backscattering_color_mult_1_33, 'inputB', (1.33, 1.33, 1.33))
			connectProperty(backscattering_color_mult_1_33, 'out', rprMaterial, 'backscatteringColor')

			setProperty(rprMaterial, "volumeScatter", (1, 1, 1))

		rpr_scatter_radius = getProperty(vrMaterial, 'scatterRadiusMult') / getProperty(vrMaterial, 'scale')
		setProperty(rprMaterial, 'subsurfaceRadius', (rpr_scatter_radius, rpr_scatter_radius, rpr_scatter_radius))

		# bump
		if not mapDoesNotExist(vrMaterial, 'bumpMap'):
			bumpType = getProperty(vrMaterial, 'bumpMapType')
			if bumpType in (0, 1):
				if bumpType == 0:
					rpr_node = cmds.shadingNode("RPRBump", asUtility=True)
				elif bumpType == 1:
					rpr_node = cmds.shadingNode("RPRNormal", asUtility=True)
				copyProperty(rpr_node, vrMaterial, 'color', 'bumpMap')
				copyProperty(rpr_node, vrMaterial, 'strength', 'bumpMult')
				setProperty(rprMaterial, 'normalMapEnable', 1)
				connectProperty(rpr_node, 'out', rprMaterial, 'normalMap')

		end_log(vrMaterial)

	if source:
		rprMaterial += "." + source
	return rprMaterial


######################## 
##  VRayMtlHair3
########################

def convertVRayMtlHair3(vrMaterial, source):

	assigned = checkAssign(vrMaterial)
	
	if cmds.objExists(vrMaterial + "_rpr"):
		rprMaterial = vrMaterial + "_rpr"
	else:
		# Creating new Uber material
		rprMaterial = cmds.shadingNode("RPRUberMaterial", asShader=True)
		rprMaterial = cmds.rename(rprMaterial, vrMaterial + "_rpr")

		# Check shading engine in vrMaterial
		if assigned:
			sg = rprMaterial + "SG"
			cmds.sets(renderable=True, noSurfaceShader=True, empty=True, name=sg)
			connectProperty(rprMaterial, "outColor", sg, "surfaceShader")
		
		# Logging to file
		start_log(vrMaterial, rprMaterial)

		setProperty(rprMaterial, 'reflections', True)
		setProperty(rprMaterial, 'reflectAnisotropy', 1)
		setProperty(rprMaterial, 'reflectAnisotropyRotation', 0.5)
		setProperty(rprMaterial, 'reflectIOR', 10)
		setProperty(rprMaterial, 'clearCoat', 1)
		setProperty(rprMaterial, 'coatWeight', 0.5)

		if getProperty(vrMaterial, 'transparency') > 0:
			setProperty(rprMaterial, 'transparencyEnable', 1)
			setProperty(rprMaterial, 'transparencyLevel', max(getProperty(vrMaterial, 'transparency')))

		avgPrimarySpecular = sum(getProperty(vrMaterial, 'primarySpecular')) / 3
		setProperty(rprMaterial, 'reflectRoughness', 1 - ((avgPrimarySpecular + getProperty(vrMaterial, 'primaryGlossiness')) / 2))

		transmissionColor = getProperty(vrMaterial, 'transmission')
		if (transmissionColor[0] > 0 or transmissionColor[1] > 0 or transmissionColor[2] > 0 or not mapDoesNotExist(vrMaterial, 'transmission')) \
			and getProperty(vrMaterial, 'transmissionAmount') > 0:

			overall_mult_diffuse = cmds.shadingNode("RPRArithmetic", asUtility=True)
			setProperty(overall_mult_diffuse, 'operation', 2)
			copyProperty(overall_mult_diffuse, vrMaterial, 'inputA', 'overallColor')
			copyProperty(overall_mult_diffuse, vrMaterial, 'inputB', 'diffuseColor')

			diffuseColor = cmds.shadingNode("RPRArithmetic", asUtility=True)
			setProperty(diffuseColor, 'operation', 0)
			copyProperty(diffuseColor, vrMaterial, 'inputA', 'transmission')
			connectProperty(overall_mult_diffuse, 'out', diffuseColor, 'inputB')

			connectProperty(diffuseColor, 'out', rprMaterial, 'diffuseColor')

			setProperty(rprMaterial, 'sssEnable', 1)
			copyProperty(rprMaterial, vrMaterial, 'volumeScatter', 'transmission')

		else:
			diffuseColor = cmds.shadingNode("RPRArithmetic", asUtility=True)
			setProperty(diffuseColor, 'operation', 2)
			copyProperty(diffuseColor, vrMaterial, 'inputA', 'overallColor')
			copyProperty(diffuseColor, vrMaterial, 'inputB', 'diffuseColor')
			onnectProperty(diffuseColor, 'out', rprMaterial, 'diffuseColor')


		end_log(vrMaterial)

	if source:
		rprMaterial += "." + source
	return rprMaterial



######################## 
##  VRayBlendMtl
########################


def convertVRayBlendMtl(vrMaterial, source):
	assigned = checkAssign(vrMaterial)
	
	if cmds.objExists(vrMaterial + "_rpr"):
		rprMaterial = vrMaterial + "_rpr"
	else:
		# Creating new Uber material
		rprMaterial = cmds.shadingNode("RPRBlendMaterial", asShader=True)
		
		# Logging to file
		start_log(vrMaterial, rprMaterial)

		baseMtl = cmds.listConnections(vrMaterial + '.base_material')
		if baseMtl:
			connectProperty(convertMaterial(baseMtl[0], ''), 'outColor', rprMaterial, 'color0')
		else:
			baseMtl = cmds.shadingNode("RPRUberMaterial", asShader=True)
			setProperty(baseMtl, 'transparencyEnable', 1)
			setProperty(baseMtl, 'transparencyLevel', 0)
			connectProperty(baseMtl, 'outColor', rprMaterial, 'color0')

		# materials count
		materials_count = 0
		for i in range(0, 9):
			if cmds.listConnections(vrMaterial + '.coat_material_{}'.format(i)):
				materials_count += 1

		first_material = True
		for i in range(0, 9):
			coatMaterial = cmds.listConnections(vrMaterial + '.coat_material_{}'.format(i))
			if coatMaterial:
				if materials_count > 1:
					if first_material:
						connectProperty(convertMaterial(coatMaterial[0], ''), 'outColor', rprMaterial, 'color1')
						copyProperty(rprMaterial, vrMaterial, 'weight', 'blend_amount_{}'.format(i))	
						first_material = False
					else:
						prev_rprMaterial = rprMaterial
						rprMaterial = cmds.shadingNode("RPRBlendMaterial", asShader=True)
						connectProperty(prev_rprMaterial, 'outColor', rprMaterial, 'color0')
						connectProperty(convertMaterial(coatMaterial[0], ''), 'outColor', rprMaterial, 'color1')
						copyProperty(rprMaterial, vrMaterial, 'weight', 'blend_amount_{}'.format(i))	
				else:
					connectProperty(convertMaterial(coatMaterial[0], ''), 'outColor', rprMaterial, 'color1')
					copyProperty(rprMaterial, vrMaterial, 'weight', 'blend_amount_{}'.format(i))	

		# rename and create SG for last blend material
		rprMaterial = cmds.rename(rprMaterial, vrMaterial + "_rpr")

		# Check shading engine in vrMaterial
		if assigned:
			sg = rprMaterial + "SG"
			cmds.sets(renderable=True, noSurfaceShader=True, empty=True, name=sg)
			connectProperty(rprMaterial, "outColor", sg, "surfaceShader")

		end_log(vrMaterial)

	if source:
		rprMaterial += "." + source
	return rprMaterial


######################## 
##  VRaySwitchMtl
########################


def convertVRaySwitchMtl(vrMaterial, source):
	assigned = checkAssign(vrMaterial)
	
	if cmds.objExists(vrMaterial + "_rpr"):
		rprMaterial = vrMaterial + "_rpr"
	else:
		# Creating new Uber material
		rprMaterial = cmds.shadingNode("RPRBlendMaterial", asShader=True)
		
		# Logging to file
		start_log(vrMaterial, rprMaterial)

		materialsSwitch = getProperty(vrMaterial, "materialsSwitch")
		if materialsSwitch < 0.499:
			active_material = 0
		elif materialsSwitch >= 0.5 and materialsSwitch < 1.499:
			active_material = 1
		elif materialsSwitch >= 1.5 and materialsSwitch < 2.499:
			active_material = 2
		elif materialsSwitch >= 2.5 and materialsSwitch < 3.499:
			active_material = 3
		elif materialsSwitch >= 3.5 and materialsSwitch < 4.499:
			active_material = 4
		elif materialsSwitch >= 4.5 and materialsSwitch < 5.499:
			active_material = 5
		elif materialsSwitch >= 5.5 and materialsSwitch < 6.499:
			active_material = 6
		elif materialsSwitch >= 6.5 and materialsSwitch < 7.499:
			active_material = 7
		elif materialsSwitch >= 7.5 and materialsSwitch < 8.499:
			active_material = 8
		elif materialsSwitch >= 8.5:
			active_material = 9

		# materials count
		materials_count = 0
		for i in range(0, 9):
			if cmds.listConnections(vrMaterial + '.material_{}'.format(i)):
				materials_count += 1

		first_material = True
		second_material = True
		for i in range(0, 9):
			material = cmds.listConnections(vrMaterial + '.material_{}'.format(i))
			if material:
				if materials_count > 1:
					if first_material:
						connectProperty(convertMaterial(material[0], ''), 'outColor', rprMaterial, 'color0')
						if i == active_material:
							setProperty(rprMaterial, "weight", 0)
						first_material = False
					elif second_material:
						connectProperty(convertMaterial(material[0], ''), 'outColor', rprMaterial, 'color1')
						if i == active_material:
							setProperty(rprMaterial, "weight", 1)
						second_material = False
					else:
						prev_rprMaterial = rprMaterial
						rprMaterial = cmds.shadingNode("RPRBlendMaterial", asShader=True)
						connectProperty(prev_rprMaterial, 'outColor', rprMaterial, 'color0')
						connectProperty(convertMaterial(material[0], ''), 'outColor', rprMaterial, 'color1')
						if i == active_material:
							setProperty(rprMaterial, "weight", 1)
				else:
					connectProperty(convertMaterial(material[0], ''), 'outColor', rprMaterial, 'color1')
					setProperty(rprMaterial, "weight", 0)	

		# rename and create SG for last blend material
		rprMaterial = cmds.rename(rprMaterial, vrMaterial + "_rpr")

		# Check shading engine in vrMaterial
		if assigned:
			sg = rprMaterial + "SG"
			cmds.sets(renderable=True, noSurfaceShader=True, empty=True, name=sg)
			connectProperty(rprMaterial, "outColor", sg, "surfaceShader")

		end_log(vrMaterial)

	if source:
		rprMaterial += "." + source
	return rprMaterial


######################## 
##  VRayBumpMtl
########################

def convertVRayBumpMtl(vrMaterial, source):

	baseMtl = cmds.listConnections(vrMaterial + '.base_material')[0]

	if cmds.objExists(baseMtl + "_rpr"):
		rprMaterial = baseMtl + "_rpr"
	else:
		# script will return material.blend_bump, we need only material name
		rprMaterial = convertMaterial(baseMtl, "blend_bump").split('.')[0]

		start_log(vrMaterial, rprMaterial)

		sg = rprMaterial + "SG"
		cmds.sets(renderable=True, noSurfaceShader=True, empty=True, name=sg)
		connectProperty(rprMaterial, "outColor", sg, "surfaceShader")

		materialType = cmds.objectType(rprMaterial)
		if materialType == 'RPRUberMaterial':
			if not mapDoesNotExist(vrMaterial, 'bumpMap'):
				base_mtl_map_type = getProperty(baseMtl, 'bumpMapType')
				bump_map_type = getProperty(vrMaterial, 'bumpMapType')
				if mapDoesNotExist(baseMtl, 'bumpMap') or base_mtl_map_type != bump_map_type:
					setProperty(rprMaterial,'normalMapEnable', 1)
					copyProperty(rprMaterial, vrMaterial, 'normalMap', 'bumpMap')
				else:
					if base_mtl_map_type in (0, 1):
						if base_mtl_map_type == 0:
							map_node = cmds.shadingNode("RPRBump", asUtility=True)
						elif base_mtl_map_type == 1:
							map_node = cmds.shadingNode("RPRNormal", asUtility=True)

						bumps_blend = cmds.shadingNode("RPRBlendValue", asUtility=True)
						bumps_blend = cmds.rename(bumps_blend, "Blend bumps")
						copyProperty(bumps_blend, baseMtl, 'inputA', 'bumpMap')
						copyProperty(bumps_blend, vrMaterial, 'inputB', 'bumpMap')
						setProperty(bumps_blend, "weight", 0.2)
						connectProperty(bumps_blend, 'out', map_node, 'color')

						setProperty(rprMaterial,'normalMapEnable', 1)
						connectProperty(map_node, 'out', rprMaterial, 'normalMap')

		end_log(vrMaterial)

	if source:
		rprMaterial += "." + source
	return rprMaterial


def convertVRayLightDomeShape(dome_light):

	if cmds.objExists("RPRIBL"):
		iblShape = "RPRIBLShape"
		iblTransform = "RPRIBL"
	else:
		# create IBL node
		iblShape = cmds.createNode("RPRIBL", n="RPRIBLShape")
		iblTransform = cmds.listRelatives(iblShape, p=True)[0]
		setProperty(iblTransform, "scale", (1001.25, 1001.25, 1001.25))

	# Logging to file 
	start_log(dome_light, iblShape)
  
	copyProperty(iblShape, dome_light, 'intensity', 'intensityMult')

	# Copy properties from vr dome light
	domeTransform = cmds.listRelatives(dome_light, p=True)[0]
	setProperty(iblTransform, "rotateY", getProperty(domeTransform, "rotateY") - 90)
	
	file = cmds.listConnections(dome_light + ".domeTex")
	if file:
		setProperty(iblTransform, "filePath", getProperty(file[0], "fileTextureName"))

		try:
			vrayPlaceEnvTex = cmds.listConnections(file, type="VRayPlaceEnvTex")
			if vrayPlaceEnvTex:
				if getProperty(vrayPlaceEnvTex[0], "useTransform"):
					transform = getProperty(vrayPlaceEnvTex[0], "transform")
					setProperty(iblTransform, "translate", (transform[0], transform[1], transform[2]))
					setProperty(iblTransform, "rotate", (transform[4], transform[5] - 90, transform[6]))
				else:
					setProperty(iblTransform, "rotateY", getProperty(vrayPlaceEnvTex[0], "horRotation") - 90)
					copyProperty(iblTransform, vrayPlaceEnvTex[0], "rotateX", "verRotation")
					copyProperty(iblTransform, vrayPlaceEnvTex[0], "rotateZ", "verRotation")
		except Exception as ex:
			traceback.print_exc()
			print("[ERROR] Failed to convert VRayPlaceEnvTex.")


	invisible = getProperty(dome_light, 'invisible')
	if invisible:
		setProperty(iblShape, 'display', 0)
	else:
		setProperty(iblShape, 'display', 1)
		   
	# Logging to file
	end_log(dome_light) 


def convertVRayLightIESShape(vr_light): 

	# Vray light transform
	splited_name = vr_light.split("|")
	vrTransform = "|".join(splited_name[0:-1])
	group = "|".join(splited_name[0:-2])

	if cmds.objExists(vrTransform + "_rpr"):
		rprTransform = vrTransform + "_rpr"
		rprLightShape = cmds.listRelatives(rprTransform)[0]
	else: 
		rprLightShape = cmds.createNode("RPRIES", n="RPRIESLight")
		rprLightShape = cmds.rename(rprLightShape, splited_name[-1] + "_rpr")
		rprTransform = cmds.listRelatives(rprLightShape, p=True)[0]
		rprTransform = cmds.rename(rprTransform, splited_name[-2] + "_rpr")
		rprLightShape = cmds.listRelatives(rprTransform)[0]

		if group:
			cmds.parent(rprTransform, group)

		rprTransform = group + "|" + rprTransform
		rprLightShape = rprTransform + "|" + rprLightShape

	# Logging to file 
	start_log(vr_light, rprLightShape)

	# Copy properties from vrLight
	vrayIntensity = getProperty(vr_light, 'intensityMult')
	setProperty(rprLightShape, 'intensity', 19.9024909 * vrayIntensity ** 3 - 14.3247047 * vrayIntensity ** 2 + 3.4341693 * vrayIntensity + 0.0762945)

	colorMode = getProperty(vr_light, 'colorMode')
	if colorMode:
		setProperty(rprLightShape, 'color', convertTemperature(getProperty(vr_light, 'temperature')))
	else:
		copyProperty(rprLightShape, vr_light, "color", "lightColor")
	
	setProperty(rprLightShape, "iesFile", getProperty(vr_light, "iesFile"))
	
	copyProperty(rprTransform, vrTransform, "translate", "translate")
	setProperty(rprTransform, 'rotateX', getProperty(vrTransform, 'rotateX') - 90)
	copyProperty(rprTransform, vrTransform, "rotateY", "rotateY")
	copyProperty(rprTransform, vrTransform, "rotateZ", "rotateZ")
	copyProperty(rprTransform, vrTransform, "scale", "scale")

	copyProperty(rprLightShape, vr_light, 'display', 'visibility')

	# Logging to file
	end_log(vr_light) 


def convertVRayLightRectShape(vr_light): 

	# Redshift light transform
	splited_name = vr_light.split("|")
	vrTransform = "|".join(splited_name[0:-1])
	group = "|".join(splited_name[0:-2])

	if cmds.objExists(vrTransform + "_rpr"):
		rprTransform = vrTransform + "_rpr"
		rprLightShape = cmds.listRelatives(rprTransform)[0]
	else: 
		rprLightShape = cmds.createNode("RPRPhysicalLight", n="RPRPhysicalLightShape")
		rprLightShape = cmds.rename(rprLightShape, splited_name[-1] + "_rpr")
		rprTransform = cmds.listRelatives(rprLightShape, p=True)[0]
		rprTransform = cmds.rename(rprTransform, splited_name[-2] + "_rpr")
		rprLightShape = cmds.listRelatives(rprTransform)[0]

		if group:
			cmds.parent(rprTransform, group)

		rprTransform = group + "|" + rprTransform
		rprLightShape = rprTransform + "|" + rprLightShape

	# Logging to file 
	start_log(vr_light, rprLightShape)

	# Copy properties from vrLight

	light_units = getProperty(vr_light, 'units')
	if light_units in (0, 1, 4):
		setProperty(rprLightShape, 'intensityUnits', 1)
		copyProperty(rprLightShape, vr_light, 'intensity', 'intensityMult')
	elif light_units == 2:
		setProperty(rprLightShape, 'intensityUnits', 1)
		setProperty(rprLightShape, 'intensity', getProperty(vr_light, 'intensityMult') / 1000)
	elif light_units == 3:
		setProperty(rprLightShape, 'intensityUnits', 2)
		copyProperty(rprLightShape, vr_light, 'intensity', 'intensityMult')

	if getProperty(vr_light, 'shapeType'):
		setProperty(rprLightShape, 'areaLightShape', 0)
		setProperty(rprTransform, 'scaleX', getProperty(vr_light, 'uSize') * getProperty(vrTransform, 'scaleX'))
		setProperty(rprTransform, 'scaleY', getProperty(vr_light, 'uSize') * getProperty(vrTransform, 'scaleY'))
	else:
		setProperty(rprLightShape, 'areaLightShape', 3)
		setProperty(rprTransform, 'scaleX', getProperty(vr_light, 'uSize') * getProperty(vrTransform, 'scaleX'))
		setProperty(rprTransform, 'scaleY', getProperty(vr_light, 'vSize') * getProperty(vrTransform, 'scaleY'))

	copyProperty(rprLightShape, vr_light, 'colorMode', 'colorMode')
	copyProperty(rprLightShape, vr_light, 'temperature', 'temperature')
	copyProperty(rprLightShape, vr_light, 'color', 'lightColor')

	if getProperty(vr_light, 'useRectTex'):
		rectTex_mult_rectTexA = cmds.shadingNode("RPRArithmetic", asUtility=True)
		setProperty(rectTex_mult_rectTexA, 'operation', 2)
		copyProperty(rectTex_mult_rectTexA, vr_light, 'inputA', 'rectTex')
		copyProperty(rectTex_mult_rectTexA, vr_light, 'inputB', 'rectTexA')
		if getProperty(vr_light, 'multiplyByTheLightColor'):
			multiplyByTheLightColor = cmds.shadingNode("RPRArithmetic", asUtility=True)
			setProperty(multiplyByTheLightColor, 'operation', 2)
			copyProperty(multiplyByTheLightColor, vr_light, 'inputA', 'lightColor')
			connectProperty(rectTex_mult_rectTexA, 'out', multiplyByTheLightColor, 'inputB')
			connectProperty(multiplyByTheLightColor, 'out', rprLightShape, 'color')
		else:
			connectProperty(rectTex_mult_rectTexA, 'out', rprLightShape, 'color')

	copyProperty(rprTransform, vrTransform, "translate", "translate")
	copyProperty(rprTransform, vrTransform, "rotate", "rotate")

	if getProperty(vr_light, 'invisible'):
		setProperty(rprLightShape, 'areaLightVisible', 0)
	else:
		setProperty(rprLightShape, 'areaLightVisible', 1)

	# Logging to file
	end_log(vr_light) 


def convertVRayLightSphereShape(vr_light): 

	# Redshift light transform
	splited_name = vr_light.split("|")
	vrTransform = "|".join(splited_name[0:-1])
	group = "|".join(splited_name[0:-2])

	if cmds.objExists(vrTransform + "_rpr"):
		rprTransform = vrTransform + "_rpr"
		rprLightShape = cmds.listRelatives(rprTransform)[0]
	else: 
		rprLightShape = cmds.createNode("RPRPhysicalLight", n="RPRPhysicalLightShape")
		rprLightShape = cmds.rename(rprLightShape, splited_name[-1] + "_rpr")
		rprTransform = cmds.listRelatives(rprLightShape, p=True)[0]
		rprTransform = cmds.rename(rprTransform, splited_name[-2] + "_rpr")
		rprLightShape = cmds.listRelatives(rprTransform)[0]

		if group:
			cmds.parent(rprTransform, group)

		rprTransform = group + "|" + rprTransform
		rprLightShape = rprTransform + "|" + rprLightShape

	# Logging to file 
	start_log(vr_light, rprLightShape)

	# Copy properties from vrLight

	light_units = getProperty(vr_light, 'units')
	if light_units in (0, 1, 4):
		setProperty(rprLightShape, 'intensityUnits', 1)
		copyProperty(rprLightShape, vr_light, 'intensity', 'intensityMult')
	elif light_units == 2:
		setProperty(rprLightShape, 'intensityUnits', 1)
		setProperty(rprLightShape, 'intensity', getProperty(vr_light, 'intensityMult') / 1000)
	elif light_units == 3:
		setProperty(rprLightShape, 'intensityUnits', 2)
		copyProperty(rprLightShape, vr_light, 'intensity', 'intensityMult')

	copyProperty(rprLightShape, vr_light, 'colorMode', 'colorMode')
	copyProperty(rprLightShape, vr_light, 'temperature', 'temperature')
	copyProperty(rprLightShape, vr_light, 'color', 'lightColor')

	if getProperty(vr_light, 'invisible'):
		setProperty(rprLightShape, 'areaLightVisible', 0)
	else:
		setProperty(rprLightShape, 'areaLightVisible', 1)

	setProperty(rprLightShape, 'areaLightShape', 4)
	sphere, polySphere = cmds.polySphere()

	try:
		cmds.select(clear=True)
		setProperty(rprLightShape, "areaLightSelectingMesh", 1)
		cmds.select(sphere)
	except Exception as ex:
		traceback.print_exc()
		print("[ERROR] Failed to attach mesh to rpr physical light")

	copyProperty(sphere, vrTransform, "translate", "translate")
	copyProperty(sphere, vrTransform, "rotate", "rotate")
	copyProperty(sphere, vrTransform, "scale", "scale")

	copyProperty(polySphere, vr_light, 'radius', 'radius')
	subdivision = int(math.modf(getProperty(vr_light, 'sphereSegments'))[1])
	setProperty(polySphere, 'subdivisionsAxis', subdivision)
	setProperty(polySphere, 'subdivisionsHeight', subdivision)
	
	# Logging to file
	end_log(vr_light) 



def convertVRayLightMeshLightLinking(vr_light): 

	# Redshift light transform
	splited_name = vr_light.split("|")
	vrTransform = "|".join(splited_name[0:-1])
	group = "|".join(splited_name[0:-2])

	if cmds.objExists(vrTransform + "_rpr"):
		rprTransform = vrTransform + "_rpr"
		rprLightShape = cmds.listRelatives(rprTransform)[0]
	else: 
		rprLightShape = cmds.createNode("RPRPhysicalLight", n="RPRPhysicalLightShape")
		rprLightShape = cmds.rename(rprLightShape, splited_name[-1] + "_rpr")
		rprTransform = cmds.listRelatives(rprLightShape, p=True)[0]
		rprTransform = cmds.rename(rprTransform, splited_name[-2] + "_rpr")
		rprLightShape = cmds.listRelatives(rprTransform)[0]

		if group:
			cmds.parent(rprTransform, group)

		rprTransform = group + "|" + rprTransform
		rprLightShape = rprTransform + "|" + rprLightShape

	# Logging to file 
	start_log(vr_light, rprLightShape)

	# Copy properties from vrLight
	lightlink_transform = cmds.listRelatives(vr_light, p=True)[0]
	vr_light = cmds.listConnections(lightlink_transform, type="VRayLightMesh")[0]

	light_units = getProperty(vr_light, 'units')
	if light_units in (0, 1, 4):
		setProperty(rprLightShape, 'intensityUnits', 1)
		copyProperty(rprLightShape, vr_light, 'intensity', 'intensityMult')
	elif light_units == 2:
		setProperty(rprLightShape, 'intensityUnits', 1)
		setProperty(rprLightShape, 'intensity', getProperty(vr_light, 'intensityMult') / 1000)
	elif light_units == 3:
		setProperty(rprLightShape, 'intensityUnits', 2)
		copyProperty(rprLightShape, vr_light, 'intensity', 'intensityMult')

	copyProperty(rprLightShape, vr_light, 'colorMode', 'colorMode')
	copyProperty(rprLightShape, vr_light, 'temperature', 'temperature')
	copyProperty(rprLightShape, vr_light, 'color', 'lightColor')

	if getProperty(vr_light, 'invisible'):
		setProperty(rprLightShape, 'areaLightVisible', 0)
	else:
		setProperty(rprLightShape, 'areaLightVisible', 1)

	setProperty(rprLightShape, 'areaLightShape', 4)
	mesh_obj = cmds.listConnections(vr_light, type='transform')

	try:
		cmds.select(clear=True)
		setProperty(rprLightShape, "areaLightSelectingMesh", 1)
		cmds.select(mesh_obj)
	except Exception as ex:
		traceback.print_exc()
		print("[ERROR] Failed to attach mesh to rpr physical light")

	if getProperty(vr_light, 'useTex'):
		tex_mult = cmds.shadingNode("RPRArithmetic", asUtility=True)
		setProperty(tex_mult, 'operation', 2)
		copyProperty(tex_mult, vr_light, 'inputA', 'tex')
		copyProperty(tex_mult, vr_light, 'inputB', 'texA')
		connectProperty(tex_mult, 'out', rprLightShape, 'color')
	
	# Logging to file
	end_log(vr_light) 


def convertVRaySky(vr_sky):

	rpr_sky = cmds.createNode("RPRSky", n="RPRSkyShape")
	
	# Check sun exists
	vr_sun = cmds.listConnections(vr_sky + ".sun")
	if vr_sun and cmds.objectType(vr_sun[0]) == "VRayGeoSun":
		# copy values from sky in override case
		if getProperty(vr_sky, "sunDirOnly"):
			vr_node = vr_sky
		else:
			vr_node = vr_sun[0]

		# convert altitude and azimuth
		translate = getProperty(vr_sun[0], "translate")

		temp_a = math.asin(translate[0] / math.sqrt(translate[2]**2 + translate[0]**2)) * (180 / math.pi)
		if translate[2] > 0:
			azimuth = 180 + temp_a
		elif translate[2] < 0:
			azimuth = -temp_a
		elif translate[2] == 0:
			azimuth = 0
		setProperty(rpr_sky, "azimuth", azimuth)
		
		temp_b = translate[1] / math.sqrt(translate[1]**2 + translate[2]**2 + translate[0]**2)
		altitude = math.asin(temp_b) * (180 / math.pi)
		setProperty(rpr_sky, "altitude", altitude)

	else:
		vr_node = vr_sky

	vr_int = getProperty(vr_node, "intensityMult")
	setProperty(rpr_sky, "intensity", 774.848266 * (vr_int**3) - 364.401165 * (vr_int**2) + 71.960137 * vr_int - 0.071448)

	copyProperty(rpr_sky, vr_node, "turbidity", "turbidity")
	copyProperty(rpr_sky, vr_node, "filterColor", "filterColor")
	copyProperty(rpr_sky, vr_node, "groundColor", "groundAlbedo")
	copyProperty(rpr_sky, vr_node, "horizonBlur", "blendAngle")
	copyProperty(rpr_sky, vr_node, "horizonHeight", "horizonOffset")


def convertVrayCamera(camera):

	vrayCameraPhysicalType = getProperty(camera, "vrayCameraPhysicalType")
	# still camera
	if vrayCameraPhysicalType == 0:
		setProperty("RadeonProRenderGlobals", "toneMappingType", 6)
		exposure = 1 / (120 * getProperty(camera, "vrayCameraPhysicalFNumber") ** 2 * getProperty(camera, "vrayCameraPhysicalShutterSpeed") / getProperty(camera, "vrayCameraPhysicalISO"))
		setProperty("RadeonProRenderGlobals", "toneMappingSimpleExposure", exposure)
	# movie camera
	elif vrayCameraPhysicalType == 1:
		setProperty("RadeonProRenderGlobals", "toneMappingType", 6)
		exposure = 3 * cmds.currentTime('1sec', edit=True) * getProperty(camera, "vrayCameraPhysicalISO") / getProperty(camera, "vrayCameraPhysicalFNumber") ** 2 * getProperty(camera, "vrayCameraPhysicalShutterAngle")
		setProperty("RadeonProRenderGlobals", "toneMappingSimpleExposure", exposure)



def convertTemperature(temperature):
	temperature = temperature / 100

	if temperature <= 66:
		colorR = 255
	else:
		colorR = temperature - 60
		colorR = 329.698727446 * colorR ** -0.1332047592
		if colorR < 0:
			colorR = 0
		if colorR > 255:
			colorR = 255


	if temperature <= 66:
		colorG = temperature
		colorG = 99.4708025861 * math.log(colorG) - 161.1195681661
		if colorG < 0:
			colorG = 0
		if colorG > 255:
			colorG = 255
	else:
		colorG = temperature - 60
		colorG = 288.1221695283 * colorG ** -0.0755148492
		if colorG < 0:
			colorG = 0
		if colorG > 255:
			colorG = 255


	if temperature >= 66:
		colorB = 255
	elif temperature <= 19:
		colorB = 0
	else:
		colorB = temperature - 10
		colorB = 138.5177312231 * math.log(colorB) - 305.0447927307
		if colorB < 0:
			colorB = 0
		if colorB > 255:
			colorB = 255

	colorR = colorR / 255
	colorG = colorG / 255
	colorB = colorB / 255

	return (colorR, colorG, colorB)



# Convert material. Returns new material name.
def convertMaterial(material, source):

	material_type = cmds.objectType(material)

	conversion_func = {

		# VRay materials
		"VRayMtl": convertVRayMtl,
		"VRayBumpMtl": convertVRayBumpMtl,
		"VRayCarPaintMtl": convertVRayCarPaintMtl,
		"VRayBlendMtl": convertVRayBlendMtl,
		"VRayLightMtl": convertVRayLightMtl,
		"VRayAlSurface": convertVRayAlSurface,
		"VRayHairNextMtl": convertVRayHairNextMtl,
		"VRayFastSSS2": convertVRayFastSSS2,
		"VRayMtlHair3": convertVRayMtlHair3,
		"VRayToonMtl": convertVRayToonMtl,
		"VRaySwitchMtl": convertVRaySwitchMtl,
		"VRayMtlWrapper": convertVRayMtlWrapper,

		"VRayFlakesMtl": convertUnsupportedMaterial,
		"VRayMeshMaterial": convertUnsupportedMaterial,
		"VRayMtl2Sided": convertUnsupportedMaterial,
		"VRayMtlGLSL": convertUnsupportedMaterial,
		"VRayMtlMDL": convertUnsupportedMaterial,
		"VRayMtlOSL": convertUnsupportedMaterial,
		"VRayMtlRenderStats": convertUnsupportedMaterial,
		"VRayPointParticleMtl": convertUnsupportedMaterial,
		"VRayScannedMtl": convertUnsupportedMaterial,
		"VRayStochasticFlakesMtl": convertUnsupportedMaterial,
		"VRayVRmatMtl": convertUnsupportedMaterial,

		# VRay Volumetric
		"VRayAerialPerspective": convertUnsupportedMaterial,
		"VRayEnvironmentFog": convertUnsupportedMaterial,
		"VRayScatterFog": convertUnsupportedMaterial,
		"VRaySimpleFog": convertUnsupportedMaterial,
		"VRaySphereFadeVolume": convertUnsupportedMaterial,

		# Standard utilities
		"clamp": convertUnsupportedNode,
		"colorCondition": convertUnsupportedNode,
		"colorComposite": convertColorComposite,
		"blendColors": convertBlendColors,
		"luminance": convertLuminance,
		"reverse": convertReverse,
		"premultiply": convertPreMultiply,
		"channels": convertChannels,
		"vectorProduct": convertVectorProduct,
		"multiplyDivide": convertmultiplyDivide,
		"bump2d": convertbump2d,

		# VRay utilities
		"VRayTemperature": convertVRayTemperature,
		"VRayFresnel": convertVRayFresnel,
		"VRayTriplanar": convertVRayTriplanar,
		"VRayVertexColors": convertVRayVertexColors,
		"VRayLayeredTex": convertVRayLayeredTex,
		"VRayDirt": convertVRayDirt,
		"VRayUserColor": convertVRayUserColor,
		"VRayUserScalar": convertVRayUserScalar,
		"VRayUserInteger": convertVRayUserInteger,
		"VRayMultiSubTex": convertVRayMultiSubTex,
		"VRayInverseExposure": convertVRayInverseExposure

	}

	if material_type in conversion_func:
		rpr = conversion_func[material_type](material, source)
	else:
		if isVRayType(material):
			rpr = convertUnsupportedNode(material, source)
		else:
			rpr = convertStandardNode(material, source)

	return rpr


# Convert light. Returns new light name.
def convertLight(light):

	light_type = cmds.objectType(light)

	conversion_func = {

		# VRay lights
		"VRayLightDomeShape": convertVRayLightDomeShape,
		"VRayLightRectShape": convertVRayLightRectShape,
		"VRayLightSphereShape": convertVRayLightSphereShape,
		"VRayLightMeshLightLinking": convertVRayLightMeshLightLinking,
		"VRayLightIESShape": convertVRayLightIESShape,

	}

	conversion_func[light_type](light)


def isVRayType(obj):

	if cmds.objExists(obj):
		if "VRay" in cmds.objectType(obj):
			return 1
	return 0


def cleanScene():

	listMaterials = cmds.ls(materials=True)
	for material in listMaterials:
		if isVRayType(material):
			shEng = cmds.listConnections(material, type="shadingEngine")
			try:
				if shEng:
					cmds.delete(shEng[0])
				cmds.delete(material)
			except:
				pass

	listLights = cmds.ls(l=True, type=["VRayLightRectShape", "VRayLightDomeShape", "VRaySunShape", "VRaySky", "VRaySunTarget", "VRayLightSphereShape", \
		"VRayLightMeshLightLinking", "VRayLightMesh", "VRayLightIESShape"])
	for light in listLights:
		transform = cmds.listRelatives(light, p=True)
		try:
			light_type = cmds.objectType(light) 
			cmds.delete(light)
			if light_type != "VRayLightMesh":
				cmds.delete(transform[0])
		except:
			pass

	listObjects = cmds.ls(l=True)
	for obj in listObjects:
		if isVRayType(obj):
			try:
				cmds.delete(obj)
			except:
				pass


def remap_value(value, maxInput, minInput, maxOutput, minOutput):

	value = maxInput if value > maxInput else value
	value = minInput if value < minInput else value

	inputDiff = maxInput - minInput
	outputDiff = maxOutput - minOutput

	remapped_value = minOutput + ((float(value - minInput) / float(inputDiff)) * outputDiff)

	return remapped_value


def clampValue(value, minValue, maxValue):
	return max(min(value, maxValue), minValue)


def checkAssign(material):

	if isVRayType(material):
		materialSG = cmds.listConnections(material, type="shadingEngine")
		if materialSG:
			cmds.hyperShade(objects=material)
			assigned = cmds.ls(sl=True)
			if assigned:
				return 1
	return 0


def defaultEnable(RPRmaterial, VRmaterial, enable, weight, color):

	if (getProperty(VRmaterial, color) != (0, 0, 0) or not mapDoesNotExist(VRmaterial, color)) and getProperty(VRmaterial, weight):
		setProperty(RPRmaterial, enable, 1)
	else:
		setProperty(RPRmaterial, enable, 0)


def repathScene():
	scene_workspace = cmds.workspace(q=True, dir=True)
	print('Your workspace located in {}'.format(scene_workspace))
	unresolved_files = cmds.filePathEditor(query=True, listFiles="", unresolved=True, attributeOnly=True)
	if unresolved_files:
		for item in unresolved_files:
			print("Repathing node {}".format(item, os.path.join(item, scene_workspace)))
			cmds.filePathEditor(item, repath=scene_workspace, recursive=True, ra=1)


def convertScene():

	# Disable caching
	maya_version = cmds.about(apiVersion=True)
	if maya_version > 20190200:
		from maya.plugin.evaluator.cache_preferences import CachePreferenceEnabled
		cache_preference_enabled = CachePreferenceEnabled().get_value()
		if cache_preference_enabled:
			CachePreferenceEnabled().set_value(False)

	# Repath paths in scene files (filePathEditor)
	repathScene()

	# Check plugins
	if not cmds.pluginInfo("vrayformaya", q=True, loaded=True):
		try:
			cmds.loadPlugin("vrayformaya", quiet=True)
		except Exception as ex:
			response = cmds.confirmDialog(title="Error",
							  message=("V-Ray plugin is not installed.\nInstall V-Ray plugin before conversion."),
							  button=["OK"],
							  defaultButton="OK",
							  cancelButton="OK",
							  dismissString="OK")
			exit("V-Ray plugin is not installed")

	if not cmds.pluginInfo("RadeonProRender", q=True, loaded=True):
		try:
			cmds.loadPlugin("RadeonProRender", quiet=True)
		except Exception as ex:
			response = cmds.confirmDialog(title="Error",
							  message=("RadeonProRender plugin is not installed.\nInstall RadeonProRender plugin before conversion."),
							  button=["OK"],
							  defaultButton="OK",
							  cancelButton="OK",
							  dismissString="OK")
			exit("RadeonProRender plugin is not installed")

	# Vray engine set before conversion
	setProperty("defaultRenderGlobals", "currentRenderer", "vray")

	# convert vray camera
	cameras = cmds.ls(type="camera")
	for cam in cameras:
		if "vrayCameraPhysicalOn" in cmds.listAttr(cam):
			if getProperty(cam, "vrayCameraPhysicalOn"):
				try:
					convertVrayCamera(cam)
				except Exception as ex:
					traceback.print_exc()
					print("[ERROR] Failed to {} camera".format(cam))


	# Vray Environment
	if getProperty("vraySettings", "cam_overrideEnvtex"):
		sky = cmds.ls(type="VRaySky")
		if sky:
			try:
				convertVRaySky(sky[0])
			except Exception as ex:
				traceback.print_exc()
				print("[ERROR] Failed to convert VRaySky")

	# Get all lights from scene
	listLights = cmds.ls(l=True, type=["VRayLightRectShape", "VRayLightDomeShape", "VRayLightMeshLightLinking", "VRayLightIESShape"])

	# Convert lights
	for light in listLights:
		try:
			convertLight(light)
		except Exception as ex:
			traceback.print_exc()
			print("[ERROR] Failed to convert {} light. \n".format(light))
		
	# Get all materials from scene
	listMaterials = cmds.ls(materials=True)
	materialsDict = {}
	for each in listMaterials:
		if checkAssign(each):
			materialsDict[each] = convertMaterial(each, "")

	for vr, rpr in materialsDict.items():
		try:
			cmds.hyperShade(objects=vr)
			rpr_sg = cmds.listConnections(rpr, type="shadingEngine")[0]
			cmds.sets(forceElement=rpr_sg)
		except Exception as ex:
			traceback.print_exc()
			print("[ERROR] Failed to convert {} material. \n".format(vr))
	
	# globals conversion
	try:
		setProperty("defaultRenderGlobals","currentRenderer", "FireRender")
		setProperty("defaultRenderGlobals", "imageFormat", 8)

		# TODO iterations conversion

		# TODO check this
		setProperty("RadeonProRenderGlobals", "giClampIrradiance", 1)
		setProperty("RadeonProRenderGlobals", "giClampIrradianceValue", 5)
		setProperty("RadeonProRenderGlobals", "raycastEpsilon", 0.001)
		if MAX_RAY_DEPTH:
			setProperty("RadeonProRenderGlobals", "maxRayDepth", MAX_RAY_DEPTH)
		
		aa_filter_conversion_map = {
			0: 1, # box
			1: 1, # area (not supported by rpr)
			2: 2, # triangle
			3: 5, # lanczos
			4: 1, # sinc (not supported by rpr)
			5: 1, # catmull rom (not supported by rpr)
			6: 3, # gaussian
			7: 1  # cook variable (not supported by rpr)
		}

		rpr_filter_value = aa_filter_conversion_map[getProperty("vraySettings", "aaFilterType")]
		setProperty("RadeonProRenderGlobals", "filter", rpr_filter_value)

		if getProperty("vraySettings", "progressiveMinSubdivs") < 16:
			setProperty("RadeonProRenderGlobals", "completionCriteriaMinIterations", 16)
		else:
			copyProperty("RadeonProRenderGlobals", "vraySettings", "completionCriteriaMinIterations", "progressiveMinSubdivs")
		copyProperty("RadeonProRenderGlobals", "vraySettings", "completionCriteriaIterations", "progressiveMaxSubdivs")
		copyProperty("RadeonProRenderGlobals", "vraySettings", "completionCriteriaMinutes", "progressiveMaxTime")
		copyProperty("RadeonProRenderGlobals", "vraySettings", "adaptiveThreshold", "progressiveThreshold")

	except:
		pass

	if maya_version > 20190200:
		if cache_preference_enabled:
			CachePreferenceEnabled().set_value(True)


def auto_launch():
	convertScene()
	cleanScene()

def manual_launch():
	print("Conversion start! Converter version: {}".format(VR2RPR_CONVERTER_VERSION))
	startTime = 0
	testTime = 0
	startTime = time.time()
	convertScene()
	testTime = time.time() - startTime
	print("Conversion was finished! Elapsed time: {}".format(round(testTime, 3)))

	response = cmds.confirmDialog(title="Completed",
							  message=("Scene conversion took {} seconds.\nWould you like to delete all VRay objects?".format(round(testTime, 3))),
							  button=["Yes", "No"],
							  defaultButton="Yes",
							  cancelButton="No",
							  dismissString="No")

	if response == "Yes":
		cleanScene()


def onMayaDroppedPythonFile(empty):
	manual_launch()

if __name__ == "__main__":
	manual_launch()



