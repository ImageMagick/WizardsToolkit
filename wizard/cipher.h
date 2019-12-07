/*
  Copyright 1999-2020 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.

  You may not use this file except in compliance with the License.
  obtain a copy of the License at

    https://imagemagick.org/script/license.php

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  Wizard's Toolkit secure cipher algorithm methods.
*/
#ifndef _WIZARDSTOOLKIT_CIPHER_H
#define _WIZARDSTOOLKIT_CIPHER_H

#include "wizard/random_.h"

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#define MaxCipherBlocksize  128

typedef enum
{
  UndefinedMode,
  CBCMode,
  CFBMode,
  CTRMode,
  ECBMode,
  OFBMode
} CipherMode;

typedef enum
{
  UndefinedCipher,
  NoCipher,
  AESCipher,
  ChachaCipher,
  SerpentCipher,
  TwofishCipher
} CipherType;

typedef struct _CipherInfo
  CipherInfo;

extern WizardExport CipherInfo
  *AcquireCipherInfo(const CipherType,const CipherMode),
  *DestroyCipherInfo(CipherInfo *);

extern WizardExport const StringInfo
  *GetCipherNonce(CipherInfo *);

extern WizardExport StringInfo
  *DecipherCipher(CipherInfo *,StringInfo *),
  *EncipherCipher(CipherInfo *,StringInfo *),
  *GenerateCipherNonce(CipherInfo *);

extern WizardExport size_t
  GetCipherBlocksize(const CipherInfo *);

extern WizardExport void
  ResetCipherNonce(CipherInfo *),
  SetCipherNonce(CipherInfo *,const StringInfo *),
  SetCipherKey(CipherInfo *,const StringInfo *);

/*
  Deprecated methods.
*/
extern WizardExport StringInfo
  *DecryptCipher(CipherInfo *,StringInfo *),
  *EncryptCipher(CipherInfo *,StringInfo *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
