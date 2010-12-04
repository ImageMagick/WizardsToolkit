/*
  Copyright 1999-2011 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.
  
  You may not use this file except in compliance with the License.
  obtain a copy of the License at
  
    http://www.wizards-toolkit.org/WizardsToolkit/script/license.php
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  Wizard's Toolkit Interface declarations.
*/

#ifndef _WIZARDSTOOLKIT_H
#define _WIZARDSTOOLKIT_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#if defined(WIN32) || defined(WIN64)
#  define WIZARDSTOOLKIT_WINDOWS_SUPPORT
#else
#  define WIZARDSTOOLKIT_POSIX_SUPPORT
#endif

#if !defined(_WIZARDSTOOLKIT_CONFIG_H)
# define _WIZARDSTOOLKIT_CONFIG_H
# if !defined(vms) && !defined(macintosh)
#  include "wizard/wizard-config.h"
# else
#  include "wizard-config.h"
# endif
#if defined(_wizardstoolkit_const) && !defined(const)
# define const _wizardstoolkit_const
#endif
#if defined(_wizardstoolkit_inline) && !defined(inline)
# define inline _wizardstoolkit_inline
#endif
# if defined(__cplusplus) || defined(c_plusplus)
#  undef inline
# endif
#endif

#include <stdio.h>
#include <sys/types.h>

#if defined(WIZARDSTOOLKIT_WINDOWS_SUPPORT) && !defined(__CYGWIN__) && !defined(__MINGW32__)
# if defined(_MT) && defined(_DLL) && !defined(_WIZARDDLL_) && !defined(_LIB)
#  define _WIZARDDLL_
# endif
# if defined(_WIZARDDLL_)
#  if defined(_VISUALC_)
#   pragma warning( disable: 4273 )  /* Disable the dll linkage warnings */
#  endif
#  if !defined(_WIZARDLIB_)
#   define WizardExport  __declspec(dllimport)
#   if defined(_VISUALC_)
#    pragma message( "Wizard lib DLL import interface" )
#   endif
#  else
#   define WizardExport  __declspec(dllexport)
#   if defined(_VISUALC_)
#    pragma message( "Wizard lib DLL export interface" )
#   endif
#  endif
# else
#  define WizardExport
#  if defined(_VISUALC_)
#   pragma message( "Wizard lib static interface" )
#  endif
# endif

# if defined(_DLL) && !defined(_LIB)
#  define ModuleExport  __declspec(dllexport)
#  if defined(_VISUALC_)
#   pragma message( "Wizard module DLL export interface" )
#  endif
# else
#  define ModuleExport
#  if defined(_VISUALC_)
#   pragma message( "Wizard module static interface" )
#  endif

# endif
# define WizardGlobal __declspec(thread)
# if defined(_VISUALC_)
#  pragma warning(disable : 4018)
#  pragma warning(disable : 4244)
#  pragma warning(disable : 4142)
#  pragma warning(disable : 4800)
#  pragma warning(disable : 4786)
# endif
#else
# define WizardExport
# define ModuleExport
# define WizardGlobal
#endif

#if !defined(MaxTextExtent)
# define MaxTextExtent  4096
#endif
#define WizardSignature  0xabacadabUL

#if !defined(wizard_attribute)
#  if !defined(__GNUC__)
#    define wizard_attribute(x)  /* nothing */
#  else
#    define wizard_attribute  __attribute__
#  endif
#endif

#if defined(WizardMethodPrefix)
# include "wizard/methods.h"
#endif
#include "wizard/wizard-type.h"
#include "wizard/string_.h"
#include "wizard/authenticate.h"
#include "wizard/blob.h"
#include "wizard/bzip.h"
#include "wizard/cipher.h"
#include "wizard/client.h"
#include "wizard/configure.h"
#include "wizard/entropy.h"
#include "wizard/exception.h"
#include "wizard/file.h"
#include "wizard/hash.h"
#include "wizard/hashmap.h"
#include "wizard/key.h"
#include "wizard/keyring.h"
#include "wizard/log.h"
#include "wizard/magick.h"
#include "wizard/memory_.h"
#include "wizard/mime.h"
#include "wizard/option.h"
#include "wizard/passphrase.h"
#include "wizard/resource_.h"
#include "wizard/semaphore.h"
#include "wizard/secret.h"
#include "wizard/splay-tree.h"
#include "wizard/timer.h"
#include "wizard/token.h"
#include "wizard/utility.h"
#include "wizard/version.h"
#include "wizard/wizard.h"
#include "wizard/xml-tree.h"
#include "wizard/zip.h"

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
