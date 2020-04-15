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


# Arnold to RadeonProRender Converter

import maya.mel as mel
import maya.cmds as cmds
import time
import os
import math
import traceback


# log functions

def write_converted_property_log(rpr_name, ai_name, rpr_attr, ai_attr):

	try:
		file_path = cmds.file(q=True, sceneName=True) + ".log"
		with open(file_path, 'a') as f:
			f.write(u"    property {}.{} is converted to {}.{}   \r\n".format(ai_name, ai_attr, rpr_name, rpr_attr).encode('utf-8'))
	except:
		pass

def write_own_property_log(text):

	try:
		file_path = cmds.file(q=True, sceneName=True) + ".log"
		with open(file_path, 'a') as f:
			f.write("    {}   \r\n".format(text))
	except:
		pass

def start_log(ai, rpr):

	try:
		text  = u"Found node: \r\n    name: {} \r\n".format(ai).encode('utf-8')
		text += "type: {} \r\n".format(cmds.objectType(ai))
		text += u"Converting to: \r\n    name: {} \r\n".format(rpr).encode('utf-8')
		text += "type: {} \r\n".format(cmds.objectType(rpr))
		text += "Conversion details: \r\n"

		file_path = cmds.file(q=True, sceneName=True) + ".log"
		with open(file_path, 'a') as f:
			f.write(text)
	except:
		pass


def end_log(ai):

	try:
		text  = u"Conversion of {} is finished.\n\n \r\n".format(ai).encode('utf-8')

		file_path = cmds.file(q=True, sceneName=True) + ".log"
		with open(file_path, 'a') as f:
			f.write(text)
	except:
		pass

# additional fucntions

# copy from 3rd party engine to rpr
def copyProperty(rpr_name, conv_name, rpr_attr, conv_attr):

	# full name of attribute
	conv_field = conv_name + "." + conv_attr
	rpr_field = rpr_name + "." + rpr_attr
	ai_type = type(getProperty(conv_name, conv_attr))
	rpr_type = type(getProperty(rpr_name, rpr_attr))

	try:
		listConnections = cmds.listConnections(conv_field)
		# connection convert
		if listConnections and cmds.objectType(listConnections[0]) != "transform":
			obj, channel = cmds.connectionInfo(conv_field, sourceFromDestination=True).split('.')
			source_name, source_attr = convertMaterial(obj, channel).split('.')
			connectProperty(source_name, source_attr, rpr_name, rpr_attr)
		# complex color conversion for each channel (RGB/XYZ/HSV)
		elif not listConnections and rpr_type == ai_type == tuple:

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
			if ai_type == rpr_type or ai_type == unicode:
				setProperty(rpr_name, rpr_attr, getProperty(conv_name, conv_attr))
			elif ai_type == tuple and rpr_type == float:
				if cmds.objExists(conv_field + "R"):
					conv_attr += "R"
				elif cmds.objExists(conv_field + "X"):
					conv_attr += "X"
				elif cmds.objExists(conv_field + "H"):
					conv_attr += "H"
				setProperty(rpr_name, rpr_attr, getProperty(conv_name, conv_attr))
			elif ai_type == float and rpr_type == tuple:
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


# dispalcement convertion
def convertDisplacement(ai_sg, rpr_name):
	try:
		displacement = cmds.listConnections(ai_sg + ".displacementShader")
		if displacement:
			displacementType = cmds.objectType(displacement[0])

			if displacementType == "displacementShader":
				displacement_file = cmds.listConnections(displacement[0], type="file")
				if displacement_file:
					setProperty(rpr_name, "displacementEnable", 1)
					connectProperty(displacement_file[0], "outColor", rpr_name, "displacementMap")
					copyProperty(rpr_name, displacement[0], "scale", "displacementMax")
					copyProperty(rpr_name, displacement[0], "displacementMin", "aiDisplacementZeroValue")

			elif displacementType == "file":
				setProperty(rpr_name, "displacementEnable", 1)
				connectProperty(displacement[0], "outColor", rpr_name, "displacementMap")

				meshs = cmds.listConnections(ai_sg, type="mesh")
				if meshs:
					shapes = cmds.listRelatives(meshs[0], type="mesh")
					copyProperty(rpr_name, shapes[0], "displacementSubdiv", "aiSubdivIterations")
					displacementMax = getProperty(shapes[0], 'aiDispHeight')
					displacementMin = getProperty(shapes[0], 'aiDispZeroValue')
					if displacementMin > displacementMax:
						copyProperty(rpr_name, shapes[0], "displacementMax", "aiDispHeight")
						copyProperty(rpr_name, shapes[0], "displacementMin", "aiDispHeight")
					else:
						copyProperty(rpr_name, shapes[0], "displacementMax", "aiDispHeight")
						copyProperty(rpr_name, shapes[0], "displacementMin", "aiDispZeroValue")

	except Exception as ex:
		traceback.print_exc()
		print(u"Failed to convert displacement for {} material".format(rpr_name).encode('utf-8'))


# dispalcement convertion
def convertShadowDisplacement(ai_sg, rpr_name):
	try:
		displacement = cmds.listConnections(ai_sg + ".displacementShader")
		if displacement:
			displacementType = cmds.objectType(displacement[0])

			if displacementType == "displacementShader":
				displacement_file = cmds.listConnections(displacement[0], type="file")
				if displacement_file:
					setProperty(rpr_name, "useDispMap", 1)
					connectProperty(displacement_file[0], "outColor", rpr_name, "dispMap")

			elif displacementType == "file":
				setProperty(rpr_name, "useDispMap", 1)
				connectProperty(displacement[0], "outColor", rpr_name, "dispMap")

			elif displacementType == "aiVectorMap":
				displacement_file = cmds.listConnections(displacement[0], type="file")
				if displacement_file:
					setProperty(rpr_name, "useDispMap", 1)
					connectProperty(displacement_file[0], "outColor", rpr_name, "dispMap")
	except Exception as ex:
		traceback.print_exc()
		print(u"Failed to convert displacement for {} material".format(rpr_name).encode('utf-8'))


def convertaiAdd(ai, source):

	if cmds.objExists(ai + "_rpr"):
		rpr = ai + "_rpr"
	else:
		rpr = cmds.shadingNode("RPRArithmetic", asUtility=True)
		rpr = cmds.rename(rpr, ai + "_rpr")

		# Logging to file
		start_log(ai, rpr)

		# Fields conversion
		setProperty(rpr, "operation", 0)
		copyProperty(rpr, ai, "inputA", "input1")
		copyProperty(rpr, ai, "inputB", "input2")
		
		# Logging to file
		end_log(ai)

	conversion_map = {
		"outColor": "out",
		"outColorR": "outX",
		"outColorG": "outY",
		"outColorB": "outZ"
	}

	rpr += "." + conversion_map[source]
	return rpr


def convertaiDivide(ai, source):

	if cmds.objExists(ai + "_rpr"):
		rpr = ai + "_rpr"
	else:
		rpr = cmds.shadingNode("RPRArithmetic", asUtility=True)
		rpr = cmds.rename(rpr, ai + "_rpr")

		# Logging to file
		start_log(ai, rpr)

		# Fields conversion
		setProperty(rpr, "operation", 3)
		copyProperty(rpr, ai, "inputA", "input1")
		copyProperty(rpr, ai, "inputB", "input2")
		
		# Logging to file
		end_log(ai)

	conversion_map = {
		"outColor": "out",
		"outColorR": "outX",
		"outColorG": "outY",
		"outColorB": "outZ"
	}

	rpr += "." + conversion_map[source]
	return rpr


def convertaiSubstract(ai, source):

	if cmds.objExists(ai + "_rpr"):
		rpr = ai + "_rpr"
	else:
		rpr = cmds.shadingNode("RPRArithmetic", asUtility=True)
		rpr = cmds.rename(rpr, ai + "_rpr")

		# Logging to file
		start_log(ai, rpr)

		# Fields conversion
		setProperty(rpr, "operation", 1)
		copyProperty(rpr, ai, "inputA", "input1")
		copyProperty(rpr, ai, "inputB", "input2")
		
		# Logging to file
		end_log(ai)

	conversion_map = {
		"outColor": "out",
		"outColorR": "outX",
		"outColorG": "outY",
		"outColorB": "outZ",
		"outTransparency": "out",
		"outTransparencyR": "outX",
		"outTransparencyG": "outY",
		"outTransparencyB": "outZ",
	}

	rpr += "." + conversion_map[source]
	return rpr


def convertaiMultiply(ai, source):

	if cmds.objExists(ai + "_rpr"):
		rpr = ai + "_rpr"
	else:
		rpr = cmds.shadingNode("RPRArithmetic", asUtility=True)
		rpr = cmds.rename(rpr, ai + "_rpr")

		# Logging to file
		start_log(ai, rpr)

		# Fields conversion
		setProperty(rpr, "operation", 2)
		copyProperty(rpr, ai, "inputA", "input1")
		copyProperty(rpr, ai, "inputB", "input2")
		
		# Logging to file
		end_log(ai)

	conversion_map = {
		"outColor": "out",
		"outColorR": "outX",
		"outColorG": "outY",
		"outColorB": "outZ"
	}

	rpr += "." + conversion_map[source]
	return rpr


def convertaiAbs(ai, source):

	if cmds.objExists(ai + "_rpr"):
		rpr = ai + "_rpr"
	else:
		rpr = cmds.shadingNode("RPRArithmetic", asUtility=True)
		rpr = cmds.rename(rpr, ai + "_rpr")

		# Logging to file
		start_log(ai, rpr)

		# Fields conversion
		setProperty(rpr, "operation", 20)
		copyProperty(rpr, ai, "inputA", "input")
		
		# Logging to file
		end_log(ai)

	conversion_map = {
		"outColor": "out",
		"outColorR": "outX",
		"outColorG": "outY",
		"outColorB": "outZ"
	}

	rpr += "." + conversion_map[source]
	return rpr


def convertaiAtan(ai, source):

	if cmds.objExists(ai + "_rpr"):
		rpr = ai + "_rpr"
	else:
		rpr = cmds.shadingNode("RPRArithmetic", asUtility=True)
		rpr = cmds.rename(rpr, ai + "_rpr")

		# Logging to file
		start_log(ai, rpr)

		# Fields conversion
		setProperty(rpr, "operation", 18)
		copyProperty(rpr, ai, "inputA", "x")
		copyProperty(rpr, ai, "inputB", "y")
		
		# Logging to file
		end_log(ai)

	conversion_map = {
		"outColor": "out",
		"outColorR": "outX",
		"outColorG": "outY",
		"outColorB": "outZ"
	}

	rpr += "." + conversion_map[source]
	return rpr


def convertaiCross(ai, source):

	if cmds.objExists(ai + "_rpr"):
		rpr = ai + "_rpr"
	else:
		rpr = cmds.shadingNode("RPRArithmetic", asUtility=True)
		rpr = cmds.rename(rpr, ai + "_rpr")

		# Logging to file
		start_log(ai, rpr)

		# Fields conversion
		setProperty(rpr, "operation", 12)
		copyProperty(rpr, ai, "inputA", "input1")
		copyProperty(rpr, ai, "inputB", "input2")
		
		# Logging to file
		end_log(ai)

	conversion_map = {
		"outValue": "out",
		"outValueX": "outX",
		"outValueY": "outY",
		"outValueZ": "outZ"
	}

	rpr += "." + conversion_map[source]
	return rpr


def convertaiDot(ai, source):

	if cmds.objExists(ai + "_rpr"):
		rpr = ai + "_rpr"
	else:
		rpr = cmds.shadingNode("RPRArithmetic", asUtility=True)
		rpr = cmds.rename(rpr, ai + "_rpr")

		# Logging to file
		start_log(ai, rpr)

		# Fields conversion
		setProperty(rpr, "operation", 11)
		copyProperty(rpr, ai, "inputA", "input1")
		copyProperty(rpr, ai, "inputB", "input2")
		
		# Logging to file
		end_log(ai)

	conversion_map = {
		"outValue": "outX"
	}

	rpr += "." + conversion_map[source]
	return rpr


def convertaiPow(ai, source):

	if cmds.objExists(ai + "_rpr"):
		rpr = ai + "_rpr"
	else:
		rpr = cmds.shadingNode("RPRArithmetic", asUtility=True)
		rpr = cmds.rename(rpr, ai + "_rpr")

		# Logging to file
		start_log(ai, rpr)

		# Fields conversion
		setProperty(rpr, "operation", 15)
		copyProperty(rpr, ai, "inputA", "base")
		copyProperty(rpr, ai, "inputB", "exponent")
		
		# Logging to file
		end_log(ai)

	conversion_map = {
		"outColor": "out",
		"outColorR": "outX",
		"outColorG": "outY",
		"outColorB": "outZ"
	}

	rpr += "." + conversion_map[source]
	return rpr


