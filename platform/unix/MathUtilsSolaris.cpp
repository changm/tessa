/* -*- Mode: C++; c-basic-offset: 4; indent-tabs-mode: nil; tab-width: 4 -*- */
/* vi: set ts=4 sw=4 expandtab: (add to ~/.vimrc: set modeline modelines=5) */
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is [Open Source Virtual Machine.].
 *
 * The Initial Developer of the Original Code is
 * Adobe System Incorporated.
 * Portions created by the Initial Developer are Copyright (C) 2004-2006
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Adobe AS3 Team
 *   leon.sha@sun.com
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

#include <math.h>

#include "avmplus.h"

#ifndef PI
#define PI 3.141592653589793
#endif

namespace avmplus
{
    // todo need asm versions from Player

    double MathUtils::abs(double value)
    {
        return ::fabs(value);
    }

    double MathUtils::acos(double value)
    {
        double result = ::acos(value);
        if(result == 0 && (value > 1 || value < -1)) {
            return MathUtils::nan();
        }
        return result;
    }

    double MathUtils::asin(double value)
    {
        double result = ::asin(value);
        if(result == 0 && (value > 1 || value < -1)) {
            return MathUtils::nan();
        }
        return result;
    }

    double MathUtils::atan(double value)
    {
        return ::atan(value);
    }

    double MathUtils::atan2(double y, double x)
    {
        double result = ::atan2(y, x);
        if(result !=0 )
            return result;

        if (MathUtils::isNegZero(y) && MathUtils::isNegZero(x))
            return -PI;
        else if(y == 0 && MathUtils::isNegZero(x))
            return PI;
        else if(x == 0 && MathUtils::isNegZero(y))
            return y;
        else if(x == 0 && y == 0)
            return y;
        return result;
    }

    double MathUtils::ceil(double value)
    {
        return ::ceil(value);
    }

    double MathUtils::cos(double value)
    {
        return ::cos(value);
    }

    double MathUtils::exp(double value)
    {
        return ::exp(value);
    }

    double MathUtils::floor(double value)
    {
        return ::floor(value);
    }

    uint64_t MathUtils::frexp(double value, int *eptr)
    {
        double fracMantissa = ::frexp(value, eptr);

        // correct mantissa and eptr to get integer values
        //  for both
        *eptr -= 53; // 52 mantissa bits + the hidden bit
        return (uint64_t)(fracMantissa * (double)(1LL << 53));
    }

    double MathUtils::log(double value)
    {
        if( value >= 0 )
            return ::log(value);
        else
            return MathUtils::nan();
    }

    double MathUtils::mod(double x, double y)
    {
        return ::fmod(x, y);
    }

    double MathUtils::powInternal(double x, double y)
    {
        return ::pow(x, y);
    }

    double MathUtils::sin(double value)
    {
        return ::sin(value);
    }

    double MathUtils::sqrt(double value)
    {
        return ::sqrt(value);
    }

    double MathUtils::tan(double value)
    {
        return ::tan(value);
    }
}
