/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%           V   V   AAA   L      IIIII  DDDD    AAA   TTTTT  EEEEE            %
%           V   V  A   A  L        I    D   D  A   A    T    E                %
%           V   V  AAAAA  L        I    D   D  AAAAA    T    EEE              %
%            V V   A   A  L        I    D   D  A   A    T    E                %
%             V    A   A  LLLLL  IIIII  DDDD   A   A    T    EEEEE            %
%                                                                             %
%                                                                             %
%                 Wizard's Toolkit Executable Validation Suite                %
%                                                                             %
%                             Software Design                                 %
%                               John Cristy                                   %
%                               March 2003                                    %
%                                                                             %
%                                                                             %
%  Copyright 1999-2010 ImageMagick Studio LLC, a non-profit organization      %
%  dedicated to making software imaging solutions freely available.           %
%                                                                             %
%  You may not use this file except in compliance with the License.  You may  %
%  obtain a copy of the License at                                            %
%                                                                             %
%    http://www.wizards-toolkit.org/script/license.php                        %
%                                                                             %
%  Unless required by applicable law or agreed to in writing, software        %
%  distributed under the License is distributed on an "AS IS" BASIS,          %
%  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   %
%  see the License for the specific language governing permissions and        %
%  limitations under the License.                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%
*/

/*
  Include declarations.
*/
#include <stdio.h>
#include <string.h>
#include "wizard/WizardsToolkit.h"
#include "validate.h"

/*
  Define declaractions.
*/
#define AbsoluteValue(x)  ((x) < 0 ? -(x) : (x))
#define CipherKey  "FakeKey"
#define CipherId  "FakeID"
#define CipherPlaintext  "1234567890abcde"

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P r i n t V a l i d a t e S t r i n g                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PrintValidateString() prints the string to the specified file.
%
%  The format of the PrintValidateString method is:
%
%      ssize_t PrintValidateString(FILE *,const char *format,...)
%
%  A description of each parameter follows:
%
%    o file: print to this file.
%
%   o format:  A string describing the format to use to write the remaining
%     arguments.
%
*/
static ssize_t PrintValidateString(FILE *file,const char *format,...)
{
  char
    string[MaxTextExtent];

  ssize_t
    length;

  va_list
    operands;

  va_start(operands,format);
  length=FormatLocaleStringList(string,MaxTextExtent,format,operands);
  va_end(operands);
  if (length < 0)
    return(-1);
  return((ssize_t) fwrite(string,(size_t) length,1,file));
}

static WizardBooleanType TestAES(void)
{
  CipherInfo
    *cipher_info;

  register ssize_t
    i;

  StringInfo
    *ciphertext,
    *key,
    *plaintext,
    *results;

  WizardBooleanType
    clone,
    pass;

  (void) PrintValidateString(stdout,"testing aes encipher:\n");
  pass=WizardTrue;
  cipher_info=AcquireCipherInfo(AESCipher,CBCMode);
  for (i=0; i < AESEncipherTestVectors; i++)
  {
    (void) PrintValidateString(stdout,"  test %.20g (%.20g bit key) ",
      (double) i+1,(double) (8*aes_encipher_test_vector[i].key_length));
    key=AcquireStringInfo(aes_encipher_test_vector[i].key_length);
    SetStringInfoDatum(key,aes_encipher_test_vector[i].key);
    SetCipherKey(cipher_info,key);
    ResetCipherNonce(cipher_info);
    plaintext=AcquireStringInfo(aes_encipher_test_vector[i].length);
    SetStringInfoDatum(plaintext,aes_encipher_test_vector[i].plaintext);
    ciphertext=EncipherCipher(cipher_info,plaintext);
    results=AcquireStringInfo(aes_encipher_test_vector[i].result_length);
    SetStringInfoDatum(results,aes_encipher_test_vector[i].result);
    clone=CompareStringInfo(ciphertext,results) == 0 ? WizardTrue : WizardFalse;
    (void) PrintValidateString(stdout,"%s.\n",clone != WizardFalse ? "pass" :
      "fail");
    if (clone == WizardFalse)
      pass=WizardFalse;
    results=DestroyStringInfo(results);
    key=DestroyStringInfo(key);
    plaintext=DestroyStringInfo(plaintext);
  }
  cipher_info=DestroyCipherInfo(cipher_info);
  (void) PrintValidateString(stdout,"testing aes decipher:\n");
  cipher_info=AcquireCipherInfo(AESCipher,CBCMode);
  for (i=0; i < AESDecipherTestVectors; i++)
  {
    (void) PrintValidateString(stdout,"  test %.20g (%.20g bit key) ",(double)
      i+1,(double) (8*aes_decipher_test_vector[i].key_length));
    key=AcquireStringInfo(aes_decipher_test_vector[i].key_length);
    SetStringInfoDatum(key,aes_decipher_test_vector[i].key);
    SetCipherKey(cipher_info,key);
    ResetCipherNonce(cipher_info);
    ciphertext=AcquireStringInfo(aes_decipher_test_vector[i].length);
    SetStringInfoDatum(ciphertext,aes_decipher_test_vector[i].plaintext);
    plaintext=DecipherCipher(cipher_info,ciphertext);
    results=AcquireStringInfo(aes_decipher_test_vector[i].length);
    SetStringInfoDatum(results,aes_decipher_test_vector[i].result);
    clone=CompareStringInfo(plaintext,results) == 0 ? WizardTrue : WizardFalse;
    (void) PrintValidateString(stdout,"%s.\n",clone != WizardFalse ? "pass" :
      "fail");
    if (clone == WizardFalse)
      pass=WizardFalse;
    results=DestroyStringInfo(results);
    key=DestroyStringInfo(key);
    ciphertext=DestroyStringInfo(ciphertext);
  }
  cipher_info=DestroyCipherInfo(cipher_info);
  /*
    Validate non-blocksize lengths.
  */
  (void) PrintValidateString(stdout,"testing aes non-blocksize "
    "encipher/decipher:\n");
  cipher_info=AcquireCipherInfo(AESCipher,CBCMode);
  key=StringToStringInfo(CipherKey);
  SetCipherKey(cipher_info,key);
  ResetCipherNonce(cipher_info);
  key=DestroyStringInfo(key);
  plaintext=StringToStringInfo(CipherPlaintext);
  (void) PrintValidateString(stdout,"  test 0 ");
  ciphertext=EncipherCipher(cipher_info,plaintext);
  key=StringToStringInfo(CipherKey);
  SetCipherKey(cipher_info,key);
  ResetCipherNonce(cipher_info);
  key=DestroyStringInfo(key);
  plaintext=DecipherCipher(cipher_info,ciphertext);
  results=StringToStringInfo(CipherPlaintext);
  clone=CompareStringInfo(plaintext,results) == 0 ? WizardTrue : WizardFalse;
  (void) PrintValidateString(stdout,"%s.\n",clone != WizardFalse ? "pass" :
    "fail");
  if (clone == WizardFalse)
    pass=WizardFalse;
  results=DestroyStringInfo(results);
  plaintext=DestroyStringInfo(plaintext);
  cipher_info=DestroyCipherInfo(cipher_info);
  /*
    Validate CTR mode.
  */
  (void) PrintValidateString(stdout,"testing aes CTR-mode encipher/decipher:\n");
  cipher_info=AcquireCipherInfo(AESCipher,CTRMode);
  key=StringToStringInfo(CipherKey);
  SetCipherKey(cipher_info,key);
  ResetCipherNonce(cipher_info);
  key=DestroyStringInfo(key);
  plaintext=StringToStringInfo(CipherPlaintext);
  (void) PrintValidateString(stdout,"  test 0 ");
  ciphertext=EncipherCipher(cipher_info,plaintext);
  key=StringToStringInfo(CipherKey);
  SetCipherKey(cipher_info,key);
  ResetCipherNonce(cipher_info);
  key=DestroyStringInfo(key);
  plaintext=DecipherCipher(cipher_info,ciphertext);
  results=StringToStringInfo(CipherPlaintext);
  clone=CompareStringInfo(plaintext,results) == 0 ? WizardTrue : WizardFalse;
  (void) PrintValidateString(stdout,"%s.\n",clone != WizardFalse ? "pass" :
     "fail");
  if (clone == WizardFalse)
    pass=WizardFalse;
  results=DestroyStringInfo(results);
  plaintext=DestroyStringInfo(plaintext);
  cipher_info=DestroyCipherInfo(cipher_info);
  return(pass);
}

