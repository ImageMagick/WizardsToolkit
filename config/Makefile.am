#  Copyright 1999-2019 ImageMagick Studio LLC, a non-profit organization
#  dedicated to making software imaging solutions freely available.
#
#  You may not use this file except in compliance with the License.  You may
#  obtain a copy of the License at
#
#    http://www.wizards-toolkit.org/script/license.php
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#
#  Makefile for the Wizard's Toolkit configuration files.

# Where architecture-independent configuration files get installed
# (share/ImageMagick-version)
configsharedir = $(SHARE_PATH)
configshare_DATA = \
	config/configure.xml

# Where architecture-dependent configuration files get installed
# (share/arch/WizardsToolkit-version)
configsharearchdir = $(SHAREARCH_PATH)
configsharearch_DATA = \
  config/configure.xml 

# Where architecture-dependent configuration files get installed
# (lib/ImageMagick-version)
configlibdir =  $(CONFIGURE_PATH)
configlib_DATA = \
	config/log.xml \
	config/mime.xml

CONFIG_EXTRA_DIST = \
  config/configure.xml \
  config/config.h.in \
  config/log.xml \
  config/mime.c \
  config/mime.xml
