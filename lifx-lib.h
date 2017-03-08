/*
 * Copyright (c) 2017, Thomas Scheffler.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/**
 * \file
 *         A rudimentary library for controlling LIFX lightbulbs 
 * \author
 *         Thomas Scheffler <scheffler@beuth-hochschule.de>
 */
 

// HEADER_TYPES
// https://lan.developer.lifx.com/docs/header-description
	/*** Device messages ***/
	//	hd.type = 2;  	/*GetService Message*/
	//	hd.type = 20;  	/*GetPowerStatus Message*/
	//	hd.type = 21;  	/*SetPowerStatus Message*/

	/*** Light messages ***/
	//	hd.type = 101;  	/*GetLightStatus Message*/
	//	hd.type = 102;  	/*SetColor Message*/
	//	hd.type = 116;  	/*GetLightLevel Status Message*/
	//	hd.type = 117;  	/*SetLightLevel Message*/
#define HEADER_TYPE_SET_COLOR 102
#define HEADER_TYPE_SET_POWER 21

/*Struct for LIFX Frame-Header*/
#pragma pack(push, 1)
typedef struct {
	/* frame */
	uint16_t size;
	uint16_t protocol:12;
	uint8_t  addressable:1;
	uint8_t  tagged:1;
	uint8_t  origin:2;
	uint32_t source;
	/* frame address */
	uint8_t  target[8];
	uint8_t  reserved[6];
	uint8_t  res:6;
	uint8_t  ack_required:1;
	uint8_t  res_required:1;
	uint8_t  sequence;
	/* protocol header */
	uint64_t resa:64;
	uint16_t type;
	uint16_t resb:16;
	/* variable length payload follows */

} lx_protocol_header_t;
#pragma pack(pop)


/*Data for different LIFX Payloads*/
/*SetColor - 102*/
#pragma pack(push, 1)
typedef struct {
	uint8_t  reserved;
	struct {
		uint16_t hue;
		uint16_t saturation;
		uint16_t brightness;
		uint16_t kelvin;
	};
	uint32_t duration;
} lx_color_t;
#pragma pack(pop)

/*SetPower - 21*/
//uint16_t lx_powerlevel;	



/**
* Generate a LIFX message that sets the power-status (on/off).
*
*
* \param buffer An appropriate buffer that stores the compiled message.
*
* \param state The desired PowerStatus of the LIFX bulb.
* 0  - sets the PowerState to off
* 1  - sets the PowerState to on
* other values are undefined (default to State: off)
*/
size_t buildLIFX_PowerMessage(char* buffer, uint8_t state);


/**
* Generate a LIFX message that sets the color-status of the bulb (color and
* brightness).
*
*
* \param buffer An appropriate buffer that stores the compiled message.
*
* \param color The desired ColorStatus of the LIFX bulb.
* Currently defined values are: red, green, blue, yellow
* \note Undefined color values default to "red".
*
* \param brightness This represents the brightness level in percent.
* Values are capped at 100(%).
*/
size_t buildLIFX_ColorMessage(char* buffer, char* color, uint8_t brightness);


/**
* Generate the protocol header for a LIFX message.
*
*
* \param hd A pointer to a 'lx_protocol_header' struct.
*
* \param type This parameter sets the protocol header value to the desired 
* message type. Currently defined are HEADER_TYPE_SET_COLOR and
* HEADER_TYPE_SET_POWER.
* \note this parameter must match the LIFX message content.
*/
void initLIFX_Header(lx_protocol_header_t* hd, uint16_t type);


