..\MPC\mwc.pl -type make -value_template platforms=mingw ^
  -value_template configurations="Debug Release" ^
  -value_template delete=del ^
  -value_template compilerflags="-std=c++1y -Wall" %*
