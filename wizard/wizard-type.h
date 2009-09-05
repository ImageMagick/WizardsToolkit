/*
  Copyright 1999-2009 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.
  
  You may not use this file except in compliance with the License.
  obtain a copy of the License at
  
    http://www.wizards-toolkit.org/script/license.php
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  Wizard's Toolkit structure members.
*/
#ifndef _WIZARDSTOOLKIT_WIZARD_TYPE_H
#define _WIZARDSTOOLKIT_WIZARD_TYPE_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

typedef enum
{
  WizardFalse = 0,
  WizardTrue = 1
} WizardBooleanType;

#if defined(__WINDOWS__) && !defined(__MINGW32__)
#  define WizardLLConstant(c)  (WizardOffsetType) (c ## i64)
#  define WizardULLConstant(c)  (WizardSizeType) (c ## ui64)
#else
#  define WizardLLConstant(c)  (WizardOffsetType) (c ## LL)
#  define WizardULLConstant(c)  (WizardSizeType) (c ## ULL)
#endif

/*
  Typedef declarations.
*/
typedef unsigned int WizardStatusType;
#if !defined(__WINDOWS__)
#if (WIZARDSTOOLKIT_SIZEOF_UNSIGNED_LONG_LONG == 8)
typedef long long WizardOffsetType;
typedef unsigned long long WizardSizeType;
#define WizardSizeFormat  "%llu"
#else
typedef long WizardOffsetType;
typedef unsigned long WizardSizeType;
#define WizardSizeFormat  "%lu"
#endif
#else
typedef __int64 WizardOffsetType;
typedef unsigned __int64 WizardSizeType;
#define WizardSizeFormat  "%llu"
#endif

#if defined(macintosh)
#define ExceptionInfo  WizardExceptionInfo
#endif

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
