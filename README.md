  hrt-test
=============

Overview
-------

A linux module to find the practical resolution of the Linux hrtimers on OMAP3
Gumstix board.

The test requires an oscope to do the measurements. 

I used a convenient pin on the Gumstix Overo Tobi expansion header, pin 19.
This pin shouldn't require any mux'ing on a default kernel.


Build
-------

There is a file you can source to set up the environment for building using
the Yocto built tools configured.

    $ git clone git://github.com/scottellis/hrt-test.git
    $ cd hrt-test
    $ export OETMP=<TMPDIR-from-your-local.conf>
    $ source yocto-env.sh
    $ make
 

Run
-------
 
Copy the hrt.ko module to the board, insert it.

    root@overo:~# insmod hrt.ko

To run a test, write the micro second delay you want between pin toggles.
For example, this will run a 50ms test.

    root@overo:~# echo 50000 > /dev/hrt


Set your scope to trigger on a rising signal.


Results
-------

The results came from a 2.6.36 kernel and an Overo Tide COM running at 720 MHz.

From a previous experiment (github.com/scottellis/irqlat) I know that the
gpio_set_value() takes about 150 nanoseconds and can be ignored here.

For the tests run with delays of 200 microseconds and above, I set the oscope 
scale to the same as the measurement. For the 100 and 50 microsecond tests, 
I left the scale at 200 microseconds since it didn't matter.

hrt-50ms-test.png came from

	root@overo:~# echo 50000 > /dev/hrt

hrt-5ms-test.png came from

	root@overo:~# echo 5000 > /dev/hrt

hrt-1ms-test.png came from

	root@overo:~# echo 1000 > /dev/hrt

hrt-500us-test.png came from

	root@overo:~# echo 500 > /dev/hrt

hrt-200us-test.png came from

	root@overo:~# echo 200 > /dev/hrt

hrt-100us-test.png came from

	root@overo:~# echo 100 > /dev/hrt

hrt-50us-test.png came from

	root@overo:~# echo 50 > /dev/hrt


I'd be happy to hear if I did something wrong. 

The motivation for this was a failed attempt to use an hrtimer in a driver 
wanting accuracies in the 20 usecond range. That didn't work out.

See the source code for more details.

