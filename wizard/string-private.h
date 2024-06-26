/*
  Copyright @ 1999 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.
  
  You may not use this file except in compliance with the License.
  obtain a copy of the License at
  
    https://imagemagick.org/script/license.php
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  Wizard's Toolkit private string methods.
*/
#ifndef _WIZARDSTOOLKIT_STRING_PRIVATE_H
#define _WIZARDSTOOLKIT_STRING_PRIVATE_H

#include <wizard/locale_.h>

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

static inline double StringToDouble(const char *string,char **sentinal)
{
  return(InterpretLocaleValue(string,sentinal));
}

static inline double StringToDoubleInterval(const char *string,
  const double interval)
{
  char
    *q;

  double
    value;

  value=InterpretSiPrefixValue(string,&q);
  if (*q == '%')
    value*=interval/100.0;
  return(value);
}

static inline int StringToInteger(const char *value)
{
  return((int) strtol(value,(char **) NULL,10));
}

static inline long StringToLong(const char *value)
{
  if (value == (const char *) NULL)
    return(0);
  return(strtol(value,(char **) NULL,10));
}

static inline unsigned long StringToUnsignedLong(const char *value)
{
  return(strtoul(value,(char **) NULL,10));
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
