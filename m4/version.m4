#  Copyright 1999-2021 ImageMagick Studio LLC, a non-profit organization
#  dedicated to making software imaging solutions freely available.
#
#  You may not use this file except in compliance with the License.  You may
#  obtain a copy of the License at
#
#    https://imagemagick.org/script/license.php
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#
#  ImageMagick versioning
#
m4_define([wizard_name], [WizardsToolkit])
m4_define([wizard_major_version], [1])
m4_define([wizard_minor_version], [0])
m4_define([wizard_micro_version], [9])
m4_define([wizard_patchlevel_version], [5])
m4_define([wizard_base_version],
          [wizard_major_version.wizard_minor_version.wizard_micro_version])
m4_define([wizard_version],
          [wizard_base_version-wizard_patchlevel_version])
m4_define([wizard_bugreport], 
          [https://github.com/ImageMagick/WizardsToolkit/issues])
m4_define([wizard_url], [https://urban-warrior.org])
m4_define([wizard_lib_version],[0x109])
m4_define([wizard_lib_version_number],
          [wizard_major_version,wizard_minor_version,wizard_micro_version,wizard_patchlevel_version])
m4_define([wizard_git_revision],
          esyscmd([
            c=$(git log --full-history --format=tformat:. HEAD | wc -l)
            h=$(git rev-parse --short HEAD)
            d=$(date +%Y%m%d)
            printf %s "$c:$h:$d"
          ]))
m4_define([wizard_tarname],[WizardsToolkit])

