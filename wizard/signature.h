/*
  Copyright @ 1999 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.
  
  You may not use this file except in compliance with the License.
  obtain a copy of the License at
  
    https://imagemagick.org/script/license.php
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  ImageMagick digital signature methods.
*/
#ifndef _WIZARDSTOOLKIT_SIGNATURE_H
#define _WIZARDSTOOLKIT_SIGNATURE_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#define WizardSignatureSize  64

typedef struct _SignatureInfo
{
  size_t
    digest[8],
    low_order,
    high_order;

  ssize_t
    offset;

  unsigned char
    message[WizardSignatureSize];

  WizardBooleanType
    lsb_first;

  size_t
    signature;
} SignatureInfo;

extern WizardExport void
  FinalizeSignature(SignatureInfo *),
  GetSignatureInfo(SignatureInfo *),
  UpdateSignature(SignatureInfo *,const unsigned char *,const size_t);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