def convertaiTrigo(ai, source):

	if cmds.objExists(ai + "_rpr"):
		rpr = ai + "_rpr"
	else:
		rpr = cmds.shadingNode("RPRArithmetic", asUtility=True)
		rpr = cmds.rename(rpr, ai + "_rpr")

		# Logging to file
		start_log(ai, rpr)

		# Fields conversion
		function = getProperty(ai, "function")
		operation_map = {
			0: 5,
			1: 4,
			2: 6
	 	}
		setProperty(rpr, "operation", operation_map[function])
		copyProperty(rpr, ai, "inputA", "input")
		
		# Logging to file
		end_log(ai)

	conversion_map = {
		"outColor": "out",
		"outColorR": "outX",
		"outColorG": "outY",
		"outColorB": "outZ"
	}

	rpr += "." + conversion_map[source]
	return rpr


def convertmultiplyDivide(ai, source):

	if cmds.objExists(ai + "_rpr"):
		rpr = ai + "_rpr"
	else:
		rpr = cmds.shadingNode("RPRArithmetic", asUtility=True)
		rpr = cmds.rename(rpr, ai + "_rpr")

		# Logging to file
		start_log(ai, rpr)

		# Fields conversion
		operation = getProperty(ai, "operation")
		operation_map = {
			1: 2,
			2: 3,
			3: 15
	 	}
		setProperty(rpr, "operation", operation_map[operation])
		copyProperty(rpr, ai, "inputA", "input1")
		copyProperty(rpr, ai, "inputB", "input2")
		
		# Logging to file
		end_log(ai)

	conversion_map = {
		"output": "out",
		"outputX": "outX",
		"outputY": "outY",
		"outputZ": "outZ"
	}

	rpr += "." + conversion_map[source]
	return rpr


def convertaiComposite(ai, source):

	if cmds.objExists(ai + "_rpr"):
		rpr = ai + "_rpr"
	else:
		rpr = cmds.shadingNode("RPRArithmetic", asUtility=True)
		rpr = cmds.rename(rpr, ai + "_rpr")

		# Logging to file
		start_log(ai, rpr)

		# Fields conversion
		operation = getProperty(ai, "operation")
		operation_map = {
			7: 3, # divide
			19: 2, # mulitply
			23: 0, # plus 
			18: 1, # minus

	 	}
		setProperty(rpr, "operation", operation_map[operation])
		copyProperty(rpr, ai, "inputA", "A")
		copyProperty(rpr, ai, "inputB", "B")
		
		# Logging to file
		end_log(ai)

	conversion_map = {
		"outColor": "out",
		"outColorR": "outX",
		"outColorG": "outY",
		"outColorB": "outZ",
		"outAlpha": "outX"
	}

	rpr += "." + conversion_map[source]
	return rpr


def convertaiExp(ai, source):

	if cmds.objExists(ai + "_rpr"):
		rpr = ai + "_rpr"
	else:
		rpr = cmds.shadingNode("RPRArithmetic", asUtility=True)
		rpr = cmds.rename(rpr, ai + "_rpr")

		# Logging to file
		start_log(ai, rpr)

		# Fields conversion
		setProperty(rpr, "operation", 15)

		arithmetic1 = cmds.shadingNode("RPRArithmetic", asUtility=True)
		setProperty(arithmetic1, "operation", 18)
		copyProperty(arithmetic1, ai, "inputA", "input")
		
		arithmetic2 = cmds.shadingNode("RPRArithmetic", asUtility=True)
		setProperty(arithmetic2, "operation", 18)
		copyProperty(arithmetic2, ai, "inputA", "input")

		connectProperty(arithmetic1, "out", rpr, "inputA")
		connectProperty(arithmetic2, "out", rpr, "inputB")
		
		# Logging to file
		end_log(ai)

	conversion_map = {
		"outColor": "out",
		"outColorR": "outX",
		"outColorG": "outY",
		"outColorB": "outZ",
	}

	rpr += "." + conversion_map[source]
	return rpr


def convertaiSqrt(ai, source):

	if cmds.objExists(ai + "_rpr"):
		rpr = ai + "_rpr"
	else:
		rpr = cmds.shadingNode("RPRArithmetic", asUtility=True)
		rpr = cmds.rename(rpr, ai + "_rpr")

		# Logging to file
		start_log(ai, rpr)

		# Fields conversion
		setProperty(rpr, "operation", 0)
		sqrt_value = []
		for value in getProperty(ai, "input"):
			sqrt_value.append(math.sqrt(float(value)))
		setProperty(rpr, "inputA", tuple(sqrt_value))
		
		# Logging to file
		end_log(ai)

	conversion_map = {
		"outColor": "out",
		"outColorR": "outX",
		"outColorG": "outY",
		"outColorB": "outZ",
	}

	rpr += "." + conversion_map[source]
	return rpr


def convertaiNegate(ai, source):

	if cmds.objExists(ai + "_rpr"):
		rpr = ai + "_rpr"
	else:
		rpr = cmds.shadingNode("RPRArithmetic", asUtility=True)
		rpr = cmds.rename(rpr, ai + "_rpr")

		# Logging to file
		start_log(ai, rpr)

		# Fields conversion
		setProperty(rpr, "operation", 1)
		copyProperty(rpr, ai, "inputB", "input")
		
		# Logging to file
		end_log(ai)

	conversion_map = {
		"outColor": "out",
		"outColorR": "outX",
		"outColorG": "outY",
		"outColorB": "outZ",
	}

	rpr += "." + conversion_map[source]
	return rpr


def convertaiColorCorrect(ai, source):

	if cmds.objExists(ai + "_rpr"):
		rpr = ai + "_rpr"
	else:
		rpr = cmds.shadingNode("colorCorrect", asUtility=True)
		rpr = cmds.rename(rpr, ai + "_rpr")

		# Logging to file
		start_log(ai, rpr)

		# Fields conversion
		copyProperty(rpr, ai, "inColor", "input")
		copyProperty(rpr, ai, "inAlpha", "mask")
		copyProperty(rpr, ai, "colGammaX", "gamma")
		copyProperty(rpr, ai, "colGammaY", "gamma")
		copyProperty(rpr, ai, "colGammaZ", "gamma")
		copyProperty(rpr, ai, "hueShift", "hueShift")
		copyProperty(rpr, ai, "satGain", "saturation")
		copyProperty(rpr, ai, "colGain", "multiply")
		copyProperty(rpr, ai, "colOffset", "add")
		copyProperty(rpr, ai, "alphaGain", "alphaMultiply")
		copyProperty(rpr, ai, "alphaOffset", "alphaAdd")
		copyProperty(rpr, ai, "valGain", "contrast")
		
		# Logging to file
		end_log(ai)


	rpr += "." + source
	return rpr


def convertbump2d(ai, source):

	if cmds.objExists(ai + "_rpr"):
		rpr = ai + "_rpr"
	else:
		bump_type = getProperty(ai, "bumpInterp")
		if not bump_type:
			rpr = cmds.shadingNode("RPRBump", asUtility=True)
			rpr = cmds.rename(rpr, ai + "_rpr")
		else:
			rpr = cmds.shadingNode("RPRNormal", asUtility=True)
			rpr = cmds.rename(rpr, ai + "_rpr")

		# Logging to file
		start_log(ai, rpr)

		# Fields conversion
		copyProperty(rpr, ai, "color", "bumpValue")

		if not bump_type:
			if getProperty(ai, "bumpDepth") * 100 > 1:
				setProperty(rpr, "strength", 1)
			else:
				setProperty(rpr, "strength", getProperty(ai, "bumpDepth") * 100)
		else:
			if getProperty(ai, "bumpDepth") > 1:
				setProperty(rpr, "strength", 1)
			else:
				setProperty(rpr, "strength", getProperty(ai, "bumpDepth") * 100)

		# max value = 1
		if getProperty(rpr, "strength") > 1:
			setProperty(rpr, "strength", 1)

		# Logging to file
		end_log(ai)

	conversion_map = {
		"outNormal": "out",
		"outNormalX": "outR",
		"outNormalY": "outG",
		"outNormalZ": "outB"
	}

	rpr += "." + conversion_map[source]
	return rpr


def convertaiBump2d(ai, source):

	if cmds.objExists(ai + "_rpr"):
		rpr = ai + "_rpr"
	else:
		if not mapDoesNotExist(ai, "bumpMap") and not mapDoesNotExist(ai, "normal"):
			rpr = cmds.shadingNode("RPRBlendMaterial", asShader=True)
			rpr = cmds.rename(rpr, ai + "_rpr")

			# Logging to file
			start_log(ai, rpr)

			bump_uber = cmds.shadingNode("RPRUberMaterial", asShader=True)
			copyProperty(bump_uber, ai, "normalMap", "bumpMap")

			normal_uber = cmds.shadingNode("RPRUberMaterial", asShader=True)
			copyProperty(normal_uber, ai, "normalMap", "normal")

			connectProperty(bump_uber, "outColor", rpr, "color0")
			connectProperty(normal_uber, "outColor", rpr, "color1")

			# Logging to file
			end_log(ai)

		else:
			rpr = cmds.shadingNode("RPRBump", asUtility=True)
			rpr = cmds.rename(rpr, ai + "_rpr")
				
			# Logging to file
			start_log(ai, rpr)

			# Fields conversion
			copyProperty(rpr, ai, "color", "bumpMap")

			if getProperty(ai, "bumpHeight") * 100 > 1:
				setProperty(rpr, "strength", 1)
			else:
				setProperty(rpr, "strength", getProperty(ai, "bumpHeight") * 100)

			# Logging to file
			end_log(ai)

			
	map_type = cmds.objectType(rpr)
	if map_type == "RPRBump":
		conversion_map = {
			"outValue": "out",
			"outValueX": "outR",
			"outValueY": "outG",
			"outValueZ": "outB"
		}
	elif map_type == "RPRNormal":
		conversion_map = {
			"outValue": "outColor",
			"outValueX": "outColorR",
			"outValueY": "outColorG",
			"outValueZ": "outColorB"
		}

	rpr += "." + conversion_map[source]
	return rpr


def convertaiBump3d(ai, source):

	if cmds.objExists(ai + "_rpr"):
		rpr = ai + "_rpr"
	else:
		rpr = cmds.shadingNode("RPRBump", asUtility=True)
		rpr = cmds.rename(rpr, ai + "_rpr")
			
		# Logging to file
		start_log(ai, rpr)

		# Fields conversion
		copyProperty(rpr, ai, "color", "bumpMap")
		# only file support (alpha and color connections)

		if getProperty(ai, "bumpHeight") * 100 > 1:
			setProperty(rpr, "strength", 1)
		else:
			setProperty(rpr, "strength", getProperty(ai, "bumpHeight") * 100)

		# Logging to file
		end_log(ai)

	conversion_map = {
		"outValue": "out",
		"outValueX": "outR",
		"outValueY": "outG",
		"outValueZ": "outB"
	}

	rpr += "." + conversion_map[source]
	return rpr


def convertaiNormalMap(ai, source):

	if cmds.objExists(ai + "_rpr"):
		rpr = ai + "_rpr"
	else:
		rpr = cmds.shadingNode("RPRNormal", asUtility=True)
		rpr = cmds.rename(rpr, ai + "_rpr")
		
		# Logging to file
		start_log(ai, rpr)

		# Fields conversion
		if mapDoesNotExist(ai, "input"):
			copyProperty(rpr, ai, "color", "normal")
		else:
			copyProperty(rpr, ai, "color", "input")

		if getProperty(ai, "strength") > 1:
			setProperty(rpr, "strength", 1)
		else:
			copyProperty(rpr, ai, "strength", "strength")

		# Logging to file
		end_log(ai)

	conversion_map = {
		"outValue": "out",
		"outValueX": "outR",
		"outValueY": "outG",
		"outValueZ": "outB"
	}

	rpr += "." + conversion_map[source]
	return rpr


def convertaiVectorMap(ai, source):

	if cmds.objExists(ai + "_rpr"):
		rpr = ai + "_rpr"
	else:
		rpr = cmds.shadingNode("RPRNormal", asUtility=True)
		rpr = cmds.rename(rpr, ai + "_rpr")
		
		# Logging to file
		start_log(ai, rpr)

		# Fields conversion
		if mapDoesNotExist(ai, "input"):
			copyProperty(rpr, ai, "color", "normal")
		else:
			copyProperty(rpr, ai, "color", "input")

		if getProperty(ai, "scale") > 1:
			setProperty(rpr, "strength", 1)
		else:
			copyProperty(rpr, ai, "strength", "scale")

		# Logging to file
		end_log(ai)

	conversion_map = {
		"outValue": "out",
		"outValueX": "outR",
		"outValueY": "outG",
		"outValueZ": "outB"
	}

	rpr += "." + conversion_map[source]
	return rpr


def convertBlendColors(ai, source):

	if cmds.objExists(ai + "_rpr"):
		rpr = ai + "_rpr"
	else:
		rpr = cmds.shadingNode("RPRBlendValue", asUtility=True)
		rpr = cmds.rename(rpr, ai + "_rpr")

		# Logging to file
		start_log(ai, rpr)

		# Fields conversion
		copyProperty(rpr, ai, "inputA", "color1")
		copyProperty(rpr, ai, "inputB", "color2")
		copyProperty(rpr, ai, "weight", "blender")

		# Logging to file
		end_log(ai)

	conversion_map = {
		"output": "out",
		"outputR": "outR",
		"outputG": "outG",
		"outputB": "outB"
	}

	rpr += "." + conversion_map[source]
	return rpr


