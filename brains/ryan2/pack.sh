#!/bin/sh



zip ryan2autopilot.2.4.3.source.zip *
cp Release/Ryan2.brn ./
zip ryan2autopilot.2.4.3.windows.zip Ryan2.brn ABOUT.txt CHANGELOG.txt README.txt
gtar cvzf ryan2autopilot.2.4.3.linux.tar.gz ryan2.so ABOUT.txt CHANGELOG.txt README.txt
