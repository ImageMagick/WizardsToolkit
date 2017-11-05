/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%               V   V  EEEEE  RRRR   SSSSS  IIIII   OOO   N   N               %
%               V   V  E      R   R  SS       I    O   O  NN  N               %
%               V   V  EEE    RRRR    SSS     I    O   O  N N N               %
%                V V   E      R R       SS    I    O   O  N  NN               %
%                 V    EEEEE  R  R   SSSSS  IIIII   OOO   N   N               %
%                                                                             %
%                                                                             %
%                        Wizard's Toolkit Version Methods                     %
%                                                                             %
%                              Software Design                                %
%                                  Cristy                                     %
%                                March 2002                                   %
%                                                                             %
%                                                                             %
%  Copyright 1999-2018 ImageMagick Studio LLC, a non-profit organization      %
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
#include "wizard/version.h"

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t W i z a r d C o p y r i g h t                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetWizardCopyright() returns the Wizard API copyright as a string.
%
%  The format of the GetWizardCopyright method is:
%
%      const char *GetWizardCopyright(void)
%
*/
WizardExport const char *GetWizardCopyright(void)
{
  return(WizardCopyright);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t W i z a r d V e r s i o n                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetWizardVersion() returns the Wizard API version as a string and
%  as a number.
%
%  The format of the GetWizardVersion method is:
%
%      const char *GetWizardVersion(size_t *version)
%
%  A description of each parameter follows:
%
%    o version: The Wizard version is returned as a number.
%
*/
WizardExport const char *GetWizardVersion(size_t *version)
{
  if (version != (size_t *) NULL)
    *version=WizardLibVersion;
  return(WizardVersion);
}