def convertLuminance(ai, source):

	if cmds.objExists(ai + "_rpr"):
		rpr = ai + "_rpr"
	else:
		rpr = cmds.shadingNode("RPRArithmetic", asUtility=True)
		rpr = cmds.rename(rpr, ai + "_rpr")

		# Logging to file
		start_log(ai, rpr)

		# Fields conversion
		copyProperty(rpr, ai, "inputA", "value")
		setProperty(rpr, "inputB", (0, 0, 0))
		setProperty(rpr, "operation", 19)

		# Logging to file
		end_log(ai)

	conversion_map = {
		"outValue": "outX"
	}

	rpr += "." + conversion_map[source]
	return rpr


def convertColorComposite(ai, source):

	operation = getProperty(ai, "operation")
	if operation == 2:
		if cmds.objExists(ai + "_rpr"):
			rpr = ai + "_rpr"
		else:
			rpr = cmds.shadingNode("RPRBlendValue", asUtility=True)
			rpr = cmds.rename(rpr, ai + "_rpr")

			# Logging to file
			start_log(ai, rpr)

			# Fields conversion
			copyProperty(rpr, ai, "inputA", "alphaA")
			copyProperty(rpr, ai, "inputB", "alphaB")
			copyProperty(rpr, ai, "weight", "factor")
			

			# Logging to file
			end_log(ai)

		conversion_map = {
			"outAlpha": "outR"
		}

		rpr += "." + conversion_map[source]
		return rpr

	else:

		if cmds.objExists(ai + "_rpr"):
			rpr = ai + "_rpr"
		else:
			rpr = cmds.shadingNode("RPRArithmetic", asUtility=True)
			rpr = cmds.rename(rpr, ai + "_rpr")

			# Logging to file
			start_log(ai, rpr)

			# Fields conversion
			if operation in (0, 4, 5):
				setProperty(rpr, "operation", 0)
				if source == "outAlpha":
					copyProperty(rpr, ai, "inputA", "alphaA")
					copyProperty(rpr, ai, "inputB", "alphaB")
				else:
					copyProperty(rpr, ai, "inputA", "colorA")
					copyProperty(rpr, ai, "inputB", "colorB")
			elif operation == 1:
				if source == "outAlpha":
					if mapDoesNotExist(ai, "alphaA") and mapDoesNotExist(ai, "alphaB"):
						alphaA = getProperty(ai, alphaA)
						alphaB = getProperty(ai, alphaB)
						if alphaA > alphaB:
							copyProperty(rpr, ai, "inputA", "alphaA")
							copyProperty(rpr, ai, "inputB", "alphaB")
						else:
							copyProperty(rpr, ai, "inputA", "alphaB")
							copyProperty(rpr, ai, "inputB", "alphaA")
					elif mapDoesNotExist(ai, "alphaA"):
						copyProperty(rpr, ai, "inputA", "alphaA")
						copyProperty(rpr, ai, "inputB", "alphaB")
					elif mapDoesNotExist(ai, "alphaB"):
						copyProperty(rpr, ai, "inputA", "alphaB")
						copyProperty(rpr, ai, "inputB", "alphaA")
					else:
						copyProperty(rpr, ai, "inputA", "alphaA")
						copyProperty(rpr, ai, "inputB", "alphaB")
				else:
					if mapDoesNotExist(ai, "colorA") and mapDoesNotExist(ai, "colorB"):
						colorA = getProperty(ai, alphaA)
						colorB = getProperty(ai, colorB)
						if colorA[0] > colorB[0] or colorA[1] > colorB[1] or colorA[2] > colorB[2]:
							copyProperty(rpr, ai, "inputA", "colorA")
							copyProperty(rpr, ai, "inputB", "colorB")
						else:
							copyProperty(rpr, ai, "inputA", "colorB")
							copyProperty(rpr, ai, "inputB", "colorA")
					elif mapDoesNotExist(ai, "colorA"):
						copyProperty(rpr, ai, "inputA", "colorA")
						copyProperty(rpr, ai, "inputB", "colorB")
					elif mapDoesNotExist(ai, "colorB"):
						copyProperty(rpr, ai, "inputA", "colorB")
						copyProperty(rpr, ai, "inputB", "colorA")
					else:
						copyProperty(rpr, ai, "inputA", "colorA")
						copyProperty(rpr, ai, "inputB", "colorB")
			elif operation == 3:
				setProperty(rpr, "operation", 2)
				if source == "outAlpha":
					copyProperty(rpr, ai, "inputA", "alphaA")
					copyProperty(rpr, ai, "inputB", "alphaB")
				else:
					copyProperty(rpr, ai, "inputA", "colorA")
					copyProperty(rpr, ai, "inputB", "colorB")
			elif operation == 6:
				setProperty(rpr, "operation", 1)
				if source == "outAlpha":
					if mapDoesNotExist(ai, "alphaA"):
						copyProperty(rpr, ai, "inputB", "alphaA")
						copyProperty(rpr, ai, "inputA", "alphaB")
					else:
						copyProperty(rpr, ai, "inputA", "alphaA")
						copyProperty(rpr, ai, "inputB", "alphaB")
				else:
					if mapDoesNotExist(ai, "alphaA"):
						copyProperty(rpr, ai, "inputB", "colorA")
						copyProperty(rpr, ai, "inputA", "colorB")
					else:
						copyProperty(rpr, ai, "inputA", "colorA")
						copyProperty(rpr, ai, "inputB", "colorB")
			elif operation == 7:
				setProperty(rpr, "operation", 25)
				if source == "outAlpha":
					copyProperty(rpr, ai, "inputA", "alphaB")
					copyProperty(rpr, ai, "inputB", "alphaA")
				else:
					copyProperty(rpr, ai, "inputA", "colorB")
					copyProperty(rpr, ai, "inputB", "colorA")
			elif operation == 8:
				setProperty(rpr, "operation", 20)
				if source == "outAlpha":
					copyProperty(rpr, ai, "inputA", "alphaA")
					copyProperty(rpr, ai, "inputB", "alphaB")
				else:
					copyProperty(rpr, ai, "inputA", "colorA")
					copyProperty(rpr, ai, "inputB", "colorB")


			# Logging to file
			end_log(ai)

		conversion_map = {
			"outAlpha": "outX",
			"outColor": "out",
			"outColorR": "outX",
			"outColorG": "outY",
			"outColorB": "outZ"
		}

		rpr += "." + conversion_map[source]
		return rpr


def convertReverse(ai, source):

	if cmds.objExists(ai + "_rpr"):
		rpr = ai + "_rpr"
	else:
		rpr = cmds.shadingNode("RPRArithmetic", asUtility=True)
		rpr = cmds.rename(rpr, ai + "_rpr")

		# Logging to file
		start_log(ai, rpr)

		# Fields conversion
		setProperty(rpr, "inputA", (1, 1, 1))
		copyProperty(rpr, ai, "inputB", "input")
		setProperty(rpr, "operation", 1)

		# Logging to file
		end_log(ai)

	conversion_map = {
		"output": "out",
		"outputX": "outX",
		"outputY": "outY",
		"outputZ": "outZ"
	}

	rpr += "." + conversion_map[source]
	return rpr


def convertPreMultiply(ai, source):

	if cmds.objExists(ai + "_rpr"):
		rpr = ai + "_rpr"
	else:
		rpr = cmds.shadingNode("RPRArithmetic", asUtility=True)
		rpr = cmds.rename(rpr, ai + "_rpr")

		# Logging to file
		start_log(ai, rpr)

		# Fields conversion
		copyProperty(rpr, ai, "inputA", "inColor")
		alpha = getProperty(ai, "inAlpha")
		setProperty(rpr, "inputB", (alpha, alpha, alpha))
		setProperty(rpr, "operation", 2)

		# Logging to file
		end_log(ai)

	conversion_map = {
		"outAlpha": "outX",
		"outColor": "out",
		"outColorR": "outX",
		"outColorG": "outY",
		"outColorB": "outZ"
	}

	rpr += "." + conversion_map[source]
	return rpr


def convertVectorProduct(ai, source):

	operation = getProperty(ai, "operation")
	if operation in (1, 2):
		if cmds.objExists(ai + "_rpr"):
			rpr = ai + "_rpr"
		else:
			rpr = cmds.shadingNode("RPRArithmetic", asUtility=True)
			rpr = cmds.rename(rpr, ai + "_rpr")

			# Logging to file
			start_log(ai, rpr)

			# Fields conversion
			if operation == 1:
				setProperty(rpr, "operation", 11)
			elif operation == 2:
				setProperty(rpr, "operation", 12)

			copyProperty(rpr, ai, "inputA", "input1")
			copyProperty(rpr, ai, "inputB", "input2")

			# Logging to file
			end_log(ai)

		conversion_map = {
			"output": "out",
			"outputX": "outX",
			"outputY": "outY",
			"outputZ": "outZ"
		}

		rpr += "." + conversion_map[source]
		return rpr
	else:
		ai += "." + source
		return ai


def convertChannels(ai, source):

	if "outColor" in source:

		if cmds.objExists(ai + "_color_rpr"):
			rpr = ai + "_color_rpr"
		else:

			rpr = cmds.shadingNode("RPRArithmetic", asUtility=True)
			rpr = cmds.rename(rpr, ai + "_color_rpr")

			# Logging to file
			start_log(ai, rpr)

			# Fields conversion
			copyProperty(rpr, ai, "inputA", "inColor")

			# Logging to file
			end_log(ai)

		conversion_map = {
			"outColor": "out",
			"outColorR": "outX",
			"outColorG": "outY",
			"outColorB": "outZ"
		}

		rpr += "." + conversion_map[source]
		return rpr

	elif "outAlpha" in source:

		if cmds.objExists(ai + "_alpha_rpr"):
			rpr = ai + "_alpha_rpr"
		else:

			rpr = cmds.shadingNode("RPRArithmetic", asUtility=True)
			rpr = cmds.rename(rpr, ai + "_alpha_rpr")

			# Logging to file
			start_log(ai, rpr)

			# Fields conversion
			copyProperty(rpr, ai, "inputA", "inAlpha")

			# Logging to file
			end_log(ai)

		conversion_map = {
			"outAlpha": "outX"
		}

		rpr += "." + conversion_map[source]
		return rpr


def convertaiFacingRatio(ai, source):

	if cmds.objExists(ai + "_rpr"):
		rpr = ai + "_rpr"
	else:
		rpr = cmds.shadingNode("RPRLookup", asUtility=True)
		rpr = cmds.rename(rpr, ai + "_rpr")
			
		# Logging to file
		start_log(ai, rpr)

		# Fields conversion
		setProperty(rpr, "type", 3)

		# Logging to file
		end_log(ai)

	conversion_map = {
		"message": "out",
		"outTransparency": "out",
		"outTransparencyR": "outX",
		"outTransparencyG": "outY",
		"outTransparencyB": "outZ",
		"outValue": "outX"
	}

	rpr += "." + conversion_map[source]
	return rpr


# TODO
def convertaiThinFilm(ai, source):

	if cmds.objExists(ai + "_rpr"):
		rpr = ai + "_rpr"
	else:
		rpr = cmds.shadingNode("RPRFresnel", asUtility=True)
		rpr = cmds.rename(rpr, ai + "_rpr")
			
		# Logging to file
		start_log(ai, rpr)

		# Fields conversion
		ior = (getProperty(ai, "iorMedium") + getProperty(ai, "iorFilm") + getProperty(ai, "iorInternal")) / 3.0
		setProperty(rpr, "ior", ior)

		# Logging to file
		end_log(ai)

	conversion_map = {
		"outColor": "out",
		"outColorR": "out",
		"outColorG": "out",
		"outColorB": "out",
	}

	rpr += "." + conversion_map[source]
	return rpr


def convertaiCurvature(ai, source):

	if cmds.objExists(ai + "_rpr"):
		rpr = ai + "_rpr"
	else:
		rpr = cmds.shadingNode("RPRAmbientOcclusion", asUtility=True)
		rpr = cmds.rename(rpr, ai + "_rpr")
			
		# Logging to file
		start_log(ai, rpr)

		# Fields conversion
		setProperty(rpr, "side", 1)
		setProperty(rpr, "occludedColor", (1, 1, 1))
		setProperty(rpr, "unoccludedColor", (0, 0, 0))
		if mapDoesNotExist(ai, "radius"):
			setProperty(rpr, "radius", getProperty(ai, "radius") / 100)

		# Logging to file
		end_log(ai)

	conversion_map = {
		"outColor": "output",
		"outColorR": "outputR",
		"outColorG": "outputG",
		"outColorB": "outputB",
	}

	rpr += "." + conversion_map[source]
	return rpr


