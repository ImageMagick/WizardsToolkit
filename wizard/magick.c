/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                  M   M   AAA    GGGG  IIIII   CCCC  K   K                   %
%                  MM MM  A   A  G        I    C      K  K                    %
%                  M M M  AAAAA  G GGG    I    C      KKK                     %
%                  M   M  A   A  G   G    I    C      K  K                    %
%                  M   M  A   A   GGGG  IIIII   CCCC  K   K                   %
%                                                                             %
%                         Wizard's Toolkit Magick Methods                     %
%                                                                             %
%                             Software Design                                 %
%                                  Cristy                                     %
%                               March 2003                                    %
%                                                                             %
%                                                                             %
%  Copyright 1999-2014 ImageMagick Studio LLC, a non-profit organization      %
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
*/

/*
  Include declarations.
*/
#include "wizard/studio.h"
#include "wizard/hash.h"
#include "wizard/magick.h"
#include "wizard/string_.h"
/*
  Global declaractions.
*/
unsigned char
  MagickWizard[4] =
  {
    (unsigned char) 0x45,
		(unsigned char) 0x45,
    (unsigned char) 0x53,
    (unsigned char) 0x7f
  },  /* EES\177 */
  WizardMagick[4] =
	{
	  (unsigned char) 0x7f,
		(unsigned char) 0x53,
		(unsigned char) 0x45,
		(unsigned char) 0x45
  };  /* \177SEE */

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t W i z a r d M a g i c k                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetWizardMagick() produces a unique signature useful for verifying that a
%  structure or file is in fact Wizard related.
%
%  The format of the GetWizardMagick method is:
%
%      StringInfo GetWizardMagick(const char *signature,const size_t length)
%
%  A description of each parameter follows:
%
%    o signature: Derive the magick from this signature.
%
%
*/
WizardExport StringInfo *GetWizardMagick(const unsigned char *signature,
  const size_t length)
{
  HashInfo
    *hash_info;

  StringInfo
    *magick,
    *tag;

  hash_info=AcquireHashInfo(SHA2256Hash);
  InitializeHash(hash_info);
  tag=AcquireStringInfo(length);
  SetStringInfoDatum(tag,signature);
  UpdateHash(hash_info,tag);
  tag=DestroyStringInfo(tag);
  FinalizeHash(hash_info);
  magick=CloneStringInfo(GetHashDigest(hash_info));
  hash_info=DestroyHashInfo(hash_info);
  return(magick);
}
