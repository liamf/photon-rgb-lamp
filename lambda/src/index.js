'use strict';

/**
 * This is a very simple example of using the Amazon Smart Home "light" API to control the Particle "Pixel" lamp 
 * It implements power on/off, dimming, and colour control
 * 
 * For more information about developing smart home skills, see
 *  https://developer.amazon.com/alexa/smart-home
 *
 * For details on the smart home API, please visit
 *  https://developer.amazon.com/public/solutions/alexa/alexa-skills-kit/docs/smart-home-skill-api-reference
 *
 * Free use for any purpose without restriction.
 *
 * (c) Liam Friel 2018
 */

var httpRequest = require('request');

var particleServer = "https://api.particle.io";
var particlePath = "/v1/devices/";

/**
 * Hard coded data for devices to be discovered : we have a single one at the moment
 * If we had more, we'd have to be smarter about this ... 
 *
 * For more information on the discovered appliance response please see
 *  https://developer.amazon.com/public/solutions/alexa/alexa-skills-kit/docs/smart-home-skill-api-reference#discoverappliancesresponse
 */
const LIAMS_SMART_DEVICE = [
    {
        // This id needs to be unique across all devices discovered for a given manufacturer
        endpointId: '51d5fe69-5a50-405b-be83-dcd51dfaa482',
        friendlyName: 'Attic light',
        description: 'Attic orb lamp with colour control',
        manufacturerName: 'Liam',
        displayCategories: ['LIGHT'],
        cookie: {
            // This is the Particle Device ID of this lamp (you'd need to fix this to be the ID of a real lamp you can control)
            deviceId: '3a001234550647343138xxxxx',
            // This is the root access token for the account user
            // For some reason the OAUTH issued tokens don't seem to work (permission denied) and the reason is not understood yet
            rootToken: '5474c0123456789015e60e34f1a1',
        },
        capabilities: [
            {
                type: 'AlexaInterface',
                interface: 'Alexa.PowerController',
                version: '3',
                properties: {
                    supported: [{name: 'powerState'}]
                },
                proactivelyReported: true,
                retrievable: true,
            },
            {
                type: 'AlexaInterface',
                interface: 'Alexa.BrightnessController',
                version: '3',
                properties: {
                    supported: [{name: 'brightness'}]
                },
                proactivelyReported: true,
                retrievable: true,
            },
            {
                type: 'AlexaInterface',
                interface: 'Alexa.ColorController',
                version: '3',
                properties: {
                    supported: [{name: 'color'}]
                },
                proactivelyReported: true,
                retrievable: true,
            },
        ],
    },
];


/**
 * Utility functions
 */
const RGB_MAX = 4095
const HUE_MAX = 360
const SV_MAX = 100

function _normalizeAngle (degrees) {
	  return (degrees % 360 + 360) % 360;
}

// Convert from Alexa HSB model to the Pixel RGB	
function HSBtoRGB(h, s, v) {
	if (typeof h === 'object') {
	    const args = h
	    h = args.h; s = args.s; v = args.v;
	}

	// Alexa passed in range 0-1.
	s = s * 100;
	v = v * 100;
	
	h = _normalizeAngle(h)
	
	h = (h === HUE_MAX) ? 1 : (h % HUE_MAX / parseFloat(HUE_MAX) * 6)
	s = (s === SV_MAX) ? 1 : (s % SV_MAX / parseFloat(SV_MAX))
	v = (v === SV_MAX) ? 1 : (v % SV_MAX / parseFloat(SV_MAX))

	var i = Math.floor(h)
	var f = h - i
	var p = v * (1 - s)
	var q = v * (1 - f * s)
	var t = v * (1 - (1 - f) * s)
	var mod = i % 6
	var r = [v, q, p, p, t, v][mod]
	var g = [t, v, v, q, p, p][mod]
	var b = [p, p, t, v, v, q][mod]

	return {
	    r: Math.floor(r * RGB_MAX),
	    g: Math.floor(g * RGB_MAX),
	    b: Math.floor(b * RGB_MAX),
	  }
}

function log(title, msg) {
    console.log(`[${title}] ${msg}`);
}

/**
 * Generate a unique message ID
 *
 * From https://stackoverflow.com/questions/105034/create-guid-uuid-in-javascript.
 */
function generateMessageID() {
  return 'xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx'.replace(/[xy]/g, function(c) {
    var r = Math.random() * 16 | 0, v = c == 'x' ? r : (r & 0x3 | 0x8);
    return v.toString(16);
  });
}