def convertaiBlackbody(ai, source):

	if cmds.objExists(ai + "_rpr"):
		rpr = ai + "_rpr"
	else:
		rpr = cmds.shadingNode("RPRArithmetic", asUtility=True)
		rpr = cmds.rename(rpr, ai + "_rpr")

		# Logging to file
		start_log(ai, rpr)

		# Fields conversion
		arithmetic1 = cmds.shadingNode("RPRArithmetic", asUtility=True)
		setProperty(arithmetic1, "operation", 19)
		intensity = getProperty(ai, "intensity")
		setProperty(arithmetic1, "inputA", (intensity, intensity, intensity))

		arithmetic2 = cmds.shadingNode("RPRArithmetic", asUtility=True)
		setProperty(arithmetic1, "operation", 2)
		connectProperty(arithmetic1, "out", arithmetic2, "inputA")
		setProperty(arithmetic2, "inputB", (0.01, 0.01, 0.01))

		arithmetic3 = cmds.shadingNode("RPRArithmetic", asUtility=True)
		setProperty(arithmetic1, "operation", 0)

		temperature_color = convertTemperature(getProperty(ai, "temperature"))

		setProperty(arithmetic3, "inputA", temperature_color)
		setProperty(arithmetic3, "inputB", temperature_color)

		setProperty(rpr, "operation", 0)
		connectProperty(arithmetic2, "out", rpr, "inputA")
		connectProperty(arithmetic3, "out", rpr, "inputB")

		# Logging to file
		end_log(ai)

	conversion_map = {
		"outColor": "out",
		"outColorR": "outX",
		"outColorG": "outY",
		"outColorB": "outZ",
	}

	rpr += "." + conversion_map[source]
	return rpr


def convertaiColorConvert(ai, source):

	from_value = getProperty(ai, "from")
	to_value = getProperty(ai, "to")

	if from_value == 0 and to_value == 1:
		objectType = "rgbToHsv"
	elif from_value == 1 and to_value == 0:
		objectType = "hsvToRgb"

	if cmds.objExists(ai + "_rpr"):
		rpr = ai + "_rpr"
	else:
		if objectType == "rgbToHsv":
			rpr = cmds.shadingNode("rgbToHsv", asUtility=True)
			rpr = cmds.rename(rpr, ai + "_rpr")

		elif objectType == "hsvToRgb":
			rpr = cmds.shadingNode("hsvToRgb", asUtility=True)
			rpr = cmds.rename(rpr, ai + "_rpr")
		else:
			print("Wrong parameters for aiColorConvert conversion")
			return
			
		# Logging to file
		start_log(ai, rpr)

		if objectType == "rgbToHsv":
			copyProperty(rpr, ai, "inRgb", "input")
		elif objectType == "hsvToRgb":
			copyProperty(rpr, ai, "inHsv", "input")

		end_log(ai)


	conversion_map_rgb = {
		"outColor": "outRgb",
		"outColorR": "outRgbR",
		"outColorG": "outRgbG",
		"outColorB": "outRgbB",
	}

	conversion_map_hsv = {
		"outColor": "outHsv",
		"outColorR": "outHsvH",
		"outColorG": "outHsvS",
		"outColorB": "outHsvV",
	}

	if objectType == "rgbToHsv":
		rpr += "." + conversion_map_hsv[source]
	elif objectType == "hsvToRgb":
		rpr += "." + conversion_map_rgb[source]

	return rpr



def convertaiImage(ai, source):

	if cmds.objExists(ai + "_rpr"):
		rpr = ai + "_rpr"
	else:
		rpr = cmds.shadingNode("file", asTexture=True, isColorManaged=True)
		rpr = cmds.rename(rpr, ai + "_rpr")
		texture = cmds.shadingNode("place2dTexture", asUtility=True)

		connectProperty(texture, "coverage", rpr, "coverage")
		connectProperty(texture, "translateFrame", rpr, "translateFrame")
		connectProperty(texture, "rotateFrame", rpr, "rotateFrame")
		connectProperty(texture, "mirrorU", rpr, "mirrorU")
		connectProperty(texture, "mirrorV", rpr, "mirrorV")
		connectProperty(texture, "stagger", rpr, "stagger")
		connectProperty(texture, "wrapU", rpr, "wrapU")
		connectProperty(texture, "wrapV", rpr, "wrapV")
		connectProperty(texture, "repeatUV", rpr, "repeatUV")
		connectProperty(texture, "offset", rpr, "offset")
		connectProperty(texture, "rotateUV", rpr, "rotateUV")
		connectProperty(texture, "noiseUV", rpr, "noiseUV")
		connectProperty(texture, "vertexUvOne", rpr, "vertexUvOne")
		connectProperty(texture, "vertexUvTwo", rpr, "vertexUvTwo")
		connectProperty(texture, "vertexUvThree", rpr, "vertexUvThree")
		connectProperty(texture, "vertexCameraOne", rpr, "vertexCameraOne")
		connectProperty(texture, "outUV", rpr, "uv")
		connectProperty(texture, "outUvFilterSize", rpr, "uvFilterSize")
			
		# Logging to file
		start_log(ai, rpr)

		# Fields conversion
		setProperty(rpr, "fileTextureName", getProperty(ai, "filename"))
		setProperty(rpr, "colorSpace", getProperty(ai, "colorSpace"))
		
		copyProperty(rpr, ai, "useFrameExtension", "useFrameExtension")
		copyProperty(rpr, ai, "frameExtension", "frame")
		copyProperty(rpr, ai, "ignoreColorSpaceFileRules", "ignoreColorSpaceFileRules")

		# Logging to file
		end_log(ai)

	conversion_map = {
		"outColor": "outColor",
		"outColorR": "outColorR",
		"outColorG": "outColorG",
		"outColorB": "outColorB"
	}

	rpr += "." + conversion_map[source]
	return rpr


def convertaiNoise(ai, source):

	if cmds.objExists(ai + "_rpr"):
		rpr = ai + "_rpr"
	else:
		rpr = cmds.shadingNode("noise", asUtility=True)
		rpr = cmds.rename(rpr, ai + "_rpr")

		texture = cmds.shadingNode("place2dTexture", asUtility=True)

		connectProperty(texture, "outUV", rpr, "uv")
		connectProperty(texture, "outUvFilterSize", rpr, "uvFilterSize")
			
		# Logging to file
		start_log(ai, rpr)

		# Fields conversion
		copyProperty(rpr, ai, "frequencyRatio", "octaves")
		copyProperty(rpr, ai, "frequency", "octaves")
		copyProperty(rpr, ai, "threshold", "distortion")
		copyProperty(rpr, ai, "ratio", "lacunarity")
		copyProperty(rpr, ai, "amplitude", "amplitude")
		copyProperty(rpr, ai, "defaultColor", "color1")
		copyProperty(rpr, ai, "colorGain", "color1")
		copyProperty(rpr, ai, "colorOffset", "color2")

		# Logging to file
		end_log(ai)

	rpr += "." + source
	return rpr


def convertaiCellNoise(ai, source):

	if cmds.objExists(ai + "_rpr"):
		rpr = ai + "_rpr"
	else:
		rpr = cmds.shadingNode("noise", asUtility=True)
		rpr = cmds.rename(rpr, ai + "_rpr")

		texture = cmds.shadingNode("place2dTexture", asUtility=True)

		connectProperty(texture, "outUV", rpr, "uv")
		connectProperty(texture, "outUvFilterSize", rpr, "uvFilterSize")
			
		# Logging to file
		start_log(ai, rpr)

		# Fields conversion
		copyProperty(rpr, ai, "frequencyRatio", "octaves")
		copyProperty(rpr, ai, "frequency", "octaves")
		copyProperty(rpr, ai, "ratio", "lacunarity")
		copyProperty(rpr, ai, "amplitude", "amplitude")
		copyProperty(rpr, ai, "defaultColor", "color")
		copyProperty(rpr, ai, "colorGain", "color")
		copyProperty(rpr, ai, "colorOffset", "palette")
		copyProperty(rpr, ai, "density", "density")
		copyProperty(rpr, ai, "randomness", "randomness")

		# Logging to file
		end_log(ai)

	rpr += "." + source
	return rpr


def convertaiTriplanar(ai, source):

	if cmds.objExists(ai + "_rpr"):
		rpr = ai + "_rpr"
	else:
		rpr = cmds.shadingNode("projection", asUtility=True)
		rpr = cmds.rename(rpr, ai + "_rpr")

		texture = cmds.shadingNode("place3dTexture", asUtility=True)

		connectProperty(texture, "worldInverseMatrix[0]", rpr, "placementMatrix")
			
		# Logging to file
		start_log(ai, rpr)

		# Fields conversion
		copyProperty(rpr, ai, "image", "input")
		copyProperty(texture, ai, "scale", "scale")
		copyProperty(texture, ai, "rotate", "rotate")

		# Logging to file
		end_log(ai)

	rpr += "." + source
	return rpr


def convertaiUvTransform(ai, source):

	if cmds.objExists(ai + "_rpr"):
		rpr = ai + "_rpr"
	else:
		rpr = cmds.shadingNode("projection", asUtility=True)
		rpr = cmds.rename(rpr, ai + "_rpr")

		texture = cmds.shadingNode("place3dTexture", asUtility=True)

		connectProperty(texture, "worldInverseMatrix[0]", rpr, "placementMatrix")
			
		# Logging to file
		start_log(ai, rpr)

		# Fields conversion
		copyProperty(rpr, ai, "image", "passthrough")
		copyProperty(texture, ai, "scaleX", "scaleFrameX")
		copyProperty(texture, ai, "scaleY", "scaleFrameY")
		copyProperty(texture, ai, "translateX", "translateFrameX")
		copyProperty(texture, ai, "translateY", "translateFrameY")
		copyProperty(texture, ai, "rotateX", "rotateFrame")
		copyProperty(texture, ai, "rotateY", "rotateFrame")
		copyProperty(texture, ai, "rotateZ", "rotateFrame")
		copyProperty(texture, ai, "rotateAxisX", "rotate")
		copyProperty(texture, ai, "rotateAxisY", "rotate")
		copyProperty(texture, ai, "rotateAxisZ", "rotate")

		# Logging to file
		end_log(ai)

	rpr += "." + source
	return rpr


def convertaiLength(ai, source):

	if cmds.objExists(ai + "_rpr"):
		rpr = ai + "_rpr"
	else:
		rpr = cmds.shadingNode("RPRArithmetic", asUtility=True)
		if not getProperty(ai, "mode"):
			rpr = cmds.rename(rpr, ai + "_rpr")
		else:
			rpr = cmds.rename(rpr, ai + "_UNSUPPORTED_NODE")

		# Logging to file
		start_log(ai, rpr)

		# Fields conversion
		setProperty(rpr, "operation", 20)
		copyProperty(rpr, ai, "inputA", "input")
		setProperty(rpr, "inputB", (1, 1, 1))

		# Logging to file
		end_log(ai)

	conversion_map = {
		"outValue": "out"
	}

	rpr += "." + conversion_map[source]
	return rpr


# standart utilities
def convertStandardNode(aiMaterial, source):

	not_converted_list = ("materialInfo", "defaultShaderList", "shadingEngine", "place2dTexture")
	try:
		for attr in cmds.listAttr(aiMaterial):
			connection = cmds.listConnections(aiMaterial + "." + attr)
			if connection:
				if cmds.objectType(connection[0]) not in not_converted_list and attr not in (source, "message"):
					obj, channel = cmds.connectionInfo(aiMaterial + "." + attr, sourceFromDestination=True).split('.')
					source_name, source_attr = convertMaterial(obj, channel).split('.')
					connectProperty(source_name, source_attr, aiMaterial, attr)
	except:
		pass

	return aiMaterial + "." + source


# unsupported utilities
def convertUnsupportedNode(aiMaterial, source):

	if cmds.objExists(aiMaterial + "_UNSUPPORTED_NODE"):
		rpr = aiMaterial + "_UNSUPPORTED_NODE"
	else:
		rpr = cmds.shadingNode("RPRArithmetic", asUtility=True)
		rpr = cmds.rename(rpr, aiMaterial + "_UNSUPPORTED_NODE")

		# Logging to file
		start_log(aiMaterial, rpr)

		# 2 connection save
		try:
			setProperty(rpr, "operation", 0)
			unsupported_connections = 0
			for attr in cmds.listAttr(aiMaterial):
				connection = cmds.listConnections(aiMaterial + "." + attr)
				if connection:
					if cmds.objectType(connection[0]) not in ("materialInfo", "defaultShaderList", "shadingEngine") and attr not in (source, "message"):
						if unsupported_connections < 2:
							obj, channel = cmds.connectionInfo(aiMaterial + "." + attr, sourceFromDestination=True).split('.')
							source_name, source_attr = convertMaterial(obj, channel).split('.')
							valueType = type(getProperty(aiMaterial, attr))
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
		end_log(aiMaterial)

	sourceType = type(getProperty(aiMaterial, source))
	if sourceType == tuple:
		rpr += ".out"
	else:
		rpr += ".outX"

	return rpr


