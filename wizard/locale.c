/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                  L       OOO    CCCC   AAA   L      EEEEE                   %
%                  L      O   O  C      A   A  L      E                       %
%                  L      O   O  C      AAAAA  L      EEE                     %
%                  L      O   O  C      A   A  L      E                       %
%                  LLLLL   OOO    CCCC  A   A  LLLLL  EEEEE                   %
%                                                                             %
%                                                                             %
%                      WizardCore Image Locale Methods                        %
%                                                                             %
%                              Software Design                                %
%                                John Cristy                                  %
%                                 July 2003                                   %
%                                                                             %
%                                                                             %
%  Copyright 1999-2011 ImageWizard Studio LLC, a non-profit organization      %
%  dedicated to making software imaging solutions freely available.           %
%                                                                             %
%  You may not use this file except in compliance with the License.  You may  %
%  obtain a copy of the License at                                            %
%                                                                             %
%    http://www.imagewizard.org/script/license.php                            %
%                                                                             %
%  Unless required by applicable law or agreed to in writing, software        %
%  distributed under the License is distributed on an "AS IS" BASIS,          %
%  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   %
%  See the License for the specific language governing permissions and        %
%  limitations under the License.                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%
*/

/*
  Include declarations.
*/
#include "wizard/studio.h"
#include "wizard/blob.h"
#include "wizard/client.h"
#include "wizard/configure.h"
#include "wizard/exception.h"
#include "wizard/exception-private.h"
#include "wizard/hashmap.h"
#include "wizard/locale_.h"
#include "wizard/log.h"
#include "wizard/memory_.h"
#include "wizard/semaphore.h"
#include "wizard/splay-tree.h"
#include "wizard/string_.h"
#include "wizard/token.h"
#include "wizard/utility.h"
#include "wizard/xml-tree.h"

/*
  Define declarations.
*/
#define LocaleFilename  "locale.xml"
#define MaxRecursionDepth  200

static SemaphoreInfo
  *locale_semaphore = (SemaphoreInfo *) NULL;

static volatile locale_t
  c_locale = (locale_t) NULL;

