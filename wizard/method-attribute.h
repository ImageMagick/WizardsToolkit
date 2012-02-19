/*
  Copyright 1999-2012 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.
  
  You may not use this file except in compliance with the License.
  obtain a copy of the License at
  
    http://www.wizards-toolkit.org/script/license.php
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  Wizard's Toolkit method attributes.
*/
#ifndef _WIZARDSTOOLKIT_METHOD_ATTRIBUTES_H
#define _WIZARDSTOOLKIT_METHOD_ATTRIBUTES_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#if defined(__BORLANDC__) && defined(_DLL)
#  pragma message("BCBMagick lib DLL export interface")
#  define _WIZARDDLL_
#  define _WIZARDLIB_
#  define WIZARDSTOOLKIT_MODULES_SUPPORT
#  undef WIZARDSTOOLKIT_BUILD_MODULES
#endif

#if defined(WIZARDSTOOLKIT_WINDOWS_SUPPORT) && !defined(__CYGWIN__)
# define WizardPrivate
# if defined(_MT) && defined(_DLL) && !defined(_WIZARDDLL_) && !defined(_LIB)
#  define _WIZARDDLL_
# endif
# if defined(_WIZARDDLL_)
#  if defined(_VISUALC_)
#   pragma warning( disable: 4273 )  /* Disable the dll linkage warnings */
#  endif
#  if !defined(_WIZARDLIB_)
#   if defined(__GNUC__)
#    define WizardExport __attribute__ ((dllimport))
#   else
#    define WizardExport __declspec(dllimport)
#   endif
#   if defined(_VISUALC_)
#    pragma message( "WizardCore lib DLL import interface" )
#   endif
#  else
#   if defined(__GNUC__)
#    define WizardExport __attribute__ ((dllexport))
#   else
#    define WizardExport __declspec(dllexport)
#   endif
#   if defined(_VISUALC_)
#    pragma message( "WizardCore lib DLL export interface" )
#   endif
#  endif
# else
#  define WizardExport
#  if defined(_VISUALC_)
#   pragma message( "WizardCore lib static interface" )
#  endif
# endif

# if defined(_DLL) && !defined(_LIB)
#   if defined(__GNUC__)
#    define ModuleExport __attribute__ ((dllexport))
#   else
#    define ModuleExport __declspec(dllexport)
#   endif
#  if defined(_VISUALC_)
#   pragma message( "WizardCore module DLL export interface" )
#  endif
# else
#  define ModuleExport
#  if defined(_VISUALC_)
#   pragma message( "WizardCore module static interface" )
#  endif

# endif
# define WizardGlobal __declspec(thread)
# if defined(_VISUALC_)
#  pragma warning(disable : 4018)
#  pragma warning(disable : 4068)
#  pragma warning(disable : 4244)
#  pragma warning(disable : 4142)
#  pragma warning(disable : 4800)
#  pragma warning(disable : 4786)
#  pragma warning(disable : 4996)
# endif
#else
# if __GNUC__ >= 4
#  define WizardExport __attribute__ ((visibility ("default")))
#  define WizardPrivate  __attribute__ ((visibility ("hidden")))
# else
#   define WizardExport
#   define WizardPrivate
# endif
# define ModuleExport  WizardExport
# define WizardGlobal
#endif

#define WizardSignature  0xabacadabUL
#if !defined(MaxTextExtent)
# define MaxTextExtent  4096
#endif

#if defined(WIZARDSTOOLKIT_HAVE___ATTRIBUTE__)
#  define wizard_aligned(x)  __attribute__((aligned(x)))
#  define wizard_attribute  __attribute__
#  define wizard_unused(x)  wizard_unused_ ## x __attribute__((unused))
#else
#  define wizard_aligned(x)  /* nothing */
#  define wizard_attribute(x)  /* nothing */
#  define wizard_unused(x) x
#endif

#if defined(WIZARDSTOOLKIT_HAVE___ALLOC_SIZE__)
#  define wizard_alloc_size(x)  __attribute__((__alloc_size__(x)))
#  define wizard_alloc_sizes(x,y)  __attribute__((__alloc_size__(x,y)))
#  define wizard_cold_spot  __attribute__((__cold__))
#  define wizard_hot_spot  __attribute__((__hot__))
#else
#  define wizard_alloc_size(x)  /* nothing */
#  define wizard_alloc_sizes(x,y)  /* nothing */
#  define wizard_cold_spot
#  define wizard_hot_spot
#endif

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