# Create default uber material for unsupported material
def convertUnsupportedMaterial(aiMaterial, source):

	assigned = checkAssign(aiMaterial)

	if cmds.objExists(aiMaterial + "_rpr"):
		rprMaterial = aiMaterial + "_rpr"
	else:

		# Creating new Uber material
		rprMaterial = cmds.shadingNode("RPRUberMaterial", asShader=True)
		rprMaterial = cmds.rename(rprMaterial, aiMaterial + "_UNSUPPORTED_MATERIAL")

		# Check assigned to any mesh
		if assigned:
			sg = rprMaterial + "SG"
			cmds.sets(renderable=True, noSurfaceShader=True, empty=True, name=sg)
			connectProperty(rprMaterial, "outColor", sg, "surfaceShader")

		# Logging to file
		start_log(aiMaterial, rprMaterial)
		
		setProperty(rprMaterial, "diffuseColor", (0, 1, 0))

		end_log(aiMaterial)

	if source:
		rprMaterial += "." + source
	return rprMaterial


#######################
## aiAmbientOcclusion 
#######################

def convertaiAmbientOcclusion(aiMaterial, source):

	assigned = checkAssign(aiMaterial)
	
	if assigned:
		# Creating new Uber material
		rprMaterial = cmds.shadingNode("RPRMaterial", asShader=True)
		rprMaterial = cmds.rename(rprMaterial, aiMaterial + "_rpr")
		setProperty(rprMaterial, "type", 10)

		sg = rprMaterial + "SG"
		cmds.sets(renderable=True, noSurfaceShader=True, empty=True, name=sg)
		connectProperty(rprMaterial, "outColor", sg, "surfaceShader")

		ao = cmds.shadingNode("RPRAmbientOcclusion", asUtility=True)
		connectProperty(ao, "output", rprMaterial, "color")
	else:
		ao = cmds.shadingNode("RPRAmbientOcclusion", asUtility=True)
		ao = cmds.rename(ao, aiMaterial + "_rpr")

	# Logging to file
	start_log(aiMaterial, ao)

	# Fields conversion
	copyProperty(ao, aiMaterial, "unoccludedColor", "white")
	copyProperty(ao, aiMaterial, "occludedColor", "black")
	copyProperty(ao, aiMaterial, "radius", "falloff")
	copyProperty(ao, aiMaterial, "samples", "samples")

	# Logging in file
	end_log(aiMaterial)

	if source:
		conversion_map = {
		"outColor": "output",
		"outColorR": "outputR",
		"outColorG": "outputG",
		"outColorB": "outputB"
		}	
		rprMaterial = ao + "." + conversion_map[source]
	return rprMaterial


#######################
## aiFlat 
#######################

def convertaiFlat(aiMaterial, source):

	assigned = checkAssign(aiMaterial)
	
	if cmds.objExists(aiMaterial + "_rpr"):
		rprMaterial = aiMaterial + "_rpr"
	else:
		# Creating new Uber material
		rprMaterial = cmds.shadingNode("RPRFlatColorMaterial", asShader=True)
		rprMaterial = cmds.rename(rprMaterial, aiMaterial + "_rpr")

		# Check shading engine in aiMaterial
		if assigned:
			sg = rprMaterial + "SG"
			cmds.sets(renderable=True, noSurfaceShader=True, empty=True, name=sg)
			connectProperty(rprMaterial, "outColor", sg, "surfaceShader")

		# Logging to file
		start_log(aiMaterial, rprMaterial)

		# Fields conversion
		copyProperty(rprMaterial, aiMaterial, "color", "color")

		# Logging in file
		end_log(aiMaterial)

	if source:
		rprMaterial += "." + source
	return rprMaterial


#######################
## aiLayerShader
#######################

def convertaiLayerShader(aiMaterial, source):

	assigned = checkAssign(aiMaterial)
	
	if cmds.objExists(aiMaterial + "_rpr"):
		rprMaterial = aiMaterial + "_rpr"
	else:
		# Creating new Uber material
		rprMaterial = cmds.shadingNode("RPRBlendMaterial", asShader=True)
		
		# Logging to file
		start_log(aiMaterial, rprMaterial)

		# Fields conversion
		first_material = True
		second_material = True
		for i in range(1, 9):
			if getProperty(aiMaterial, "enable" + str(i)):
				input_value = cmds.listConnections(aiMaterial + ".input" + str(i))
				if input_value:
					if first_material:
						copyProperty(rprMaterial, aiMaterial, "color0", "input" + str(i))
						first_material = False
					elif second_material:
						copyProperty(rprMaterial, aiMaterial, "color1", "input" + str(i))
						copyProperty(rprMaterial, aiMaterial, "weight", "mix" + str(i))
						second_material = False
					else:
						old_rpr = rprMaterial
						rprMaterial = cmds.shadingNode("RPRBlendMaterial", asShader=True)
						connectProperty(old_rpr, "outColor", rprMaterial, "color0")
						copyProperty(rprMaterial, aiMaterial, "color1", "input" + str(i))
						copyProperty(rprMaterial, aiMaterial, "weight", "mix" + str(i))


		rprMaterial = cmds.rename(rprMaterial, aiMaterial + "_rpr")

		# Check shading engine in aiMaterial
		if assigned:
			sg = rprMaterial + "SG"
			cmds.sets(renderable=True, noSurfaceShader=True, empty=True, name=sg)
			connectProperty(rprMaterial, "outColor", sg, "surfaceShader")

		# Logging in file
		end_log(aiMaterial)

	if source:
		rprMaterial += "." + source
	return rprMaterial


#######################
## aiToon
#######################

def convertaiToon(aiMaterial, source):

	assigned = checkAssign(aiMaterial)

	if cmds.objExists(aiMaterial + "_rpr"):
		rprMaterial = aiMaterial + "_rpr"
	else:
	
		# Creating new Uber material
		rprMaterial = cmds.shadingNode("RPRUberMaterial", asShader=True)
		rprMaterial = cmds.rename(rprMaterial, aiMaterial + "_rpr")

		# Check shading engine in aiMaterial
		if assigned:
			sg = rprMaterial + "SG"
			cmds.sets(renderable=True, noSurfaceShader=True, empty=True, name=sg)
			connectProperty(rprMaterial, "outColor", sg, "surfaceShader")

		# Enable properties, which are default in Arnold
		defaultEnable(rprMaterial, aiMaterial, "diffuse", "base")
		defaultEnable(rprMaterial, aiMaterial, "reflections", "specular")
		defaultEnable(rprMaterial, aiMaterial, "refraction", "transmission")
		defaultEnable(rprMaterial, aiMaterial, "emissive", "emission")

		# Logging to file
		start_log(aiMaterial, rprMaterial)

		# Fields conversion
		copyProperty(rprMaterial, aiMaterial, "diffuseColor", "baseColor")
		copyProperty(rprMaterial, aiMaterial, "diffuseWeight", "base")

		copyProperty(rprMaterial, aiMaterial, "reflectWeight", "specular")
		copyProperty(rprMaterial, aiMaterial, "reflectColor", "specularColor")
		copyProperty(rprMaterial, aiMaterial, "reflectRoughness", "specularRoughness")
		copyProperty(rprMaterial, aiMaterial, "reflectAnisotropy", "specularAnisotropy")
		copyProperty(rprMaterial, aiMaterial, "reflectAnisotropyRotation", "specularRotation")
		if getProperty(aiMaterial, "specular"):
			setProperty(rprMaterial, "reflectMetalMaterial", 1)
			copyProperty(rprMaterial, aiMaterial, "reflectMetalness", "specular")

		copyProperty(rprMaterial, aiMaterial, "refractWeight", "transmission")
		copyProperty(rprMaterial, aiMaterial, "refractColor", "transmissionColor")
		copyProperty(rprMaterial, aiMaterial, "refractRoughness", "transmissionRoughness")
		copyProperty(rprMaterial, aiMaterial, "refractIor", "IOR")

		copyProperty(rprMaterial, aiMaterial, "emissiveWeight", "emission")
		copyProperty(rprMaterial, aiMaterial, "emissiveIntensity", "emission")
		copyProperty(rprMaterial, aiMaterial, "emissiveColor", "emissionColor")

		# Logging in file
		end_log(aiMaterial)

	if source:
		rprMaterial += "." + source
	return rprMaterial


#######################
## aiMixShader 
#######################

def convertaiMixShader(aiMaterial, source):

	assigned = checkAssign(aiMaterial)
	
	if cmds.objExists(aiMaterial + "_rpr"):
		rprMaterial = aiMaterial + "_rpr"
	else:
		# Creating new Uber material
		rprMaterial = cmds.shadingNode("RPRBlendMaterial", asShader=True)
		rprMaterial = cmds.rename(rprMaterial, aiMaterial + "_rpr")

		# Check shading engine in aiMaterial
		if assigned:
			sg = rprMaterial + "SG"
			cmds.sets(renderable=True, noSurfaceShader=True, empty=True, name=sg)
			connectProperty(rprMaterial, "outColor", sg, "surfaceShader")

		# Logging to file
		start_log(aiMaterial, rprMaterial)

		# Fields conversion
		copyProperty(rprMaterial, aiMaterial, "color0", "shader1")
		copyProperty(rprMaterial, aiMaterial, "color1", "shader2")
		copyProperty(rprMaterial, aiMaterial, "weight", "mix")

		# Logging in file
		end_log(aiMaterial)

	if source:
		rprMaterial += "." + source
	return rprMaterial


#######################
## aiStandardSurface 
#######################

def convertaiStandardSurface(aiMaterial, source):

	assigned = checkAssign(aiMaterial)
	
	if cmds.objExists(aiMaterial + "_rpr"):
		rprMaterial = aiMaterial + "_rpr"
	else:
		# Creating new Uber material
		rprMaterial = cmds.shadingNode("RPRUberMaterial", asShader=True)
		rprMaterial = cmds.rename(rprMaterial, aiMaterial + "_rpr")

		# Check shading engine in aiMaterial
		if assigned:
			sg = rprMaterial + "SG"
			cmds.sets(renderable=True, noSurfaceShader=True, empty=True, name=sg)
			connectProperty(rprMaterial, "outColor", sg, "surfaceShader")

			ai_materialSG = cmds.listConnections(aiMaterial, type="shadingEngine")[0]
			convertDisplacement(ai_materialSG, rprMaterial)

		# Enable properties, which are default in Arnold
		defaultEnable(rprMaterial, aiMaterial, "diffuse", "base")
		defaultEnable(rprMaterial, aiMaterial, "reflections", "specular")
		defaultEnable(rprMaterial, aiMaterial, "refraction", "transmission")
		defaultEnable(rprMaterial, aiMaterial, "sssEnable", "subsurface")
		defaultEnable(rprMaterial, aiMaterial, "emissive", "emission")
		defaultEnable(rprMaterial, aiMaterial, "clearCoat", "coat")

		# Logging to file
		start_log(aiMaterial, rprMaterial)

		# Fields conversion
		copyProperty(rprMaterial, aiMaterial, "diffuseColor", "baseColor")
		copyProperty(rprMaterial, aiMaterial, "diffuseWeight", "base")
		copyProperty(rprMaterial, aiMaterial, "diffuseRoughness", "diffuseRoughness")

		copyProperty(rprMaterial, aiMaterial, "reflectColor", "specularColor")
		copyProperty(rprMaterial, aiMaterial, "reflectWeight", "specular")
		copyProperty(rprMaterial, aiMaterial, "reflectRoughness", "specularRoughness")
		copyProperty(rprMaterial, aiMaterial, "reflectAnisotropy", "specularAnisotropy")
		copyProperty(rprMaterial, aiMaterial, "reflectAnisotropyRotation", "specularRotation")
		copyProperty(rprMaterial, aiMaterial, "reflectIOR", "specularIOR")

		metalness = getProperty(aiMaterial, "metalness")
		if metalness:
			setProperty(rprMaterial, "reflections", 1)
			setProperty(rprMaterial, "diffuse", 1)
			setProperty(rprMaterial, "reflectMetalMaterial", 1)
			setProperty(rprMaterial, "reflectWeight", 1)
			copyProperty(rprMaterial, aiMaterial, "reflectMetalness", "metalness")
			copyProperty(rprMaterial, aiMaterial, "reflectColor", "baseColor")
		else:
			setProperty(rprMaterial, 'reflectIsFresnelApproximationOn', 1)

		copyProperty(rprMaterial, aiMaterial, "refractColor", "transmissionColor")
		copyProperty(rprMaterial, aiMaterial, "refractWeight", "transmission")
		copyProperty(rprMaterial, aiMaterial, "refractRoughness", "transmissionExtraRoughness")
		setProperty(rprMaterial, "refractThinSurface", getProperty(aiMaterial, "thinWalled"))

		copyProperty(rprMaterial, aiMaterial, "volumeScatter", "subsurfaceColor")
		copyProperty(rprMaterial, aiMaterial, "sssWeight", "subsurface")
		copyProperty(rprMaterial, aiMaterial, "backscatteringWeight", "subsurface")
		if mapDoesNotExist(aiMaterial, "subsurfaceRadius"):
			setProperty(rprMaterial, "subsurfaceRadius", getProperty(aiMaterial, "subsurfaceRadius"))
		else:
			copyProperty(rprMaterial, aiMaterial, "subsurfaceRadius", "subsurfaceRadius")

		subsurface = getProperty(aiMaterial, "subsurface")
		if subsurface:
			setProperty(rprMaterial, "diffuse", 1)
			setProperty(rprMaterial, "diffuseWeight", 1)
			setProperty(rprMaterial, "separateBackscatterColor", 0)
			setProperty(rprMaterial, "multipleScattering", 0)
			setProperty(rprMaterial, "backscatteringWeight", 0.75)

			if getProperty(aiMaterial, "subsurfaceType") == 0: # diffusion type
				copyProperty(rprMaterial, aiMaterial, "diffuseColor", "subsurfaceColor")
				setProperty(rprMaterial, "backscatteringWeight", 0.125)
			
		copyProperty(rprMaterial, aiMaterial, "coatColor", "coatColor")
		copyProperty(rprMaterial, aiMaterial, "coatTransmissionColor", "coatColor")
		copyProperty(rprMaterial, aiMaterial, "coatWeight", "coat")
		copyProperty(rprMaterial, aiMaterial, "coatRoughness", "coatRoughness")
		copyProperty(rprMaterial, aiMaterial, "coatIor", "coatIOR")
		copyProperty(rprMaterial, aiMaterial, "coatNormal", "coatNormal")
		setProperty(rprMaterial, "coatThickness", 1.5)

		copyProperty(rprMaterial, aiMaterial, "emissiveColor", "emissionColor")
		setProperty(rprMaterial, "emissiveWeight", 0.35)
		setProperty(rprMaterial, "emissiveIntensity", getProperty(aiMaterial, "emission") * 2.5)

		if getProperty(aiMaterial, "opacity") != (1, 1, 1):
			if mapDoesNotExist(aiMaterial, "opacity"):
				transparency = 1 - max(getProperty(aiMaterial, "opacity"))
				setProperty(rprMaterial, "transparencyLevel", transparency)
			else:
				arithmetic = cmds.shadingNode("RPRArithmetic", asUtility=True)
				setProperty(arithmetic, "operation", 1)
				setProperty(arithmetic, "inputA", (1, 1, 1))
				copyProperty(arithmetic, aiMaterial, "inputB", "opacity")
				connectProperty(arithmetic, "outX", rprMaterial, "transparencyLevel")
			setProperty(rprMaterial, "transparencyEnable", 1)

		bumpConnections = cmds.listConnections(aiMaterial + ".normalCamera")
		if bumpConnections:
			setProperty(rprMaterial, "normalMapEnable", 1)
			copyProperty(rprMaterial, aiMaterial, "normalMap", "normalCamera")
			if getProperty(aiMaterial, "base"):
				copyProperty(rprMaterial, aiMaterial, "diffuseNormal", "normalCamera")
			if getProperty(aiMaterial, "specular"):
				copyProperty(rprMaterial, aiMaterial, "reflectNormal", "normalCamera")
			if getProperty(aiMaterial, "transmission"):
				copyProperty(rprMaterial, aiMaterial, "refractNormal", "normalCamera")
			if getProperty(aiMaterial, "coat"):
				copyProperty(rprMaterial, aiMaterial, "coatNormal", "normalCamera")
		
		# Logging in file
		end_log(aiMaterial)

	if source:
		rprMaterial += "." + source
	return rprMaterial