static WizardBooleanType TestAuthenticate(const char *passphrase)
{
  AuthenticateInfo
    *authenticate_info;

  ExceptionInfo
    *exception;

  StringInfo
    *key;

  WizardBooleanType
    clone,
    pass;

  (void) PrintValidateString(stdout,"testing secret authentication:\n");
  authenticate_info=AcquireAuthenticateInfo(SecretAuthenticateMethod,
    (const char *) NULL,SHA2256Hash);
  (void) PrintValidateString(stdout,"  generate authenticate key:\n");
  if (passphrase != (const char *) NULL)
    SetAuthenticatePassphrase(authenticate_info,passphrase);
  SetAuthenticateKeyLength(authenticate_info,1024);
  exception=AcquireExceptionInfo();
  pass=GenerateAuthenticateKey(authenticate_info,exception);
  (void) PrintValidateString(stdout,"%s.\n",pass != WizardFalse ? "pass" :
    "fail");
  if (pass != WizardFalse)
    {
      key=CloneStringInfo(GetAuthenticateKey(authenticate_info));
      (void) PrintValidateString(stdout,"  authenticate key:\n");
      if (passphrase != (const char *) NULL)
        SetAuthenticatePassphrase(authenticate_info,passphrase);
      clone=AuthenticateKey(authenticate_info,exception);
      exception=DestroyExceptionInfo(exception);
      if (clone != WizardFalse)
        clone=CompareStringInfo(GetAuthenticateKey(authenticate_info),key) == 0
          ? WizardTrue : WizardFalse;
      if (clone == WizardFalse)
        pass=WizardFalse;
      (void) PrintValidateString(stdout,"%s.\n",clone != WizardFalse ? "pass" :
        "fail");
      key=DestroyStringInfo(key);
    }
  authenticate_info=DestroyAuthenticateInfo(authenticate_info);
  return(pass);
}

static WizardBooleanType TestBZIPEntropy(void)
{
  EntropyInfo
    *entropy_info;

  ExceptionInfo
    *exception;

  register ssize_t
    i;

  StringInfo
    *chaos,
    *plaintext;

  WizardBooleanType
    clone,
    pass,
    status;

  (void) PrintValidateString(stdout,"testing bzip entropy:\n");
  pass=WizardTrue;
  exception=AcquireExceptionInfo();
  entropy_info=AcquireEntropyInfo(BZIPEntropy,6);
  for (i=0; i < ZipTestVectors; i++)
  {
    (void) PrintValidateString(stdout,"  test %.20g ",(double) i);
    plaintext=StringToStringInfo((char *) bzip_test_vector[i].plaintext);
    status=IncreaseEntropy(entropy_info,plaintext,exception);
    chaos=AcquireStringInfo(bzip_test_vector[i].chaossize);
    SetStringInfoDatum(chaos,bzip_test_vector[i].chaos);
    clone=CompareStringInfo(GetEntropyChaos(entropy_info),chaos) == 0 ?
      WizardTrue : WizardFalse;
    (void) PrintValidateString(stdout,"%s.\n",clone != WizardFalse ? "pass" :
      "fail");
    if (clone == WizardFalse)
      pass=WizardFalse;
    plaintext=DestroyStringInfo(plaintext);
    chaos=DestroyStringInfo(chaos);
  }
  entropy_info=DestroyEntropyInfo(entropy_info);
  (void) PrintValidateString(stdout,"testing bzip restore entropy:\n");
  pass=WizardTrue;
  entropy_info=AcquireEntropyInfo(BZIPEntropy,6);
  for (i=0; i < ZipTestVectors; i++)
  {
    (void) PrintValidateString(stdout,"  test %.20g ",(double) i);
    chaos=AcquireStringInfo(bzip_test_vector[i].chaossize);
    SetStringInfoDatum(chaos,bzip_test_vector[i].chaos);
    status=RestoreEntropy(entropy_info,
      strlen((char *) bzip_test_vector[i].plaintext),chaos,exception);
    plaintext=StringToStringInfo((char *) bzip_test_vector[i].plaintext);
    clone=CompareStringInfo(GetEntropyChaos(entropy_info),plaintext) == 0 ?
      WizardTrue : WizardFalse;
    (void) PrintValidateString(stdout,"%s.\n",clone != WizardFalse ? "pass" :
      "fail");
    if (clone == WizardFalse)
      pass=WizardFalse;
    plaintext=DestroyStringInfo(plaintext);
    chaos=DestroyStringInfo(chaos);
  }
  entropy_info=DestroyEntropyInfo(entropy_info);
  exception=DestroyExceptionInfo(exception);
  (void) status;
  return(pass);
}

static WizardBooleanType TestCRC64(void)
{
  HashInfo
    *hash_info;

  register ssize_t
    i;

  StringInfo
    *plaintext,
    *results;

  WizardBooleanType
    clone,
    pass;

  (void) PrintValidateString(stdout,"testing crc64:\n");
  pass=WizardTrue;
  hash_info=AcquireHashInfo(CRC64Hash);
  for (i=0; i < CRC64TestVectors; i++)
  {
    (void) PrintValidateString(stdout,"  test %.20g ",(double) i);
    InitializeHash(hash_info);
    plaintext=StringToStringInfo((char *) crc64_test_vector[i].plaintext);
    UpdateHash(hash_info,plaintext);
    plaintext=DestroyStringInfo(plaintext);
    FinalizeHash(hash_info);
    results=AcquireStringInfo(GetStringInfoLength(GetHashDigest(hash_info)));
    SetStringInfoDatum(results,crc64_test_vector[i].digest);
    clone=CompareStringInfo(GetHashDigest(hash_info),results) == 0 ?
      WizardTrue : WizardFalse;
    (void) PrintValidateString(stdout,"%s.\n",clone != WizardFalse ? "pass" :
      "fail");
    if (clone == WizardFalse)
      pass=WizardFalse;
    results=DestroyStringInfo(results);
  }
  /*
    Multiple update test.
  */
  (void) PrintValidateString(stdout,"  test %.20g ",(double) i);
  InitializeHash(hash_info);
  plaintext=StringToStringInfo("ABCDEFGHIJKLMNOPQRSTUVWXYZabcde");
  UpdateHash(hash_info,plaintext);
  plaintext=DestroyStringInfo(plaintext);
  plaintext=StringToStringInfo("fghijklmnopqrstuvwxyz0123456789");
  UpdateHash(hash_info,plaintext);
  plaintext=DestroyStringInfo(plaintext);
  FinalizeHash(hash_info);
  results=AcquireStringInfo(GetStringInfoLength(GetHashDigest(hash_info)));
  SetStringInfoDatum(results,crc64_test_vector[5].digest);
  clone=CompareStringInfo(GetHashDigest(hash_info),results) == 0 ?
    WizardTrue : WizardFalse;
  (void) PrintValidateString(stdout,"%s.\n",clone != WizardFalse ? "pass" :
     "fail");
  if (clone == WizardFalse)
    pass=WizardFalse;
  results=DestroyStringInfo(results);
  hash_info=DestroyHashInfo(hash_info);
  return(pass);
}

