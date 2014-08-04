# bitmap-playground

---

Some standard C code to play around with bitmaps, I'm just doing this for fun and to learn a bit **OpenCL**.

Currently it supports only reading *JPG* and *PNG* files, and saving *JPG*, *PNG* and *TGA*. Maybe I'll add support for other formats later.

All the filtering operations are done in **RGBA** colorspace, however saving can be done in simple **RGB**

It works on OS X and should work fine on Linux but I didn't try.


# Building

---

You will need *libpng*, *libjpeg* and *OpenCL*, then execute the script corresponding to your OS.

## OS X

Run `build-osx.sh`


## Linux

Run `build-linux.sh`


# License

---

[WTFPL](http://www.wtfpl.net/about/ "WTFPL"), see the COPYING file.