/**
 * Generate a response message
 *
 * @param {string} name - Directive name
 * @param {Object} payload - Any special payload required for the response
 * @returns {Object} Response object
 */
function generateControlResponse(request, payload) {
    
    var now = new Date();
    
    log( 'DEBUG', 'Building Control Response');
    
    var props = {
            namespace: request.directive.header.namespace,
            timeOfSample: now.toISOString(),
            uncertaintyInMilliseconds: 500 
    }
    
    // Add in the command specific fields
    switch(request.directive.header.name)
    {
        case 'TurnOn':
            props.name = 'powerState';
            props.value = 'ON';
            break;
            
        case 'TurnOff':
        	props.name = 'powerState';
        	props.value = 'OFF';
            break;            
           
        // This is only correct for the "set" case, not the "dim" case and will throw an error if we used
        // the "dim" command (so while it will work, Alexa won't think it worked)
        // To make the "dim" case work, we'd need to query the Particle after setting the value (or before ..)
        // for the power level. We can do this (it is an exposed variable) but we don't do it currently
        case 'SetBrightness':
        	props.name = 'brightness';
        	props.value = request.directive.payload.brightness;        	
        	break;
        	
        case 'SetColor':
        	props.name = 'color';
        	props.value = {
        		hue: request.directive.payload.color.hue, 
        		saturation: request.directive.payload.color.saturation,
        		brightness: request.directive.payload.color.brightness, 
        	}
            break;
    }
 
    var context = {
            properties: [props]    
    }
    
    var event = {
        header: {
            namespace: 'Alexa',
            name: 'Response',
            payloadVersion: '3',
            messageId: generateMessageID(),
            correlationToken: request.directive.header.correlationToken,
        },
        endpoint: {
            scope: {
                type: 'BearerToken',
                token: request.directive.endpoint.scope.token
            },
            endpointId: request.directive.endpoint.endpointId
        },
        payload: payload
    }
    
    // Combine and return
    var response = {
        context: context,
        event: event
    };     
    
    return response;
}

function generateControlError(name, code, description) {
    var headers = {
        namespace: 'Control',
        name: name,
        payloadVersion: '3'
    };

    var payload = {
        exception: {
            code: code,
            description: description
        }
    };

    var result = {
        header: headers,
        payload: payload
    };

    return result;
}

/**
 * Mock functions to access device cloud.
 *
 * TODO: Pass a user access token and call cloud APIs in production.
 */

function getDevicesFromPartnerCloud() {
    /**
     * For the purposes of this simple example, we return our array of hardcoded
     * devices above: currently, the attic particle light only
     * 
     * Otherwise we *could* query the particle API, but we have more devices
     * than we can control so that would be harder and not really gain us anything
     * 
     */
    return LIAMS_SMART_DEVICE;
}

function setLightColour( commandArgs, request, context) {
	
    const deviceId = request.directive.endpoint.cookie.deviceId;
    const rootToken = request.directive.endpoint.cookie.rootToken
    const accessToken = request.directive.endpoint.scope.token;
    
    var respCallback = function( error, httpResponse, body) {

        if( error)
        {
            context.fail(error);
        }
        else
        {
        	let resp = generateControlResponse(request,{})
        	log('DEBUG', 'Successful Particle Command!')
        	log('DEBUG', JSON.stringify(resp))
            context.succeed(resp)
        }
    };
    
    // The Particle API command data 
    var postData = {
        url: particleServer + particlePath + deviceId + "/" + "colour",
        form: {
        	access_token: rootToken,
            args: commandArgs,
        },
    };

    log( 'DEBUG', "command is " + JSON.stringify(postData))
    httpRequest.post(postData, respCallback)	
}

// Turn on the light: white colour, full on
function turnOn(request, context) {

	let commandArgs = "SET 4095 4095 4095"		
	setLightColour(commandArgs, request, context)

}

// Turn off the light
function turnOff(request, context) {

	let commandArgs = "SET 0 0 0"		
	setLightColour(commandArgs, request, context)
}

function setColor(request, context) {

    const hue = request.directive.payload.color.hue;
    const saturation = request.directive.payload.color.saturation; 
    const brightness = request.directive.payload.color.brightness;
     
    // Convert and set the colour command for the Pixel
    let converted = HSBtoRGB(hue, saturation, brightness);
   
    let commandArg = `SET ${converted.r} ${converted.g} ${converted.b}`;    
    
    setLightColour(commandArg, request, context)
}