static WizardBooleanType TestHMACMD5(void)
{
  register ssize_t
    i;

  HMACInfo
    *hmac_info;

  StringInfo
    *key,
    *message,
    *results;

  WizardBooleanType
    clone,
    pass;

  (void) PrintValidateString(stdout,"testing hmac-md5:\n");
  pass=WizardTrue;
  hmac_info=AcquireHMACInfo(MD5Hash);
  for (i=0; i < HMACMD5TestVectors; i++)
  {
    (void) PrintValidateString(stdout,"  test %.20g ",(double) i);
    key=StringToStringInfo((char *) hmac_md5_test_vector[i].key);
    message=StringToStringInfo((char *) hmac_md5_test_vector[i].plaintext);
    ConstructHMAC(hmac_info,key,message);
    results=AcquireStringInfo(GetStringInfoLength(GetHMACDigest(hmac_info)));
    SetStringInfoDatum(results,hmac_md5_test_vector[i].digest);
    clone=CompareStringInfo(GetHMACDigest(hmac_info),results) == 0 ?
      WizardTrue : WizardFalse;
    (void) PrintValidateString(stdout,"%s.\n",clone != WizardFalse ? "pass" :
      "fail");
    if (clone == WizardFalse)
      pass=WizardFalse;
    results=DestroyStringInfo(results);
    key=DestroyStringInfo(key);
    message=DestroyStringInfo(message);
  }
  hmac_info=DestroyHMACInfo(hmac_info);
  return(pass);
}

static WizardBooleanType TestHMACSHA1(void)
{
  register ssize_t
    i;

  HMACInfo
    *hmac_info;

  StringInfo
    *key,
    *message,
    *results;

  WizardBooleanType
    clone,
    pass;

  (void) PrintValidateString(stdout,"testing hmac_sha1:\n");
  pass=WizardTrue;
  hmac_info=AcquireHMACInfo(SHA1Hash);
  for (i=0; i < HMACSHA1TestVectors; i++)
  {
    (void) PrintValidateString(stdout,"  test %.20g ",(double) i);
    key=StringToStringInfo((char *) hmac_sha1_test_vector[i].key);
    message=StringToStringInfo((char *) hmac_sha1_test_vector[i].plaintext);
    SetStringInfoDatum(message,hmac_sha1_test_vector[i].plaintext);
    ConstructHMAC(hmac_info,key,message);
    results=AcquireStringInfo(GetStringInfoLength(GetHMACDigest(hmac_info)));
    SetStringInfoDatum(results,hmac_sha1_test_vector[i].digest);
    clone=CompareStringInfo(GetHMACDigest(hmac_info),results) == 0 ?
      WizardTrue : WizardFalse;
    (void) PrintValidateString(stdout,"%s.\n",clone != WizardFalse ? "pass" :
      "fail");
    if (clone == WizardFalse)
      pass=WizardFalse;
    results=DestroyStringInfo(results);
    key=DestroyStringInfo(key);
    message=DestroyStringInfo(message);
  }
  hmac_info=DestroyHMACInfo(hmac_info);
  return(pass);
}

static WizardBooleanType TestHMACSHA2256(void)
{
  register ssize_t
    i;

  HMACInfo
    *hmac_info;

  StringInfo
    *key,
    *message,
    *results;

  WizardBooleanType
    clone,
    pass;

  (void) PrintValidateString(stdout,"testing hmac-sha2256:\n");
  pass=WizardTrue;
  hmac_info=AcquireHMACInfo(SHA2256Hash);
  for (i=0; i < HMACSHA2256TestVectors; i++)
  {
    (void) PrintValidateString(stdout,"  test %.20g ",(double) i);
    key=StringToStringInfo((char *) hmac_sha2256_test_vector[i].key);
    message=StringToStringInfo((char *) hmac_sha2256_test_vector[i].plaintext);
    ConstructHMAC(hmac_info,key,message);
    results=AcquireStringInfo(GetStringInfoLength(GetHMACDigest(hmac_info)));
    SetStringInfoDatum(results,hmac_sha2256_test_vector[i].digest);
    clone=CompareStringInfo(GetHMACDigest(hmac_info),results) == 0 ?
      WizardTrue : WizardFalse;
    (void) PrintValidateString(stdout,"%s.\n",clone != WizardFalse ? "pass" :
      "fail");
    if (clone == WizardFalse)
      pass=WizardFalse;
    results=DestroyStringInfo(results);
    key=DestroyStringInfo(key);
    message=DestroyStringInfo(message);
  }
  hmac_info=DestroyHMACInfo(hmac_info);
  return(pass);
}

static WizardBooleanType TestKeymap(void)
{
  KeyInfo
    *key_info;

  StringInfo
    *id,
    *key,
    *target_key;

  WizardBooleanType
    clone,
    pass;

  (void) PrintValidateString(stdout,"testing key map:\n");
  key_info=AcquireKeyInfo();
  id=StringToStringInfo(CipherId);
  key=StringToStringInfo(CipherKey);
  (void) PrintValidateString(stdout,"  test 0 ");
  pass=SetKeyInfo(key_info,id,key);
  (void) PrintValidateString(stdout,"%s.\n",
    pass != WizardFalse ? "pass" : "fail");
  (void) PrintValidateString(stdout,"  test 1 ");
  target_key=GetKeyInfo(key_info,id);
  clone=WizardFalse;
  if (target_key != (StringInfo *) NULL)
    clone=CompareStringInfo(key,target_key) == 0 ? WizardTrue : WizardFalse;
  (void) PrintValidateString(stdout,"%s.\n",clone != WizardFalse ? "pass" :
    "fail");
  if (clone == WizardFalse)
    pass=WizardFalse;
  target_key=DestroyStringInfo(target_key);
  key=DestroyStringInfo(key);
  id=DestroyStringInfo(id);
  DestroyKeyInfo(key_info);
  return(pass);
}

static WizardBooleanType TestLogEvent(void)
{
  WizardBooleanType
    pass;

  (void) PrintValidateString(stdout,"testing log event:\n");
  (void) SetLogEventMask("User");
  pass=LogWizardEvent(UserEvent,__FILE__,"TestLogEvent",__LINE__,
    "Testing validation suite");
  (void) PrintValidateString(stdout,"  test 0 ");
  (void) PrintValidateString(stdout,"%s.\n",pass != WizardFalse ? "pass" :
    "fail");
  return(pass);
}