#######################
## aiCarPaint 
#######################

def convertaiCarPaint(aiMaterial, source):

	assigned = checkAssign(aiMaterial)
	
	if cmds.objExists(aiMaterial + "_rpr"):
		rprMaterial = aiMaterial + "_rpr"
	else:
		# Creating new Uber material
		rprMaterial = cmds.shadingNode("RPRUberMaterial", asShader=True)
		rprMaterial = cmds.rename(rprMaterial, aiMaterial + "_rpr")

		# Check shading engine in aiMaterial
		if assigned:
			sg = rprMaterial + "SG"
			cmds.sets(renderable=True, noSurfaceShader=True, empty=True, name=sg)
			connectProperty(rprMaterial, "outColor", sg, "surfaceShader")

		# Enable properties, which are default in Arnold
		defaultEnable(rprMaterial, aiMaterial, "diffuse", "base")
		defaultEnable(rprMaterial, aiMaterial, "reflections", "specular")
		defaultEnable(rprMaterial, aiMaterial, "clearCoat", "coat")

		# Logging to file
		start_log(aiMaterial, rprMaterial)

		# Fields conversion
		copyProperty(rprMaterial, aiMaterial, "diffuseColor", "baseColor")
		copyProperty(rprMaterial, aiMaterial, "diffuseWeight", "base")
		copyProperty(rprMaterial, aiMaterial, "diffuseRoughness", "baseRoughness")

		copyProperty(rprMaterial, aiMaterial, "reflectColor", "specularColor")
		copyProperty(rprMaterial, aiMaterial, "reflectWeight", "specular")
		copyProperty(rprMaterial, aiMaterial, "reflectRoughness", "specularRoughness")
		copyProperty(rprMaterial, aiMaterial, "reflectIOR", "specularIOR")

		if getProperty(aiMaterial, "coatColor") != (1, 1, 1):

			arith = cmds.shadingNode("RPRArithmetic", asUtility=True)
			setProperty(arith, "operation", 2)
			copyProperty(arith, aiMaterial, "inputA", "baseColor")
			copyProperty(arith, aiMaterial, "inputB", "coatColor")

			if mapDoesNotExist(aiMaterial, "coat"):
				connectProperty(arith, "out", rprMaterial, "diffuseColor")
			else:
				blend_value = cmds.shadingNode("RPRBlendValue", asUtility=True)
				copyProperty(blend_value, aiMaterial, "inputA", "baseColor")
				connectProperty(arith, "out", blend_value, "inputB")
				copyProperty(blend_value, aiMaterial, "weight", "coat")

				connectProperty(blend_value, "out", rprMaterial, "diffuseColor")

		copyProperty(rprMaterial, aiMaterial, "coatColor", "coatColor")
		copyProperty(rprMaterial, aiMaterial, "coatWeight", "coat")
		copyProperty(rprMaterial, aiMaterial, "coatRoughness", "coatRoughness")
		copyProperty(rprMaterial, aiMaterial, "coatIor", "coatIOR")

		bumpConnections = cmds.listConnections(aiMaterial + ".coatNormal")
		if bumpConnections:
			setProperty(rprMaterial, "normalMapEnable", 1)
			copyProperty(rprMaterial, aiMaterial, "normalMap", "coatNormal")
			setProperty(rprMaterial, "useShaderNormal", 1)
			setProperty(rprMaterial, "reflectUseShaderNormal", 1)
			setProperty(rprMaterial, "refractUseShaderNormal", 1)
			setProperty(rprMaterial, "coatUseShaderNormal", 1)

		# Logging in file
		end_log(aiMaterial)

	if source:
		rprMaterial += "." + source
	return rprMaterial


#######################
## aiShadowMatte 
#######################

def convertaiShadowMatte(aiMaterial, source):

	assigned = checkAssign(aiMaterial)
	
	if cmds.objExists(aiMaterial + "_rpr"):
		rprMaterial = aiMaterial + "_rpr"
	else:
		# Creating new Uber material
		rprMaterial = cmds.shadingNode("RPRMatteMaterial", asShader=True)
		rprMaterial = cmds.rename(rprMaterial, aiMaterial + "_rpr")

		# Check shading engine in aiMaterial
		if assigned:
			sg = rprMaterial + "SG"
			cmds.sets(renderable=True, noSurfaceShader=True, empty=True, name=sg)
			connectProperty(rprMaterial, "outColor", sg, "surfaceShader")

			ai_materialSG = cmds.listConnections(aiMaterial, type="shadingEngine")[0]
			convertShadowDisplacement(ai_materialSG, rprMaterial)

		# Logging to file
		start_log(aiMaterial, rprMaterial)

		# Fields conversion
		copyProperty(rprMaterial, aiMaterial, "shadowColor", "shadowColor")

		if mapDoesNotExist(aiMaterial, "shadowOpacity"):
			transparency = 1 - getProperty(aiMaterial, "shadowOpacity")
			setProperty(rprMaterial, "shadowTransp", transparency)
		else:
			arithmetic = cmds.shadingNode("RPRArithmetic", asUtility=True)
			setProperty(arithmetic, "operation", 1)
			setProperty(arithmetic, "inputA", (1, 1, 1))
			copyProperty(arithmetic, aiMaterial, "inputBX", "shadowOpacity")
			connectProperty(arithmetic, "outX", rprMaterial, "shadowTransp")

		copyProperty(rprMaterial, aiMaterial, "bgColor", "backgroundColor")

		# Logging in file
		end_log(aiMaterial)

	if source:
		rprMaterial += "." + source
	return rprMaterial



#######################
## aiMatte 
#######################

def convertaiMatte(aiMaterial, source):

	assigned = checkAssign(aiMaterial)
	
	if cmds.objExists(aiMaterial + "_rpr"):
		rprMaterial = aiMaterial + "_rpr"
	else:
		# Creating new Uber material
		rprMaterial = cmds.shadingNode("RPRBlendMaterial", asShader=True)
		rprMaterial = cmds.rename(rprMaterial, aiMaterial + "_rpr")

		# Check shading engine in aiMaterial
		if assigned:
			sg = rprMaterial + "SG"
			cmds.sets(renderable=True, noSurfaceShader=True, empty=True, name=sg)
			connectProperty(rprMaterial, "outColor", sg, "surfaceShader")

		# Logging to file
		start_log(aiMaterial, rprMaterial)

		# Fields conversion
		uber = cmds.shadingNode("RPRUberMaterial", asShader=True)
		flat = cmds.shadingNode("RPRFlatColorMaterial", asShader=True)

		connectProperty(uber, "outColor", rprMaterial, "color0")
		connectProperty(flat, "outColor", rprMaterial, "color1")
		setProperty(rprMaterial, "weight", 0.25)

		copyProperty(flat, aiMaterial, "color", "color")
		copyProperty(uber, aiMaterial, "diffuseColor", "color")
		setProperty(uber, "normalMapEnable", 0)

		if getProperty(aiMaterial, "opacity") != (1, 1, 1):
			if mapDoesNotExist(aiMaterial, "opacity"):
				transparency = 1 - max(getProperty(aiMaterial, "opacity"))
				setProperty(uber, "transparencyLevel", transparency)
			else:
				arithmetic = cmds.shadingNode("RPRArithmetic", asUtility=True)
				setProperty(arithmetic, "operation", 1)
				setProperty(arithmetic, "inputA", (1, 1, 1))
				copyProperty(arithmetic, aiMaterial, "inputB", "opacity")
				connectProperty(arithmetic, "outX", uber, "transparencyLevel")
			setProperty(uber, "transparencyEnable", 1)

		# Logging in file
		end_log(aiMaterial)

	if source:
		rprMaterial += "." + source
	return rprMaterial


#######################
## aiPassthrough 
#######################

def convertaiPassthrough(aiMaterial, source):

	assigned = checkAssign(aiMaterial)
	
	if cmds.objExists(aiMaterial + "_rpr"):
		rprMaterial = aiMaterial + "_rpr"
	else:
		# Creating new Uber material
		rprMaterial = cmds.shadingNode("RPRMaterial", asShader=True)
		rprMaterial = cmds.rename(rprMaterial, aiMaterial + "_rpr")
		setProperty(rprMaterial, "type", 10)

		# Check shading engine in aiMaterial
		if assigned:
			sg = rprMaterial + "SG"
			cmds.sets(renderable=True, noSurfaceShader=True, empty=True, name=sg)
			connectProperty(rprMaterial, "outColor", sg, "surfaceShader")

		# Logging to file
		start_log(aiMaterial, rprMaterial)

		# Fields conversion
		copyProperty(rprMaterial, aiMaterial, "color", "passthrough")
		copyProperty(rprMaterial, aiMaterial, "normalMap", "normalCamera")

		# Logging in file
		end_log(aiMaterial)

	if source:
		rprMaterial += "." + source
	return rprMaterial


#######################
## aiStandardVolume 
#######################

def convertaiStandardVolume(aiMaterial, source):

	assigned = checkAssign(aiMaterial)
	
	if cmds.objExists(aiMaterial + "_rpr"):
		rprMaterial = aiMaterial + "_rpr"
	else:
		# Creating new Uber material
		rprMaterial = cmds.shadingNode("RPRVolumeMaterial", asShader=True)
		rprMaterial = cmds.rename(rprMaterial, aiMaterial + "_rpr")

		# Check shading engine in aiMaterial
		if assigned:
			sg = rprMaterial + "SG"
			cmds.sets(renderable=True, noSurfaceShader=True, empty=True, name=sg)
			connectProperty(rprMaterial, "outColor", sg, "surfaceShader")

		# Logging to file
		start_log(aiMaterial, rprMaterial)

		# Fields conversion
		copyProperty(rprMaterial, aiMaterial, "scatterColor", "scatterColor")
		copyProperty(rprMaterial, aiMaterial, "emissionColor", "emissionColor")
		copyProperty(rprMaterial, aiMaterial, "transmissionColor", "transparent")
		copyProperty(rprMaterial, aiMaterial, "density", "density")

		# Logging in file
		end_log(aiMaterial)

	if source:
		rprMaterial += "." + source
	return rprMaterial