// This is also a variety of "colour" command
function setBrightness(request, context) {

	let subcommand;
	
	switch (request.directive.header.name) {
    	case 'SetBrightness': 
    		subcommand = `SET ${request.directive.payload.brightness}`;
    		break;
    		
    	case 'AdjustBrightness': 
    		subcommand = `DIM ${request.directive.payload.brightnessDelta}`;
    		break;
    		
	}
	
    let commandArg = `LEVEL ${subcommand}`;  
    
    setLightColour(commandArg, request, context)
}


/**
 * Main logic
 */

/**
 * This function is invoked when we receive a "Discovery" message from Alexa Smart Home Skill.
 * We are expected to respond back with a list of appliances that we have discovered for a given customer.
 *
 */
function handleDiscovery(request, context) {
    log('DEBUG', `Discovery Request: ${JSON.stringify(request)}`);

    /**
     * This is just hard coded: no need to query the cloud
     *
     * For more information on a discovery response see
     *  https://developer.amazon.com/public/solutions/alexa/alexa-skills-kit/docs/smart-home-skill-api-reference#discoverappliancesresponse
     */
    const response = {
        event: {
            header: {
            namespace: 'Alexa.Discovery',
            name: 'Discover.Response',
            payloadVersion: '3',
            messageId: request.directive.header.messageId,
            },
            payload: {
                endpoints: getDevicesFromPartnerCloud(),
            },
        },
    };

    /**
     * Log the response. These messages will be stored in CloudWatch.
     */
    log('DEBUG', `Discovery Response: ${JSON.stringify(response)}`);

    /**
     * Return result with successful message.
     */
    context.succeed(response);
}

/**
 * A function to handle control events.
 * This is called when Alexa requests an action such as turning off an appliance.
 *
 * @param {Object} request - The full request object from the Alexa smart home service.
 * @param {function} callback - The callback object on which to succeed or fail the response.
 */
function handleControl(request, context) {
    log('DEBUG', `Control Request: ${JSON.stringify(request)}`);

    /**
     * Grab the applianceId from the request.
     */
    const deviceId = request.directive.endpoint.cookie.deviceID;
    const accessToken = request.directive.endpoint.scope.token;
    
    // We ignore the "namespace" : the name is unique for this simple Skill
    switch (request.directive.header.name) {
        case 'TurnOn':
            turnOn(request, context);
            break;

        case 'TurnOff':
            turnOff(request, context);
            break;

        case 'SetBrightness': 
        case 'AdjustBrightness':
            setBrightness(request, context);
            break;

        case 'SetColor': {
            setColor(request, context);
            break;
        }

        default: {
            log('ERROR', `No supported directive name: ${request.header.name}`);
            return;
        }
    }

}


/**
 * Main entry point.
 * Incoming events from Alexa service through Smart Home API are all handled by this function.
 *
 * It is recommended to validate the request and response with Alexa Smart Home Skill API Validation package.
 *  https://github.com/alexa/alexa-smarthome-validation
 */
exports.handler = (request, context) => {
    
    // Dump out the messages received
    log('DEBUG',JSON.stringify(request));
    log('DEBUG', JSON.stringify(context));
    
    
    switch (request.directive.header.namespace) {
        /**
         * The namespace of 'Alexa.Discovery' indicates a request is being made to the Lambda for
         * discovering all appliances associated with the customer's appliance cloud account.
         *
         * For more information on device discovery, please see
         *  https://developer.amazon.com/public/solutions/alexa/alexa-skills-kit/docs/smart-home-skill-api-reference#discovery-messages
         */
        case 'Alexa.Discovery':
            handleDiscovery(request, context);
            break;

        /**
         * We look at the namespace to determine the command sent
         *  https://developer.amazon.com/public/solutions/alexa/alexa-skills-kit/docs/smart-home-skill-api-reference#payload
         */
        case 'Alexa.PowerController':
        case 'Alexa.BrightnessController':
        case 'Alexa.ColorController':
            handleControl(request, context);
            break;

        /**
         * The namespace of "Alexa.Query" indicates a request is being made to query devices about
         * information like temperature or lock state. The full list of Query events sent to your lambda are described below.
         *  https://developer.amazon.com/public/solutions/alexa/alexa-skills-kit/docs/smart-home-skill-api-reference#payload
         *
         * TODO: In this sample, query handling is not implemented. Implement it to retrieve temperature or lock state.
         */
        // case 'Alexa.Query':
        //     handleQuery(request, context);
        //     break;

        /**
         * Received an unexpected message
         */
        default: {
            const errorMessage = `unsupported namespace, ignoring: ${request.directive.header.namespace}`;
            log('ERROR', errorMessage);
            context.fail()

        }
    }
    
};
