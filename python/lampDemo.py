#!/usr/bin/env python
# -*- coding: utf-8 -*-
#############################################################################
# lampDemo
#
# Sample script to play with the lamp
#
# Not all firmware commands supported in this script, this is just an indication of how
# to interact with the lamp using Python and the spyrk library, however you can use the
# --raw command to issue any command to the lamp
#
#################################################################################

#################################################################################
# Import modules
#################################################################################

# import everything required here, not somewhere else in the source file
# only import modules which are required
import os
import sys
import random
from argparse import ArgumentParser

try:
    from spyrk import SparkCloud
except ImportError:
    print "This code requires the spyrk module. You can install it with \"pip install spyrk\""
    exit()
    
def isANumber(s):
    try:
        float(s)
        return True
    except ValueError:
        pass
        
    return False
    
# Shortcut way to handle any command, by entering it directly
# e.g. lampDemo.py -a <key> -d <dev_name> --raw COLOUR SET 20 30 40    is equivalent to
#      lampDemp.py -a <key> -d <dev_name> -c 20 30 40
#
# Use the --raw command to issue commands which don't have shortcut commands implemented in the python wrapper yet
#
def runRawCommand(lamp, rawCommand):

    # first command word determines what the raw command is
    # we understand "colour", "admin", "pulse"
    # Pop it off the list, leaving the rest of the command in rawCommand
    
    cmd = rawCommand.pop(0).upper()
    cmdArgs = ' '.join(rawCommand);
    
    if cmd == "COLOUR":
        lamp.colour(cmdArgs)
    elif cmd == "PULSE":
        lamp.pulse(cmdArgs)
    elif cmd == "ADMIN":
        lamp.admin(cmdArgs)
    else:
        print "Didn't understand raw command {com} {args}".format(com=cmd, args=cmdArgs)
        return
        
    return
    
def runPastelColourAlgorithm(lamp):
    # Setting a colour is fairly slow (via the Spark cloud)
    # So no need for a delay in the loops here
    redMix = 4095
    blueMix = 4095
    greenMix = 4095
    
    # 256 colours ...
    
    for i in range(256):    
        red = random.randint(0,4095)
        green = random.randint(0,4095)
        blue = random.randint(0,4095)

        # mix the color
        red = (red + redMix) / 2
        green = (green + blueMix) / 2
        blue = (blue + greenMix) / 2
    
        print "RGB: {r} {g} {b}".format(r=red, g=green, b=blue)
        
        lamp.colour("SET {r} {g} {b}".format(r=red,
                                             g=green,
                                             b=blue))
        
    return 0
   
def runRandomColourAlgorithm(lamp):
    # Setting a colour is fairly slow (via the Spark cloud)
    # So no need for a delay in the loops here  
    # Setting a colour is fairly slow (via the Spark cloud)
    # So no need for a delay in the loops here

    # 256 colours ...
    
    for i in range(256):    
        red = random.randint(0,4095)
        green = random.randint(0,4095)
        blue = random.randint(0,4095)
    
        print "RGB: {r} {g} {b}".format(r=red, g=green, b=blue)
        
        lamp.colour("SET {r} {g} {b}".format(r=red,
                                             g=green,
                                             b=blue))
            
    return
  
