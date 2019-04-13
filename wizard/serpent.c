/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%               SSSSS  EEEEE  RRRR   PPPP   EEEEE  N   N  TTTTT               %
%               SS     E      R   R  P   P  E      NN  N    T                 %
%                SSS   EEE    RRRR   PPPP   EEE    N N N    T                 %
%                  SS  E      R R    P      E      N  NN    T                 %
%               SSSSS  EEEEE  R  R   P      EEEEE  N   N    T                 %
%                                                                             %
%                                                                             %
%                    Wizard's Toolkit Serpent Cipher Methods                  %
%                                                                             %
%                               Software Design                               %
%                                   Cristy                                    %
%                                 March 2003                                  %
%                                                                             %
%  Copyright 1999-2019 ImageMagick Studio LLC, a non-profit organization      %
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
%
*/

/*
  Include declarations.
*/
#include "wizard/studio.h"
#include "wizard/exception.h"
#include "wizard/exception-private.h"
#include "wizard/memory_.h"
#include "wizard/serpent.h"

/*
  Typedef declarations.
*/
struct _SerpentInfo
{
  unsigned int
    blocksize,
    crypt_key[132];

  time_t
    timestamp;

  size_t
    signature;
};

/*
  Define declarations.
*/
#define Read32Bits(value,p) \
{ \
  (value)=(*p++); \
  (value)|=(*p++) << 8; \
  (value)|=(*p++) << 16; \
  (value)|=(*p++) << 24; \
}
#define SerpentBlocksize 32
#define Write32Bits(p,value) \
{ \
  *p++=(unsigned char) (value); \
  *p++=(unsigned char) ((value) >> 8); \
  *p++=(unsigned char) ((value) >> 16); \
  *p++=(unsigned char) ((value) >> 24); \
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A c q u i r e S e r p e n t I n f o                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireSerpentInfo() allocate the SerpentInfo structure.
%
%  The format of the AcquireSerpentInfo method is:
%
%      SerpentInfo *AcquireSerpentInfo(void)
%
*/
WizardExport SerpentInfo *AcquireSerpentInfo(void)
{
  SerpentInfo
    *serpent_info;

  serpent_info=(SerpentInfo *) AcquireWizardMemory(sizeof(*serpent_info));
  if (serpent_info == (SerpentInfo *) NULL)
    ThrowWizardFatalError(CipherError,MemoryError);
  (void) ResetWizardMemory(serpent_info,0,sizeof(*serpent_info));
  serpent_info->blocksize=SerpentBlocksize;
  serpent_info->timestamp=time((time_t *) NULL);
  serpent_info->signature=WizardSignature;
  return(serpent_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e c i p h e r S e r p e n t B l o c k                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DecipherSerpentBlock() deciphers a single block of ciphertext to produce a
%  block of plaintext.
%
%  The format of the DecipherSerpentBlock method is:
%
%     void DecipherSerpentBlock(SerpentInfo *serpent_info,
%       const unsigned char *ciphertext,unsigned char plaintext)
%
%  A description of each parameter follows:
%
%    o serpent_info: The cipher context.
%
%    o ciphertext: The cipher text.
%
%    o plaintext: The plaint text.
%
*/

static inline unsigned int Trunc32(unsigned int x)
{
  return((unsigned int) (x & 0xffffffffUL));
}

static inline unsigned int RotateRight(unsigned int x,unsigned int n)
{
  return(Trunc32((x >> n) | (x << (32-n))));
}

WizardExport void DecipherSerpentBlock(SerpentInfo *serpent_info,
  const unsigned char *ciphertext,unsigned char *plaintext)
{
#define DecipherSbox0(alpha,beta,gamma,delta,epsilon) \
{ \
  beta^=alpha; epsilon=delta; delta|=beta; epsilon^=beta; \
  alpha=(~alpha); gamma^=delta; delta^=alpha; alpha&=beta; \
  alpha^=gamma; gamma&=delta; delta^=epsilon; gamma^=delta; \
  beta^=delta; delta&=alpha; beta^=alpha; alpha^=gamma; epsilon^=delta; \
}
#define DecipherSbox1(alpha,beta,gamma,delta,epsilon) \
{ \
  beta^=delta; epsilon=alpha; alpha^=gamma; gamma=(~gamma); \
  epsilon|=beta; epsilon^=delta; delta&=beta; beta^=gamma; \
  gamma&=epsilon; epsilon^=beta; beta|=delta; delta^=alpha; \
  gamma^=alpha; alpha|=epsilon; gamma^=epsilon; beta^=alpha; epsilon^=beta; \
}
#define DecipherSbox2(alpha,beta,gamma,delta,epsilon) \
{ \
  gamma^=beta; epsilon=delta; delta=(~delta); delta|=gamma; \
  gamma^=epsilon; epsilon^=alpha; delta^=beta; beta|=gamma; \
  gamma^=alpha;  beta^=epsilon; epsilon|=delta; gamma^=delta; \
  epsilon^=gamma; gamma&=beta; gamma^=delta; delta^=epsilon; epsilon^=alpha; \
}
#define DecipherSbox3(alpha,beta,gamma,delta,epsilon) \
{ \
  gamma^=beta; epsilon=beta; beta&=gamma; beta^=alpha; \
  alpha|=epsilon; epsilon^=delta; alpha^=delta; delta|=beta; \
  beta^=gamma; beta^=delta; alpha^=gamma; gamma^=delta; delta&=beta; \
  beta^=alpha; alpha&=gamma; epsilon^=delta; delta^=alpha; alpha^=beta; \
}
#define DecipherSbox4(alpha,beta,gamma,delta,epsilon) \
{ \
  gamma^=delta; epsilon=alpha; alpha&=beta; alpha^=gamma; \
  gamma|=delta; epsilon=(~epsilon); beta^=alpha; alpha^=gamma; \
  gamma&=epsilon; gamma^=alpha; alpha|=epsilon; alpha^=delta; \
  delta&=gamma; epsilon^=delta; delta^=beta; beta&=alpha; \
  epsilon^=beta; alpha^=delta; \
}
#define DecipherSbox5(alpha,beta,gamma,delta,epsilon) \
{ \
  epsilon=beta; beta|=gamma; gamma^=epsilon; beta^=delta; \
  delta&=epsilon; gamma^=delta; delta|=alpha; alpha=(~alpha); \
  delta^=gamma; gamma|=alpha; epsilon^=beta; gamma^=epsilon; \
  epsilon&=alpha; alpha^=beta; beta^=delta; alpha&=gamma; \
  gamma^=delta; alpha^=gamma; gamma^=epsilon; epsilon^=delta; \
}
#define DecipherSbox6(alpha,beta,gamma,delta,epsilon) \
{ \
  alpha^=gamma; epsilon=alpha; alpha&=delta; gamma^=delta; \
  alpha^=gamma; delta^=beta; gamma|=epsilon; gamma^=delta; \
  delta&=alpha; alpha=(~alpha); delta^=beta; beta&=gamma; \
  epsilon^=alpha; delta^=epsilon; epsilon^=gamma; alpha^=beta; gamma^=alpha; \
}
#define DecipherSbox7(alpha,beta,gamma,delta,epsilon) \
{ \
  epsilon=delta; delta&=alpha; alpha^=gamma; gamma|=epsilon; \
  epsilon^=beta; alpha=(~alpha); beta|=delta; epsilon^=alpha; \
  alpha&=gamma; alpha^=beta; beta&=gamma; delta^=gamma; \
  epsilon^=delta; gamma&=delta; delta|=alpha; beta^=epsilon; \
  delta^=epsilon; epsilon&=alpha;  epsilon^=gamma; \
}
#define DecipherK(alpha,beta,gamma,delta,epsilon,i) \
{ \
  MixKey(alpha,beta,gamma,delta,i) \
  alpha=RotateRight(alpha,5); gamma=RotateRight(gamma,22); \
  alpha^=delta; gamma^=delta; epsilon=beta << 7; alpha^=beta; \
  beta=RotateRight(beta,1); gamma^=epsilon; delta=RotateRight(delta,7); \
  epsilon=alpha << 3; beta^=alpha; delta^=epsilon; \
  alpha=RotateRight(alpha,13); beta^=gamma; delta^=gamma; \
  gamma=RotateRight(gamma,3); \
}
#define MixKey(alpha,beta,gamma,delta,i) \
{ \
  alpha^=crypt_key[4*(i)+0]; beta^=crypt_key[4*(i)+1]; \
  gamma^=crypt_key[4*(i)+2]; delta^=crypt_key[4*(i)+3]; \
}

  register const unsigned char
    *p;

  register unsigned char
    *q;

  register unsigned int
    *crypt_key;

  unsigned int
    alpha,
    beta,
    gamma,
    delta,
    epsilon;

  /*
    Exercise 32 deciphering rounds.
  */
  p=(const unsigned char *) ciphertext;
  Read32Bits(alpha,p);
  Read32Bits(beta,p);
  Read32Bits(gamma,p);
  Read32Bits(delta,p);
  crypt_key=serpent_info->crypt_key;
  MixKey(alpha,beta,gamma,delta,32);
  DecipherSbox7(alpha,beta,gamma,delta,epsilon);
  DecipherK(beta,delta,alpha,epsilon,gamma,31);
  DecipherSbox6(beta,delta,alpha,epsilon,gamma);
  DecipherK(alpha,gamma,epsilon,beta,delta,30);
  DecipherSbox5(alpha,gamma,epsilon,beta,delta);
  DecipherK(gamma,delta,alpha,epsilon,beta,29);
  DecipherSbox4(gamma,delta,alpha,epsilon,beta);
  DecipherK(gamma,alpha,beta,epsilon,delta,28);
  DecipherSbox3(gamma,alpha,beta,epsilon,delta);
  DecipherK(beta,gamma,delta,epsilon,alpha,27);
  DecipherSbox2(beta,gamma,delta,epsilon,alpha);
  DecipherK(gamma,alpha,epsilon,delta,beta,26);
  DecipherSbox1(gamma,alpha,epsilon,delta,beta);
  DecipherK(beta,alpha,epsilon,delta,gamma,25);
  DecipherSbox0(beta,alpha,epsilon,delta,gamma);
  DecipherK(epsilon,gamma,alpha,beta,delta,24);
  DecipherSbox7(epsilon,gamma,alpha,beta,delta);
  DecipherK(gamma,beta,epsilon,delta,alpha,23);
  DecipherSbox6(gamma,beta,epsilon,delta,alpha);
  DecipherK(epsilon,alpha,delta,gamma,beta,22);
  DecipherSbox5(epsilon,alpha,delta,gamma,beta);
  DecipherK(alpha,beta,epsilon,delta,gamma,21);
  DecipherSbox4(alpha,beta,epsilon,delta,gamma);
  DecipherK(alpha,epsilon,gamma,delta,beta,20);
  DecipherSbox3(alpha,epsilon,gamma,delta,beta);
  DecipherK(gamma,alpha,beta,delta,epsilon,19);
  DecipherSbox2(gamma,alpha,beta,delta,epsilon);
  DecipherK(alpha,epsilon,delta,beta,gamma,18);
  DecipherSbox1(alpha,epsilon,delta,beta,gamma);
  DecipherK(gamma,epsilon,delta,beta,alpha,17);
  DecipherSbox0(gamma,epsilon,delta,beta,alpha);
  DecipherK(delta,alpha,epsilon,gamma,beta,16);
  DecipherSbox7(delta,alpha,epsilon,gamma,beta);
  DecipherK(alpha,gamma,delta,beta,epsilon,15);
  DecipherSbox6(alpha,gamma,delta,beta,epsilon);
  DecipherK(delta,epsilon,beta,alpha,gamma,14);
  DecipherSbox5(delta,epsilon,beta,alpha,gamma);
  DecipherK(epsilon,gamma,delta,beta,alpha,13);
  DecipherSbox4(epsilon,gamma,delta,beta,alpha);
  DecipherK(epsilon,delta,alpha,beta,gamma,12);
  DecipherSbox3(epsilon,delta,alpha,beta,gamma);
  DecipherK(alpha,epsilon,gamma,beta,delta,11);
  DecipherSbox2(alpha,epsilon,gamma,beta,delta);
  DecipherK(epsilon,delta,beta,gamma,alpha,10);
  DecipherSbox1(epsilon,delta,beta,gamma,alpha);
  DecipherK(alpha,delta,beta,gamma,epsilon,9);
  DecipherSbox0(alpha,delta,beta,gamma,epsilon);
  DecipherK(beta,epsilon,delta,alpha,gamma,8);
  DecipherSbox7(beta,epsilon,delta,alpha,gamma);
  DecipherK(epsilon,alpha,beta,gamma,delta,7);
  DecipherSbox6(epsilon,alpha,beta,gamma,delta);
  DecipherK(beta,delta,gamma,epsilon,alpha,6);
  DecipherSbox5(beta,delta,gamma,epsilon,alpha);
  DecipherK(delta,alpha,beta,gamma,epsilon,5);
  DecipherSbox4(delta,alpha,beta,gamma,epsilon);
  DecipherK(delta,beta,epsilon,gamma,alpha,4);
  DecipherSbox3(delta,beta,epsilon,gamma,alpha);
  DecipherK(epsilon,delta,alpha,gamma,beta,3);
  DecipherSbox2(epsilon,delta,alpha,gamma,beta);
  DecipherK(delta,beta,gamma,alpha,epsilon,2);
  DecipherSbox1(delta,beta,gamma,alpha,epsilon);
  DecipherK(epsilon,beta,gamma,alpha,delta,1);
  DecipherSbox0(epsilon,beta,gamma,alpha,delta);
  MixKey(gamma,delta,beta,epsilon,0);
  q=(unsigned char *) plaintext;
  Write32Bits(q,gamma);
  Write32Bits(q,delta);
  Write32Bits(q,beta);
  Write32Bits(q,epsilon);
  /*
    Reset registers.
  */
  alpha=0;
  beta=0;
  gamma=0;
  delta=0;
  epsilon=0;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e s t r o y S e r p e n t I n f o                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroySerpentInfo() zeros memory associated with the SerpentInfo structure.
%
%  The format of the DestroySerpentInfo method is:
%
%      SerpentInfo *DestroySerpentInfo(SerpentInfo *serpent_info)
%
%  A description of each parameter follows:
%
%    o serpent_info: The cipher context.
%
*/
WizardExport SerpentInfo *DestroySerpentInfo(SerpentInfo *serpent_info)
{
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(CipherDomain,serpent_info != (SerpentInfo *) NULL);
  WizardAssert(CipherDomain,serpent_info->signature == WizardSignature);
  serpent_info->signature=(~WizardSignature);
  serpent_info=(SerpentInfo *) RelinquishWizardMemory(serpent_info);
  return(serpent_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   E n c i p h e r S e r p e n t B l o c k                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  EncipherSerpentBlock() enciphers a single block of plaintext to produce a
%  block of ciphertext.
%
%  The format of the EncipherSerpentBlock method is:
%
%      void EncipherSerpentBlock(SerpentInfo *serpent_info,
%        const unsigned char *plaintext,unsigned char ciphertext)
%
%  A description of each parameter follows:
%
%    o serpent_info: The cipher context.
%
%    o plaintext: The plain text.
%
%    o ciphertext: The cipher text.
%
*/

static inline unsigned int RotateLeft(unsigned int x,unsigned int n)
{
  return(Trunc32((x << n) | (x >> (32-n))));
}

WizardExport void EncipherSerpentBlock(SerpentInfo *serpent_info,
  const unsigned char *plaintext,unsigned char *ciphertext)
{
#define EncipherK(alpha,beta,gamma,delta,epsilon,i) \
{ \
  alpha=RotateLeft(alpha,13); gamma=RotateLeft(gamma,3); \
  beta^=alpha; epsilon=alpha << 3; delta^=gamma; beta^=gamma; \
  beta=RotateLeft(beta,1); delta^=epsilon; delta=RotateLeft(delta,7); \
  epsilon=beta << 7; alpha^=beta; gamma^=delta; alpha^=delta; \
  gamma^=epsilon; delta^=crypt_key[4*(i)+3]; beta^=crypt_key[4*(i)+1]; \
  alpha=RotateLeft(alpha,5); gamma=RotateLeft(gamma,22); \
  alpha^=crypt_key[4*(i)+0]; gamma^=crypt_key[4*(i)+2]; \
}
#define EncipherSbox0(alpha,beta,gamma,delta,epsilon) \
{ \
  epsilon=delta; delta|=alpha; alpha^=epsilon; epsilon^=gamma; \
  epsilon=(~epsilon); delta^=beta; beta&=alpha; beta^=epsilon; \
  gamma^=alpha; alpha^=delta; epsilon|=alpha; alpha^=gamma; \
  gamma&=beta; delta^=gamma; beta=(~beta); gamma^=epsilon; beta^=gamma; \
}
#define EncipherSbox1(alpha,beta,gamma,delta,epsilon) \
{ \
  epsilon=beta; beta^=alpha; alpha^=delta; delta=~delta; \
  epsilon&=beta; alpha|=beta; delta^=gamma; alpha^=delta; \
  beta^=delta; delta^=epsilon; beta|=epsilon; epsilon^=gamma; \
  gamma&=alpha; gamma^=beta; beta|=alpha; alpha=(~alpha); \
  alpha^=gamma; epsilon^=beta; \
}
#define EncipherSbox2(alpha,beta,gamma,delta,epsilon) \
{ \
  delta=(~delta); beta^=alpha; epsilon=alpha; alpha&=gamma; \
  alpha^=delta; delta|=epsilon; gamma^=beta; delta^=beta; \
  beta&=alpha; alpha^=gamma; gamma&=delta; delta|=beta; \
  alpha=(~alpha); delta^=alpha; epsilon^=alpha; alpha^=gamma; beta|=gamma; \
}
#define EncipherSbox3(alpha,beta,gamma,delta,epsilon) \
{ \
  epsilon=beta; beta^=delta; delta|=alpha; epsilon&=alpha; \
  alpha^=gamma; gamma^=beta; beta&=delta; gamma^=delta; \
  alpha|=epsilon; epsilon^=delta; beta^=alpha; alpha&=delta; \
  delta&=epsilon; delta^=gamma; epsilon|=beta; gamma&=beta; \
  epsilon^=delta; alpha^=delta; delta^=gamma; \
}
#define EncipherSbox4(alpha,beta,gamma,delta,epsilon) \
{ \
  epsilon=delta; delta&=alpha; alpha^=epsilon; delta^=gamma; \
  gamma|=epsilon; alpha^=beta; epsilon^=delta; gamma|=alpha; \
  gamma^=beta; beta&=alpha; beta^=epsilon; epsilon&=gamma; \
  gamma^=delta; epsilon^=alpha; delta|=beta; beta=(~beta); delta^=alpha; \
}
#define EncipherSbox5(alpha,beta,gamma,delta,epsilon) \
{ \
  epsilon=beta;  beta|=alpha; gamma^=beta; delta=(~delta); \
  epsilon^=alpha; alpha^=gamma; beta&=epsilon; epsilon|=delta; \
  epsilon^=alpha; alpha&=delta; beta^=delta; delta^=gamma; \
  alpha^=beta; gamma&=epsilon; beta^=gamma; gamma&=alpha; delta^=gamma; \
}
#define EncipherSbox6(alpha,beta,gamma,delta,epsilon) \
{ \
  epsilon=beta;  delta^=alpha;  beta^=gamma;  gamma^=alpha; \
  alpha&=delta; beta|=delta; epsilon=(~epsilon); alpha^=beta; \
  beta^=gamma; delta^=epsilon; epsilon^=alpha; gamma&=alpha; \
  epsilon^=beta; gamma^=delta; delta&=beta; delta^=alpha; beta^=gamma; \
}
#define EncipherSbox7(alpha,beta,gamma,delta,epsilon) \
{ \
  beta=(~beta); epsilon=beta; alpha=(~alpha); beta&=gamma; \
  beta^=delta; delta|=epsilon; epsilon^=gamma; gamma^=delta; \
  delta^=alpha; alpha|=beta;  gamma&=alpha; alpha^=epsilon; \
  epsilon^=delta; delta&=alpha; epsilon^=beta; gamma^=epsilon; \
  delta^=beta; epsilon|=alpha; epsilon^=beta; \
}

  register const unsigned char
    *p;

  register unsigned char
    *q;

  register unsigned int
    *crypt_key;

  unsigned int
    alpha,
    beta,
    gamma,
    delta,
    epsilon;

  /*
    Exercise 32 enciphering rounds.
  */
  p=(const unsigned char *) plaintext;
  Read32Bits(alpha,p);
  Read32Bits(beta,p);
  Read32Bits(gamma,p);
  Read32Bits(delta,p);
  crypt_key=serpent_info->crypt_key;
  MixKey(alpha,beta,gamma,delta,0);
  EncipherSbox0(alpha,beta,gamma,delta,epsilon);
  EncipherK(gamma,beta,delta,alpha,epsilon,1);
  EncipherSbox1(gamma,beta,delta,alpha,epsilon);
  EncipherK(epsilon,delta,alpha,gamma,beta,2);
  EncipherSbox2(epsilon,delta,alpha,gamma,beta);
  EncipherK(beta,delta,epsilon,gamma,alpha,3);
  EncipherSbox3(beta,delta,epsilon,gamma,alpha);
  EncipherK(gamma,alpha,delta,beta,epsilon,4);
  EncipherSbox4(gamma,alpha,delta,beta,epsilon);
  EncipherK(alpha,delta,beta,epsilon,gamma,5);
  EncipherSbox5(alpha,delta,beta,epsilon,gamma);
  EncipherK(gamma,alpha,delta,epsilon,beta,6);
  EncipherSbox6(gamma,alpha,delta,epsilon,beta);
  EncipherK(delta,beta,alpha,epsilon,gamma,7);
  EncipherSbox7(delta,beta,alpha,epsilon,gamma);
  EncipherK(gamma,alpha,epsilon,delta,beta,8);
  EncipherSbox0(gamma,alpha,epsilon,delta,beta);
  EncipherK(epsilon,alpha,delta,gamma,beta,9);
  EncipherSbox1(epsilon,alpha,delta,gamma,beta);
  EncipherK(beta,delta,gamma,epsilon,alpha,10);
  EncipherSbox2(beta,delta,gamma,epsilon,alpha);
  EncipherK(alpha,delta,beta,epsilon,gamma,11);
  EncipherSbox3(alpha,delta,beta,epsilon,gamma);
  EncipherK(epsilon,gamma,delta,alpha,beta,12);
  EncipherSbox4(epsilon,gamma,delta,alpha,beta);
  EncipherK(gamma,delta,alpha,beta,epsilon,13);
  EncipherSbox5(gamma,delta,alpha,beta,epsilon);
  EncipherK(epsilon,gamma,delta,beta,alpha,14);
  EncipherSbox6(epsilon,gamma,delta,beta,alpha);
  EncipherK(delta,alpha,gamma,beta,epsilon,15);
  EncipherSbox7(delta,alpha,gamma,beta,epsilon);
  EncipherK(epsilon,gamma,beta,delta,alpha,16);
  EncipherSbox0(epsilon,gamma,beta,delta,alpha);
  EncipherK(beta,gamma,delta,epsilon,alpha,17);
  EncipherSbox1(beta,gamma,delta,epsilon,alpha);
  EncipherK(alpha,delta,epsilon,beta,gamma,18);
  EncipherSbox2(alpha,delta,epsilon,beta,gamma);
  EncipherK(gamma,delta,alpha,beta,epsilon,19);
  EncipherSbox3(gamma,delta,alpha,beta,epsilon);
  EncipherK(beta,epsilon,delta,gamma,alpha,20);
  EncipherSbox4(beta,epsilon,delta,gamma,alpha);
  EncipherK(epsilon,delta,gamma,alpha,beta,21);
  EncipherSbox5(epsilon,delta,gamma,alpha,beta);
  EncipherK(beta,epsilon,delta,alpha,gamma,22);
  EncipherSbox6(beta,epsilon,delta,alpha,gamma);
  EncipherK(delta,gamma,epsilon,alpha,beta,23);
  EncipherSbox7(delta,gamma,epsilon,alpha,beta);
  EncipherK(beta,epsilon,alpha,delta,gamma,24);
  EncipherSbox0(beta,epsilon,alpha,delta,gamma);
  EncipherK(alpha,epsilon,delta,beta,gamma,25);
  EncipherSbox1(alpha,epsilon,delta,beta,gamma);
  EncipherK(gamma,delta,beta,alpha,epsilon,26);
  EncipherSbox2(gamma,delta,beta,alpha,epsilon);
  EncipherK(epsilon,delta,gamma,alpha,beta,27);
  EncipherSbox3(epsilon,delta,gamma,alpha,beta);
  EncipherK(alpha,beta,delta,epsilon,gamma,28);
  EncipherSbox4(alpha,beta,delta,epsilon,gamma);
  EncipherK(beta,delta,epsilon,gamma,alpha,29);
  EncipherSbox5(beta,delta,epsilon,gamma,alpha);
  EncipherK(alpha,beta,delta,gamma,epsilon,30);
  EncipherSbox6(alpha,beta,delta,gamma,epsilon);
  EncipherK(delta,epsilon,beta,gamma,alpha,31);
  EncipherSbox7(delta,epsilon,beta,gamma,alpha);
  MixKey(alpha,beta,gamma,delta,32);
  q=(unsigned char * ) ciphertext;
  Write32Bits(q,alpha);
  Write32Bits(q,beta);
  Write32Bits(q,gamma);
  Write32Bits(q,delta);
  /*
    Reset registers.
  */
  alpha=0;
  beta=0;
  gamma=0;
  delta=0;
  epsilon=0;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t S e r p e n t B l o c k s i z e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetSerpentBlocksize() returns the Serpent blocksize.
%
%  The format of the GetSerpentBlocksize method is:
%
%      unsigned int *GetSerpentBlocksize(const SerpentInfo *serpent_info)
%
%  A description of each parameter follows:
%
%    o serpent_info: The serpent info.
%
*/
WizardExport unsigned int GetSerpentBlocksize(const SerpentInfo *serpent_info)
{
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(CipherDomain,serpent_info != (SerpentInfo *) NULL);
  WizardAssert(CipherDomain,serpent_info->signature == WizardSignature);
  return(serpent_info->blocksize);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t S e r p e n t K e y                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetSerpentKey() sets the key for the Serpent cipher.  The key length is
%  specified in bits.  Valid values are 128,192,or 256 requiring a key
%  buffer length in bytes of 16,24,and 32 respectively.
%
%  The format of the SetSerpentKey method is:
%
%      SetSerpentKey(SerpentInfo *serpent_info,const StringInfo *key)
%
%  A description of each parameter follows:
%
%    o serpent_info: The cipher context.
%
%    o key: The key.
%
*/
WizardExport void SetSerpentKey(SerpentInfo *serpent_info,const StringInfo *key)
{
#define ChurnKey(alpha,beta,gamma,delta,i,j) \
{ \
  beta^=delta; beta^=gamma; beta^=alpha; beta^=0x9e3779b9UL ^ i; \
  beta=RotateLeft(beta,11); crypt_key[j]=beta; \
}
#define LoadKey(alpha,beta,gamma,delta,i) \
{ \
  alpha=crypt_key[i]; beta=crypt_key[(i)+1]; \
  gamma=crypt_key[(i)+2]; delta=crypt_key[(i)+3]; \
}
#define MaximumSerpentKeyLength  32
#define StoreKey(alpha,beta,gamma,delta,i) \
{ \
  crypt_key[i]=alpha; crypt_key[(i)+1]=beta; \
  crypt_key[(i)+2]=gamma; crypt_key[(i)+3]=delta; \
}

  register size_t
    i;

  register unsigned char
    *p;

  register unsigned int
    *crypt_key;

  unsigned char
    *datum;

  unsigned int
    alpha,
    beta,
    gamma,
    delta,
    epsilon,
    rho;

  /*
    Generate crypt key.
  */
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(CipherDomain,serpent_info != (SerpentInfo *) NULL);
  WizardAssert(CipherDomain,serpent_info->signature == WizardSignature);
  WizardAssert(CipherDomain,key != (StringInfo *) NULL);
  crypt_key=serpent_info->crypt_key;
  p=(unsigned char *) crypt_key;
  datum=GetStringInfoDatum(key);
  for (i=0; i < Min(GetStringInfoLength(key),MaximumSerpentKeyLength); i++)
    p[i]=datum[i];
  if (i < MaximumSerpentKeyLength)
    p[i++]=1;
  while (i < MaximumSerpentKeyLength)
    p[i++]=0;
  p=(unsigned char *) crypt_key;
  p+=12;
  Read32Bits(alpha,p);
  Read32Bits(beta,p);
  Read32Bits(gamma,p);
  Read32Bits(delta,p);
  Read32Bits(epsilon,p);
  p=(unsigned char *) crypt_key;
  Read32Bits(rho,p);
  ChurnKey(rho,alpha,epsilon,gamma,0,0);
  Read32Bits(rho,p);
  ChurnKey(rho,beta,alpha,delta,1,1);
  Read32Bits(rho,p);
  ChurnKey(rho,gamma,beta,epsilon,2,2);
  Read32Bits(rho,p);
  ChurnKey(rho,delta,gamma,alpha,3,3);
  Read32Bits(rho,p);
  ChurnKey(rho,epsilon,delta,beta,4,4);
  Read32Bits(rho,p);
  ChurnKey(rho,alpha,epsilon,gamma,5,5);
  Read32Bits(rho,p);
  ChurnKey(rho,beta,alpha,delta,6,6);
  Read32Bits(rho,p);
  ChurnKey(rho,gamma,beta,epsilon,7,7);
  ChurnKey(crypt_key[0],delta,gamma,alpha,8,8);
  ChurnKey(crypt_key[1],epsilon,delta,beta,9,9);
  ChurnKey(crypt_key[2],alpha,epsilon,gamma,10,10);
  ChurnKey(crypt_key[3],beta,alpha,delta,11,11);
  ChurnKey(crypt_key[4],gamma,beta,epsilon,12,12);
  ChurnKey(crypt_key[5],delta,gamma,alpha,13,13);
  ChurnKey(crypt_key[6],epsilon,delta,beta,14,14);
  ChurnKey(crypt_key[7],alpha,epsilon,gamma,15,15);
  ChurnKey(crypt_key[8],beta,alpha,delta,16,16);
  ChurnKey(crypt_key[9],gamma,beta,epsilon,17,17);
  ChurnKey(crypt_key[10],delta,gamma,alpha,18,18);
  ChurnKey(crypt_key[11],epsilon,delta,beta,19,19);
  ChurnKey(crypt_key[12],alpha,epsilon,gamma,20,20);
  ChurnKey(crypt_key[13],beta,alpha,delta,21,21);
  ChurnKey(crypt_key[14],gamma,beta,epsilon,22,22);
  ChurnKey(crypt_key[15],delta,gamma,alpha,23,23);
  ChurnKey(crypt_key[16],epsilon,delta,beta,24,24);
  ChurnKey(crypt_key[17],alpha,epsilon,gamma,25,25);
  ChurnKey(crypt_key[18],beta,alpha,delta,26,26);
  ChurnKey(crypt_key[19],gamma,beta,epsilon,27,27);
  ChurnKey(crypt_key[20],delta,gamma,alpha,28,28);
  ChurnKey(crypt_key[21],epsilon,delta,beta,29,29);
  ChurnKey(crypt_key[22],alpha,epsilon,gamma,30,30);
  ChurnKey(crypt_key[23],beta,alpha,delta,31,31);
  crypt_key+=50;
  ChurnKey(crypt_key[-26],gamma,beta,epsilon,32,-18);
  ChurnKey(crypt_key[-25],delta,gamma,alpha,33,-17);
  ChurnKey(crypt_key[-24],epsilon,delta,beta,34,-16);
  ChurnKey(crypt_key[-23],alpha,epsilon,gamma,35,-15);
  ChurnKey(crypt_key[-22],beta,alpha,delta,36,-14);
  ChurnKey(crypt_key[-21],gamma,beta,epsilon,37,-13);
  ChurnKey(crypt_key[-20],delta,gamma,alpha,38,-12);
  ChurnKey(crypt_key[-19],epsilon,delta,beta,39,-11);
  ChurnKey(crypt_key[-18],alpha,epsilon,gamma,40,-10);
  ChurnKey(crypt_key[-17],beta,alpha,delta,41,-9);
  ChurnKey(crypt_key[-16],gamma,beta,epsilon,42,-8);
  ChurnKey(crypt_key[-15],delta,gamma,alpha,43,-7);
  ChurnKey(crypt_key[-14],epsilon,delta,beta,44,-6);
  ChurnKey(crypt_key[-13],alpha,epsilon,gamma,45,-5);
  ChurnKey(crypt_key[-12],beta,alpha,delta,46,-4);
  ChurnKey(crypt_key[-11],gamma,beta,epsilon,47,-3);
  ChurnKey(crypt_key[-10],delta,gamma,alpha,48,-2);
  ChurnKey(crypt_key[-9],epsilon,delta,beta,49,-1);
  ChurnKey(crypt_key[-8],alpha,epsilon,gamma,50,0);
  ChurnKey(crypt_key[-7],beta,alpha,delta,51,1);
  ChurnKey(crypt_key[-6],gamma,beta,epsilon,52,2);
  ChurnKey(crypt_key[-5],delta,gamma,alpha,53,3);
  ChurnKey(crypt_key[-4],epsilon,delta,beta,54,4);
  ChurnKey(crypt_key[-3],alpha,epsilon,gamma,55,5);
  ChurnKey(crypt_key[-2],beta,alpha,delta,56,6);
  ChurnKey(crypt_key[-1],gamma,beta,epsilon,57,7);
  ChurnKey(crypt_key[0],delta,gamma,alpha,58,8);
  ChurnKey(crypt_key[1],epsilon,delta,beta,59,9);
  ChurnKey(crypt_key[2],alpha,epsilon,gamma,60,10);
  ChurnKey(crypt_key[3],beta,alpha,delta,61,11);
  ChurnKey(crypt_key[4],gamma,beta,epsilon,62,12);
  ChurnKey(crypt_key[5],delta,gamma,alpha,63,13);
  ChurnKey(crypt_key[6],epsilon,delta,beta,64,14);
  ChurnKey(crypt_key[7],alpha,epsilon,gamma,65,15);
  ChurnKey(crypt_key[8],beta,alpha,delta,66,16);
  ChurnKey(crypt_key[9],gamma,beta,epsilon,67,17);
  ChurnKey(crypt_key[10],delta,gamma,alpha,68,18);
  ChurnKey(crypt_key[11],epsilon,delta,beta,69,19);
  ChurnKey(crypt_key[12],alpha,epsilon,gamma,70,20);
  ChurnKey(crypt_key[13],beta,alpha,delta,71,21);
  ChurnKey(crypt_key[14],gamma,beta,epsilon,72,22);
  ChurnKey(crypt_key[15],delta,gamma,alpha,73,23);
  ChurnKey(crypt_key[16],epsilon,delta,beta,74,24);
  ChurnKey(crypt_key[17],alpha,epsilon,gamma,75,25);
  ChurnKey(crypt_key[18],beta,alpha,delta,76,26);
  ChurnKey(crypt_key[19],gamma,beta,epsilon,77,27);
  ChurnKey(crypt_key[20],delta,gamma,alpha,78,28);
  ChurnKey(crypt_key[21],epsilon,delta,beta,79,29);
  ChurnKey(crypt_key[22],alpha,epsilon,gamma,80,30);
  ChurnKey(crypt_key[23],beta,alpha,delta,81,31);
  crypt_key+=50;
  ChurnKey(crypt_key[-26],gamma,beta,epsilon,82,-18);
  ChurnKey(crypt_key[-25],delta,gamma,alpha,83,-17);
  ChurnKey(crypt_key[-24],epsilon,delta,beta,84,-16);
  ChurnKey(crypt_key[-23],alpha,epsilon,gamma,85,-15);
  ChurnKey(crypt_key[-22],beta,alpha,delta,86,-14);
  ChurnKey(crypt_key[-21],gamma,beta,epsilon,87,-13);
  ChurnKey(crypt_key[-20],delta,gamma,alpha,88,-12);
  ChurnKey(crypt_key[-19],epsilon,delta,beta,89,-11);
  ChurnKey(crypt_key[-18],alpha,epsilon,gamma,90,-10);
  ChurnKey(crypt_key[-17],beta,alpha,delta,91,-9);
  ChurnKey(crypt_key[-16],gamma,beta,epsilon,92,-8);
  ChurnKey(crypt_key[-15],delta,gamma,alpha,93,-7);
  ChurnKey(crypt_key[-14],epsilon,delta,beta,94,-6);
  ChurnKey(crypt_key[-13],alpha,epsilon,gamma,95,-5);
  ChurnKey(crypt_key[-12],beta,alpha,delta,96,-4);
  ChurnKey(crypt_key[-11],gamma,beta,epsilon,97,-3);
  ChurnKey(crypt_key[-10],delta,gamma,alpha,98,-2);
  ChurnKey(crypt_key[-9],epsilon,delta,beta,99,-1);
  ChurnKey(crypt_key[-8],alpha,epsilon,gamma,100,0);
  ChurnKey(crypt_key[-7],beta,alpha,delta,101,1);
  ChurnKey(crypt_key[-6],gamma,beta,epsilon,102,2);
  ChurnKey(crypt_key[-5],delta,gamma,alpha,103,3);
  ChurnKey(crypt_key[-4],epsilon,delta,beta,104,4);
  ChurnKey(crypt_key[-3],alpha,epsilon,gamma,105,5);
  ChurnKey(crypt_key[-2],beta,alpha,delta,106,6);
  ChurnKey(crypt_key[-1],gamma,beta,epsilon,107,7);
  ChurnKey(crypt_key[0],delta,gamma,alpha,108,8);
  ChurnKey(crypt_key[1],epsilon,delta,beta,109,9);
  ChurnKey(crypt_key[2],alpha,epsilon,gamma,110,10);
  ChurnKey(crypt_key[3],beta,alpha,delta,111,11);
  ChurnKey(crypt_key[4],gamma,beta,epsilon,112,12);
  ChurnKey(crypt_key[5],delta,gamma,alpha,113,13);
  ChurnKey(crypt_key[6],epsilon,delta,beta,114,14);
  ChurnKey(crypt_key[7],alpha,epsilon,gamma,115,15);
  ChurnKey(crypt_key[8],beta,alpha,delta,116,16);
  ChurnKey(crypt_key[9],gamma,beta,epsilon,117,17);
  ChurnKey(crypt_key[10],delta,gamma,alpha,118,18);
  ChurnKey(crypt_key[11],epsilon,delta,beta,119,19);
  ChurnKey(crypt_key[12],alpha,epsilon,gamma,120,20);
  ChurnKey(crypt_key[13],beta,alpha,delta,121,21);
  ChurnKey(crypt_key[14],gamma,beta,epsilon,122,22);
  ChurnKey(crypt_key[15],delta,gamma,alpha,123,23);
  ChurnKey(crypt_key[16],epsilon,delta,beta,124,24);
  ChurnKey(crypt_key[17],alpha,epsilon,gamma,125,25);
  ChurnKey(crypt_key[18],beta,alpha,delta,126,26);
  ChurnKey(crypt_key[19],gamma,beta,epsilon,127,27);
  ChurnKey(crypt_key[20],delta,gamma,alpha,128,28);
  ChurnKey(crypt_key[21],epsilon,delta,beta,129,29);
  ChurnKey(crypt_key[22],alpha,epsilon,gamma,130,30);
  ChurnKey(crypt_key[23],beta,alpha,delta,131,31);
  /*
    Apply S-boxes.
  */
  EncipherSbox3(delta,epsilon,alpha,beta,gamma);
  StoreKey(beta,gamma,epsilon,delta,28);
  LoadKey(beta,gamma,epsilon,delta,24);
  EncipherSbox4(beta,gamma,epsilon,delta,alpha);
  StoreKey(gamma,epsilon,delta,alpha,24);
  LoadKey(gamma,epsilon,delta,alpha,20);
  EncipherSbox5(gamma,epsilon,delta,alpha,beta);
  StoreKey(beta,gamma,epsilon,alpha,20);
  LoadKey(beta,gamma,epsilon,alpha,16);
  EncipherSbox6(beta,gamma,epsilon,alpha,delta);
  StoreKey(epsilon,delta,gamma,alpha,16);
  LoadKey(epsilon,delta,gamma,alpha,12);
  EncipherSbox7(epsilon,delta,gamma,alpha,beta);
  StoreKey(beta,gamma,alpha,epsilon,12);
  LoadKey(beta,gamma,alpha,epsilon,8);
  EncipherSbox0(beta,gamma,alpha,epsilon,delta);
  StoreKey(alpha,gamma,epsilon,beta,8);
  LoadKey(alpha,gamma,epsilon,beta,4);
  EncipherSbox1(alpha,gamma,epsilon,beta,delta);
  StoreKey(delta,epsilon,beta,alpha,4);
  LoadKey(delta,epsilon,beta,alpha,0);
  EncipherSbox2(delta,epsilon,beta,alpha,gamma);
  StoreKey(gamma,epsilon,delta,alpha,0);
  LoadKey(gamma,epsilon,delta,alpha,-4);
  EncipherSbox3(gamma,epsilon,delta,alpha,beta);
  StoreKey(alpha,beta,epsilon,gamma,-4);
  LoadKey(alpha,beta,epsilon,gamma,-8);
  EncipherSbox4(alpha,beta,epsilon,gamma,delta);
  StoreKey(beta,epsilon,gamma,delta,-8);
  LoadKey(beta,epsilon,gamma,delta,-12);
  EncipherSbox5(beta,epsilon,gamma,delta,alpha);
  StoreKey(alpha,beta,epsilon,delta,-12);
  LoadKey(alpha,beta,epsilon,delta,-16);
  EncipherSbox6(alpha,beta,epsilon,delta,gamma);
  StoreKey(epsilon,gamma,beta,delta,-16);
  LoadKey(epsilon,gamma,beta,delta,-20);
  EncipherSbox7(epsilon,gamma,beta,delta,alpha);
  StoreKey(alpha,beta,delta,epsilon,-20);
  LoadKey(alpha,beta,delta,epsilon,-24);
  EncipherSbox0(alpha,beta,delta,epsilon,gamma);
  StoreKey(delta,beta,epsilon,alpha,-24);
  LoadKey(delta,beta,epsilon,alpha,-28);
  crypt_key-=50;
  EncipherSbox1(delta,beta,epsilon,alpha,gamma);
  StoreKey(gamma,epsilon,alpha,delta,22);
  LoadKey(gamma,epsilon,alpha,delta,18);
  EncipherSbox2(gamma,epsilon,alpha,delta,beta);
  StoreKey(beta,epsilon,gamma,delta,18);
  LoadKey(beta,epsilon,gamma,delta,14);
  EncipherSbox3(beta,epsilon,gamma,delta,alpha);
  StoreKey(delta,alpha,epsilon,beta,14);
  LoadKey(delta,alpha,epsilon,beta,10);
  EncipherSbox4(delta,alpha,epsilon,beta,gamma);
  StoreKey(alpha,epsilon,beta,gamma,10);
  LoadKey(alpha,epsilon,beta,gamma,6);
  EncipherSbox5(alpha,epsilon,beta,gamma,delta);
  StoreKey(delta,alpha,epsilon,gamma,6);
  LoadKey(delta,alpha,epsilon,gamma,2);
  EncipherSbox6(delta,alpha,epsilon,gamma,beta);
  StoreKey(epsilon,beta,alpha,gamma,2);
  LoadKey(epsilon,beta,alpha,gamma,-2);
  EncipherSbox7(epsilon,beta,alpha,gamma,delta);
  StoreKey(delta,alpha,gamma,epsilon,-2);
  LoadKey(delta,alpha,gamma,epsilon,-6);
  EncipherSbox0(delta,alpha,gamma,epsilon,beta);
  StoreKey(gamma,alpha,epsilon,delta,-6);
  LoadKey(gamma,alpha,epsilon,delta,-10);
  EncipherSbox1(gamma,alpha,epsilon,delta,beta);
  StoreKey(beta,epsilon,delta,gamma,-10);
  LoadKey(beta,epsilon,delta,gamma,-14);
  EncipherSbox2(beta,epsilon,delta,gamma,alpha);
  StoreKey(alpha,epsilon,beta,gamma,-14);
  LoadKey(alpha,epsilon,beta,gamma,-18);
  EncipherSbox3(alpha,epsilon,beta,gamma,delta);
  StoreKey(gamma,delta,epsilon,alpha,-18);
  LoadKey(gamma,delta,epsilon,alpha,-22);
  crypt_key-=50;
  EncipherSbox4(gamma,delta,epsilon,alpha,beta);
  StoreKey(delta,epsilon,alpha,beta,28);
  LoadKey(delta,epsilon,alpha,beta,24);
  EncipherSbox5(delta,epsilon,alpha,beta,gamma);
  StoreKey(gamma,delta,epsilon,beta,24);
  LoadKey(gamma,delta,epsilon,beta,20);
  EncipherSbox6(gamma,delta,epsilon,beta,alpha);
  StoreKey(epsilon,alpha,delta,beta,20);
  LoadKey(epsilon,alpha,delta,beta,16);
  EncipherSbox7(epsilon,alpha,delta,beta,gamma);
  StoreKey(gamma,delta,beta,epsilon,16);
  LoadKey(gamma,delta,beta,epsilon,12);
  EncipherSbox0(gamma,delta,beta,epsilon,alpha);
  StoreKey(beta,delta,epsilon,gamma,12);
  LoadKey(beta,delta,epsilon,gamma,8);
  EncipherSbox1(beta,delta,epsilon,gamma,alpha);
  StoreKey(alpha,epsilon,gamma,beta,8);
  LoadKey(alpha,epsilon,gamma,beta,4);
  EncipherSbox2(alpha,epsilon,gamma,beta,delta);
  StoreKey(delta,epsilon,alpha,beta,4);
  LoadKey(delta,epsilon,alpha,beta,0);
  EncipherSbox3(delta,epsilon,alpha,beta,gamma);
  StoreKey(beta,gamma,epsilon,delta,0);
  /*
    Reset registers.
  */
  alpha=0;
  beta=0;
  gamma=0;
  delta=0;
  epsilon=0;
  rho=0;
}