static WizardBooleanType TestLZMAEntropy(void)
{
  EntropyInfo
    *entropy_info;

  ExceptionInfo
    *exception;

  register ssize_t
    i;

  StringInfo
    *chaos,
    *plaintext;

  WizardBooleanType
    clone,
    pass,
    status;

  (void) PrintValidateString(stdout,"testing lzma entropy:\n");
  pass=WizardTrue;
  exception=AcquireExceptionInfo();
  entropy_info=AcquireEntropyInfo(LZMAEntropy,6);
  for (i=0; i < LZMATestVectors; i++)
  {
    (void) PrintValidateString(stdout,"  test %.20g ",(double) i);
    plaintext=StringToStringInfo((char *) lzma_test_vector[i].plaintext);
    status=IncreaseEntropy(entropy_info,plaintext,exception);
    chaos=AcquireStringInfo(lzma_test_vector[i].chaossize);
    SetStringInfoDatum(chaos,lzma_test_vector[i].chaos);
    clone=CompareStringInfo(GetEntropyChaos(entropy_info),chaos) == 0 ?
      WizardTrue : WizardFalse;
    (void) PrintValidateString(stdout,"%s.\n",clone != WizardFalse ? "pass" :
      "fail");
    if (clone == WizardFalse)
      pass=WizardFalse;
    plaintext=DestroyStringInfo(plaintext);
    chaos=DestroyStringInfo(chaos);
  }
  entropy_info=DestroyEntropyInfo(entropy_info);
  (void) PrintValidateString(stdout,"testing lzma restore entropy:\n");
  pass=WizardTrue;
  entropy_info=AcquireEntropyInfo(LZMAEntropy,6);
  for (i=0; i < ZipTestVectors; i++)
  {
    (void) PrintValidateString(stdout,"  test %.20g ",(double) i);
    chaos=AcquireStringInfo(lzma_test_vector[i].chaossize);
    SetStringInfoDatum(chaos,lzma_test_vector[i].chaos);
    status=RestoreEntropy(entropy_info,
      strlen((char *) lzma_test_vector[i].plaintext),chaos,exception);
    plaintext=StringToStringInfo((char *) lzma_test_vector[i].plaintext);
    clone=CompareStringInfo(GetEntropyChaos(entropy_info),plaintext) == 0 ?
      WizardTrue : WizardFalse;
    (void) PrintValidateString(stdout,"%s.\n",clone != WizardFalse ? "pass" :
      "fail");
    if (clone == WizardFalse)
      pass=WizardFalse;
    plaintext=DestroyStringInfo(plaintext);
    chaos=DestroyStringInfo(chaos);
  }
  entropy_info=DestroyEntropyInfo(entropy_info);
  exception=DestroyExceptionInfo(exception);
  (void) status;
  return(pass);
}

static WizardBooleanType TestMD5(void)
{
  HashInfo
    *hash_info;

  register ssize_t
    i;

  StringInfo
    *plaintext,
    *results;

  WizardBooleanType
    clone,
    pass;

  (void) PrintValidateString(stdout,"testing md5:\n");
  pass=WizardTrue;
  hash_info=AcquireHashInfo(MD5Hash);
  for (i=0; i < MD5TestVectors; i++)
  {
    (void) PrintValidateString(stdout,"  test %.20g ",(double) i);
    InitializeHash(hash_info);
    plaintext=StringToStringInfo((char *) md5_test_vector[i].plaintext);
    UpdateHash(hash_info,plaintext);
    plaintext=DestroyStringInfo(plaintext);
    FinalizeHash(hash_info);
    results=AcquireStringInfo(GetHashDigestsize(hash_info));
    SetStringInfoDatum(results,md5_test_vector[i].digest);
    clone=CompareStringInfo(GetHashDigest(hash_info),results) == 0 ?
      WizardTrue : WizardFalse;
    (void) PrintValidateString(stdout,"%s.\n",clone != WizardFalse ? "pass" :
      "fail");
    if (clone == WizardFalse)
      pass=WizardFalse;
    results=DestroyStringInfo(results);
  }
  /*
    Multiple update test.
  */
  (void) PrintValidateString(stdout,"  test %.20g ",(double) i);
  InitializeHash(hash_info);
  plaintext=StringToStringInfo("ABCDEFGHIJKLMNOPQRSTUVWXYZabcde");
  UpdateHash(hash_info,plaintext);
  plaintext=DestroyStringInfo(plaintext);
  plaintext=StringToStringInfo("fghijklmnopqrstuvwxyz0123456789");
  UpdateHash(hash_info,plaintext);
  plaintext=DestroyStringInfo(plaintext);
  FinalizeHash(hash_info);
  results=AcquireStringInfo(GetHashDigestsize(hash_info));
  SetStringInfoDatum(results,md5_test_vector[5].digest);
  clone=CompareStringInfo(GetHashDigest(hash_info),results) == 0 ?
    WizardTrue : WizardFalse;
  (void) PrintValidateString(stdout,"%s.\n",clone != WizardFalse ? "pass" :
    "fail");
  if (clone == WizardFalse)
    pass=WizardFalse;
  results=DestroyStringInfo(results);
  hash_info=DestroyHashInfo(hash_info);
  return(pass);
}

static WizardBooleanType TestMemory(void)
{
  register ssize_t
    i;

  WizardBooleanType
    pass;

  void
    *memory;

  (void) PrintValidateString(stdout,"testing memory:\n");
  pass=WizardTrue;
  for (i=1; i < 8; i++)
  {
    (void) PrintValidateString(stdout,"  test %.20g ",(double) i);
    memory=AcquireQuantumMemory((size_t) (i << i << i),sizeof(unsigned char));
    (void) PrintValidateString(stdout,"%s.\n",memory != (void *) NULL ? "pass" :
      "fail");
    pass=memory != (void *) NULL ? WizardTrue : WizardFalse;
    if (memory != (void *) NULL)
      memory=RelinquishWizardMemory(memory);
  }
  return(pass);
}

static WizardBooleanType TestMime(void)
{
  const MimeInfo
    *mime_info;

  ExceptionInfo
    *exception;

  StringInfo
    *content;

  WizardBooleanType
    pass;

  (void) PrintValidateString(stdout,"testing mime:\n");
  (void) PrintValidateString(stdout,"  test 0 ");
  pass=WizardFalse;
  content=StringToStringInfo("#include <stdio.h> int main() { exit(0); }");
  exception=AcquireExceptionInfo();
  mime_info=GetMimeInfo("main.c",GetStringInfoDatum(content),
    GetStringInfoLength(content),exception);
  content=DestroyStringInfo(content);
  exception=DestroyExceptionInfo(exception);
  if (mime_info != (const MimeInfo *) NULL)
    pass=strcmp(GetMimeType(mime_info),"text/x-csrc") == 0 ?
      WizardTrue : WizardFalse;
  (void) PrintValidateString(stdout,"%s.\n",pass != WizardFalse ? "pass" :
    "fail");
  return(pass);
}

static WizardBooleanType TestRandomKey(void)
{
  double
    value;

  RandomInfo
    *random_info;

  register ssize_t
    i;

  StringInfo
    *key;

  WizardBooleanType
    clone,
    pass;

  (void) PrintValidateString(stdout,"testing random key:\n");
  random_info=AcquireRandomInfo(SHA2256Hash);
  key=GetRandomKey(random_info,256);
  key=DestroyStringInfo(key);
  pass=WizardTrue;
  (void) PrintValidateString(stdout,"  test 0 ");
  (void) PrintValidateString(stdout,"%s.\n",pass != WizardFalse ? "pass" :
    "fail");
  (void) PrintValidateString(stdout,"  test 1 ");
  value=0.0;
  for (i=0; i < 1000000; i++)
    value+=GetRandomValue(random_info);
  value/=i;
  clone=(WizardBooleanType) (AbsoluteValue(value-0.5) < 0.001);
  (void) PrintValidateString(stdout,"%s.\n",clone != WizardFalse ? "pass" :
    "fail");
  if (clone == WizardFalse)
    pass=WizardFalse;
  random_info=DestroyRandomInfo(random_info);
  return(pass);
}