def runWalkColourAlgorithm(lamp):
    # Setting a colour is fairly slow (via the Spark cloud)
    # So no need for a delay in the loops here  

    # we'll do a rough walk of the vertices of the RGB colour cube.
    # Gives an idea of the colours which are possible
    # Can still play with the intensity a lot more to explore the full range of colours
    
    red = 0
    green = 0
    blue = 0
    
    for blue in range (0, 4100, 128):
        print "RGB: {r} {g} {b}".format(r=red, g=green, b=blue)
        lamp.colour("SET {r} {g} {b}".format(r=red,
                                             g=green,
                                             b=blue))
    for green in range (0, 4100, 128):
        print "RGB: {r} {g} {b}".format(r=red, g=green, b=blue)
        lamp.colour("SET {r} {g} {b}".format(r=red,
                                             g=green,
                                             b=blue))
                                             
    for blue in range (4100, 0, -128):
        print "RGB: {r} {g} {b}".format(r=red, g=green, b=blue)
        lamp.colour("SET {r} {g} {b}".format(r=red,
                                             g=green,
                                             b=blue))   

    for red in range (0, 4100, 128):
        print "RGB: {r} {g} {b}".format(r=red, g=green, b=blue)
        lamp.colour("SET {r} {g} {b}".format(r=red,
                                             g=green,
                                             b=blue))                                                
 
    for green in range (4100, 0, -128):
        print "RGB: {r} {g} {b}".format(r=red, g=green, b=blue)
        lamp.colour("SET {r} {g} {b}".format(r=red,
                                             g=green,
                                             b=blue)) 
                                             
    for blue in range (0, 4100, 128):
        print "RGB: {r} {g} {b}".format(r=red, g=green, b=blue)
        lamp.colour("SET {r} {g} {b}".format(r=red,
                                             g=green,
                                             b=blue))
                                             
    for green in range (0, 4100, 128):
        print "RGB: {r} {g} {b}".format(r=red, g=green, b=blue)
        lamp.colour("SET {r} {g} {b}".format(r=red,
                                             g=green,
                                             b=blue))
                                             
    return

def runDimWalkColourAlgorithm(lamp):
    # Setting a colour is fairly slow (via the Spark cloud)
    # So no need for a delay in the loops here  

    # we'll do a rough walk of the vertices of the RGB colour cube.
    # Gives an idea of the colours which are possible
    # Can still play with the intensity a lot more to explore the full range of colours
    
    red = 0
    green = 0
    blue = 0
    maxVal = 400
    
    for blue in range (0, maxVal, maxVal/20):
        print "RGB: {r} {g} {b}".format(r=red, g=green, b=blue)
        lamp.colour("SET {r} {g} {b}".format(r=red,
                                             g=green,
                                             b=blue))
    for green in range (0, maxVal, maxVal/20):
        print "RGB: {r} {g} {b}".format(r=red, g=green, b=blue)
        lamp.colour("SET {r} {g} {b}".format(r=red,
                                             g=green,
                                             b=blue))
                                             
    for blue in range (maxVal, 0, -maxVal/20):
        print "RGB: {r} {g} {b}".format(r=red, g=green, b=blue)
        lamp.colour("SET {r} {g} {b}".format(r=red,
                                             g=green,
                                             b=blue))   

    for red in range (0, maxVal, maxVal/20):
        print "RGB: {r} {g} {b}".format(r=red, g=green, b=blue)
        lamp.colour("SET {r} {g} {b}".format(r=red,
                                             g=green,
                                             b=blue))                                                
 
    for green in range (maxVal, 0, -maxVal/20):
        print "RGB: {r} {g} {b}".format(r=red, g=green, b=blue)
        lamp.colour("SET {r} {g} {b}".format(r=red,
                                             g=green,
                                             b=blue)) 
                                             
    for blue in range (0, maxVal, maxVal/20):
        print "RGB: {r} {g} {b}".format(r=red, g=green, b=blue)
        lamp.colour("SET {r} {g} {b}".format(r=red,
                                             g=green,
                                             b=blue))
                                             
    for green in range (0, maxVal, maxVal/20):
        print "RGB: {r} {g} {b}".format(r=red, g=green, b=blue)
        lamp.colour("SET {r} {g} {b}".format(r=red,
                                             g=green,
                                             b=blue))
                                             
    return
    
