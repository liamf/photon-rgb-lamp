'use strict';

/**
 * This is a very simple example of using the Amazon Smart Home "light" API to control the Particle "Pixel" lamp in the attic
 * 
 * For more information about developing smart home skills, see
 *  https://developer.amazon.com/alexa/smart-home
 *
 * For details on the smart home API, please visit
 *  https://developer.amazon.com/public/solutions/alexa/alexa-skills-kit/docs/smart-home-skill-api-reference
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
            // This is the Particle Device ID of this lamp
            deviceId: '3a0034000647343138333038',
            rootToken: '5474c087d755d05d7b3e90e20aa615e60e34f1a1',
        },
        capabilities: [
            {
                type: 'AlexaInterface',
                interface: 'Alexa',
                version: '3'
            },
            {
                type: 'AlexaInterface',
                interface: 'Alexa.PowerController',
                version: '3',
                properties: {
                    supported: [{name: 'powerState'}], 
                    proactivelyReported: false,
                    retrievable: true
                },
            },
            {
                type: 'AlexaInterface',
                interface: 'Alexa.BrightnessController',
                version: '3',
                properties: {
                    supported: [{name: 'brightness'}],
                    proactivelyReported: false,
                    retrievable: true
                },
            },
            {
                type: 'AlexaInterface',
                interface: 'Alexa.ColorController',
                version: '3',
                properties: {
                    supported: [{name: 'color'}],
                    proactivelyReported: false,
                    retrievable: true
                },
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

// Not sure where this fn came from ... 
function rgb2hsv (r,g,b) {
 var computedH = 0;
 var computedS = 0;
 var computedV = 0;

 //remove spaces from input RGB values, convert to int
 //No real sanity checking or error catching here. 
 var r = parseInt( (''+r).replace(/\s/g,''),10 ); 
 var g = parseInt( (''+g).replace(/\s/g,''),10 ); 
 var b = parseInt( (''+b).replace(/\s/g,''),10 ); 

// assumes our default colour ranges for the lamp
 r=r/4095; g=g/4095; b=b/4095;
 var minRGB = Math.min(r,Math.min(g,b));
 var maxRGB = Math.max(r,Math.max(g,b));

 // Black-gray-white
 if (minRGB==maxRGB) {
  computedV = minRGB;
  return {
      h: 0,
      s: 0,
      v: computedV
    };
 }

 // Colors other than black-gray-white:
 var d = (r==minRGB) ? g-b : ((b==minRGB) ? r-g : b-r);
 var h = (r==minRGB) ? 3 : ((b==minRGB) ? 1 : 5);
 computedH = 60*(h - d/(maxRGB - minRGB));
 computedS = (maxRGB - minRGB)/maxRGB;
 computedV = maxRGB;
 
 return {
     h: computedH,
     s: computedS,
     v: computedV
     };
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

function generateStateResponse(request, varList) {
    var response = {};
    var now = new Date();
        
    var powerState = "ON";
    
    var HSV = rgb2hsv( varList.red, varList.green, varList.blue);
    
    if( varList.red == 0 && varList.green == 0 && varList.blue === 0 )
    {
        powerState = "OFF";
    }
    
    response.context = {
        properties: [
        {
            namespace: 'Alexa.PowerController',
            name: 'powerState',
            value: powerState,
            timeOfSample: now.toISOString(),
            uncertaintyInMilliseconds: 500 
        },
        {
            namespace: 'Alexa.BrightnessController',
            name: 'brightness',
            value: varList.level,
            timeOfSample: now.toISOString(),
            uncertaintyInMilliseconds: 500             
        },
        {
            namespace: 'Alexa.BrightnessController',
            name: 'color',
            value: {
                hue: HSV.h.toFixed(3),
                saturation: HSV.s.toFixed(3),
                brightness: HSV.v.toFixed(3)
            },
            timeOfSample: now.toISOString(),
            uncertaintyInMilliseconds: 500   
        }
        ]
    };
    
    // Rest of the response is generic
    response.event = {
        header: {
            messageId: generateMessageID(),
            correlationToken: request.directive.header.correlationToken,
            namespace: 'Alexa',
            name: 'StateReport',
            payloadVersion: '3'
        },
        endpoint: {
            scope: {
                type: request.directive.endpoint.scope.type,
                token: request.directive.endpoint.scope.token
            },    
            endpointId: request. directive.endpoint.endpointId
        },
        payload: {
        }
    };
    
    log( 'DEBUG', 'StateReport is ' + JSON.stringify(response));
    
    return response;
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

function getParticleVariable(varName, request, callbackContext) {
    
    // Generic parameters
    const deviceId = request.directive.endpoint.cookie.deviceId;
    const rootToken = request.directive.endpoint.cookie.rootToken
    const accessToken = request.directive.endpoint.scope.token; 
    
    // The Particle API command data 
    var getData = {
        method: 'GET',
        url: particleServer + particlePath + deviceId + "/" + varName,
        qs: {
        	access_token: rootToken
        },
    };

    // log('DEBUG', 'Requesting Variable: ' + JSON.stringify(getData));
    
    // Request the data and callback with the result
    httpRequest(getData, function(error, response, body) {
        var d = JSON.parse(body) ; 
        var result = d.result;
        if( !error ) {
            callbackContext.particleVars[varName] = result;
            callbackContext.respCallback(callbackContext.particleVars)
        } 
    });
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

    // log( 'DEBUG', "command is " + JSON.stringify(postData))
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

function getState(request, context) {
    
    /**
     * Grab the deviceId from the request.
     */
    const deviceId = request.directive.endpoint.cookie.deviceID;
    const accessToken = request.directive.endpoint.scope.token;
    
    // Store the results here, process when we have them all
    let particleVars = {}
    var respCallback = function(varList) {
        // We only execute the callback when we have all 4 variables filled in
        if( Object.keys(varList).length == 4 )
        {
            // OK, we have all the variables.
            var stateResponse = generateStateResponse(request, varList);
            context.succeed(stateResponse);
        }
    };
 
    // Well this looks fugly
    var callbackContext = {
        respCallback,
        particleVars
    }
    
    // Ask for a few variables ... and handle them in the callback
    getParticleVariable( 'red', request, callbackContext);
    getParticleVariable( 'green', request, callbackContext);
    getParticleVariable( 'blue', request, callbackContext);
    getParticleVariable( 'level', request, callbackContext);
    
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
 * 
 * A functiont to handle some more generic "Alexa" queries
 * The only one we care about at the moment is "ReportState"
 */
 
function handleGenericAlexaMessage(request, context)
{
    log('DEBUG', 'Generic Alexa query');

    switch( request.directive.header.name)
    {
        case 'ReportState': 
            getState(request, context);
            log('DEBUG', 'Reporting State');
            break;
            
        default: 
            log('ERROR', `No supported directive name: ${request.header.name}`);
            return;        
    }
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

        case 'SetColor': 
            setColor(request, context);
            break;

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
    log('DEBUG', 'REQUEST was: ' + JSON.stringify(request));
    log('DEBUG', JSON.stringify(context));
    
    
    switch (request.directive.header.namespace) {
        /**
         * The namespace of 'Alexa.ConnectedHome.Discovery' indicates a request is being made to the Lambda for
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

        // Handler for the ReportState interface
        case 'Alexa':
            handleGenericAlexaMessage(request, context);
            break;
            

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