def convertaiSkyDomeLight(dome_light):

	skyNode = cmds.listConnections(dome_light + ".color")
	if skyNode:
		if cmds.objectType(skyNode[0]) == "aiPhysicalSky":
			return

	if cmds.objExists("RPRIBL"):
		iblShape = "RPRIBLShape"
		iblTransform = "RPRIBL"
	else:
		# create IBL node
		iblShape = cmds.createNode("RPRIBL", n="RPRIBLShape")
		iblTransform = cmds.listRelatives(iblShape, p=True)[0]
		setProperty(iblTransform, "scale", (1001.25663706144, 1001.25663706144, 1001.25663706144))

	# Logging to file 
	start_log(dome_light, iblShape)
  
	# display IBL option
	exposure = getProperty(dome_light, "exposure")
	intensity = getProperty(dome_light, "intensity")
	setProperty(iblShape, "intensity", intensity * 2 ** exposure)

	# Copy properties from ai dome light
	domeTransform = cmds.listRelatives(dome_light, p=True)[0]
	setProperty(iblTransform, "rotateY", getProperty(domeTransform, "rotateY") + 180)
	
	file = cmds.listConnections(dome_light + ".color")
	if file:
		setProperty(iblTransform, "filePath", getProperty(file[0], "fileTextureName"))
		   
	# Logging to file
	end_log(dome_light)  


def convertaiSky(sky):

	if cmds.objExists("RPRIBL"):
		iblShape = "RPRIBLShape"
		iblTransform = "RPRIBL"
	else:
		# create IBL node
		iblShape = cmds.createNode("RPRIBL", n="RPRIBLShape")
		iblTransform = cmds.listRelatives(iblShape, p=True)[0]
		setProperty(iblTransform, "scale", (1001.25663706144, 1001.25663706144, 1001.25663706144))

	# Logging to file 
	start_log(sky, iblShape)
  
	# Copy properties from ai dome light
	setProperty(iblShape, "intensity", getProperty(sky, "intensity"))

	file = cmds.listConnections(sky + ".color")
	if file:
		setProperty(iblTransform, "filePath", getProperty(file[0], "fileTextureName"))
	 
	# Logging to file
	end_log(sky)  


def convertaiPhysicalSky(sky):
	
	if cmds.objExists("RPRSky"):
		skyNode = "RPRSkyShape"
	else:
		# create RPRSky node
		skyNode = cmds.createNode("RPRSky", n="RPRSkyShape")
  
	# Logging to file
	start_log(sky, skyNode)

	# Copy properties from rsPhysicalSky
	setProperty(skyNode, "turbidity", getProperty(sky, "turbidity"))
	setProperty(skyNode, "intensity", getProperty(sky, "intensity"))
	setProperty(skyNode, "altitude", getProperty(sky, "elevation"))
	setProperty(skyNode, "azimuth", getProperty(sky, "azimuth"))
	setProperty(skyNode, "groundColor", getProperty(sky, "groundAlbedo"))
	setProperty(skyNode, "sunDiskSize", getProperty(sky, "sunSize"))

	# Logging to file
	end_log(sky)  


def convertaiPhotometricLight(ai_light):

	# Arnold light transform
	splited_name = ai_light.split("|")
	aiTransform = "|".join(splited_name[0:-1])
	group = "|".join(splited_name[0:-2])

	if cmds.objExists(aiTransform + "_rpr"):
		rprTransform = aiTransform + "_rpr"
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
	start_log(ai_light, rprLightShape)

	# Copy properties from rsLight
	copyProperty(rprTransform, aiTransform, "translate", "translate")
	setProperty(rprTransform, "rotateX", getProperty(aiTransform, "rotateX") + 90)
	copyProperty(rprTransform, aiTransform, "rotateY", "rotateY")
	copyProperty(rprTransform, aiTransform, "rotateZ", "rotateZ")
	copyProperty(rprTransform, aiTransform, "scale", "scale")

	intensity = getProperty(ai_light, "intensity")
	exposure = getProperty(ai_light, "exposure")
	setProperty(rprLightShape, "intensity", intensity * 2 ** exposure / 1600)

	copyProperty(rprLightShape, ai_light, "color", "color")

	if getProperty(ai_light, "aiUseColorTemperature"):
		temperature_color = convertTemperature(getProperty(ai_light, "aiColorTemperature"))
		setProperty(rprLightShape, "color", temperature_color)

	setProperty(rprLightShape, "iesFile", getProperty(ai_light, "aiFilename"))
	
	# Logging to file
	end_log(ai_light) 


def convertaiAreaLight(ai_light):

	splited_name = ai_light.split("|")
	aiTransform = "|".join(splited_name[0:-1])
	group = "|".join(splited_name[0:-2])

	# Arnold light transform
	if cmds.objExists(aiTransform + "_rpr"):
		rprTransform = aiTransform + "_rpr"
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
	start_log(ai_light, rprLightShape)

	# Copy properties from aiLight
	# Copy properties from aiLight
	setProperty(rprLightShape, "lightType", 0)
	setProperty(rprLightShape, "intensityUnits", 2)

	intensity = getProperty(ai_light, "intensity")
	exposure = getProperty(ai_light, "exposure")
	setProperty(rprLightShape, "intensity", intensity / 160 * 2 ** exposure)

	copyProperty(rprLightShape, ai_light, "color", "color")
	copyProperty(rprLightShape, ai_light, "temperature", "aiColorTemperature")

	if getProperty(ai_light, "aiUseColorTemperature"):
		setProperty(rprLightShape, "colorMode", 1)

	copyProperty(rprLightShape, ai_light, "shadowsSoftness", "aiShadowDensity")
	
	copyProperty(rprTransform, aiTransform, "translate", "translate")
	copyProperty(rprTransform, aiTransform, "rotate", "rotate")
	copyProperty(rprTransform, aiTransform, "scale", "scale")

	# Logging to file
	end_log(ai_light)  


def convertaiMeshLight(ai_light):

	# Arnold light transform
	splited_name = ai_light.split("|")
	aiTransform = "|".join(splited_name[0:-1])
	group = "|".join(splited_name[0:-2])

	if cmds.objExists(aiTransform + "_rpr"):
		rprTransform = aiTransform + "_rpr"
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
	start_log(ai_light, rprLightShape)

	# Copy properties from aiLight
	setProperty(rprLightShape, "lightType", 0)
	setProperty(rprLightShape, "areaLightShape", 4)

	copyProperty(rprLightShape, ai_light, "intensity", "intensity")
	copyProperty(rprLightShape, ai_light, "color", "color")
	if getProperty(ai_light, "aiUseColorTemperature"):
		setProperty(rprLightShape, "colorMode", 1)
	copyProperty(rprLightShape, ai_light, "temperature", "aiColorTemperature")
	copyProperty(rprLightShape, ai_light, "shadowsEnabled", "aiCastShadows")
	copyProperty(rprLightShape, ai_light, "shadowsSoftness", "aiShadowDensity")

	copyProperty(rprLightShape, ai_light, "areaLightVisible", "lightVisible")
	copyProperty(rprTransform, aiTransform, "translate", "translate")
	copyProperty(rprTransform, aiTransform, "rotate", "rotate")
	copyProperty(rprTransform, aiTransform, "scale", "scale")

	try:
		light_mesh = cmds.listConnections(ai_light, type="mesh")[1]
		cmds.delete(ai_light)
		cmds.delete(aiTransform)
		cmds.select(clear=True)
		setProperty(rprLightShape, "areaLightSelectingMesh", 1)
		cmds.select(light_mesh)
	except Exception as ex:
		traceback.print_exc()
		print("Failed to convert mesh in Physical light")

	# Logging to file
	end_log(ai_light)


def convertareaLight(ai_light):

	splited_name = ai_light.split("|")
	aiTransform = "|".join(splited_name[0:-1])
	group = "|".join(splited_name[0:-2])

	# Arnold light transform
	if cmds.objExists(aiTransform + "_rpr"):
		rprTransform = aiTransform + "_rpr"
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
	start_log(ai_light, rprLightShape)

	# Copy properties from aiLight
	setProperty(rprLightShape, "lightType", 0)
	setProperty(rprLightShape, "intensityUnits", 2)

	intensity = getProperty(ai_light, "intensity")
	setProperty(rprLightShape, "intensity", -0.000014 * (intensity ** 3) + 0.000977 * (intensity ** 2) + 0.005465 * intensity + 0.047492)

	copyProperty(rprLightShape, ai_light, "color", "color")
	copyProperty(rprLightShape, ai_light, "temperature", "aiColorTemperature")

	if getProperty(ai_light, "aiUseColorTemperature"):
		setProperty(rprLightShape, "colorMode", 1)

	copyProperty(rprLightShape, ai_light, "shadowsSoftness", "aiShadowDensity")

	copyProperty(rprTransform, aiTransform, "translate", "translate")
	copyProperty(rprTransform, aiTransform, "rotate", "rotate")
	copyProperty(rprTransform, aiTransform, "scale", "scale")

	# Logging to file
	end_log(ai_light)  

	# hide 
	cmds.hide(aiTransform)


def convertspotLight(ai_light):

	splited_name = ai_light.split("|")
	aiTransform = "|".join(splited_name[0:-1])
	group = "|".join(splited_name[0:-2])

	# Arnold light transform
	if cmds.objExists(aiTransform + "_rpr"):
		rprTransform = aiTransform + "_rpr"
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
	start_log(ai_light, rprLightShape)

	# Copy properties from aiLight
	setProperty(rprLightShape, "lightType", 1)
	setProperty(rprLightShape, "intensityUnits", 2)

	scaleX = getProperty(aiTransform, "scaleX")
	scaleY = getProperty(aiTransform, "scaleY")
	intensity = getProperty(ai_light, "intensity")
	exposure = getProperty(ai_light, "aiExposure")
	setProperty(rprLightShape, "intensity", (intensity / 160) * (2 ** exposure) * scaleX * scaleY )

	copyProperty(rprLightShape, ai_light, "color", "color")
	copyProperty(rprLightShape, ai_light, "temperature", "aiColorTemperature")

	if getProperty(ai_light, "aiUseColorTemperature"):
		setProperty(rprLightShape, "colorMode", 1)

	#copyProperty(rprLightShape, ai_light, "spotLightOuterConeFalloff", "penumbraAngle")
	copyProperty(rprLightShape, ai_light, "spotLightOuterConeFalloff", "coneAngle")
	copyProperty(rprLightShape, ai_light, "spotLightInnerConeAngle", "coneAngle")

	copyProperty(rprTransform, aiTransform, "translate", "translate")
	copyProperty(rprTransform, aiTransform, "rotate", "rotate")
	copyProperty(rprTransform, aiTransform, "scale", "scale")

	# Logging to file
	end_log(ai_light)

	# hide 
	cmds.hide(aiTransform)


def convertpointLight(ai_light):

	splited_name = ai_light.split("|")
	aiTransform = "|".join(splited_name[0:-1])
	group = "|".join(splited_name[0:-2])

	# Arnold light transform
	if cmds.objExists(aiTransform + "_rpr"):
		rprTransform = aiTransform + "_rpr"
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
	start_log(ai_light, rprLightShape)

	# Copy properties from aiLight
	setProperty(rprLightShape, "lightType", 2)
	setProperty(rprLightShape, "intensityUnits", 2)

	scaleX = getProperty(aiTransform, "scaleX")
	scaleY = getProperty(aiTransform, "scaleY")
	intensity = getProperty(ai_light, "intensity")
	exposure = getProperty(ai_light, "aiExposure")
	setProperty(rprLightShape, "intensity", (intensity / 160) * (2 ** exposure) * scaleX * scaleY )

	copyProperty(rprLightShape, ai_light, "color", "color")
	copyProperty(rprLightShape, ai_light, "temperature", "aiColorTemperature")

	if getProperty(ai_light, "aiUseColorTemperature"):
		setProperty(rprLightShape, "colorMode", 1)

	copyProperty(rprTransform, aiTransform, "translate", "translate")
	copyProperty(rprTransform, aiTransform, "rotate", "rotate")
	copyProperty(rprTransform, aiTransform, "scale", "scale")

	# Logging to file
	end_log(ai_light)

	# hide 
	cmds.hide(aiTransform)