static WizardBooleanType TestSerpent(void)
{
  CipherInfo
    *cipher_info;

  register ssize_t
    i;

  StringInfo
    *ciphertext,
    *key,
    *plaintext,
    *results;

  WizardBooleanType
    clone,
    pass;

  (void) PrintValidateString(stdout,"testing serpent encipher:\n");
  pass=WizardTrue;
  cipher_info=AcquireCipherInfo(SerpentCipher,CBCMode);
  for (i=0; i < SerpentEncipherTestVectors; i++)
  {
    (void) PrintValidateString(stdout,"  test %.20g (%.20g bit key) ",(double)
      i+1,(double) (8*serpent_encipher_test_vector[i].key_length));
    key=AcquireStringInfo(serpent_encipher_test_vector[i].key_length);
    SetStringInfoDatum(key,serpent_encipher_test_vector[i].key);
    SetCipherKey(cipher_info,key);
    ResetCipherNonce(cipher_info);
    plaintext=AcquireStringInfo(sizeof(
      serpent_encipher_test_vector[i].plaintext));
    SetStringInfoDatum(plaintext,serpent_encipher_test_vector[i].plaintext);
    ciphertext=EncipherCipher(cipher_info,plaintext);
    results=AcquireStringInfo(sizeof(serpent_encipher_test_vector[i].result));
    SetStringInfoDatum(results,serpent_encipher_test_vector[i].result);
    clone=CompareStringInfo(ciphertext,results) == 0 ? WizardTrue : WizardFalse;
    (void) PrintValidateString(stdout,"%s.\n",clone != WizardFalse ? "pass" :
      "fail");
    if (clone == WizardFalse)
      pass=WizardFalse;
    results=DestroyStringInfo(results);
    key=DestroyStringInfo(key);
    plaintext=DestroyStringInfo(plaintext);
  }
  cipher_info=DestroyCipherInfo(cipher_info);
  (void) PrintValidateString(stdout,"testing serpent decipher:\n");
  cipher_info=AcquireCipherInfo(SerpentCipher,CBCMode);
  for (i=0; i < SerpentDecipherTestVectors; i++)
  {
    (void) PrintValidateString(stdout,"  test %.20g (%.20g bit key) ",(double)
      i+1,(double) (8*serpent_decipher_test_vector[i].key_length));
    key=AcquireStringInfo(serpent_decipher_test_vector[i].key_length);
    SetStringInfoDatum(key,serpent_decipher_test_vector[i].key);
    SetCipherKey(cipher_info,key);
    ResetCipherNonce(cipher_info);
    ciphertext=AcquireStringInfo(sizeof(
      serpent_decipher_test_vector[i].plaintext));
    SetStringInfoDatum(ciphertext,serpent_decipher_test_vector[i].plaintext);
    plaintext=DecipherCipher(cipher_info,ciphertext);
    results=AcquireStringInfo(sizeof(serpent_decipher_test_vector[i].result));
    SetStringInfoDatum(results,serpent_decipher_test_vector[i].result);
    clone=CompareStringInfo(plaintext,results) == 0 ? WizardTrue : WizardFalse;
    (void) PrintValidateString(stdout,"%s.\n",clone != WizardFalse ? "pass" :
      "fail");
    if (clone == WizardFalse)
      pass=WizardFalse;
    results=DestroyStringInfo(results);
    key=DestroyStringInfo(key);
    ciphertext=DestroyStringInfo(ciphertext);
  }
  cipher_info=DestroyCipherInfo(cipher_info);
  /*
    Validate non-blocksize lengths.
  */
  (void) PrintValidateString(stdout,"testing serpent non-blocksize "
    "encipher/decipher:\n");
  cipher_info=AcquireCipherInfo(SerpentCipher,CBCMode);
  key=StringToStringInfo(CipherKey);
  SetCipherKey(cipher_info,key);
  ResetCipherNonce(cipher_info);
  key=DestroyStringInfo(key);
  plaintext=StringToStringInfo(CipherPlaintext);
  (void) PrintValidateString(stdout,"  test 0 ");
  ciphertext=EncipherCipher(cipher_info,plaintext);
  key=StringToStringInfo(CipherKey);
  SetCipherKey(cipher_info,key);
  ResetCipherNonce(cipher_info);
  key=DestroyStringInfo(key);
  plaintext=DecipherCipher(cipher_info,ciphertext);
  results=StringToStringInfo(CipherPlaintext);
  clone=CompareStringInfo(plaintext,results) == 0 ? WizardTrue : WizardFalse;
  (void) PrintValidateString(stdout,"%s.\n",clone != WizardFalse ? "pass" :
    "fail");
  if (clone == WizardFalse)
    pass=WizardFalse;
  results=DestroyStringInfo(results);
  plaintext=DestroyStringInfo(plaintext);
  cipher_info=DestroyCipherInfo(cipher_info);
  /*
    Validate CTR-mode.
  */
  (void) PrintValidateString(stdout,"testing serpent CTR-mode "
    "encipher/decipher:\n");
  cipher_info=AcquireCipherInfo(SerpentCipher,CTRMode);
  key=StringToStringInfo(CipherKey);
  SetCipherKey(cipher_info,key);
  ResetCipherNonce(cipher_info);
  key=DestroyStringInfo(key);
  plaintext=StringToStringInfo(CipherPlaintext);
  (void) PrintValidateString(stdout,"  test 0 ");
  ciphertext=EncipherCipher(cipher_info,plaintext);
  key=StringToStringInfo(CipherKey);
  SetCipherKey(cipher_info,key);
  ResetCipherNonce(cipher_info);
  key=DestroyStringInfo(key);
  plaintext=DecipherCipher(cipher_info,ciphertext);
  results=StringToStringInfo(CipherPlaintext);
  clone=CompareStringInfo(plaintext,results) == 0 ? WizardTrue : WizardFalse;
  (void) PrintValidateString(stdout,"%s.\n",clone != WizardFalse ? "pass" :
    "fail");
  if (clone == WizardFalse)
    pass=WizardFalse;
  results=DestroyStringInfo(results);
  plaintext=DestroyStringInfo(plaintext);
  cipher_info=DestroyCipherInfo(cipher_info);
  return(pass);
}

static WizardBooleanType TestSHA1(void)
{
  HashInfo
    *hash_info;

  register ssize_t
    i;

  StringInfo
    *plaintext,
    *results;

  WizardBooleanType
    clone,
    pass;

  (void) PrintValidateString(stdout,"testing sha1:\n");
  pass=WizardTrue;
  hash_info=AcquireHashInfo(SHA1Hash);
  for (i=0; i < SHA1TestVectors; i++)
  {
    (void) PrintValidateString(stdout,"  test %.20g ",(double) i);
    InitializeHash(hash_info);
    plaintext=StringToStringInfo((char *) sha1_test_vector[i].plaintext);
    UpdateHash(hash_info,plaintext);
    FinalizeHash(hash_info);
    results=AcquireStringInfo(GetStringInfoLength(GetHashDigest(hash_info)));
    SetStringInfoDatum(results,sha1_test_vector[i].digest);
    clone=CompareStringInfo(GetHashDigest(hash_info),results) == 0 ?
      WizardTrue : WizardFalse;
    (void) PrintValidateString(stdout,"%s.\n",clone != WizardFalse ? "pass" :
      "fail");
    if (clone == WizardFalse)
      pass=WizardFalse;
    results=DestroyStringInfo(results);
    plaintext=DestroyStringInfo(plaintext);
  }
  /*
    Multiple update test.
  */
  (void) PrintValidateString(stdout,"  test %.20g ",(double) i);
  InitializeHash(hash_info);
  plaintext=StringToStringInfo("abcdbcdecdefdefgefghfghighij");
  UpdateHash(hash_info,plaintext);
  plaintext=DestroyStringInfo(plaintext);
  plaintext=StringToStringInfo("hijkijkljklmklmnlmnomnopnopq");
  UpdateHash(hash_info,plaintext);
  FinalizeHash(hash_info);
  plaintext=DestroyStringInfo(plaintext);
  results=AcquireStringInfo(GetStringInfoLength(GetHashDigest(hash_info)));
  SetStringInfoDatum(results,sha1_test_vector[1].digest);
  clone=CompareStringInfo(GetHashDigest(hash_info),results) == 0 ?
    WizardTrue : WizardFalse;
  (void) PrintValidateString(stdout,"%s.\n",clone != WizardFalse ? "pass" :
    "fail");
  if (clone == WizardFalse)
    pass=WizardFalse;
  results=DestroyStringInfo(results);
  hash_info=DestroyHashInfo(hash_info);
  return(pass);
}

