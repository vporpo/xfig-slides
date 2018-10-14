About
-----
Xfig with the additional functionality of designing and generating animated slides.
This is particularly useful for easily animating a static .fig figure as part of a beamer (latex) presentation.

Tutorial
--------
Please visit http://vporpo.me/xfig_slides/index.html for a more detailed tutorial on how to use the slides feature.

Quick Installation Guide for Xfig-slides
----------------------------------------
The process is similar to the one described in the INSTALL file.
For enabling slides, you just need to make sure you are using the `--enable-slides` option when configuring.
For a detailed description please read the INSTALL file.

~~~
$ ./configure --enable-slides --with-appdefaultdir=/etc/X11/app-defaults/
$ make
$ sudo make install
~~~
Note: If you skip "--with-appdefaultdir=/etc/X11/app-defaults/", make install will install the key bindings file "Fig" in the default /usr/share/X11/app-defaults/ directory instead, which does not seem to work on my system.


For a debug build:
~~~
$ CFLAGS="-O0 -g3" ./configure --enable-slides --with-appdefaultdir=/etc/X11/app-defaults/
$ make
~~~


Versions
--------
The current version of Xfig-slides is based on xfig-3.2.6a