static volatile WizardBooleanType
  instantiate_locale = WizardFalse;

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   A c q u i r e C L o c a l e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireCLocale() allocates the C locale object, or (locale_t) 0 with
%  errno set if it cannot be acquired.
%
%  The format of the AcquireCLocale method is:
%
%      locale_t AcquireCLocale(void)
%
*/
static locale_t AcquireCLocale(void)
{
#if defined(WIZARDSTOOLKIT_HAVE_NEWLOCALE)
  if (c_locale == (locale_t) NULL)
    c_locale=newlocale(LC_ALL_MASK,"C",(locale_t) 0);
#elif defined(WIZARDSTOOLKIT_WINDOWS_SUPPORT)
  if (c_locale == (locale_t) NULL)
    c_locale=_create_locale(LC_ALL,"C");
#endif
  return(c_locale);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   D e s t r o y C L o c a l e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroyCLocale() releases the resources allocated for a locale object
%  returned by a call to the AcquireCLocale() method.
%
%  The format of the DestroyCLocale method is:
%
%      void DestroyCLocale(void)
%
*/
static void DestroyCLocale(void)
{
#if defined(WIZARDSTOOLKIT_HAVE_NEWLOCALE)
  if (c_locale != (locale_t) NULL)
    freelocale(c_locale);
#elif defined(WIZARDSTOOLKIT_WINDOWS_SUPPORT)
  if (c_locale != (locale_t) NULL)
    _free_locale(c_locale);
#endif
  c_locale=(locale_t) NULL;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+  F o r m a t L o c a l e F i l e                                            %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  FormatLocaleFile() prints formatted output of a variable argument list to a
%  file in the "C" locale.
%
%  The format of the FormatLocaleFile method is:
%
%      ssize_t FormatLocaleFile(FILE *file,const char *format,...)
%
%  A description of each parameter follows.
%
%   o file:  the file.
%
%   o format:  A file describing the format to use to write the remaining
%     arguments.
%
*/

WizardExport ssize_t FormatLocaleFileList(FILE *file,
  const char *restrict format,va_list operands)
{
  int
    n;

#if defined(WIZARDSTOOLKIT_HAVE_VFPRINTF_L)
 {
    locale_t
      locale;

    locale=AcquireCLocale();
    if (locale == (locale_t) NULL)
      n=vfprintf(file,format,operands);
    else
#if defined(__APPLE__)
      n=vfprintf_l(file,locale,format,operands);
#else
      n=vfprintf_l(file,format,locale,operands);
#endif
  }
#else
#if defined(WIZARDSTOOLKIT_HAVE_USELOCALE)
  {
    locale_t
      locale,
      previous_locale;

    locale=AcquireCLocale();
    if (locale == (locale_t) NULL)
      n=vfprintf(file,format,operands);
    else
      {
        previous_locale=uselocale(locale);
        n=vfprintf(file,format,operands);
        uselocale(previous_locale);
      }
  }
#else
  n=vfprintf(file,format,operands);
#endif
#endif
  return((ssize_t) n);
}

WizardExport ssize_t FormatLocaleFile(FILE *file,const char *restrict format,
  ...)
{
  ssize_t
    n;

  va_list
    operands;

  va_start(operands,format);
  n=(ssize_t) FormatLocaleFileList(file,format,operands);
  va_end(operands);
  return(n);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+  F o r m a t L o c a l e S t r i n g                                        %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  FormatLocaleString() prints formatted output of a variable argument list to
%  a string buffer in the "C" locale.
%
%  The format of the FormatLocaleString method is:
%
%      ssize_t FormatLocaleString(char *string,const size_t length,
%        const char *format,...)
%
%  A description of each parameter follows.
%
%   o string:  FormatLocaleString() returns the formatted string in this
%     character buffer.
%
%   o length: the maximum length of the string.
%
%   o format:  A string describing the format to use to write the remaining
%     arguments.
%
*/

WizardExport ssize_t FormatLocaleStringList(char *restrict string,
  const size_t length,const char *restrict format,va_list operands)
{
  int
    n;

#if defined(WIZARDSTOOLKIT_HAVE_VSNPRINTF_L)
  {
    locale_t
      locale;

    locale=AcquireCLocale();
    if (locale == (locale_t) NULL)
      n=vsnprintf(string,length,format,operands);
    else
#if defined(__APPLE__)
      n=vsnprintf_l(string,length,locale,format,operands);
#else
      n=vsnprintf_l(string,length,format,locale,operands);
#endif
  }
#elif defined(WIZARDSTOOLKIT_HAVE_VSNPRINTF)
#if defined(WIZARDSTOOLKIT_HAVE_USELOCALE)
  {
    locale_t
      locale,
      previous_locale;

    locale=AcquireCLocale();
    if (locale == (locale_t) NULL)
      n=vsnprintf(string,length,format,operands);
    else
      {
        previous_locale=uselocale(locale);
        n=vsnprintf(string,length,format,operands);
        uselocale(previous_locale);
      }
  }
#else
  n=vsnprintf(string,length,format,operands);
#endif
#else
  n=vsprintf(string,format,operands);
#endif
  if (n < 0)
    string[length-1]='\0';
  return((ssize_t) n);
}

WizardExport ssize_t FormatLocaleString(char *restrict string,
  const size_t length,const char *restrict format,...)
{
  ssize_t
    n;

  va_list
    operands;

  va_start(operands,format);
  n=(ssize_t) FormatLocaleStringList(string,length,format,operands);
  va_end(operands);
  return(n);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   I n t e r p r e t L o c a l e V a l u e                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  InterpretLocaleValue() interprets the string as a floating point number in
%  the "C" locale and returns its value as a double. If sentinal is not a null
%  pointer, the method also sets the value pointed by sentinal to point to the
%  first character after the number.
%
%  The format of the InterpretLocaleValue method is:
%
%      double InterpretLocaleValue(const char *value,char **sentinal)
%
%  A description of each parameter follows:
%
%    o value: the string value.
%
%    o sentinal:  if sentinal is not NULL, a pointer to the character after the
%      last character used in the conversion is stored in the location
%      referenced by sentinal.
%
*/
WizardExport double InterpretLocaleValue(const char *restrict string,
  char **restrict sentinal)
{
  double
    value;

#if defined(WIZARDSTOOLKIT_HAVE_STRTOD_L)
  {
    locale_t
      locale;

    locale=AcquireCLocale();
    if (locale == (locale_t) NULL)
      value=strtod(string,sentinal);
    else
      value=strtod_l(string,sentinal,locale);
  }
#else
  value=strtod(string,sentinal);
#endif
  return(value);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   L o c a l e C o m p o n e n t G e n e s i s                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  LocaleComponentGenesis() instantiates the locale component.
%
%  The format of the LocaleComponentGenesis method is:
%
%      WizardBooleanType LocaleComponentGenesis(void)
%
*/
WizardExport WizardBooleanType LocaleComponentGenesis(void)
{
  AcquireSemaphoreInfo(&locale_semaphore);
  return(WizardTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   L o c a l e C o m p o n e n t T e r m i n u s                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  LocaleComponentTerminus() destroys the locale component.
%
%  The format of the LocaleComponentTerminus method is:
%
%      LocaleComponentTerminus(void)
%
*/
WizardExport void LocaleComponentTerminus(void)
{
  if (locale_semaphore == (SemaphoreInfo *) NULL)
    AcquireSemaphoreInfo(&locale_semaphore);
  LockSemaphoreInfo(locale_semaphore);
  DestroyCLocale();
  instantiate_locale=WizardFalse;
  UnlockSemaphoreInfo(locale_semaphore);
  DestroySemaphoreInfo(&locale_semaphore);
}
