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

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "lifx-lib.h"

size_t buildLIFX_PowerMessage(char* buffer, uint8_t state)
{
	size_t offset;
	lx_protocol_header_t hd;
	uint16_t lx_powerlevel;

	initLIFX_Header(&hd, HEADER_TYPE_SET_POWER);

	//---payload---
	//switch light on or of
	if(state == 0) 
	{
		lx_powerlevel = 0; //0 - off, 65535 - on
	} 
	else if (state==1) 
	{
		lx_powerlevel = 65535; //0 - off, 65535 - on
	}
	else  
	{
		printf("Error: Powerstate not defined!!!\nDefault to off\n");
		lx_powerlevel = 0;
	}

	// we copy the payload into a buffer (serialisation)
	// LIFX requires     
	offset = sizeof(hd);

	// send power message
	memcpy(buffer + offset, &lx_powerlevel, sizeof(lx_powerlevel));
	offset = offset + sizeof(lx_powerlevel);

	//---frame size---
	// only now do we have the complete framesize
	// so we can also copy the frame header
	hd.size = offset;
	memcpy(buffer, &hd, sizeof(hd));
	return offset;
}

size_t buildLIFX_ColorMessage(char* buffer, char* color, uint8_t brightness)
{
	size_t offset;
	lx_protocol_header_t hd;
	lx_color_t lx_color;

	initLIFX_Header(&hd, HEADER_TYPE_SET_COLOR);

	//---payload---    
	//set to Zero
	memset(&lx_color,0,sizeof(lx_color_t));

	//set hsbk values
	if(strcmp(color,"red")==0) 
	{
		lx_color.hue = 0;
	} 
	else if (strcmp(color,"yellow")==0) 
	{
		lx_color.hue = 11000; 
	}
	else if (strcmp(color,"green")==0) 
	{
		lx_color.hue = 22000; 
	}
	else if (strcmp(color,"blue")==0) 
	{
		lx_color.hue = 44000; 
	}
	else  
	{
		printf("Error: Colorvalue not found!!!\nDefault to red\n");
		lx_color.hue = 0; 
	}

	lx_color.saturation = 65535;
	
	//set brightness level
	if (brightness > 100)
	{
		brightness = 100;
	}
	lx_color.brightness = (uint16_t)(655.35 * brightness);
	lx_color.kelvin = 4000;
	
	//time between color changes im Milliseconds
	lx_color.duration = 100;

	// we copy the payload into a buffer (serialisation)
	// LIFX requires     
	offset = sizeof(hd);

	// send color message
	memcpy(buffer + offset, &lx_color, sizeof(lx_color));
	offset = offset + sizeof(lx_color);

	//---frame size---
	// only now do we have the complete framesize
	// so we can also copy the frame header
	hd.size = offset;
	memcpy(buffer, &hd, sizeof(hd));
	return offset;
}


void initLIFX_Header(lx_protocol_header_t* hd, uint16_t type)
{
	int i = 0;
	//set to Zero
	memset(hd,0,sizeof(lx_protocol_header_t));

	//Setting the header
	//---frame---
	hd->protocol = 1024;
	hd->addressable = 1;
	hd->tagged = 1;
	hd->origin = 0;
	hd->source = 1;

	//---frame address--- //d0:73:d5:01:c5:e1
	// hd.target[0] = 208;
	// hd.target[1] = 115;
	// hd.target[2] = 213;
	// hd.target[3] = 1;
	// hd.target[4] = 197;
	// hd.target[5] = 225;
	// hd.target[6] = 0;
	// hd.target[7] = 0;

	// normally not necessary, because struct has been set to Zero
	for (i=0; i<6; i++)
	{
		hd->reserved[i] = 0;
	}

	hd->res = 0;
	hd->ack_required = 0;
	hd->res_required = 0;
	hd->sequence = 2;

	//---protocol header---
	hd->resa = 0;
	hd->type = type;
	hd->resb = 0;
}

