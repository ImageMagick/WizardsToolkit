/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                   CCCC  H   H   AAA    CCCC  H   H   AAA                    %
%                  C      H   H  A   A  C      H   H  A   A                   %
%                  C      HHHHH  AAAAA  C      HHHHH  AAAAA                   %
%                  C      H   H  A   A  C      H   H  A   A                   %
%                   CCCC  H   H  A   A   CCCC  H   H  A   A                   %
%                                                                             %
%                                                                             %
%                   Wizard's Toolkit Chacha Cipher Methods                    %
%                                                                             %
%                               Software Design                               %
%                                   Cristy                                    %
%                                 March 2009                                  %
%                                                                             %
%                                                                             %
%  Copyright 1999-2020 ImageMagick Studio LLC, a non-profit organization      %
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
#include "wizard/chacha.h"
#include "wizard/exception.h"
#include "wizard/exception-private.h"
#include "wizard/memory_.h"

/*
  Typedef declarations.
*/
struct _ChachaInfo
{
  unsigned int
    blocksize,
    key[16];

  ssize_t
    rounds;

  time_t
    timestamp;

  size_t
    signature;
};

/*
  Define declarations.
*/
#define ChachaBlocksize  64
#define PushChachaWord(p) \
  (((unsigned int) ((p)[0]) <<  0) | \
   ((unsigned int) ((p)[1]) <<  8) | \
   ((unsigned int) ((p)[2]) << 16) | \
   ((unsigned int) ((p)[3]) << 24))

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A c q u i r e C h a c h a I n f o                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireChachaInfo() allocate the ChachaInfo structure.
%
%  The format of the AcquireChachaInfo method is:
%
%      ChachaInfo *AcquireChachaInfo(void)
%
*/
WizardExport ChachaInfo *AcquireChachaInfo(void)
{
  ChachaInfo
    *chacha_info;

  chacha_info=(ChachaInfo *) AcquireWizardMemory(sizeof(*chacha_info));
  if (chacha_info == (ChachaInfo *) NULL)
    ThrowWizardFatalError(CipherDomain,MemoryError);
  (void) ResetWizardMemory(chacha_info,0,sizeof(*chacha_info));
  chacha_info->blocksize=ChachaBlocksize;
  chacha_info->timestamp=time((time_t *) NULL);
  chacha_info->signature=WizardSignature;
  return(chacha_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e c i p h e r C h a c h a B l o c k                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DecipherChachaBlock() deciphers a single block of ciphertext to produce a
%  block of plaintext.
%
%  The format of the DecipherChachaBlock method is:
%
%     void DecipherChacha(ChachaInfo *chacha_info,
%       const unsigned char *ciphertext,unsigned char *plaintext)
%
%  A description of each parameter follows:
%
%    o chacha_info: The cipher context.
%
%    o ciphertext: The cipher text.
%
%    o plainttext: The plaint text.
%
*/
WizardExport void DecipherChachaBlock(ChachaInfo *chacha_info,
  const unsigned char *ciphertext,unsigned char *plaintext)
{
  EncipherChachaBlock(chacha_info,ciphertext,plaintext);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e s t r o y C h a c h a I n f o                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroyChachaInfo() zeros memory associated with the ChachaInfo structure.
%
%  The format of the DestroyChachaInfo method is:
%
%      ChachaInfo *DestroyChachaInfo(ChachaInfo *chacha_info)
%
%  A description of each parameter follows:
%
%    o chacha_info: The cipher context.
%
*/
WizardExport ChachaInfo *DestroyChachaInfo(ChachaInfo *chacha_info)
{
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(CipherDomain,chacha_info != (ChachaInfo *) NULL);
  WizardAssert(CipherDomain,chacha_info->signature == WizardSignature);
  chacha_info->signature=(~WizardSignature);
  chacha_info=(ChachaInfo *) RelinquishWizardMemory(chacha_info);
  return(chacha_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% %                                                                             %
%                                                                             %
%                                                                             %
%   E n c i p h e r C h a c h a B l o c k                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  EncipherChachaBlock() enciphers a single block of plaintext to produce a
%  block of ciphertext.
%
%  The format of the EncipherChachaBlock method is:
%
%      void EncipherChacha(ChachaInfo *chacha_info,
%        const unsigned char *plaintext,unsigned char *ciphertext)
%
%  A description of each parameter follows:
%
%    o chacha_info: The cipher context.
%
%    o plaintext: The plain text.
%
%    o ciphertext: The cipher text.
%
*/
WizardExport void EncipherChachaBlock(ChachaInfo *chacha_info,
  const unsigned char *plaintext,unsigned char *ciphertext)
{
#define ChachaAdd(v,w)  ((unsigned int) ((v)+(w)) & 0xFFFFFFFFU)
#define ChachaQuarterRound(a,b,c,d) \
{ \
  a=ChachaAdd(a,b); d=ChachaRotate((d) ^ (a),16); \
  c=ChachaAdd(c,d); b=ChachaRotate((b) ^ (c),12); \
  a=ChachaAdd(a,b); d=ChachaRotate((d) ^ (a), 8); \
  c=ChachaAdd(c,d); b=ChachaRotate((b) ^ (c), 7); \
}
#define ChachaRotate(v,n) \
  (((unsigned int) ((v) << (n)) & 0xFFFFFFFFU) | ((v) >> (32-(n))))
#define PopChachaWord(p,v) \
{ \
  (p)[0]=((unsigned char) (((v) >>  0) & 0xff)); \
  (p)[1]=((unsigned char) (((v) >>  8) & 0xff)); \
  (p)[2]=((unsigned char) (((v) >> 16) & 0xff)); \
  (p)[3]=((unsigned char) (((v) >> 24) & 0xff)); \
}

  register ssize_t
    i;

  unsigned int
    x0,
    x1,
    x2,
    x3,
    x4,
    x5,
    x6,
    x7,
    x8,
    x9,
    x10,
    x11,
    x12,
    x13,
    x14,
    x15;

  /*
    Encipher one block.
  */
  x0=chacha_info->key[0];
  x1=chacha_info->key[1];
  x2=chacha_info->key[2];
  x3=chacha_info->key[3];
  x4=chacha_info->key[4];
  x5=chacha_info->key[5];
  x6=chacha_info->key[6];
  x7=chacha_info->key[7];
  x8=chacha_info->key[8];
  x9=chacha_info->key[9];
  x10=chacha_info->key[10];
  x11=chacha_info->key[11];
  x12=chacha_info->key[12];
  x13=chacha_info->key[13];
  x14=chacha_info->key[14];
  x15=chacha_info->key[15];
  for (i=20; i > 0; i-=2)
  {
    ChachaQuarterRound(x0,x4,x8,x12);
    ChachaQuarterRound(x1,x5,x9,x13);
    ChachaQuarterRound(x2,x6,x10,x14);
    ChachaQuarterRound(x3,x7,x11,x15);
    ChachaQuarterRound(x0,x5,x10,x15);
    ChachaQuarterRound(x1,x6,x11,x12);
    ChachaQuarterRound(x2,x7,x8,x13);
    ChachaQuarterRound(x3,x4,x9,x14);
  }
  x0=ChachaAdd(x0,chacha_info->key[0]);
  x1=ChachaAdd(x1,chacha_info->key[1]);
  x2=ChachaAdd(x2,chacha_info->key[2]);
  x3=ChachaAdd(x3,chacha_info->key[3]);
  x4=ChachaAdd(x4,chacha_info->key[4]);
  x5=ChachaAdd(x5,chacha_info->key[5]);
  x6=ChachaAdd(x6,chacha_info->key[6]);
  x7=ChachaAdd(x7,chacha_info->key[7]);
  x8=ChachaAdd(x8,chacha_info->key[8]);
  x9=ChachaAdd(x9,chacha_info->key[9]);
  x10=ChachaAdd(x10,chacha_info->key[10]);
  x11=ChachaAdd(x11,chacha_info->key[11]);
  x12=ChachaAdd(x12,chacha_info->key[12]);
  x13=ChachaAdd(x13,chacha_info->key[13]);
  x14=ChachaAdd(x14,chacha_info->key[14]);
  x15=ChachaAdd(x15,chacha_info->key[15]);
  x0^=PushChachaWord(plaintext+0);
  x1^=PushChachaWord(plaintext+4);
  x2^=PushChachaWord(plaintext+8);
  x3^=PushChachaWord(plaintext+12);
  x4^=PushChachaWord(plaintext+16);
  x5^=PushChachaWord(plaintext+20);
  x6^=PushChachaWord(plaintext+24);
  x7^=PushChachaWord(plaintext+28);
  x8^=PushChachaWord(plaintext+32);
  x9^=PushChachaWord(plaintext+36);
  x10^=PushChachaWord(plaintext+40);
  x11^=PushChachaWord(plaintext+44);
  x12^=PushChachaWord(plaintext+48);
  x13^=PushChachaWord(plaintext+52);
  x14^=PushChachaWord(plaintext+56);
  x15^=PushChachaWord(plaintext+60);
  PopChachaWord(ciphertext+0,x0);
  PopChachaWord(ciphertext+4,x1);
  PopChachaWord(ciphertext+8,x2);
  PopChachaWord(ciphertext+12,x3);
  PopChachaWord(ciphertext+16,x4);
  PopChachaWord(ciphertext+20,x5);
  PopChachaWord(ciphertext+24,x6);
  PopChachaWord(ciphertext+28,x7);
  PopChachaWord(ciphertext+32,x8);
  PopChachaWord(ciphertext+36,x9);
  PopChachaWord(ciphertext+40,x10);
  PopChachaWord(ciphertext+44,x11);
  PopChachaWord(ciphertext+48,x12);
  PopChachaWord(ciphertext+52,x13);
  PopChachaWord(ciphertext+56,x14);
  PopChachaWord(ciphertext+60,x15);
  /*
    Reset registers.
  */
  x0=0;  x1=0;  x2=0;  x3=0;  x4=0;  x5=0;  x6=0;  x7=0;
  x8=0;  x9=0; x10=0; x11=0; x12=0; x13=0; x14=0; x15=0;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t C h a c h a B l o c k s i z e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetChachaBlocksize() returns the Chacha blocksize.
%
%  The format of the GetChachaBlocksize method is:
%
%      unsigned int *GetChachaBlocksize(const ChachaInfo *chacha_info)
%
%  A description of each parameter follows:
%
%    o chacha_info: The aes info.
%
*/
WizardExport unsigned int GetChachaBlocksize(const ChachaInfo *chacha_info)
{
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(CipherDomain,chacha_info != (ChachaInfo *) NULL);
  WizardAssert(CipherDomain,chacha_info->signature == WizardSignature);
  return(chacha_info->blocksize);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t C h a c h a K e y                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetChachaKey() sets the key for the Chacha cipher.  The key length is
%  specified in bits.  Valid values are 128, 192, or 256 requiring a key buffer
%  length in bytes of 16, 24, and 32 respectively.
%
%  The format of the SetChachaKey method is:
%
%      SetChachaKey(ChachaInfo *chacha_info,const StringInfo *key)
%
%  A description of each parameter follows:
%
%    o chacha_info: The cipher context.
%
%    o key: The key.
%
*/
WizardExport void SetChachaKey(ChachaInfo *chacha_info,const StringInfo *key)
{
  const char
    *constants;

  static const char
    sigma[17] = "expand 32-byte k",
    tau[17] = "expand 16-byte k";

  unsigned char
    *datum;

  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(CipherDomain,chacha_info != (ChachaInfo *) NULL);
  WizardAssert(CipherDomain,chacha_info->signature == WizardSignature);
  WizardAssert(CipherDomain,key != (StringInfo *) NULL);
  datum=GetStringInfoDatum(key);
  if (GetStringInfoLength(key) >= 4)
    chacha_info->key[4]=PushChachaWord(datum+0);
  if (GetStringInfoLength(key) >= 8)
    chacha_info->key[5]=PushChachaWord(datum+4);
  if (GetStringInfoLength(key) >= 12)
    chacha_info->key[6]=PushChachaWord(datum+8);
  if (GetStringInfoLength(key) >= 16)
    chacha_info->key[7]=PushChachaWord(datum+12);
  constants=tau;
  if ((8*GetStringInfoLength(key)) == 256)
    {
      datum+=16;
      constants=sigma;
    }
  if (GetStringInfoLength(key) >= 4)
    chacha_info->key[8]=PushChachaWord(datum+0);
  if (GetStringInfoLength(key) >= 8)
    chacha_info->key[9]=PushChachaWord(datum+4);
  if (GetStringInfoLength(key) >= 12)
    chacha_info->key[10]=PushChachaWord(datum+8);
  if (GetStringInfoLength(key) >= 16)
    chacha_info->key[11]=PushChachaWord(datum+12);
  chacha_info->key[0]=PushChachaWord(constants+0);
  chacha_info->key[1]=PushChachaWord(constants+4);
  chacha_info->key[2]=PushChachaWord(constants+8);
  chacha_info->key[3]=PushChachaWord(constants+12);
  chacha_info->key[12]=0;  /* we don't use counter in key */
  chacha_info->key[13]=0;  /* we don't use counter in key */
  chacha_info->key[14]=0;
  chacha_info->key[15]=0;
  /*
    Reset registers.
  */
  constants=(const char *) NULL;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t C h a c h a N o n c e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetChachaNonce() sets the nonce for the Chacha cipher.  The nonce must be
%  at least 8 bytes.  The counter can be NULL, if not it must be at least
%  8 bytes.
%
%  The format of the SetChachaKey method is:
%
%      SetChachaKey(ChachaInfo *chacha_info,const unsigned char *nonce,
%        const unsigned char *counter)
%
%  A description of each parameter follows:
%
%    o chacha_info: The cipher context.
%
%    o nonce: The nonce.
%
%    o counter: The counter.
%
*/
WizardExport void SetChachaNonce(ChachaInfo *chacha_info,
  const unsigned char *nonce,const unsigned char *counter)
{
  chacha_info->key[12]=counter == (unsigned char *) NULL ? 0 :
    PushChachaWord(counter+0);
  chacha_info->key[13]=counter == (unsigned char *) NULL ? 0 :
    PushChachaWord(counter+4);
  chacha_info->key[14]=PushChachaWord(nonce+0);
  chacha_info->key[15]=PushChachaWord(nonce+4);
}