# Command line arg handler for this script
def handleArguments():
    """
    lampDemo: This script shows basic playing with the lamp   
    """
    parser = ArgumentParser(description='Simple lamp control script')
    
    # Specify the access token
    parser.add_argument(
        '--access','-a',
        required='true',
        help='The authentication token for your Particle account')
      
    # Specify the device to control
    parser.add_argument(
        '--device', '-d',
        required='true',
        help='The device name for the lamp to control')
        
    # Reboot the lamp
    parser.add_argument(
        '--reset', '-x',
        action='store_true',
        help='Reboot the lamp')
    
    # Switch LED control to manual/auto : by default led control is enabled
    ledman_auto = parser.add_mutually_exclusive_group(required=False)
    ledman_auto.add_argument(
        '--manual',
        dest='ledcontrol',
        action='store_true',
        help='Allow manual control of the lamp colour (normal operation)')
    ledman_auto.add_argument(
        '--auto',
        dest='ledcontrol',
        action='store_false',
        help='Disable manual control of the lamp colour (debug operation)')
    parser.set_defaults(ledcontrol=True)

    parser.add_argument(
        '--raw',
        nargs='+',
        help='Sends a command verbatim to the lamp, if recognised\ne.g --raw COLOUR RAMP 35 0 100')
    
    # Set the colour of the lamp (RGB)
    parser.add_argument(
        '--colour', '-c',
        nargs='+',
        help='Set the lamp colour to a specific R G B value')
        
    parser.add_argument(
        '--random',
        action='store_true',
        help='Run the lamp through some random colour sequences')
 
    parser.add_argument(
        '--pastel',
        action='store_true',
        help='Run the lamp through some pastel colour sequences')

    parser.add_argument(
        '--walk',
        action='store_true',
        help='Run the lamp through the RGB colour space')
        
    parser.add_argument(
        '--dimwalk',
        action='store_true',
        help='Run the lamp through the RGB colour space')
        
    parser.add_argument(
        '--pulse', '-p',
        help='Fade the lamp colour in/out')
        
    # Check the command line and build the argument list
    parsed_args = parser.parse_args()
    
    return parsed_args

    
def main(argv):
    """
    Simple command line interface to a photon-controlled RGB lamp
    """

    # Parse command line
    parsed_args = handleArguments()

    # Connect to the Particle Cloud and fetch our device control point
    spark = SparkCloud(parsed_args.access)
    
    try:
        lamp = spark.devices[parsed_args.device]
    except KeyError:
        print "Could not find device {dev} in account {acc}. Check naming of your lamp".format(dev=parsed_args.device,acc=parsed_args.access)
    except Exception as ex:
        print ex
        exit()
    
    # OK, there's a device with this name in this account
    # Doesn't mean it's online though ...
    if not lamp.connected:
        print "Lamp {dev} found in this account, but it is not online so we can't control it". format(dev=parsed_args.device)
        exit()
        
    # When we get to here, there is a lamp, we found it, and it's online, so we can control it
    # We don't assume that you passed just a single command so we don't exit after finding one
    if parsed_args.random:
        runRandomColourAlgorithm(lamp)
    
    if parsed_args.pastel:
        runPastelColourAlgorithm(lamp)
        
    if parsed_args.dimwalk:
        runDimWalkColourAlgorithm(lamp)

    if parsed_args.walk:
        runWalkColourAlgorithm(lamp)

    # Set the lamp into pulse mode (or turn pulse off) 
    if parsed_args.pulse:
        if isANumber(parsed_args.pulse):
            lamp.pulse("PERIOD {p}".format(p=parsed_args.pulse))
        else:
            lamp.pulse(parsed_args.pulse)
        
    # Change the manual/automatic control of the lamp. This is really only useful to see amusing
    # lamp colours during flashing
    if parsed_args.ledcontrol:
        lamp.admin("led manual")
    else:
        lamp.admin("led auto")
                
    # Set RGB colour
    if parsed_args.colour is not None:
        if len(parsed_args.colour) != 3:
            print "You must specify colour as R G B (three numbers)"
        else:
            lamp.colour("SET {r} {g} {b}".format(r=parsed_args.colour[0], 
                                                 g=parsed_args.colour[1], 
                                                 b=parsed_args.colour[2]))
           
    # Handle a raw admin command: just send verbatim to the lamp
    # We should really make a wrapper command for 
    if parsed_args.raw is not None:
        runRawCommand(lamp, parsed_args.raw)
        
    # Reboot if we asked for this
    if parsed_args.reset:
        lamp.admin("reboot")

    return 0

if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))