static WizardBooleanType TestSHA2256(void)
{
  HashInfo
    *hash_info;

  register ssize_t
    i;

  StringInfo
    *plaintext,
    *results;

  WizardBooleanType
    clone,
    pass;

  (void) PrintValidateString(stdout,"testing sha2256:\n");
  pass=WizardTrue;
  hash_info=AcquireHashInfo(SHA2256Hash);
  for (i=0; i < SHA2256TestVectors; i++)
  {
    (void) PrintValidateString(stdout,"  test %.20g ",(double) i);
    InitializeHash(hash_info);
    plaintext=StringToStringInfo((char *) sha2256_test_vector[i].plaintext);
    UpdateHash(hash_info,plaintext);
    FinalizeHash(hash_info);
    results=AcquireStringInfo(GetStringInfoLength(GetHashDigest(hash_info)));
    SetStringInfoDatum(results,sha2256_test_vector[i].digest);
    clone=CompareStringInfo(GetHashDigest(hash_info),results) == 0 ?
      WizardTrue : WizardFalse;
    (void) PrintValidateString(stdout,"%s.\n",clone != WizardFalse ? "pass" :
      "fail");
    if (clone == WizardFalse)
      pass=WizardFalse;
    results=DestroyStringInfo(results);
    plaintext=DestroyStringInfo(plaintext);
  }
  /*
    Multiple update test.
  */
  (void) PrintValidateString(stdout,"  test %.20g ",(double) i);
  InitializeHash(hash_info);
  plaintext=StringToStringInfo("abcdbcdecdefdefgefghfghighij");
  UpdateHash(hash_info,plaintext);
  plaintext=DestroyStringInfo(plaintext);
  plaintext=StringToStringInfo("hijkijkljklmklmnlmnomnopnopq");
  UpdateHash(hash_info,plaintext);
  FinalizeHash(hash_info);
  plaintext=DestroyStringInfo(plaintext);
  results=AcquireStringInfo(GetStringInfoLength(GetHashDigest(hash_info)));
  SetStringInfoDatum(results,sha2256_test_vector[1].digest);
  clone=CompareStringInfo(GetHashDigest(hash_info),results) == 0 ?
    WizardTrue : WizardFalse;
  (void) PrintValidateString(stdout,"%s.\n",clone != WizardFalse ? "pass" :
    "fail");
  if (clone == WizardFalse)
    pass=WizardFalse;
  results=DestroyStringInfo(results);
  hash_info=DestroyHashInfo(hash_info);
  return(pass);
}

static WizardBooleanType TestSHA2384(void)
{
  HashInfo
    *hash_info;

  register ssize_t
    i;

  StringInfo
    *plaintext,
    *results;

  WizardBooleanType
    clone,
    pass;

  (void) PrintValidateString(stdout,"testing sha2384:\n");
  pass=WizardTrue;
  hash_info=AcquireHashInfo(SHA2384Hash);
  for (i=0; i < SHA2384TestVectors; i++)
  {
    (void) PrintValidateString(stdout,"  test %.20g ",(double) i);
    InitializeHash(hash_info);
    plaintext=StringToStringInfo((char *) sha2384_test_vector[i].plaintext);
    UpdateHash(hash_info,plaintext);
    FinalizeHash(hash_info);
    results=AcquireStringInfo(GetStringInfoLength(GetHashDigest(hash_info)));
    SetStringInfoDatum(results,sha2384_test_vector[i].digest);
    clone=CompareStringInfo(GetHashDigest(hash_info),results) == 0 ?
      WizardTrue : WizardFalse;
    (void) PrintValidateString(stdout,"%s.\n",clone != WizardFalse ? "pass" :
      "fail");
    if (clone == WizardFalse)
      pass=WizardFalse;
    results=DestroyStringInfo(results);
    plaintext=DestroyStringInfo(plaintext);
  }
  /*
    Multiple update test.
  */
  (void) PrintValidateString(stdout,"  test %.20g ",(double) i);
  InitializeHash(hash_info);
  plaintext=StringToStringInfo("abcdbcdecdefdefgefghfghighij");
  UpdateHash(hash_info,plaintext);
  plaintext=DestroyStringInfo(plaintext);
  plaintext=StringToStringInfo("hijkijkljklmklmnlmnomnopnopq");
  UpdateHash(hash_info,plaintext);
  plaintext=DestroyStringInfo(plaintext);
  FinalizeHash(hash_info);
  results=AcquireStringInfo(GetStringInfoLength(GetHashDigest(hash_info)));
  SetStringInfoDatum(results,sha2384_test_vector[1].digest);
  clone=CompareStringInfo(GetHashDigest(hash_info),results) == 0 ?
    WizardTrue : WizardFalse;
  (void) PrintValidateString(stdout,"%s.\n",clone != WizardFalse ? "pass" :
    "fail");
  if (clone == WizardFalse)
    pass=WizardFalse;
  results=DestroyStringInfo(results);
  hash_info=DestroyHashInfo(hash_info);
  return(pass);
}

static WizardBooleanType TestSHA2512(void)
{
  HashInfo
    *hash_info;

  register ssize_t
    i;

  StringInfo
    *plaintext,
    *results;

  WizardBooleanType
    clone,
    pass;

  (void) PrintValidateString(stdout,"testing sha2512:\n");
  pass=WizardTrue;
  hash_info=AcquireHashInfo(SHA2512Hash);
  for (i=0; i < SHA2512TestVectors; i++)
  {
    (void) PrintValidateString(stdout,"  test %.20g ",(double) i);
    InitializeHash(hash_info);
    plaintext=StringToStringInfo((char *) sha2512_test_vector[i].plaintext);
    UpdateHash(hash_info,plaintext);
    FinalizeHash(hash_info);
    results=AcquireStringInfo(GetStringInfoLength(GetHashDigest(hash_info)));
    SetStringInfoDatum(results,sha2512_test_vector[i].digest);
    clone=CompareStringInfo(GetHashDigest(hash_info),results) == 0 ?
      WizardTrue : WizardFalse;
    (void) PrintValidateString(stdout,"%s.\n",clone != WizardFalse ? "pass" :
      "fail");
    if (clone == WizardFalse)
      pass=WizardFalse;
    results=DestroyStringInfo(results);
    plaintext=DestroyStringInfo(plaintext);
  }
  /*
    Multiple update test.
  */
  (void) PrintValidateString(stdout,"  test %.20g ",(double) i);
  InitializeHash(hash_info);
  plaintext=StringToStringInfo("abcdbcdecdefdefgefghfghighij");
  UpdateHash(hash_info,plaintext);
  plaintext=DestroyStringInfo(plaintext);
  plaintext=StringToStringInfo("hijkijkljklmklmnlmnomnopnopq");
  UpdateHash(hash_info,plaintext);
  plaintext=DestroyStringInfo(plaintext);
  FinalizeHash(hash_info);
  results=AcquireStringInfo(GetStringInfoLength(GetHashDigest(hash_info)));
  SetStringInfoDatum(results,sha2512_test_vector[1].digest);
  clone=CompareStringInfo(GetHashDigest(hash_info),results) == 0 ?
    WizardTrue : WizardFalse;
  (void) PrintValidateString(stdout,"%s.\n",clone != WizardFalse ? "pass" :
    "fail");
  if (clone == WizardFalse)
    pass=WizardFalse;
  results=DestroyStringInfo(results);
  hash_info=DestroyHashInfo(hash_info);
  return(pass);
}

