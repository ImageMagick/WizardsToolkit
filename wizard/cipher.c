/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                   CCCC  IIIII  PPPP   H   H  EEEEE  RRRR                    %
%                  C        I    P   P  H   H  E      R   R                   %
%                  C        I    PPPP   HHHHH  EEE    RRRR                    %
%                  C        I    P      H   H  E      R R                     %
%                   CCCC  IIIII  P      H   H  EEEEE  R  R                    %
%                                                                             %
%                                                                             %
%              Wizard's Toolkit Secure Cipher Algorithm Methods               %
%                                                                             %
%                             Software Design                                 %
%                                 Cristy                                      %
%                               March  2003                                   %
%                                                                             %
%                                                                             %
%  Copyright 1999-2021 ImageMagick Studio LLC, a non-profit organization      %
%  dedicated to making software imaging solutions freely available.           %
%                                                                             %
%  You may not use this file except in compliance with the License.  You may  %
%  obtain a copy of the License at                                            %
%                                                                             %
%    https://imagemagick.org/script/license.php                               %
%                                                                             %
%  Unless required by applicable law or agreed to in writing, software        %
%  distributed under the License is distributed on an "AS IS" BASIS,          %
%  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   %
%  See the License for the specific language governing permissions and        %
%  limitations under the License.                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
% See http://csrc.nist.gov/groups/ST/toolkit/index.html.
%
*/

/*
  Include declarations.
*/
#include "wizard/studio.h"
#include "wizard/aes.h"
#include "wizard/chacha.h"
#include "wizard/cipher.h"
#include "wizard/exception.h"
#include "wizard/exception-private.h"
#include "wizard/memory_.h"
#include "wizard/random_.h"
#include "wizard/serpent.h"
#include "wizard/twofish.h"

/*
  Define declarations.
*/
#define CipherRandomHash  SHA2256Hash

/*
  Typedef declarations.
*/
typedef void
  (*DecipherBlock)(void *,const unsigned char *,const unsigned char *),
  (*EncipherBlock)(void *,const unsigned char *,const unsigned char *);

struct _CipherInfo
{
  void
    *handle;

  CipherType
    cipher;

  CipherMode
    mode;

  size_t
    blocksize;

  DecipherBlock
    decipher_block;

  DecipherBlock
    encipher_block;

  StringInfo
    *nonce;

  RandomInfo
    *random_info;

  time_t
    timestamp;

  size_t
    signature;
};

