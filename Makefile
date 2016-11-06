all:
	g++ luaspy.cpp -shared -o gmcl_luaspy_win32.dll -I D:\gmbase\lua -lpsapi