static WizardBooleanType TestSHA3(void)
{
  HashInfo
    *hash_info;

  register ssize_t
    i;

  StringInfo
    *plaintext,
    *results;

  WizardBooleanType
    clone,
    pass;

  (void) PrintValidateString(stdout,"testing sha3:\n");
  pass=WizardTrue;
  hash_info=AcquireHashInfo(SHA3256Hash);
  for (i=0; i < SHA3TestVectors; i++)
  {
    (void) PrintValidateString(stdout,"  test %.20g ",(double) i);
    InitializeHash(hash_info);
    plaintext=StringToStringInfo((char *) sha1_test_vector[i].plaintext);
    UpdateHash(hash_info,plaintext);
    FinalizeHash(hash_info);
    results=AcquireStringInfo(GetStringInfoLength(GetHashDigest(hash_info)));
    SetStringInfoDatum(results,sha3_test_vector[i].digest);
    clone=CompareStringInfo(GetHashDigest(hash_info),results) == 0 ?
      WizardTrue : WizardFalse;
    (void) PrintValidateString(stdout,"%s.\n",clone != WizardFalse ? "pass" :
      "fail");
    if (clone == WizardFalse)
      pass=WizardFalse;
    results=DestroyStringInfo(results);
    plaintext=DestroyStringInfo(plaintext);
  }
  /*
    Multiple update test.
  */
  (void) PrintValidateString(stdout,"  test %.20g ",(double) i);
  InitializeHash(hash_info);
  plaintext=StringToStringInfo("abcdbcdecdefdefgefghfghi");
  UpdateHash(hash_info,plaintext);
  plaintext=DestroyStringInfo(plaintext);
  plaintext=StringToStringInfo("ghijhijkijkljklmklmnlmnomnopnopq");
  UpdateHash(hash_info,plaintext);
  FinalizeHash(hash_info);
  plaintext=DestroyStringInfo(plaintext);
  results=AcquireStringInfo(GetStringInfoLength(GetHashDigest(hash_info)));
  SetStringInfoDatum(results,sha3_test_vector[1].digest);
  clone=CompareStringInfo(GetHashDigest(hash_info),results) == 0 ?
    WizardTrue : WizardFalse;
  (void) PrintValidateString(stdout,"%s.\n",clone != WizardFalse ? "pass" :
    "fail");
  if (clone == WizardFalse)
    pass=WizardFalse;
  results=DestroyStringInfo(results);
  hash_info=DestroyHashInfo(hash_info);
  return(pass);
}

static WizardBooleanType TestString(void)
{
  StringInfo
    *string_info;

  WizardBooleanType
    pass;

  WizardSizeType
    crc;

  (void) PrintValidateString(stdout,"testing string:\n");
  (void) PrintValidateString(stdout,"  test 0 ");
  string_info=StringToStringInfo("abcdbcdecdefdefgefghfghighij");
  crc=GetStringInfoCRC(string_info);
  pass=crc == (WizardSizeType) 0xb8823aef888dfdd3LL ? WizardTrue : WizardFalse;
  (void) PrintValidateString(stdout,"%s.\n",pass != WizardFalse ? "pass" :
    "fail");
  string_info=DestroyStringInfo(string_info);
  return(pass);
}

static WizardBooleanType TestTwofish(void)
{
  CipherInfo
    *cipher_info;

  register ssize_t
    i;

  StringInfo
    *ciphertext,
    *key,
    *nonce,
    *plaintext,
    *results;

  WizardBooleanType
    clone,
    pass;

  (void) PrintValidateString(stdout,"testing twofish encipher:\n");
  pass=WizardTrue;
  cipher_info=AcquireCipherInfo(TwofishCipher,CBCMode);
  for (i=0; i < TwofishEncipherTestVectors; i++)
  {
    (void) PrintValidateString(stdout,"  test %.20g (%.20g bit key) ",(double)
      i+1,(double) (8*twofish_encipher_test_vector[i].key_length));
    key=AcquireStringInfo(twofish_encipher_test_vector[i].key_length);
    SetStringInfoDatum(key,twofish_encipher_test_vector[i].key);
    SetCipherKey(cipher_info,key);
    ResetCipherNonce(cipher_info);
    nonce=AcquireStringInfo(sizeof(twofish_encipher_test_vector[i].nonce));
    SetStringInfoDatum(nonce,twofish_encipher_test_vector[i].nonce);
    SetCipherNonce(cipher_info,nonce);
    plaintext=AcquireStringInfo(twofish_encipher_test_vector[i].length);
    SetStringInfoDatum(plaintext,twofish_encipher_test_vector[i].plaintext);
    ciphertext=EncipherCipher(cipher_info,plaintext);
    results=AcquireStringInfo(twofish_encipher_test_vector[i].result_length);
    SetStringInfoDatum(results,twofish_encipher_test_vector[i].result);
    clone=CompareStringInfo(ciphertext,results) == 0 ? WizardTrue : WizardFalse;
    (void) PrintValidateString(stdout,"%s.\n",clone != WizardFalse ? "pass" :
      "fail");
    if (clone == WizardFalse)
      pass=WizardFalse;
    results=DestroyStringInfo(results);
    key=DestroyStringInfo(key);
    nonce=DestroyStringInfo(nonce);
    plaintext=DestroyStringInfo(plaintext);
  }
  cipher_info=DestroyCipherInfo(cipher_info);
  (void) PrintValidateString(stdout,"testing twofish decipher:\n");
  cipher_info=AcquireCipherInfo(TwofishCipher,CBCMode);
  for (i=0; i < TwofishDecipherTestVectors; i++)
  {
    (void) PrintValidateString(stdout,"  test %.20g (%.20g bit key) ",(double)
      i+1,(double) (8*twofish_decipher_test_vector[i].key_length));
    key=AcquireStringInfo(twofish_decipher_test_vector[i].key_length);
    SetStringInfoDatum(key,twofish_decipher_test_vector[i].key);
    SetCipherKey(cipher_info,key);
    ResetCipherNonce(cipher_info);
    nonce=AcquireStringInfo(sizeof(twofish_encipher_test_vector[i].nonce));
    SetStringInfoDatum(nonce,twofish_encipher_test_vector[i].nonce);
    SetCipherNonce(cipher_info,nonce);
    ciphertext=AcquireStringInfo(twofish_decipher_test_vector[i].length);
    SetStringInfoDatum(ciphertext,twofish_decipher_test_vector[i].plaintext);
    plaintext=DecipherCipher(cipher_info,ciphertext);
    results=AcquireStringInfo(twofish_decipher_test_vector[i].length);
    SetStringInfoDatum(results,twofish_decipher_test_vector[i].result);
    clone=CompareStringInfo(plaintext,results) == 0 ? WizardTrue : WizardFalse;
    (void) PrintValidateString(stdout,"%s.\n",clone != WizardFalse ? "pass" :
      "fail");
    if (clone == WizardFalse)
      pass=WizardFalse;
    results=DestroyStringInfo(results);
    key=DestroyStringInfo(key);
    nonce=DestroyStringInfo(nonce);
    ciphertext=DestroyStringInfo(ciphertext);
  }
  cipher_info=DestroyCipherInfo(cipher_info);
  /*
    Validate non-blocksize lengths.
  */
  (void) PrintValidateString(stdout,"testing twofish non-blocksize "
    "encipher/decipher:\n");
  cipher_info=AcquireCipherInfo(TwofishCipher,CBCMode);
  key=StringToStringInfo(CipherKey);
  SetCipherKey(cipher_info,key);
  ResetCipherNonce(cipher_info);
  key=DestroyStringInfo(key);
  plaintext=StringToStringInfo(CipherPlaintext);
  (void) PrintValidateString(stdout,"  test 0 ");
  ciphertext=EncipherCipher(cipher_info,plaintext);
  key=StringToStringInfo(CipherKey);
  SetCipherKey(cipher_info,key);
  ResetCipherNonce(cipher_info);
  key=DestroyStringInfo(key);
  plaintext=DecipherCipher(cipher_info,ciphertext);
  results=StringToStringInfo(CipherPlaintext);
  clone=CompareStringInfo(plaintext,results) == 0 ? WizardTrue : WizardFalse;
  (void) PrintValidateString(stdout,"%s.\n",clone != WizardFalse ? "pass" :
    "fail");
  if (clone == WizardFalse)
    pass=WizardFalse;
  results=DestroyStringInfo(results);
  plaintext=DestroyStringInfo(plaintext);
  cipher_info=DestroyCipherInfo(cipher_info);
  /*
    Validate CTR-mode.
  */
  (void) PrintValidateString(stdout,"testing twofish CTR-mode "
    "encipher/decipher:\n");
  cipher_info=AcquireCipherInfo(TwofishCipher,CTRMode);
  key=StringToStringInfo(CipherKey);
  SetCipherKey(cipher_info,key);
  ResetCipherNonce(cipher_info);
  key=DestroyStringInfo(key);
  plaintext=StringToStringInfo(CipherPlaintext);
  (void) PrintValidateString(stdout,"  test 0 ");
  ciphertext=EncipherCipher(cipher_info,plaintext);
  key=StringToStringInfo(CipherKey);
  SetCipherKey(cipher_info,key);
  ResetCipherNonce(cipher_info);
  key=DestroyStringInfo(key);
  plaintext=DecipherCipher(cipher_info,ciphertext);
  results=StringToStringInfo(CipherPlaintext);
  clone=CompareStringInfo(plaintext,results) == 0 ? WizardTrue : WizardFalse;
  (void) PrintValidateString(stdout,"%s.\n",clone != WizardFalse ? "pass" :
    "fail");
  if (clone == WizardFalse)
    pass=WizardFalse;
  results=DestroyStringInfo(results);
  plaintext=DestroyStringInfo(plaintext);
  cipher_info=DestroyCipherInfo(cipher_info);
  return(pass);
}

