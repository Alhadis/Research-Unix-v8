.TL
Blit Owner Information
.AU
Bart Locanthi
.PP
Congratulations!  You have chosen the finest in terminal technology.
With a little care, your new Blit should provide years of service
and enjoyment to you and your family.  We want you to be able
to make the best of your Blit.  However, reading this little blurb
can only get you started.
.ft 3
The Blit Programmer's Manual
.ft 1
is must reading for the serious Blit programmer.  The various user
programs (mpx, jim, jx, etc.) are covered by the copious on-line
documentation.
.SH
Setting up your Blit
.PP
First, you need to find space.  The monitor enclosure is large,
the keyboard is wide, and the mouse needs a little area for running
around.  You will need a clear area about 30" square for your
jerqstation.  This area must be near an electrical outlet
and an RS-232 terminal line.
.PP
The connectors in the back of the monitor enclosure are labeled and
keyed, so there should be no confusion about plugging them in.  In
order to work, your Blit
.B must
be connected to power, the keyboard
.B must
be plugged in, and the RS232 port
.B must
be connected to a running computer.  The only thing that doesn't
need to be plugged in to get started is the mouse.
.PP
Your Blit should power up successfully and
display a rectangular cursor and possibly a login message.
If the screen displays a set of ragged horizontal stripes,
you probably have an old (pre-TTY) Blit which needs DCD to
run the internal processor, but DCD is not high; contact your computing services
representative with your problem.
.PP
If the login message is garbled, you will have to set the baud rate.
This is done by removing the back of the monitor enclosure (with the
power OFF!)  and setting
the DIP switch on the logic board.  The switch settings are shown below.
.PS < dipsw.pic
.DS C
Figure 1.  DIP switch settings
.DE
.PP
There is a small pushbutton behind the left side of the keyboard.
When pressed, this
"boot" switch will reset your terminal and/or hang up the phone,
depending on the settings on the DIP switch.  This is necessary when
the terminal crashes or when the host-terminal communication gets
confused.  Normally the boot switch will be set for
.B
local reset
.R
only.  If you are afforded the luxury of a hard-wired line to your host you
may wish to set it also for
.B hangup.
If both settings are selected,
you can flick the boot switch to reset just the terminal or you can
hold it down to hang up your host connection.
.PP
If you are running the ROM terminal program (the one that runs at
power-up), the key PF4 will toggle DTR to the computer.
This is useful if you must connect to a commercial data switch.
.SH
The Mouse
.PP
The standard pointing device for Blits is a round palm-sized object
called a
.B mouse.
A mouse is an
.B incremental
pointing device.  That is, it detects changes in position when you
slide it across a surface.  Because it detects incremental motion, you can pick
it up and roll it several times to move a large distance.  An area
the size of a normal piece of paper is more than enough space for
a mouse.
.PP
Those programs that make use of the mouse will display a cursor that
tracks the mouse position.  A text editor may display two cursors,
one to indicate typing position in the text, the other tracking the mouse
position.
The cursor shape may be anything
from an arrow to a little square or a meditating Buddha, depending on
the needs of the program.
Programs using the mouse will also make use of the three mouse pushbuttons
to indicate selection or actions to be performed on the selected object.
The mouse button functions, like the cursor shape, depend on the program.
.PP
What the user should remember about the mouse is that its pushbuttons
are not like those of a keyboard.  Pressing and releasing a mouse button
are two distinct events, and many programs will make use of that distinction.
.B Menus
are an example of this.  Pressing a menu button will cause
a menu to appear.  The menu selection is
.B made,
however, when the button is released.  Between these two events the
possible menu selection will track the cursor.  Each program will have
its own way of dealing with the mouse.  Read the documentation for
a program if you have any questions about how it uses the mouse.
.SH
Altered States
.PP
Your Blit is different from ordinary terminals in that the program running
inside is loadable.  This means that
.in +5
.br
\(bu  your Blit is more fun than a barrel full of 2621's.
.br
\(bu  your Blit can crash.
.br
\(bu  your Blit may not always know how to act like a terminal.
.br
\(bu  your Blit may act like a terminal to a program on Unix that
doesn't want to talk to a terminal.
.br
.in -5
The middle two points are easy to deal with by re-booting your Blit.  Dealing
with the last case may demand hanging up from Unix.  An example of this case
is the troff proofing program
.B proof.
Before sending troff output to the screen,
.B proof
first loads a program to simulate some of the functions of a typesetter.
If for some reason your Blit reverts to being a terminal without having
been told to do so by
.B proof,
you have lost control and must hang up in order to get it back.
.PP
Upon being reset (this includes power-on) your Blit starts running a
terminal program that makes it look like an ordinary terminal with a
large screen.   Setting TERM=blit will satisfy most Unix programs that
care about what kind of terminal they are talking to.
.PP
The curious may enjoy looking in /usr/jerq/lib/*term and seeing what
other, mostly dull, terminal programs there are to play with.
Other interesting demonstration programs can be found in /usr/jerq/demo.
.PP
A final caveat.  Loading a Blit program is currently possible
.B only
on a VAX or a 3B, and
.B only
when your Blit is connected either by a direct hardwired or phone line or TDK
(and not, for example, through \f2cu\f1 or \f2dcon\f1).
