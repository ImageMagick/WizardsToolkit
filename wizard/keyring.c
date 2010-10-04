/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%               K   K  EEEEE  Y   Y  RRRR   IIIII  N   N   GGGG               %
%               K  K   E       Y Y   R   R    I    NN  N  G                   %
%               KKK    EEE      Y    RRRR     I    N N N  G GGG               %
%               K  K   E        Y    R R      I    N  NN  G   G               %
%               K   K  EEEEE    Y    R  R   IIIII  N   N   GGGG               %
%                                                                             %
%                                                                             %
%                      Wizard's Toolkit Keyring Methods                       %
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
%  See the License for the specific language governing permissions and        %
%  limitations under the License.                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%
%
*/

/*
  Include declarations.
*/
#include "wizard/studio.h"
#include "wizard/blob-private.h"
#include "wizard/exception.h"
#include "wizard/exception-private.h"
#include "wizard/file.h"
#include "wizard/keyring.h"
#include "wizard/magick.h"
#include "wizard/memory_.h"
#include "wizard/utility.h"
#include "wizard/xml-tree.h"

/*
  Define declarations.
*/
#define KeyringFilename  "keyring.xdm"
#define KeyringFiletype  "keyring"
#define KeyringProtocolMajorVersion  1
#define KeyringProtocolMinorVersion  1

/*
  Typedef declarations.
*/
struct _KeyringInfo
{
  char
    *path;

  StringInfo
    *id,
    *key,
    *nonce;

  unsigned short
    protocol_major,
    protocol_minor;

  time_t
    timestamp;