static WizardBooleanType TestZIPEntropy(void)
{
  EntropyInfo
    *entropy_info;

  ExceptionInfo
    *exception;

  register ssize_t
    i;

  StringInfo
    *chaos,
    *plaintext;

  WizardBooleanType
    clone,
    pass,
    status;

  (void) PrintValidateString(stdout,"testing zip entropy:\n");
  pass=WizardTrue;
  exception=AcquireExceptionInfo();
  entropy_info=AcquireEntropyInfo(ZIPEntropy,6);
  for (i=0; i < ZipTestVectors; i++)
  {
    (void) PrintValidateString(stdout,"  test %.20g ",(double) i);
    plaintext=StringToStringInfo((char *) zip_test_vector[i].plaintext);
    status=IncreaseEntropy(entropy_info,plaintext,exception);
    chaos=AcquireStringInfo(zip_test_vector[i].chaossize);
    SetStringInfoDatum(chaos,zip_test_vector[i].chaos);
    clone=CompareStringInfo(GetEntropyChaos(entropy_info),chaos) == 0 ?
      WizardTrue : WizardFalse;
    (void) PrintValidateString(stdout,"%s.\n",clone != WizardFalse ? "pass" :
      "fail");
    if (clone == WizardFalse)
      pass=WizardFalse;
    plaintext=DestroyStringInfo(plaintext);
    chaos=DestroyStringInfo(chaos);
  }
  entropy_info=DestroyEntropyInfo(entropy_info);
  (void) PrintValidateString(stdout,"testing zip restore entropy:\n");
  pass=WizardTrue;
  entropy_info=AcquireEntropyInfo(ZIPEntropy,6);
  for (i=0; i < ZipTestVectors; i++)
  {
    (void) PrintValidateString(stdout,"  test %.20g ",(double) i);
    chaos=AcquireStringInfo(zip_test_vector[i].chaossize);
    SetStringInfoDatum(chaos,zip_test_vector[i].chaos);
    status=RestoreEntropy(entropy_info,
      strlen((char *) zip_test_vector[i].plaintext),chaos,exception);
    plaintext=StringToStringInfo((char *) zip_test_vector[i].plaintext);
    clone=CompareStringInfo(GetEntropyChaos(entropy_info),plaintext) == 0 ?
      WizardTrue : WizardFalse;
    (void) PrintValidateString(stdout,"%s.\n",clone != WizardFalse ? "pass" :
      "fail");
    if (clone == WizardFalse)
      pass=WizardFalse;
    plaintext=DestroyStringInfo(plaintext);
    chaos=DestroyStringInfo(chaos);
  }
  entropy_info=DestroyEntropyInfo(entropy_info);
  exception=DestroyExceptionInfo(exception);
  (void) status;
  return(pass);
}

int main(int argc,char **argv)
{
  const char
    *passphrase;

  WizardBooleanType
    pass;

  /*
    Validate the Wizard's Toolkit.
  */
  passphrase=(const char *) NULL;
  if (argc > 0)
    passphrase=argv[1];
  WizardsToolkitGenesis(*argv);
  (void) PrintValidateString(stdout,"Version: %s\n",
    GetWizardVersion((size_t *) NULL));
  (void) PrintValidateString(stdout,"Copyright: %s\n\n",
    GetWizardCopyright());
  (void) PrintValidateString(stdout,"Wizard Validation Suite\n\n");
  pass=TestMemory();
  if (TestString() == WizardFalse)
    pass=WizardFalse;
  if (TestLogEvent() == WizardFalse)
    pass=WizardFalse;
#if !defined(WIZARDSTOOLKIT_WINDOWS_SUPPORT)
  if (TestMime() == WizardFalse)
    pass=WizardFalse;
#endif
  if (TestCRC64() == WizardFalse)
    pass=WizardFalse;
  if (TestMD5() == WizardFalse)
    pass=WizardFalse;
  if (TestSHA1() == WizardFalse)
    pass=WizardFalse;
  if (TestSHA2256() == WizardFalse)
    pass=WizardFalse;
  if (TestSHA2384() == WizardFalse)
    pass=WizardFalse;
  if (TestSHA2512() == WizardFalse)
    pass=WizardFalse;
  if (TestSHA3() == WizardFalse)
    pass=WizardFalse;
  if (TestHMACMD5() == WizardFalse)
    pass=WizardFalse;
  if (TestHMACSHA1() == WizardFalse)
    pass=WizardFalse;
  if (TestHMACSHA2256() == WizardFalse)
    pass=WizardFalse;
  if (TestAES() == WizardFalse)
    pass=WizardFalse;
  if (TestSerpent() == WizardFalse)
    pass=WizardFalse;
  if (TestTwofish() == WizardFalse)
    pass=WizardFalse;
  if (TestBZIPEntropy() == WizardFalse)
    pass=WizardFalse;
#if defined(WIZARDSTOOLKIT_LZMA_DELEGATE)
  if (TestLZMAEntropy() == WizardFalse)
    pass=WizardFalse;
#endif
  if (TestZIPEntropy() == WizardFalse)
    pass=WizardFalse;
  if (TestRandomKey() == WizardFalse)
    pass=WizardFalse;
  if (TestKeymap() == WizardFalse)
    pass=WizardFalse;
  if (TestAuthenticate(passphrase) == WizardFalse)
    pass=WizardFalse;
  if (pass != WizardFalse)
    (void) PrintValidateString(stdout,"validation: passed.\n");
  else
    (void) PrintValidateString(stdout,"validation: failed.\n");
  WizardsToolkitTerminus();
  return(pass == WizardFalse ? 1 : 0);
}