/*
  Forward declaration.
*/
static StringInfo
  *DecipherCTRMode(CipherInfo *,StringInfo *),
  *DecipherECBMode(CipherInfo *,StringInfo *),
  *DecipherOFBMode(CipherInfo *,StringInfo *),
  *EncipherCTRMode(CipherInfo *,StringInfo *),
  *EncipherECBMode(CipherInfo *,StringInfo *),
  *EncipherOFBMode(CipherInfo *,StringInfo *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A c q u i r e C i p h e r I n f o                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireCipherInfo() allocates the CipherInfo structure.
%
%  The format of the AcquireCipherInfo method is:
%
%      CipherInfo *AcquireCipherInfo(const CipherType cipher,
%        const CipherMode mode)
%
%  A description of each parameter follows:
%
%    o cipher: The cipher type.
%
%    o mode: The cipher mode.
%
*/
WizardExport CipherInfo *AcquireCipherInfo(const CipherType cipher,
  const CipherMode mode)
{
  CipherInfo
    *cipher_info;

  cipher_info=(CipherInfo *) AcquireWizardMemory(sizeof(*cipher_info));
  if (cipher_info == (CipherInfo *) NULL)
    ThrowWizardFatalError(CipherDomain,MemoryError);
  (void) ResetWizardMemory(cipher_info,0,sizeof(*cipher_info));
  cipher_info->cipher=cipher;
  switch (cipher_info->cipher)
  {
    case AESCipher:
    {
      AESInfo
        *aes_info;

      aes_info=AcquireAESInfo();
      cipher_info->handle=(CipherInfo *) aes_info;
      cipher_info->blocksize=GetAESBlocksize(aes_info);
      cipher_info->decipher_block=(DecipherBlock) DecipherAESBlock;
      cipher_info->encipher_block=(DecipherBlock) EncipherAESBlock;
      break;
    }
    case ChachaCipher:
    {
      ChachaInfo
        *chacha_info;

      chacha_info=AcquireChachaInfo();
      cipher_info->handle=(CipherInfo *) chacha_info;
      cipher_info->blocksize=GetChachaBlocksize(chacha_info);
      cipher_info->decipher_block=(DecipherBlock) DecipherChachaBlock;
      cipher_info->encipher_block=(DecipherBlock) EncipherChachaBlock;
      break;
    }
    case SerpentCipher:
    {
      SerpentInfo
        *serpent_info;

      serpent_info=AcquireSerpentInfo();
      cipher_info->handle=(CipherInfo *) serpent_info;
      cipher_info->blocksize=GetSerpentBlocksize(serpent_info);
      cipher_info->decipher_block=(DecipherBlock) DecipherSerpentBlock;
      cipher_info->encipher_block=(DecipherBlock) EncipherSerpentBlock;
      break;
    }
    case TwofishCipher:
    {
      TwofishInfo
        *twofish_info;

      twofish_info=AcquireTwofishInfo();
      cipher_info->handle=(CipherInfo *) twofish_info;
      cipher_info->blocksize=GetTwofishBlocksize(twofish_info);
      cipher_info->decipher_block=(DecipherBlock) DecipherTwofishBlock;
      cipher_info->encipher_block=(DecipherBlock) EncipherTwofishBlock;
      break;
    }
    default:
      ThrowWizardFatalError(CipherDomain,EnumerateError);
  }
  cipher_info->mode=mode;
  if (cipher_info->nonce != (StringInfo *) NULL)
    cipher_info->nonce=DestroyStringInfo(cipher_info->nonce);
  cipher_info->random_info=AcquireRandomInfo(CipherRandomHash);
  cipher_info->timestamp=time((time_t *) NULL);
  cipher_info->signature=WizardSignature;
  cipher_info->nonce=GenerateCipherNonce(cipher_info);
  return(cipher_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   D e c i p h e r C B C M o d e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DecipherCBCMode() deciphers with the cipher in Cipher Block Chaining mode.
%  This mode is a confidentiality mode whose enciphering process features the
%  combining ("chaining") of the plaintext blocks with the previous ciphertext
%  blocks.  The CBC mode requires an IV to combine with the first plaintext
%  block. The IV need not be secret, but it must be unpredictable.
%
%  The format of the DecipherCBCMode method is:
%
%     StringInfo *DecipherCBCMode(CipherInfo *cipher_info,
%       StringInfo *ciphertext)
%
%  A description of each parameter follows:
%
%    o cipher_info: The cipher context.
%
%    o ciphertext: The cipher text.
%
*/
static StringInfo *DecipherCBCMode(CipherInfo *cipher_info,
  StringInfo *ciphertext)
{
  register size_t
    i;

  register unsigned char
    *p,
    *q;

  size_t
    blocksize;

  StringInfo
    *plaintext;

  unsigned char
    input_block[MaxCipherBlocksize],
    output_block[MaxCipherBlocksize];

  /*
    Decipher in CBC mode.
  */
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(CipherDomain,cipher_info != (CipherInfo *) NULL);
  WizardAssert(CipherDomain,cipher_info->signature == WizardSignature);
  blocksize=cipher_info->blocksize;
  WizardAssert(CipherDomain,blocksize <= MaxCipherBlocksize);
  WizardAssert(CipherDomain,ciphertext != (StringInfo *) NULL);
  plaintext=ciphertext;
  p=GetStringInfoDatum(cipher_info->nonce);
  for (i=0; i < blocksize; i++)
    input_block[i]=p[i];
  q=GetStringInfoDatum(ciphertext)+GetStringInfoLength(ciphertext);
  for (p=GetStringInfoDatum(ciphertext); p < q; p+=blocksize)
  {
    for (i=0; i < blocksize; i++)
      output_block[i]=p[i];
    cipher_info->decipher_block(cipher_info->handle,p,p);
    for (i=0; i < blocksize; i++)
      p[i]^=input_block[i];
    for (i=0; i < blocksize; i++)
      input_block[i]=output_block[i];
  }
  /*
    Reset registers.
  */
  (void) ResetWizardMemory(input_block,0,sizeof(input_block));
  (void) ResetWizardMemory(output_block,0,sizeof(output_block));
  return(plaintext);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   D e c i p h e r C F B M o d e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DecipherCFBMode() deciphers with the cipher in Cipher Feedback mode. This
%  mode is a confidentiality mode that features the feedback of successive
%  ciphertext segments into the input blocks of the forward cipher to generate
%  output blocks that are exclusive-ORed with the plaintext to produce the
%  ciphertext, and vice versa. The CFB mode requires an IV as the initial
%  input block. The IV need not be secret, but it must be unpredictable.
%
%  The format of the DecipherCFBMode method is:
%
%     StringInfo *DecipherCFBMode(CipherInfo *cipher_info,
%       StringInfo *ciphertext)
%
%  A description of each parameter follows:
%
%    o cipher_info: The cipher context.
%
%    o ciphertext: The cipher text.
%
*/
static StringInfo *DecipherCFBMode(CipherInfo *cipher_info,
  StringInfo *ciphertext)
{
  register size_t
    i;

  register unsigned char
    *p,
    *q;

  size_t
    blocksize;

  StringInfo
    *plaintext;

  unsigned char
    input_block[MaxCipherBlocksize],
    output_block[MaxCipherBlocksize];

  /*
    Decipher in CFB mode.
  */
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(CipherDomain,cipher_info != (CipherInfo *) NULL);
  WizardAssert(CipherDomain,cipher_info->signature == WizardSignature);
  blocksize=cipher_info->blocksize;
  WizardAssert(CipherDomain,blocksize != 0);
  WizardAssert(CipherDomain,blocksize <= MaxCipherBlocksize);
  WizardAssert(CipherDomain,ciphertext != (StringInfo *) NULL);
  plaintext=ciphertext;
  p=GetStringInfoDatum(cipher_info->nonce);
  for (i=0; i < blocksize; i++)
    input_block[i]=p[i];
  q=GetStringInfoDatum(ciphertext)+GetStringInfoLength(ciphertext);
  for (p=GetStringInfoDatum(ciphertext); p < q; p++)
  {
    for (i=0; i < blocksize; i++)
      output_block[i]=input_block[i];
    cipher_info->encipher_block(cipher_info->handle,output_block,output_block);
    for (i=0; i < (blocksize-1); i++)
      input_block[i]=input_block[i+1];
    input_block[blocksize-1]=(*p);
    *p^=(*output_block);
  }
  /*
    Reset registers.
  */
  (void) ResetWizardMemory(input_block,0,sizeof(input_block));
  (void) ResetWizardMemory(output_block,0,sizeof(output_block));
  return(plaintext);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e c i p h e r C i p h e r                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DecipherCipher() deciphers ciphertext and returns plaintext. The deciphering
%  is performed in-place and DecipherCipher() returns a pointer to the
%  ciphertext string.
%
%  The format of the DecipherCipher method is:
%
%     StringInfo *DecipherCipher(CipherInfo *cipher_info,StringInfo *ciphertext)
%
%  A description of each parameter follows:
%
%    o cipher_info: The cipher context.
%
%    o ciphertext: The cipher text.
%
*/

WizardExport StringInfo *DecryptCipher(CipherInfo *cipher_info,
  StringInfo *plaintext)
{
  return(DecipherCipher(cipher_info,plaintext));
}

WizardExport StringInfo *DecipherCipher(CipherInfo *cipher_info,
  StringInfo *ciphertext)
{
  StringInfo
    *plaintext;

  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(CipherDomain,cipher_info != (CipherInfo *) NULL);
  WizardAssert(CipherDomain,cipher_info->signature == WizardSignature);
  WizardAssert(CipherDomain,ciphertext != (StringInfo *) NULL);
  plaintext=(StringInfo *) NULL;
  switch (cipher_info->mode)
  {
    case CBCMode:
    {
      plaintext=DecipherCBCMode(cipher_info,ciphertext);
      break;
    }
    case CFBMode:
    {
      plaintext=DecipherCFBMode(cipher_info,ciphertext);
      break;
    }
    case CTRMode:
    {
      plaintext=DecipherCTRMode(cipher_info,ciphertext);
      break;
    }
    case ECBMode:
    {
      plaintext=DecipherECBMode(cipher_info,ciphertext);
      break;
    }
    case OFBMode:
    {
      plaintext=DecipherOFBMode(cipher_info,ciphertext);
      break;
    }
    default:
      ThrowWizardFatalError(CipherDomain,EnumerateError);
  }
  return(plaintext);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   D e c i p h e r C T R M o d e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DecipherCTRMode() deciphers with the cipher in Counter mode.  This mode is a
%  confidentiality mode that features the application of the forward cipher to
%  a set of input blocks, called counters, to produce a sequence of output
%  blocks that are exclusive-ORed with the plaintext to produce the ciphertext,
%  and vice versa. The sequence of counters must have the property that each
%  block in the sequence is different from every other block. This condition is
%  not restricted to a single message: across all of the messages that are
%  enciphered under the given key, all of the counters must be distinct.
%
%  The format of the DecipherCTRMode method is:
%
%     StringInfo *DecipherCTRMode(CipherInfo *cipher_info,
%       StringInfo *ciphertext)
%
%  A description of each parameter follows:
%
%    o cipher_info: The cipher context.
%
%    o ciphertext: The cipher text.
%
*/

static inline void IncrementCipherNonce(const size_t length,
  unsigned char *nonce)
{
  register ssize_t
    i;

  for (i=(ssize_t) (length-1); i >= 0; i--)
  {
    nonce[i]++;
    if (nonce[i] != 0)
      return;
  }
  ThrowFatalException(CipherFatalError,"Sequence wrap error `%s'");
}

static StringInfo *DecipherCTRMode(CipherInfo *cipher_info,
  StringInfo *ciphertext)
{
  register size_t
    i;

  register unsigned char
    *p,
    *q;

  size_t
    blocksize;

  StringInfo
    *plaintext;

  unsigned char
    input_block[MaxCipherBlocksize],
    output_block[MaxCipherBlocksize];

  /*
    Decipher in CTR mode.
  */
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(CipherDomain,cipher_info != (CipherInfo *) NULL);
  WizardAssert(CipherDomain,cipher_info->signature == WizardSignature);
  blocksize=cipher_info->blocksize;
  WizardAssert(CipherDomain,blocksize <= MaxCipherBlocksize);
  WizardAssert(CipherDomain,ciphertext != (StringInfo *) NULL);
  plaintext=ciphertext;
  p=GetStringInfoDatum(cipher_info->nonce);
  for (i=0; i < blocksize; i++)
    input_block[i]=p[i];
  q=GetStringInfoDatum(ciphertext)+GetStringInfoLength(ciphertext);
  for (p=GetStringInfoDatum(ciphertext); p < q; p+=blocksize)
  {
    for (i=0; i < blocksize; i++)
      output_block[i]=input_block[i];
    cipher_info->encipher_block(cipher_info->handle,output_block,output_block);
    for (i=0; i < blocksize; i++)
      p[i]^=output_block[i];
    IncrementCipherNonce(blocksize,input_block);
  }
  /*
    Reset registers.
  */
  (void) ResetWizardMemory(input_block,0,sizeof(input_block));
  (void) ResetWizardMemory(output_block,0,sizeof(output_block));
  return(plaintext);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   D e c i p h e r E C B M o d e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DecipherECBMode() deciphers with the cipher in Electronic Codebook mode.
%  This mode is a confidentiality mode that features, for a given key, the
%  assignment of a fixed ciphertext block to each plaintext block, analogous to
%  the assignment of code words in a codebook.
%
%  The format of the DecipherECBMode method is:
%
%     StringInfo *DecipherECBMode(CipherInfo *cipher_info,
%       StringInfo *ciphertext)
%
%  A description of each parameter follows:
%
%    o cipher_info: The cipher context.
%
%    o ciphertext: The cipher text.
%
*/
static StringInfo *DecipherECBMode(CipherInfo *cipher_info,
  StringInfo *ciphertext)
{
  register unsigned char
    *p,
    *q;

  size_t
    blocksize;

  StringInfo
    *plaintext;

  /*
    Decipher in ECB mode.
  */
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(CipherDomain,cipher_info != (CipherInfo *) NULL);
  WizardAssert(CipherDomain,cipher_info->signature == WizardSignature);
  blocksize=cipher_info->blocksize;
  WizardAssert(CipherDomain,blocksize <= MaxCipherBlocksize);
  WizardAssert(CipherDomain,ciphertext != (StringInfo *) NULL);
  plaintext=ciphertext;
  q=GetStringInfoDatum(ciphertext)+GetStringInfoLength(ciphertext);
  for (p=GetStringInfoDatum(ciphertext); p < q; p+=blocksize)
    cipher_info->decipher_block(cipher_info->handle,p,p);
  return(plaintext);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   D e c i p h e r O F B M o d e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DecipherOFBMode() deciphers with the cipher in Output Feedback mode.  This
%  mode is a confidentiality mode that features the iteration of the forward
%  cipher on a nonce to generate a sequence of output blocks that are
%  exclusive-ORed with the plaintext to produce the ciphertext, and vice versa.
%  The OFB mode requires that the IV is a nonce, i.e., the IV must be unique
%  for each execution of the mode under the given key.
%
%  The format of the DecipherOFBMode method is:
%
%     StringInfo *DecipherOFBMode(CipherInfo *cipher_info,
%       StringInfo *ciphertext)
%
%  A description of each parameter follows:
%
%    o cipher_info: The cipher context.
%
%    o ciphertext: The cipher text.
%
*/
static StringInfo *DecipherOFBMode(CipherInfo *cipher_info,
  StringInfo *ciphertext)
{
  register size_t
    i;

  register unsigned char
    *p,
    *q;

  size_t
    blocksize;

  StringInfo
    *plaintext;

  unsigned char
    input_block[MaxCipherBlocksize];

  /*
    Decipher in OFB mode.
  */
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(CipherDomain,cipher_info != (CipherInfo *) NULL);
  WizardAssert(CipherDomain,cipher_info->signature == WizardSignature);
  blocksize=cipher_info->blocksize;
  WizardAssert(CipherDomain,blocksize <= MaxCipherBlocksize);
  WizardAssert(CipherDomain,ciphertext != (StringInfo *) NULL);
  plaintext=ciphertext;
  p=GetStringInfoDatum(cipher_info->nonce);
  for (i=0; i < blocksize; i++)
    input_block[i]=p[i];
  q=GetStringInfoDatum(ciphertext)+GetStringInfoLength(ciphertext);
  for (p=GetStringInfoDatum(ciphertext); p < q; p+=blocksize)
  {
    cipher_info->encipher_block(cipher_info->handle,input_block,input_block);
    for (i=0; i < blocksize; i++)
      p[i]^=input_block[i];
  }
  /*
    Reset registers.
  */
  (void) ResetWizardMemory(input_block,0,sizeof(input_block));
  return(plaintext);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e s t r o y C i p h e r I n f o                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroyCipherInfo() zeros memory associated with the CipherInfo structure.
%
%  The format of the DestroyCipherInfo method is:
%
%      CipherInfo *DestroyCipherInfo(CipherInfo *cipher_info)
%
%  A description of each parameter follows:
%
%    o cipher_info: The cipher info.
%
*/
WizardExport CipherInfo *DestroyCipherInfo(CipherInfo *cipher_info)
{
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(CipherDomain,cipher_info != (CipherInfo *) NULL);
  WizardAssert(CipherDomain,cipher_info->signature == WizardSignature);
  if (cipher_info->handle != (CipherInfo *) NULL)
    switch (cipher_info->cipher)
    {
      case AESCipher:
      {
        cipher_info->handle=DestroyAESInfo((AESInfo *) cipher_info->handle);
        break;
      }
      case ChachaCipher:
      {
        cipher_info->handle=DestroyChachaInfo((ChachaInfo *)
          cipher_info->handle);
        break;
      }
      case SerpentCipher:
      {
        cipher_info->handle=DestroySerpentInfo((SerpentInfo *)
          cipher_info->handle);
        break;
      }
      case TwofishCipher:
      {
        cipher_info->handle=DestroyTwofishInfo((TwofishInfo *)
          cipher_info->handle);
        break;
      }
      default:
        ThrowWizardFatalError(CipherDomain,EnumerateError);
    }
  if (cipher_info->nonce != (StringInfo *) NULL)
    cipher_info->nonce=DestroyStringInfo(cipher_info->nonce);
  if (cipher_info->random_info != (RandomInfo *) NULL)
    cipher_info->random_info=DestroyRandomInfo(cipher_info->random_info);
  cipher_info->signature=(~WizardSignature);
  cipher_info=(CipherInfo *) RelinquishWizardMemory(cipher_info);
  return(cipher_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   E n c i p h e r C B C M o d e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  EncipherCBCMode() enciphers with the cipher in Cipher Block Chaining mode.
%  This mode is a confidentiality mode whose enciphering process features the
%  combining ("chaining") of the plaintext blocks with the previous ciphertext
%  blocks.  The CBC mode requires an IV to combine with the first plaintext
%  block. The IV need not be secret, but it must be unpredictable.
%
%  The format of the EncipherCBCMode method is:
%
%      StringInfo *EncipherCBCMode(CipherInfo *cipher_info,
%        StringInfo *plaintext)
%
%  A description of each parameter follows:
%
%    o cipher_info: The cipher context.
%
%    o plaintext: The plain text.
%
*/
static StringInfo *EncipherCBCMode(CipherInfo *cipher_info,
  StringInfo *plaintext)
{
  register unsigned char
    *p,
    *q;

  register size_t
    i;

  size_t
    blocksize,
    pad;

  StringInfo
    *ciphertext;

  unsigned char
    input_block[MaxCipherBlocksize];

  /*
    Encipher in CBC mode.
  */
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(CipherDomain,cipher_info != (CipherInfo *) NULL);
  WizardAssert(CipherDomain,cipher_info->signature == WizardSignature);
  blocksize=cipher_info->blocksize;
  WizardAssert(CipherDomain,blocksize != 0);
  WizardAssert(CipherDomain,blocksize <= MaxCipherBlocksize);
  WizardAssert(CipherDomain,plaintext != (StringInfo *) NULL);
  ciphertext=plaintext;
  p=GetStringInfoDatum(cipher_info->nonce);
  for (i=0; i < blocksize; i++)
    input_block[i]=p[i];
  q=GetStringInfoDatum(plaintext)+GetStringInfoLength(plaintext);
  pad=blocksize-GetStringInfoLength(plaintext) % blocksize;
  SetRandomKey(cipher_info->random_info,pad-1,q);
  q[pad-1]=(unsigned char) (pad-1);
  if (pad == blocksize)
    q+=blocksize;
  for (p=GetStringInfoDatum(plaintext); p < q; p+=blocksize)
  {
    for (i=0; i < blocksize; i++)
      p[i]^=input_block[i];
    cipher_info->encipher_block(cipher_info->handle,p,p);
    for (i=0; i < blocksize; i++)
      input_block[i]=p[i];
  }
  /*
    Reset registers.
  */
  (void) ResetWizardMemory(input_block,0,sizeof(input_block));
  return(ciphertext);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   E n c i p h e r C F B M o d e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  EncipherCFBMode() enciphers with the cipher in Cipher Feedback mode.  This
%  mode is a confidentiality mode that features the feedback of successive
%  ciphertext segments into the input blocks of the forward cipher to generate
%  output blocks that are exclusive-ORed with the plaintext to produce the
%  ciphertext, and vice versa. The CFB mode requires a nonce as the initial
%  input block. The nonce need not be secret, but it must be unpredictable.
%
%  The format of the EncipherCFBMode method is:
%
%      StringInfo *EncipherCFBMode(CipherInfo *cipher_info,
%        StringInfo *plaintext)
%
%  A description of each parameter follows:
%
%    o cipher_info: The cipher context.
%
%    o plaintext: The plain text.
%
*/
static StringInfo *EncipherCFBMode(CipherInfo *cipher_info,
  StringInfo *plaintext)
{
  register size_t
    i;

  register unsigned char
    *p,
    *q;

  size_t
    blocksize;

  StringInfo
    *ciphertext;

  unsigned char
    input_block[MaxCipherBlocksize],
    output_block[MaxCipherBlocksize];

  /*
    Encipher in CFB mode.
  */
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(CipherDomain,cipher_info != (CipherInfo *) NULL);
  WizardAssert(CipherDomain,cipher_info->signature == WizardSignature);
  blocksize=cipher_info->blocksize;
  WizardAssert(CipherDomain,blocksize != 0);
  WizardAssert(CipherDomain,blocksize <= MaxCipherBlocksize);
  WizardAssert(CipherDomain,plaintext != (StringInfo *) NULL);
  ciphertext=plaintext;
  p=GetStringInfoDatum(cipher_info->nonce);
  for (i=0; i < blocksize; i++)
    input_block[i]=p[i];
  q=GetStringInfoDatum(plaintext)+GetStringInfoLength(plaintext);
  for (p=GetStringInfoDatum(plaintext); p < q; p++)
  {
    for (i=0; i < blocksize; i++)
      output_block[i]=input_block[i];
    cipher_info->encipher_block(cipher_info->handle,output_block,output_block);
    *p^=(*output_block);
    for (i=0; i < (blocksize-1); i++)
      input_block[i]=input_block[i+1];
    input_block[blocksize-1]=(*p);
  }
  /*
    Reset registers.
  */
  (void) ResetWizardMemory(input_block,0,sizeof(input_block));
  (void) ResetWizardMemory(output_block,0,sizeof(output_block));
  return(ciphertext);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   E n c i p h e r C i p h e r                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  EncipherCipher() enciphers plaintext and returns ciphertext.  The
%  enciphering is performed in-place and EncipherCipher() returns a pointer to
%  the plaintext string.
%
%  The format of the EncipherCipher method is:
%
%     StringInfo *EncipherCipher(CipherInfo *cipher_info,StringInfo *plaintext)
%
%  A description of each parameter follows:
%
%    o cipher_info: The cipher context.
%
%    o plaintext: The plain text.
%
*/

WizardExport StringInfo *EncryptCipher(CipherInfo *cipher_info,
  StringInfo *plaintext)
{
  return(EncipherCipher(cipher_info,plaintext));
}

WizardExport StringInfo *EncipherCipher(CipherInfo *cipher_info,
  StringInfo *plaintext)
{
  StringInfo
    *ciphertext;

  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(CipherDomain,cipher_info != (CipherInfo *) NULL);
  WizardAssert(CipherDomain,cipher_info->signature == WizardSignature);
  WizardAssert(CipherDomain,plaintext != (StringInfo *) NULL);
  ciphertext=(StringInfo *) NULL;
  switch (cipher_info->mode)
  {
    case CBCMode:
    {
      ciphertext=EncipherCBCMode(cipher_info,plaintext);
      break;
    }
    case CFBMode:
    {
      ciphertext=EncipherCFBMode(cipher_info,plaintext);
      break;
    }
    case CTRMode:
    {
      ciphertext=EncipherCTRMode(cipher_info,plaintext);
      break;
    }
    case ECBMode:
    {
      ciphertext=EncipherECBMode(cipher_info,plaintext);
      break;
    }
    case OFBMode:
    {
      ciphertext=EncipherOFBMode(cipher_info,plaintext);
      break;
    }
    default:
      ThrowWizardFatalError(CipherDomain,EnumerateError);
  }
  return(ciphertext);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   E n c i p h e r C T R M o d e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  EncipherCTRMode() enciphers with the cipher in Counter mode.  This mode is a
%  confidentiality mode that features the application of the forward cipher to
%  a set of input blocks, called counters, to produce a sequence of output
%  blocks that are exclusive-ORed with the plaintext to produce the ciphertext,
%  and vice versa. The sequence of counters must have the property that each
%  block in the sequence is different from every other block. This condition is
%  not restricted to a single message: across all of the messages that are
%  enciphered under the given key, all of the counters must be distinct.
%
%  The format of the EncipherCTRMode method is:
%
%      StringInfo *EncipherCTRMode(CipherInfo *cipher_info,
%        StringInfo *plaintext)
%
%  A description of each parameter follows:
%
%    o cipher_info: The cipher context.
%
%    o plaintext: The plain text.
%
*/
static StringInfo *EncipherCTRMode(CipherInfo *cipher_info,
  StringInfo *plaintext)
{
  register size_t
    i;

  register unsigned char
    *p,
    *q;

  size_t
    blocksize,
    pad;

  StringInfo
    *ciphertext;

  unsigned char
    input_block[MaxCipherBlocksize],
    output_block[MaxCipherBlocksize];

  /*
    Encipher in CTR mode.
  */
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(CipherDomain,cipher_info != (CipherInfo *) NULL);
  WizardAssert(CipherDomain,cipher_info->signature == WizardSignature);
  blocksize=cipher_info->blocksize;
  WizardAssert(CipherDomain,blocksize != 0);
  WizardAssert(CipherDomain,blocksize <= MaxCipherBlocksize);
  WizardAssert(CipherDomain,plaintext != (StringInfo *) NULL);
  ciphertext=plaintext;
  p=GetStringInfoDatum(cipher_info->nonce);
  for (i=0; i < blocksize; i++)
    input_block[i]=p[i];
  q=GetStringInfoDatum(plaintext)+GetStringInfoLength(plaintext);
  pad=blocksize-GetStringInfoLength(plaintext) % blocksize;
  SetRandomKey(cipher_info->random_info,pad-1,q);
  q[pad-1]=(unsigned char) (pad-1);
  if (pad == blocksize)
    q+=blocksize;
  for (p=GetStringInfoDatum(plaintext); p < q; p+=blocksize)
  {
    for (i=0; i < blocksize; i++)
      output_block[i]=input_block[i];
    cipher_info->encipher_block(cipher_info->handle,output_block,output_block);
    for (i=0; i < blocksize; i++)
      p[i]^=output_block[i];
    IncrementCipherNonce(blocksize,input_block);
  }
  /*
    Reset registers.
  */
  (void) ResetWizardMemory(input_block,0,sizeof(input_block));
  (void) ResetWizardMemory(output_block,0,sizeof(output_block));
  return(ciphertext);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   E n c i p h e r E C B M o d e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  EncipherECBMode() enciphers with the cipher in Electronic Codebook mode.
%  This mode is a confidentiality mode that features, for a given key, the
%  assignment of a fixed ciphertext block to each plaintext block, analogous to
%  the assignment of code words in a codebook.
%
%  The format of the EncipherECBMode method is:
%
%      StringInfo *EncipherECBMode(CipherInfo *cipher_info,
%        StringInfo *plaintext)
%
%  A description of each parameter follows:
%
%    o cipher_info: The cipher context.
%
%    o plaintext: The plain text.
%
*/
static StringInfo *EncipherECBMode(CipherInfo *cipher_info,
  StringInfo *plaintext)
{
  register unsigned char
    *p,
    *q;

  size_t
    blocksize,
    pad;

  StringInfo
    *ciphertext;

  /*
    Encipher in ECB mode.
  */
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(CipherDomain,cipher_info != (CipherInfo *) NULL);
  WizardAssert(CipherDomain,cipher_info->signature == WizardSignature);
  blocksize=cipher_info->blocksize;
  WizardAssert(CipherDomain,blocksize <= MaxCipherBlocksize);
  WizardAssert(CipherDomain,plaintext != (StringInfo *) NULL);
  ciphertext=plaintext;
  q=GetStringInfoDatum(plaintext)+GetStringInfoLength(plaintext);
  pad=blocksize-GetStringInfoLength(plaintext) % blocksize;
  SetRandomKey(cipher_info->random_info,pad-1,q);
  q[pad-1]=(unsigned char) (pad-1);
  if (pad == blocksize)
    q+=blocksize;
  for (p=GetStringInfoDatum(plaintext); p < q; p+=blocksize)
    cipher_info->encipher_block(cipher_info->handle,p,p);
  return(ciphertext);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   E n c i p h e r O F B M o d e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  EncipherOFBMode() enciphers with the cipher in Output Feedback mode.  This
%  mode is a confidentiality mode that features the iteration of the forward
%  cipher on a nonce to generate a sequence of output blocks that are
%  exclusive-ORed with the plaintext to produce the ciphertext, and vice versa.
%  The OFB mode requires that the IV is a nonce, i.e., the IV must be unique
%  for each execution of the mode under the given key.
%
%  The format of the EncipherOFBMode method is:
%
%      StringInfo *EncipherOFBMode(CipherInfo *cipher_info,
%        StringInfo *plaintext)
%
%  A description of each parameter follows:
%
%    o cipher_info: The cipher context.
%
%    o plaintext: The plain text.
%
*/
static StringInfo *EncipherOFBMode(CipherInfo *cipher_info,
  StringInfo *plaintext)
{
  register size_t
    i;

  register unsigned char
    *p,
    *q;

  size_t
    blocksize,
    pad;

  StringInfo
    *ciphertext;

  unsigned char
    input_block[MaxCipherBlocksize];

  /*
    Encipher in OBC mode.
  */
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(CipherDomain,cipher_info != (CipherInfo *) NULL);
  WizardAssert(CipherDomain,cipher_info->signature == WizardSignature);
  blocksize=cipher_info->blocksize;
  WizardAssert(CipherDomain,blocksize != 0);
  WizardAssert(CipherDomain,blocksize <= MaxCipherBlocksize);
  WizardAssert(CipherDomain,plaintext != (StringInfo *) NULL);
  ciphertext=plaintext;
  p=GetStringInfoDatum(cipher_info->nonce);
  for (i=0; i < blocksize; i++)
    input_block[i]=p[i];
  q=GetStringInfoDatum(plaintext)+GetStringInfoLength(plaintext);
  pad=blocksize-GetStringInfoLength(plaintext) % blocksize;
  SetRandomKey(cipher_info->random_info,pad-1,q);
  q[pad-1]=(unsigned char) (pad-1);
  if (pad == blocksize)
    q+=blocksize;
  for (p=GetStringInfoDatum(plaintext); p < q; p+=blocksize)
  {
    cipher_info->encipher_block(cipher_info->handle,input_block,input_block);
    for (i=0; i < blocksize; i++)
      p[i]^=input_block[i];
  }
  /*
    Reset registers.
  */
  (void) ResetWizardMemory(input_block,0,sizeof(input_block));
  return(ciphertext);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t C i p h e r B l o c k s i z e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetCipherBlocksize() returns the cipher blocksize.
%
%  The format of the GetCipherBlocksize method is:
%
%      size_t GetCipherBlocksize(const CipherInfo *cipher_info)
%
%  A description of each parameter follows:
%
%    o cipher_info: The cipher info.
%
*/
WizardExport size_t GetCipherBlocksize(const CipherInfo *cipher_info)
{
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(CipherDomain,cipher_info != (CipherInfo *) NULL);
  WizardAssert(CipherDomain,cipher_info->signature == WizardSignature);
  return(cipher_info->blocksize);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e n e r a t e C i p h e r N o n c e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GenerateCipherNonce() generate a nonce for the given cipher.
%
%  The format of the GenerateCipherNonce method is:
%
%     StringInfo *GenerateCipherNonce(CipherInfo *cipher_info)
%
%  A description of each parameter follows:
%
%    o cipher_info: The cipher context.
%
*/
WizardExport StringInfo *GenerateCipherNonce(CipherInfo *cipher_info)
{
  StringInfo
    *nonce;

  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(CipherDomain,cipher_info != (CipherInfo *) NULL);
  WizardAssert(CipherDomain,cipher_info->signature == WizardSignature);
  switch (cipher_info->mode)
  {
    case CBCMode:
    case CFBMode:
    case ECBMode:
    case OFBMode:
    {
      nonce=GetRandomKey(cipher_info->random_info,cipher_info->blocksize);
      break;
    }
    case CTRMode:
    {
      nonce=AcquireStringInfo(cipher_info->blocksize);
      ResetStringInfo(nonce);
      SetRandomKey(cipher_info->random_info,(cipher_info->blocksize+1)/2,
        GetStringInfoDatum(nonce));
      break;
    }
    default:
      ThrowWizardFatalError(CipherDomain,EnumerateError);
  }
  return(nonce);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t C i p h e r N o n c e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetCipherNonce() returns a nonce of the cipher.
%
%  The format of the GetCipherNonce method is:
%
%     const StringInfo *GetCipherNonce(CipherInfo *cipher_info)
%
%  A description of each parameter follows:
%
%    o cipher_info: The cipher context.
%
*/
WizardExport const StringInfo *GetCipherNonce(CipherInfo *cipher_info)
{
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(CipherDomain,cipher_info != (CipherInfo *) NULL);
  WizardAssert(CipherDomain,cipher_info->signature == WizardSignature);
  return(cipher_info->nonce);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e s e t C i p h e r N o n c e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ResetCipherNonce() resets the initialization vector for the cipher.
%
%  The format of the ResetCipherNonce method is:
%
%      ResetCipherNonce(CipherInfo *cipher_info)
%
%  A description of each parameter follows:
%
%    o cipher_info: The cipher context.
%
*/
WizardExport void ResetCipherNonce(CipherInfo *cipher_info)
{
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(CipherDomain,cipher_info != (CipherInfo *) NULL);
  WizardAssert(CipherDomain,cipher_info->signature == WizardSignature);
  ResetStringInfo(cipher_info->nonce);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t C i p h e r N o n c e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetCipherNonce() sets the initialization vector for the cipher.
%
%  The format of the SetCipherNonce method is:
%
%      SetCipherNonce(CipherInfo *cipher_info,const StringInfo *nonce)
%
%  A description of each parameter follows:
%
%    o cipher_info: The cipher context.
%
%    o nonce: The initialization vector.
%
*/
WizardExport void SetCipherNonce(CipherInfo *cipher_info,
  const StringInfo *nonce)
{
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(CipherDomain,cipher_info != (CipherInfo *) NULL);
  WizardAssert(CipherDomain,cipher_info->signature == WizardSignature);
  WizardAssert(CipherDomain,nonce != (StringInfo *) NULL);
  SetStringInfo(cipher_info->nonce,nonce);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t C i p h e r K e y                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetCipherKey() sets the key for the cipher.  The key length is specified
%  in bits.  Valid values are 128, 192, or 256.
%
%  The format of the SetCipherKey method is:
%
%      SetCipherKey(CipherInfo *cipher_info,const StringInfo *key)
%
%  A description of each parameter follows:
%
%    o cipher_info: The cipher context.
%
%    o key: The key.
%
*/
WizardExport void SetCipherKey(CipherInfo *cipher_info,const StringInfo *key)
{
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(CipherDomain,cipher_info != (CipherInfo *) NULL);
  WizardAssert(CipherDomain,cipher_info->signature == WizardSignature);
  WizardAssert(CipherDomain,key != (StringInfo *) NULL);
  switch (cipher_info->cipher)
  {
    case AESCipher:
    {
      SetAESKey((AESInfo *) cipher_info->handle,key);
      break;
    }
    case ChachaCipher:
    {
      SetChachaKey((ChachaInfo *) cipher_info->handle,key);
      break;
    }
    case SerpentCipher:
    {
      SetSerpentKey((SerpentInfo *) cipher_info->handle,key);
      break;
    }
    case TwofishCipher:
    {
      SetTwofishKey((TwofishInfo *) cipher_info->handle,key);
      break;
    }
    default:
      ThrowWizardFatalError(CipherDomain,EnumerateError);
  }
}