  size_t
    signature;
};

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A c q u i r e K e y r i n g I n f o                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireKeyringInfo() allocates the KeyringInfo structure.
%
%  The format of the AcquireKeyringInfo method is:
%
%      KeyringInfo *AcquireKeyringInfo(const char *path)
%
%  A description of each parameter follows:
%
%    o path: The keyring path.
%
*/
WizardExport KeyringInfo *AcquireKeyringInfo(const char *path)
{
  KeyringInfo
    *keyring_info;

  keyring_info=(KeyringInfo *) AcquireQuantumMemory(1,sizeof(*keyring_info));
  if (keyring_info == (KeyringInfo *) NULL)
    ThrowWizardFatalError(KeyringDomain,MemoryError);
  (void) ResetWizardMemory(keyring_info,0,sizeof(*keyring_info));
  keyring_info->path=(char *) NULL;
  if (path != (char *) NULL)
    keyring_info->path=ConstantString(path);
  keyring_info->id=(StringInfo *) NULL;
  keyring_info->key=(StringInfo *) NULL;
  keyring_info->nonce=(StringInfo *) NULL;
  keyring_info->protocol_major=KeyringProtocolMajorVersion;
  keyring_info->protocol_minor=KeyringProtocolMinorVersion;
  keyring_info->timestamp=time((time_t *) NULL);
  keyring_info->signature=WizardSignature;
  return(keyring_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e s t r o y K e y r i n g I n f o                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroyKeyringInfo() zeros memory associated with the KeyringInfo structure.
%
%  The format of the DestroyKeyringInfo method is:
%
%      KeyringInfo *DestroyKeyringInfo(KeyringInfo *keyring_info)
%
%  A description of each parameter follows:
%
%    o keyring_info: The ring info.
%
*/
WizardExport KeyringInfo *DestroyKeyringInfo(KeyringInfo *keyring_info)
{
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(KeymapDomain,keyring_info != (KeyringInfo *) NULL);
  WizardAssert(KeymapDomain,keyring_info->signature == WizardSignature);
  if (keyring_info->nonce != (StringInfo *) NULL)
    keyring_info->nonce=DestroyStringInfo(keyring_info->nonce);
  if (keyring_info->key != (StringInfo *) NULL)
    keyring_info->key=DestroyStringInfo(keyring_info->key);
  if (keyring_info->id != (StringInfo *) NULL)
    keyring_info->id=DestroyStringInfo(keyring_info->id);
  if (keyring_info->path != (char *) NULL)
    keyring_info->path=DestroyString(keyring_info->path);
  keyring_info->signature=(~WizardSignature);
  keyring_info=(KeyringInfo *) RelinquishWizardMemory(keyring_info);
  return(keyring_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   E x p o r t K e y r i n g K e y                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ExportKeyringKey() exports a key from the keyring.
%
%  The format of the ExportKeyringKey method is:
%
%      WizardBooleanType ExportKeyringKey(KeyringInfo *keyring_info,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o keyring_info: The ring info.
%
%    o exception: Return any errors or warnings in this structure.
%
*/
WizardExport WizardBooleanType ExportKeyringKey(KeyringInfo *keyring_info,
  ExceptionInfo *exception)
{
  FileInfo
    *file_info;

  WizardStatusType
    status;

  StringInfo
    *filetype,
    *id,
    *key,
    *magick,
    *nonce,
    *target;

  size_t
    length,
    signature;

  WizardSizeType
    timestamp;

  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(KeymapDomain,keyring_info != (KeyringInfo *) NULL);
  WizardAssert(KeymapDomain,keyring_info->signature == WizardSignature);
  WizardAssert(KeymapDomain,exception != (ExceptionInfo *) NULL);
  file_info=AcquireFileInfo(keyring_info->path,KeyringFilename,ReadFileMode,
    exception);
  if (file_info == (FileInfo *) NULL)
    return(WizardFalse);
  magick=GetWizardMagick(WizardMagick,sizeof(WizardMagick));
  target=CloneStringInfo(magick);
  status=ReadFileChunk(file_info,GetStringInfoDatum(target),
    GetStringInfoLength(target));
  if ((status == WizardFalse) || (CompareStringInfo(target,magick) != 0))
    {
      file_info=DestroyFileInfo(file_info,exception);
      (void) ThrowWizardException(exception,GetWizardModule(),KeyringError,
        "corrupt key ring file `%s'",GetFilePath(file_info));
      return(WizardFalse);
    }
  magick=DestroyStringInfo(magick);
  target=DestroyStringInfo(target);
  filetype=GetWizardMagick((unsigned char *) KeyringFiletype,
    strlen(KeyringFiletype));
  target=CloneStringInfo(filetype);
  status|=ReadFileChunk(file_info,GetStringInfoDatum(target),
    GetStringInfoLength(target));
  if ((status == WizardFalse) || (CompareStringInfo(target,filetype) != 0))
    {
      file_info=DestroyFileInfo(file_info,exception);
      (void) ThrowWizardException(exception,GetWizardModule(),KeyringError,
        "corrupt key ring file `%s'",GetFilePath(file_info));
      return(WizardFalse);
    }
  filetype=DestroyStringInfo(filetype);
  target=DestroyStringInfo(target);
  signature=0;
  length=0;
  while (ReadFile32Bits(file_info,&signature) != 0)
  {
    if (signature != WizardSignature)
      {
        file_info=DestroyFileInfo(file_info,exception);
        (void) ThrowWizardException(exception,GetWizardModule(),KeyringError,
          "corrupt key ring file `%s'",GetFilePath(file_info));
        return(WizardFalse);
      }
    keyring_info->signature=signature;
    status|=ReadFile32Bits(file_info,&length);
    status|=ReadFile16Bits(file_info,&keyring_info->protocol_major);
    status|=ReadFile16Bits(file_info,&keyring_info->protocol_minor);
    if ((keyring_info->protocol_major == 1) &&
        (keyring_info->protocol_minor == 0))
      timestamp=(time_t) length;
    else
      status|=ReadFile64Bits(file_info,&timestamp);
    keyring_info->timestamp=(time_t) timestamp;
    status|=ReadFile32Bits(file_info,&length);
    if (status == WizardFalse)
      {
        file_info=DestroyFileInfo(file_info,exception);
        (void) ThrowWizardException(exception,GetWizardModule(),KeyringError,
          "corrupt key ring file `%s'",GetFilePath(file_info));
        return(WizardFalse);
      }
    id=AcquireStringInfo(length);
    status|=ReadFileChunk(file_info,GetStringInfoDatum(id),
      GetStringInfoLength(id));
    status|=ReadFile32Bits(file_info,&length);
    if (status == WizardFalse)
      {
        file_info=DestroyFileInfo(file_info,exception);
        (void) ThrowWizardException(exception,GetWizardModule(),KeyringError,
          "corrupt key ring file `%s'",GetFilePath(file_info));
        return(WizardFalse);
      }
    key=AcquireStringInfo(length);
    status|=ReadFileChunk(file_info,GetStringInfoDatum(key),
      GetStringInfoLength(key));
    status|=ReadFile32Bits(file_info,&length);
    if (status == WizardFalse)
      {
        file_info=DestroyFileInfo(file_info,exception);
        (void) ThrowWizardException(exception,GetWizardModule(),KeyringError,
          "corrupt key ring file `%s'",GetFilePath(file_info));
        return(WizardFalse);
      }
    nonce=AcquireStringInfo(length);
    status|=ReadFileChunk(file_info,GetStringInfoDatum(nonce),
      GetStringInfoLength(nonce));
    if (CompareStringInfo(keyring_info->id,id) == 0)
      {
        SetKeyringKey(keyring_info,key);
        SetKeyringNonce(keyring_info,nonce);
        nonce=DestroyStringInfo(nonce);
        key=DestroyStringInfo(key);
        id=DestroyStringInfo(id);
        file_info=DestroyFileInfo(file_info,exception);
        return(WizardTrue);
      }
    nonce=DestroyStringInfo(nonce);
    key=DestroyStringInfo(key);
    id=DestroyStringInfo(id);
    if (status == WizardFalse)
      {
        file_info=DestroyFileInfo(file_info,exception);
        (void) ThrowWizardException(exception,GetWizardModule(),KeyringError,
          "corrupt key ring file `%s'",GetFilePath(file_info));
        return(WizardFalse);
      }
  }
  file_info=DestroyFileInfo(file_info,exception);
  return(WizardFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t K e y r i n g K e y                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetKeyringKey() returns the keyring key.
%
%  The format of the GetKeyringKey method is:
%
%      const StringInfo *GetKeyringKey(const KeyringInfo *keyring_info)
%
%  A description of each parameter follows:
%
%    o keyring_info: The keyring info.
%
*/
WizardExport const StringInfo *GetKeyringKey(const KeyringInfo *keyring_info)
{
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(KeyringDomain,keyring_info != (KeyringInfo *) NULL);
  WizardAssert(KeyringDomain,keyring_info->signature == WizardSignature);
  return(keyring_info->key);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t K e y r i n g N o n c e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetKeyringNonce() returns the keyring nonce.
%
%  The format of the GetKeyringNonce method is:
%
%      const StringInfo *GetKeyringNonce(const KeyringInfo *keyring_info)
%
%  A description of each parameter follows:
%
%    o keyring_info: The keyring info.
%
*/
WizardExport const StringInfo *GetKeyringNonce(const KeyringInfo *keyring_info)
{
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(KeyringDomain,keyring_info != (KeyringInfo *) NULL);
  WizardAssert(KeyringDomain,keyring_info->signature == WizardSignature);
  return(keyring_info->nonce);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I m p o r t K e y r i n g K e y                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ImportKeyringKey() imports a key to the keyring.
%
%  The format of the ImportKeyringKey method is:
%
%      WizardBooleanType ImportKeyringKey(KeyringInfo *keyring_info,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o keyring_info: The ring info.
%
%    o exception: Return any errors or warnings in this structure.
%
*/
WizardExport WizardBooleanType ImportKeyringKey(KeyringInfo *keyring_info,
  ExceptionInfo *exception)
{
  FileInfo
    *file_info;

  KeyringInfo
    *import_info;

  off_t
    offset;

  WizardStatusType
    status;

  size_t
    length;

  StringInfo
    *filetype,
    *magick;

  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(KeymapDomain,keyring_info != (KeyringInfo *) NULL);
  WizardAssert(KeymapDomain,keyring_info->signature == WizardSignature);
  WizardAssert(KeymapDomain,exception != (ExceptionInfo *) NULL);
  import_info=AcquireKeyringInfo(keyring_info->path);
  SetKeyringId(import_info,keyring_info->id);
  if (ExportKeyringKey(import_info,exception) != WizardFalse)
    {
      char
        *id;

      /*
        Duplicate keys are not allowed in the keyring.
      */
      id=StringInfoToHexString(keyring_info->id);
      (void) ThrowWizardException(exception,GetWizardModule(),KeyringError,
        "unable to import key `%s' (its already in the keyring): %s",id,
        keyring_info->path);
      id=DestroyString(id);
      import_info=DestroyKeyringInfo(import_info);
      return(WizardFalse);
    }
  import_info=DestroyKeyringInfo(import_info);
  file_info=AcquireFileInfo(keyring_info->path,KeyringFilename,WriteFileMode,
    exception);
  if (file_info == (FileInfo *) NULL)
    return(WizardFalse);
  magick=GetWizardMagick(WizardMagick,sizeof(WizardMagick));
  status=WriteFileChunk(file_info,GetStringInfoDatum(magick),
    GetStringInfoLength(magick));
  magick=DestroyStringInfo(magick);
  filetype=GetWizardMagick((unsigned char *) KeyringFiletype,
    strlen(KeyringFiletype));
  status|=WriteFileChunk(file_info,GetStringInfoDatum(filetype),
    GetStringInfoLength(filetype));
  filetype=DestroyStringInfo(filetype);
  offset=WizardSeek(GetFileDescriptor(file_info),0,SEEK_END);
  if (offset == -1)
    {
      file_info=DestroyFileInfo(file_info,exception);
      (void) ThrowWizardException(exception,GetWizardModule(),KeyringError,
        "unable to seek keyring `%s': %s",GetFilePath(file_info),
        strerror(errno));
      return(WizardFalse);
    }
  status|=WriteFile32Bits(file_info,keyring_info->signature);
  status|=WriteFile32Bits(file_info,0U);
  status|=WriteFile16Bits(file_info,keyring_info->protocol_major);
  status|=WriteFile16Bits(file_info,keyring_info->protocol_minor);
  status|=WriteFile64Bits(file_info,(WizardSizeType) keyring_info->timestamp);
  length=GetStringInfoLength(keyring_info->id);
  status|=WriteFile32Bits(file_info,(size_t) length);
  status|=WriteFileChunk(file_info,GetStringInfoDatum(keyring_info->id),length);
  length=GetStringInfoLength(keyring_info->key);
  status|=WriteFile32Bits(file_info,(size_t) length);
  status|=WriteFileChunk(file_info,GetStringInfoDatum(keyring_info->key),
    length);
  length=GetStringInfoLength(keyring_info->nonce);
  status|=WriteFile32Bits(file_info,(size_t) length);
  status|=WriteFileChunk(file_info,GetStringInfoDatum(keyring_info->nonce),
    length);
  if (status == WizardFalse)
    (void) ThrowWizardException(exception,GetWizardModule(),KeyringError,
      "unable to write keyring `%s': %s",GetFilePath(file_info),
      strerror(errno));
  file_info=DestroyFileInfo(file_info,exception);
  return(WizardTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P r i n t K e y r i n g P r o p e r t i e s                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PrintKeyringProperties() prints properties associated with each key in the
%  keyring.
%
%  The format of the PrintKeyringProperties method is:
%
%      WizardBooleanType PrintKeyringProperties(const char *path,
%        BlobInfo *keyring_blob,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o path: the keyring path.
%
%    o blob_ibfo: list the key properties to this blob.
%
%    o exception: Return any errors or warnings in this structure.
%
*/
WizardExport WizardBooleanType PrintKeyringProperties(const char *path,
  BlobInfo *keyring_blob,ExceptionInfo *exception)
{
  char
    *canonical_path,
    *hex,
    *keyring_rdf,
    message[MaxTextExtent];

  const struct stat
    *properties;

  FileInfo
    *file_info;

  KeyringInfo
    keyring_info;

  ssize_t
    count;

  StringInfo
    *filetype,
    *id,
    *key,
    *magick,
    *nonce,
    *target;

  size_t
    length;

  WizardSizeType
    timestamp;

  WizardStatusType
    status;

  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(KeymapDomain,exception != (ExceptionInfo *) NULL);
  file_info=AcquireFileInfo(path,KeyringFilename,ReadFileMode,exception);
  if (file_info == (FileInfo *) NULL)
    return(WizardTrue);
  keyring_rdf=AcquireString("  <keyring:Keyring rdf:about=\"");
  canonical_path=CanonicalXMLContent(GetFilePath(file_info),WizardFalse);
  (void) ConcatenateString(&keyring_rdf,canonical_path);
  (void) ConcatenateString(&keyring_rdf,"\">\n");
  properties=GetFileProperties(file_info);
  (void) ConcatenateString(&keyring_rdf,"    <keyring:modify-date>");
  (void) FormatWizardTime(properties->st_mtime,MaxTextExtent,message);
  (void) ConcatenateString(&keyring_rdf,message);
  (void) ConcatenateString(&keyring_rdf,"</keyring:modify-date>\n");
  (void) ConcatenateString(&keyring_rdf,"    <keyring:create-date>");
  (void) FormatWizardTime(properties->st_mtime,MaxTextExtent,message);
  (void) ConcatenateString(&keyring_rdf,message);
  (void) ConcatenateString(&keyring_rdf,"</keyring:create-date>\n");
  (void) ConcatenateString(&keyring_rdf,"    <keyring:timestamp>");
  (void) FormatWizardTime(time((time_t *) NULL),MaxTextExtent,message);
  (void) ConcatenateString(&keyring_rdf,message);
  (void) ConcatenateString(&keyring_rdf,"</keyring:timestamp>\n");
  (void) ConcatenateString(&keyring_rdf,"  </keyring:Keyring>\n");
  length=strlen(keyring_rdf);
  count=WriteBlob(keyring_blob,length,(unsigned char *) keyring_rdf);
  keyring_rdf=DestroyString(keyring_rdf);
  if (count != (ssize_t) length)
    ThrowFileException(exception,FileError,GetFilePath(file_info));
  magick=GetWizardMagick(WizardMagick,sizeof(WizardMagick));
  target=CloneStringInfo(magick);
  status=ReadFileChunk(file_info,GetStringInfoDatum(target),
    GetStringInfoLength(target));
  if ((status == WizardFalse) || (CompareStringInfo(target,magick) != 0))
    {
      file_info=DestroyFileInfo(file_info,exception);
      (void) ThrowWizardException(exception,GetWizardModule(),KeyringError,
        "corrupt key ring file `%s'",GetFilePath(file_info));
      return(WizardFalse);
    }
  magick=DestroyStringInfo(magick);
  target=DestroyStringInfo(target);
  filetype=GetWizardMagick((unsigned char *) KeyringFiletype,
    strlen(KeyringFiletype));
  target=CloneStringInfo(filetype);
  status|=ReadFileChunk(file_info,GetStringInfoDatum(target),
    GetStringInfoLength(target));
  if ((status == WizardFalse) || (CompareStringInfo(target,filetype) != 0))
    {
      file_info=DestroyFileInfo(file_info,exception);
      (void) ThrowWizardException(exception,GetWizardModule(),KeyringError,
        "corrupt key ring file `%s'",GetFilePath(file_info));
      return(WizardFalse);
    }
  filetype=DestroyStringInfo(filetype);
  target=DestroyStringInfo(target);
  length=0;
  (void) ResetWizardMemory(&keyring_info,0,sizeof(keyring_info));
  while (ReadFile32Bits(file_info,&keyring_info.signature) != 0)
  {
    if (keyring_info.signature != WizardSignature)
      {
        file_info=DestroyFileInfo(file_info,exception);
        (void) ThrowWizardException(exception,GetWizardModule(),KeyringError,
          "corrupt key ring file `%s'",GetFilePath(file_info));
        return(WizardFalse);
      }
    status|=ReadFile32Bits(file_info,&length);
    status|=ReadFile16Bits(file_info,&keyring_info.protocol_major);
    status|=ReadFile16Bits(file_info,&keyring_info.protocol_minor);
    if ((keyring_info.protocol_major == 1) &&
        (keyring_info.protocol_minor == 0))
      timestamp=(time_t) length;
    else
      status|=ReadFile64Bits(file_info,&timestamp);
    keyring_info.timestamp=(time_t) timestamp;
    status|=ReadFile32Bits(file_info,&length);
    if (status == WizardFalse)
      {
        file_info=DestroyFileInfo(file_info,exception);
        (void) ThrowWizardException(exception,GetWizardModule(),KeyringError,
          "corrupt key ring file `%s'",GetFilePath(file_info));
        return(WizardFalse);
      }
    id=AcquireStringInfo(length);
    status|=ReadFileChunk(file_info,GetStringInfoDatum(id),
      GetStringInfoLength(id));
    status|=ReadFile32Bits(file_info,&length);
    if (status == WizardFalse)
      {
        file_info=DestroyFileInfo(file_info,exception);
        (void) ThrowWizardException(exception,GetWizardModule(),KeyringError,
          "corrupt key ring file `%s'",GetFilePath(file_info));
        return(WizardFalse);
      }
    key=AcquireStringInfo(length);
    status|=ReadFileChunk(file_info,GetStringInfoDatum(key),
      GetStringInfoLength(key));
    status|=ReadFile32Bits(file_info,&length);
    if (status == WizardFalse)
      {
        file_info=DestroyFileInfo(file_info,exception);
        (void) ThrowWizardException(exception,GetWizardModule(),KeyringError,
          "corrupt key ring file `%s'",GetFilePath(file_info));
        return(WizardFalse);
      }
    nonce=AcquireStringInfo(length);
    status|=ReadFileChunk(file_info,GetStringInfoDatum(nonce),
      GetStringInfoLength(nonce));
    keyring_rdf=AcquireString("  <keyring:Key rdf:about=\"");
    hex=StringInfoToHexString(id);
    (void) ConcatenateString(&keyring_rdf,hex);
    hex=DestroyString(hex);
    (void) ConcatenateString(&keyring_rdf,"\">\n");
    (void) ConcatenateString(&keyring_rdf,"    <keyring:memberOf "
      "rdf:resource=\"");
    (void) ConcatenateString(&keyring_rdf,canonical_path);
    (void) ConcatenateString(&keyring_rdf,"\"/>\n");
    (void) ConcatenateString(&keyring_rdf,"    <keyring:nonce>");
    hex=StringInfoToHexString(nonce);
    (void) ConcatenateString(&keyring_rdf,hex);
    hex=DestroyString(hex);
    (void) ConcatenateString(&keyring_rdf,"</keyring:nonce>\n");
    (void) ConcatenateString(&keyring_rdf,"    <keyring:timestamp>");
    (void) FormatWizardTime(keyring_info.timestamp,MaxTextExtent,message);
    (void) ConcatenateString(&keyring_rdf,message);
    (void) ConcatenateString(&keyring_rdf,"</keyring:timestamp>\n");
    (void) ConcatenateString(&keyring_rdf,"    <keyring:protocol>");
    (void) FormatWizardString(message,MaxTextExtent,"%u.%u",
      keyring_info.protocol_major,(unsigned int) keyring_info.protocol_minor);
    (void) ConcatenateString(&keyring_rdf,message);
    (void) ConcatenateString(&keyring_rdf,"</keyring:protocol>\n");
    (void) ConcatenateString(&keyring_rdf,"  </keyring:Key>\n");
    length=strlen(keyring_rdf);
    count=WriteBlob(keyring_blob,length,(unsigned char *) keyring_rdf);
    keyring_rdf=DestroyString(keyring_rdf);
    if (count != (ssize_t) length)
      ThrowFileException(exception,FileError,GetFilePath(file_info));
    nonce=DestroyStringInfo(nonce);
    key=DestroyStringInfo(key);
    id=DestroyStringInfo(id);
    if (status == WizardFalse)
      {
        file_info=DestroyFileInfo(file_info,exception);
        (void) ThrowWizardException(exception,GetWizardModule(),KeyringError,
          "corrupt key ring file `%s'",GetFilePath(file_info));
        return(WizardFalse);
      }
  }
  canonical_path=DestroyString(canonical_path);
  file_info=DestroyFileInfo(file_info,exception);
  return(WizardTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t K e y r i n g I d                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetKeyringId() sets the keyring id.
%
%  The format of the SetKeyringId method is:
%
%      void SetKeyringId(KeyringInfo *keyring_info,const StringInfo *id)
%
%  A description of each parameter follows:
%
%    o keyring_info: The keyring info.
%
%    o id: The id.
%
*/
WizardExport void SetKeyringId(KeyringInfo *keyring_info,const StringInfo *id)
{
  WizardAssert(KeymapDomain,keyring_info != (KeyringInfo *) NULL);
  WizardAssert(KeymapDomain,keyring_info->signature == WizardSignature);
  if (keyring_info->id != (StringInfo *) NULL)
    keyring_info->id=DestroyStringInfo(keyring_info->id);
  keyring_info->id=CloneStringInfo(id);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t K e y r i n g K e y                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetKeyringKey() sets the keyring key.
%
%  The format of the SetKeyringKey method is:
%
%      void SetKeyringKey(KeyringInfo *keyring_info,const StringInfo *key)
%
%  A description of each parameter follows:
%
%    o keyring_info: The keyring info.
%
%    o key: The key.
%
*/
WizardExport void SetKeyringKey(KeyringInfo *keyring_info,const StringInfo *key)
{
  WizardAssert(KeymapDomain,keyring_info != (KeyringInfo *) NULL);
  WizardAssert(KeymapDomain,keyring_info->signature == WizardSignature);
  if (keyring_info->key != (StringInfo *) NULL)
    keyring_info->key=DestroyStringInfo(keyring_info->key);
  keyring_info->key=CloneStringInfo(key);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t K e y r i n g N o n c e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetKeyringNonce() sets the keyring nonce.
%
%  The format of the SetKeyringNonce method is:
%
%      void SetKeyringNonce(KeyringInfo *keyring_info,const StringInfo *nonce)
%
%  A description of each parameter follows:
%
%    o keyring_info: The keyring info.
%
%    o nonce: The nonce.
%
*/
WizardExport void SetKeyringNonce(KeyringInfo *keyring_info,
  const StringInfo *nonce)
{
  WizardAssert(KeymapDomain,keyring_info != (KeyringInfo *) NULL);
  WizardAssert(KeymapDomain,keyring_info->signature == WizardSignature);
  if (keyring_info->nonce != (StringInfo *) NULL)
    keyring_info->nonce=DestroyStringInfo(keyring_info->nonce);
  keyring_info->nonce=CloneStringInfo(nonce);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t K e y r i n g P a t h                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetKeyringPath() sets the keyring path.
%
%  The format of the SetKeyringPath method is:
%
%      void SetKeyringPath(KeyringInfo *keyring_info,const char *path)
%
%  A description of each parameter follows:
%
%    o keyring_info: The keyring info.
%
%    o path: The keyring path.
%
*/
WizardExport void SetKeyringPath(KeyringInfo *keyring_info,const char *path)
{
  WizardAssert(KeymapDomain,keyring_info != (KeyringInfo *) NULL);
  WizardAssert(KeymapDomain,keyring_info->signature == WizardSignature);
  (void) CloneString(&keyring_info->path,path);
}