def convertdirectionalLight(ai_light):

	splited_name = ai_light.split("|")
	aiTransform = "|".join(splited_name[0:-1])
	group = "|".join(splited_name[0:-2])

	# Arnold light transform
	if cmds.objExists(aiTransform + "_rpr"):
		rprTransform = aiTransform + "_rpr"
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
	start_log(ai_light, rprLightShape)

	# Copy properties from aiLight
	setProperty(rprLightShape, "lightType", 3)
	setProperty(rprLightShape, "intensityUnits", 1)

	intensity = getProperty(ai_light, "intensity")
	exposure = getProperty(ai_light, "aiExposure")
	setProperty(rprLightShape, "intensity", intensity  * 2 ** exposure )

	copyProperty(rprLightShape, ai_light, "color", "color")
	copyProperty(rprLightShape, ai_light, "temperature", "aiColorTemperature")

	if getProperty(ai_light, "aiUseColorTemperature"):
		setProperty(rprLightShape, "colorMode", 1)

	copyProperty(rprTransform, aiTransform, "translate", "translate")
	copyProperty(rprTransform, aiTransform, "rotate", "rotate")
	copyProperty(rprTransform, aiTransform, "scale", "scale")

	# Logging to file
	end_log(ai_light)

	# hide 
	cmds.hide(aiTransform)


def convertaiAtmosphere(aiAtmosphere):

	# Creating new Volume material
	rprMaterial = cmds.shadingNode("RPRVolumeMaterial", asShader=True)
	rprMaterial = cmds.rename(rprMaterial, aiAtmosphere + "_rpr")
	
	sg = rprMaterial + "SG"
	cmds.sets(renderable=True, noSurfaceShader=True, empty=True, name=sg)
	connectProperty(rprMaterial, "outColor", sg, "volumeShader")

	# create sphere
	cmds.polySphere(n="Volume")
	setProperty("Volume", "scale", (2000, 2000, 2000))

	# assign material
	cmds.select("Volume")
	cmds.sets(e=True, forceElement=sg)

	# Logging to file 
	start_log(aiAtmosphere, rprMaterial) 

	# Fields conversion
	setProperty(rprMaterial, "multiscatter", 0)

	aiAtmosphereType = cmds.objectType(aiAtmosphere)
	if aiAtmosphereType == "aiFog":
		copyProperty(rprMaterial, aiAtmosphere, "emissionColor", "color")
		avg_color = getProperty(aiAtmosphere, "color") / 3.0
		setProperty(rprMaterial, "density", avg_color)
	elif aiAtmosphereType == "aiAtmosphereVolume":
		copyProperty(rprMaterial, aiAtmosphere, "density", "density")
		copyProperty(rprMaterial, aiAtmosphere, "scatterColor", "rgbDensity")
		copyProperty(rprMaterial, aiAtmosphere, "transmissionColor", "rgbDensity")
		copyProperty(rprMaterial, aiAtmosphere, "scatteringDirection", "eccentricity")
	
	# Logging to file
	end_log(aiAtmosphere)  


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
def convertMaterial(aiMaterial, source):

	ai_type = cmds.objectType(aiMaterial)

	conversion_func = {

		# Arnold materials
		"aiAmbientOcclusion": convertaiAmbientOcclusion,
		"aiCarPaint": convertaiCarPaint,
		"aiFlat": convertaiFlat,
		"aiLayerShader": convertaiLayerShader,
		"aiMatte": convertaiMatte,
		"aiMixShader": convertaiMixShader,
		"aiPassthrough": convertaiPassthrough,
		"aiRaySwitch": convertUnsupportedMaterial,
		"aiShadowMatte": convertaiShadowMatte,
		"aiStandardHair": convertUnsupportedMaterial,
		"aiStandardSurface": convertaiStandardSurface,
		"aiSwitch": convertUnsupportedMaterial,
		"aiToon": convertaiToon,
		"aiTwoSided": convertUnsupportedMaterial,
		"aiUtility": convertUnsupportedMaterial,
		"aiWireframe": convertUnsupportedMaterial,
		"aiStandardVolume": convertaiStandardVolume,

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

		# Arnold utilities
		"aiBump2d": convertaiBump2d,
		"aiBump3d": convertaiBump3d,
		"aiNormalMap": convertaiNormalMap,
		"aiVectorMap": convertaiVectorMap,
		"aiAdd": convertaiAdd,
		"aiMultiply": convertaiMultiply,
		"aiDivide": convertaiDivide,
		"aiSubtract": convertaiSubstract,
		"aiAbs": convertaiAbs,
		"aiAtan": convertaiAtan,
		"aiCross": convertaiCross,
		"aiDot": convertaiDot,
		"aiPow": convertaiPow,
		"aiTrigo": convertaiTrigo,
		"aiComposite": convertaiComposite,
		"aiSqrt": convertaiSqrt,
		"aiNegate": convertaiNegate,
		"aiImage": convertaiImage,
		"aiFacingRatio": convertaiFacingRatio,
		"aiThinFilm": convertaiThinFilm,
		"aiColorConvert": convertaiColorConvert,
		"aiCellNoise": convertaiCellNoise,
		"aiNoise": convertaiNoise,
		"aiBlackbody": convertaiBlackbody,
		"aiCurvature": convertaiCurvature,
		"aiColorCorrect": convertaiColorCorrect,
		"aiTriplanar": convertaiTriplanar,
		"aiUvTransform": convertaiUvTransform,
		"aiLength": convertaiLength,
		"aiExp": convertaiExp
	}

	if ai_type in conversion_func:
		rpr = conversion_func[ai_type](aiMaterial, source)
	else:
		if isArnoldType(aiMaterial):
			rpr = convertUnsupportedNode(aiMaterial, source)
		else:
			rpr = convertStandardNode(aiMaterial, source)

	return rpr


# Convert light. Returns new light name.
def convertLight(light):

	ai_type = cmds.objectType(light)

	conversion_func = {
		"aiAreaLight": convertaiAreaLight,
		"aiMeshLight": convertaiMeshLight,
		"aiPhotometricLight": convertaiPhotometricLight,
		"aiSkyDomeLight": convertaiSkyDomeLight,
		#"aiPortalLight": convertaiPortalLight,
		"areaLight": convertareaLight,
		"spotLight": convertspotLight,
		"pointLight": convertpointLight,
		"directionalLight": convertdirectionalLight
	}

	conversion_func[ai_type](light)


def isArnoldType(obj):

	if cmds.objExists(obj):
		objType = cmds.objectType(obj)
		if objType.startswith("ai"):
			return 1
	return 0


def cleanScene():

	listMaterials= cmds.ls(materials=True)
	for material in listMaterials:
		if isArnoldType(material):
			shEng = cmds.listConnections(material, type="shadingEngine")
			try:
				cmds.delete(shEng[0])
				cmds.delete(material)
			except:
				pass

	listLights = cmds.ls(l=True, type=["aiAreaLight", "aiMeshLight", "aiPhotometricLight", "aiSkyDomeLight", "areaLight", "spotLight", "pointLight", "directionalLight"])
	for light in listLights:
		transform = cmds.listRelatives(light, p=True)[0]
		try:
			cmds.delete(light)
			cmds.delete(transform)
		except:
			pass

	listObjects = cmds.ls(l=True)
	for obj in listObjects:
		if isArnoldType(object):
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

	if isArnoldType(material):
		materialSG = cmds.listConnections(material, type="shadingEngine")
		if materialSG:
			cmds.hyperShade(objects=material)
			assigned = cmds.ls(sl=True)
			if assigned:
				return 1
	return 0


def defaultEnable(RPRmaterial, aiMaterial, enable, value):

	weight = getProperty(aiMaterial, value)
	if weight > 0:
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

	# Repath paths in scene files (filePathEditor)
	repathScene()

	# Check plugins
	if not cmds.pluginInfo("mtoa", q=True, loaded=True):
		try:
			cmds.loadPlugin("mtoa", quiet=True)
		except Exception as ex:
			response = cmds.confirmDialog(title="Error",
							  message=("Arnold plugin is not installed.\nInstall Arnold plugin before conversion."),
							  button=["OK"],
							  defaultButton="OK",
							  cancelButton="OK",
							  dismissString="OK")
			exit("Arnold plugin is not installed")

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

	# arnold engine set before conversion 
	setProperty("defaultRenderGlobals", "currentRenderer", "arnold")

	# disable cache
	maya_version = cmds.about(apiVersion=True)
	if maya_version > 20190200:
	    cmds.evaluator(n="cache", en=False)

	# Convert aiAtmosphere
	env = cmds.ls(type=("aiAtmosphereVolume", "aiFog"))
	if env:
		try:
			convertaiAtmosphere(env[0])
		except Exception as ex:
			traceback.print_exc()
			print("Error while converting Atmosphere. ")

	# Convert ArnoldPhysicalSky
	sky = cmds.ls(type=("aiPhysicalSky", "aiSky"))
	if sky:
		try:
			sky_type = cmds.objectType(sky[0])
			conversion_func_sky = {
				"aiPhysicalSky": convertaiPhysicalSky,
				"aiSky": convertaiSky
			}
			conversion_func_sky[sky_type](sky[0])
		except Exception as ex:
			traceback.print_exc()
			print("Error while converting physical sky. \n")

	
	# Get all lights from scene
	listLights = cmds.ls(l=True, type=["aiAreaLight", "aiMeshLight", "aiPhotometricLight", "aiSkyDomeLight", "areaLight", "spotLight", "pointLight", "directionalLight"])

	# Convert lights
	for light in listLights:
		try:
			convertLight(light)
		except Exception as ex:
			traceback.print_exc()
			print("Error while converting {} light. \n".format(light))
		

	# Get all materials from scene
	listMaterials = cmds.ls(materials=True)
	materialsDict = {}
	for each in listMaterials:
		if checkAssign(each):
			materialsDict[each] = convertMaterial(each, "")

	for ai, rpr in materialsDict.items():
		try:
			cmds.hyperShade(objects=ai)
			rpr_sg = cmds.listConnections(rpr, type="shadingEngine")[0]
			cmds.sets(e=True, forceElement=rpr_sg)
		except Exception as ex:
			traceback.print_exc()
			print("Error while converting {} material. \n".format(ai))
	
	setProperty("defaultRenderGlobals", "currentRenderer", "FireRender")
	setProperty("defaultRenderGlobals", "imageFormat", 8)

	setProperty("RadeonProRenderGlobals", "completionCriteriaSeconds", 0)
	if getProperty("defaultArnoldRenderOptions", "enableAdaptiveSampling"):
		copyProperty("RadeonProRenderGlobals", "defaultArnoldRenderOptions", "adaptiveThreshold", "AAAdaptiveThreshold")
	aa_sample = getProperty("defaultArnoldRenderOptions", "AASamples")
	if aa_sample < 1:
		setProperty("RadeonProRenderGlobals", "completionCriteriaIterations", 500)
	else:
		setProperty("RadeonProRenderGlobals", "completionCriteriaIterations", 1000 * aa_sample)

	filter_type = getProperty("defaultArnoldFilter", "aiTranslator")
	if filter_type == "box":
		setProperty("RadeonProRenderGlobals", "filter", 1)
	elif filter_type == "triangle":
		setProperty("RadeonProRenderGlobals", "filter", 2)
	elif filter_type == "gaussian":
		setProperty("RadeonProRenderGlobals", "filter", 3)
	elif filter_type == "blackman_harris":
		setProperty("RadeonProRenderGlobals", "filter", 6)
	copyProperty("RadeonProRenderGlobals", "defaultArnoldFilter", "filterSize", "width")
	copyProperty("RadeonProRenderGlobals", "defaultArnoldRenderOptions", "maxRayDepth", "GITotalDepth")
	
	# camera settings
	cameras = cmds.ls(type="camera")
	for cam in cameras:
	    if getProperty(cam, "renderable"):
	    	if getProperty(cam, "aiEnableDOF"):
		        setProperty(cam, "depthOfField", 1)
		        copyProperty(cam, cam, "focusDistance", "aiFocusDistance")
		        fStop = getProperty(cam, "aiApertureSize") * 1000
		        if fStop > 64:
		        	setProperty(cam, "fStop", 64)
		        else:
		        	setProperty(cam, "fStop", fStop)

	matteShadowCatcher = cmds.ls(materials=True, type="aiShadowMatte")
	if matteShadowCatcher:
		try:
			setProperty("RadeonProRenderGlobals", "aovOpacity", 1)
			setProperty("RadeonProRenderGlobals", "aovBackground", 1)
			setProperty("RadeonProRenderGlobals", "aovShadowCatcher", 1)
		except Exception as ex:
			traceback.print_exc()

	# enable cache back
	maya_version = cmds.about(apiVersion=True)
	if maya_version > 20190200:
	    cmds.evaluator(n="cache", en=True)



def auto_launch():
	convertScene()
	cleanScene()

def manual_launch():
	print("Convertion start!")
	startTime = 0
	testTime = 0
	startTime = time.time()
	convertScene()
	testTime = time.time() - startTime
	print("Conversion was finished! Elapsed time: {}".format(round(testTime, 3)))

	response = cmds.confirmDialog(title="Completed",
							  message=("Scene conversion took {} seconds.\nWould you like to delete all Arnold objects?".format(round(testTime, 3))),
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
