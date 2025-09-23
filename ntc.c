
/* ******************************************************************************
 * 	VSCP (Very Simple Control Protocol) 
 * 	http://www.vscp.org
 *
 *  Kelvin NTC10KA Module
 *  =====================
 *
 *  Copyright (C) 2015-2020 Ake Hedman, Grodans Paradis AB
 *                          http://www.grodansparadis.com
 *                          <akhe@grodansparadis.com>
 *
 *  This work is licensed under the Creative Common 
 *  Attribution-NonCommercial-ShareAlike 3.0 Unported license. The full
 *  license is available in the top folder of this project (LICENSE) or here
 *  http://creativecommons.org/licenses/by-nc-sa/3.0/legalcode
 *  It is also available in a human readable form here 
 *  http://creativecommons.org/licenses/by-nc-sa/3.0/
 * 
 *	This file is part of VSCP - Very Simple Control Protocol 	
 *	http://www.vscp.org
 *
 * ******************************************************************************
 */

#include "vscp-compiler.h"
#include "vscp-projdefs.h"

#include <p18cxxx.h>
#include <float.h>
#include <math.h>
#include "ntc.h"


///////////////////////////////////////////////////////////////////////////////
// Celsius2Fahrenheit
//

double Celsius2Fahrenheit(double tc)
{
    return ( (9 * tc + 16000) / 5);
}

///////////////////////////////////////////////////////////////////////////////
// Fahrenheit2Celsius
//

double Fahrenheit2Celsius(double tf)
{
    return ( (5 * tf - 16000) / 9);
}


///////////////////////////////////////////////////////////////////////////////
// Celsius2Kelvin
//

double Celsius2Kelvin(double tc)
{
    return (27316.0 + tc);
}

///////////////////////////////////////////////////////////////////////////////
// Kelvin2Celsius
//

double Kelvin2Celsius(double tf)
{
    return ( tf - 273.16);
}
