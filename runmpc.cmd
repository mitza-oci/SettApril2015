..\MPC\mwc.pl -type make -value_template platforms=mingw ^
  -value_template configurations="Debug Release" -value_template obj_dir=obj/ ^
  -value_template touch="compat\touch.pl" -value_template delete=del ^
  -features boost=1 -value_template compilerflags="-std=c++1y -Wall" %*
