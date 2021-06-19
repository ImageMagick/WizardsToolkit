/*
  Copyright 1999-2021 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.
  
  You may not use this file except in compliance with the License.
  obtain a copy of the License at
  
    https://imagemagick.org/script/license.php
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  The Wizard's Toolkit private memory methods.
*/
#ifndef _WIZARDSTOOLKIT_MEMORY_PRIVATE_H
#define _WIZARDSTOOLKIT_MEMORY_PRIVATE_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif
#if defined(__powerpc__)
#  define CACHE_LINE_SIZE 128
#else
#  define CACHE_LINE_SIZE 64
#endif

#define CacheAlign(size)  ((size) < CACHE_LINE_SIZE ? CACHE_LINE_SIZE : (size))

#if (__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ > 6))
#if !defined(__ICC)
#define WizardAssumeAligned(address) \
  __builtin_assume_aligned((address),CACHE_LINE_SIZE)
#else
#define WizardAssumeAligned(address)  (address)
#endif
#else
#define WizardAssumeAligned(address)  (address)
#endif

static inline size_t OverAllocateMemory(const size_t length)
{
  size_t
    extent;

  /*
    Over allocate memory, typically used when concatentating strings.
  */
  extent=length;
  if (extent < 131072)
    for (extent=256; extent < length; extent*=2);
  return(extent);
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
