/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%          SSSSS   AAA   N   N  IIIII  TTTTT  IIIII  ZZZZZ  EEEEE             %
%          SS     A   A  NN  N    I      T      I       ZZ  E                 %
%           SSS   AAAAA  N N N    I      T      I     ZZZ   EEE               %
%             SS  A   A  N  NN    I      T      I    ZZ     E                 %
%          SSSSS  A   A  N   N  IIIII    T    IIIII  ZZZZZ  EEEEE             %
%                                                                             %
%                                                                             %
%              Wizard's Toolkit Environment Sanitazion Methods                %
%                                                                             %
%                             Software Design                                 %
%                                 Cristy                                      %
%                               March 2003                                    %
%                                                                             %
%                                                                             %
%  Copyright 1999-2015 ImageMagick Studio LLC, a non-profit organization      %
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
#include "wizard/sanitize.h"

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S a n i t i z e E n v i r o n m e n t                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SanitizeEnvironment sanitizes the environment to help ensure the Wizard's
%  Toolkit operates in a secure manner.
%
%  The format of the SanitizeEnvironment method is:
%
%      void SanitizeEnvironment(void)
%
*/
WizardExport void SanitizeEnvironment(void)
{
#if defined(SANS)
  {
    struct rlimit
      limit;

    /*
      Prevent core dumps.
    */
    limit.rlim_cur=0;
    limit.rlim_max=0;
    setrlimit(RLIMIT_CORE,&limit);
  }
#endif
}
