/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

Info: ryan2.2.4.3.i386.source.zip
	This directory contains all of the source files necessary to compile the
	Ryan2 autopilot/brain for WinBolo/Linbolo.  Feel free to edit this code
	and use it to create your own brains, or simple improve this one.  I do
	ask that you change the brains name and site that it was created using
	the Ryan2 template.  If you simple want the brain file, you should download
	either ryan2autopilot.2.4.3.windows.i386.zip or
	ryan2autopilot.2.4.3.linux.i386.tar.gz

Compile:
	Windows: Ryan2 can be compiled on Windows by opening Ryan.dsw in Visual
	C++ 6 or above, then going to Build -> Rebuild All.  The brain will
	build to a file called Ryan2.brn and should end up in the Release
	directory, if it is not there, check in the Debug directory.  You can
	edit the install.bat file to make the brain install it's self after each
	compile.
	Linux: Ryan2 was origionally programmed on linux, and should run very
	well on it.  (Even though the WinBolo client is a little flaky, the
	brain seems to work fine.)  To install in linux simple cd into the source
	directory, and type make.  Fairly simple.  You can edit Makefile to
	change where ryan2 is installed, or you can simply copy ryan2.so to your
	brains directory.
