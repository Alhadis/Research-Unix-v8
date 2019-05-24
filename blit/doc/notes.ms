.EQ
delim $$
.EN
.TL
Blit Notes
.AU
Brian W. Kernighan
.AI
...\*(DY
June 7, 1982
.PP
The Blit is a bitmap computer terminal
with hardware and software designed by Bart Locanthi
and Rob Pike.
These notes have been thrown together hastily
as background for discussions about Blits.\(dg
.FS
\(dg This is neither an offer to sell nor a solicitation
of an offer to buy.
The offering of Blits is made only by the Prospectus
and only in those States in which Blits may legally be offered.
The Prospectus contains more information
including all sales charges and expenses.
Read it carefully before you invest or send money.
.FE
.NH
Hardware 
.PP
A Blit contains
a Motorola 68000 processor,
256K bytes of memory, an 8\(mu10 screen with
800 \(mu 1024 points
(i.e., 100 dots/inch horizontally and vertically),
a keyboard,
a mouse, and an RS-232 communications interface.
The electronics fit on one board.
.PS
A: box invis "host computer" "running Unix"
line
R: box invis "RS-232" "interface"
line
C: box  "68000" "cpu"
line <->
box "memory" "256Kb"
line <->
box ht 1.7*boxht "screen"
box invis "keyboard" with .s at R.n
	line from last box.e to 1/2 <C.w,C.nw>
box invis "mouse" with .n at R.s
	line from last box.e to 1/2 <C.w,C.sw>
.PE
.PP
The memory is ``dual-ported'':
both the CPU and the screen access it.
The screen has 800K dots;
any contiguous 100K bytes of memory
may be displayed on the screen
by setting a single register to the address of the first byte.
.PP
The hardware is designed to be ``neutral'' \(em
it is not specialized to any particular task,
nor does the terminal have hardware features to make
certain tasks easier or faster.
For instance, there are no graphics assists
to copy a block of bits from one place to another
or to draw lines.
There is no character generator,
nor is there hardware to 
display the cursor.
All of these are done by software when needed.
.PP
The Blit is cheap:
it is available (handmade, in small quantities)
from Teletype Corp. to Bell Labs customers
for $dollar 3500$.
.NH
Software
.PP
Software for the Blit is mostly written in C and compiled on
a Unix host system.
The ``object'' program is loaded into Blit memory
down the communications line,
typically at 19.2K baud, but for home terminals at 1200 baud.
Control is turned over to the program after it is loaded.
.PP
Once loaded, the program can talk to the peripheral devices (keyboard,
mouse, communications line) and draw on the screen.
The picture on the screen is determined simply
by what is stored in the 100K region of memory
that is currently being mapped onto the screen.
.PP
The most commonly used program causes the Blit
to simulate a standard dumb terminal.
It copies whatever characters are typed on the keyboard
onto the communications line,
and copies characters from the line onto the screen.
The screen is scrolled as necessary so that new lines
appear at the bottom and old ones disappear off the top,
but this scrolling is done by moving bytes
within the memory
\(em there is no hardware scrolling.
(The ``dumb terminal'' program is so frequently used that, by default,
it is executed
from a small read-only memory whenever the Blit is turned on or reset.)
.PP
When the Blit is pretending to be a dumb terminal,
the terminal program talks to whatever Unix program
is currently running in the host computer \(em
the shell, an editor, mail, or whatever.
None of these programs knows or cares that
there is a Blit at the other end.
.PP
Of course, any program can be written and loaded,
so the Blit can present any desired face to its users:
it is limited mainly by the ingenuity of the programs written for it.
The next sections describe some of the software currently in use
on the Blit,
roughly in increasing order of novelty, intrinsic interest or complexity.
It should be noted that each of these ``programs''
is really two cooperating programs,
one that runs in the terminal and one that runs on Unix,
exchanging information via the communications line.
.SH
Graphics output
.PP
100 dots per inch is enough resolution
that the Blit can do reasonable graphics output.
There are a couple of programs
that simulate a typesetter
for proofreading of output
before it is printed.
Roman, italic and bold fonts
can be distinguished at normal type sizes,
but the quality isn't all that high.
.PP
There are programs that display integrated circuit layouts,
with windowing and zooming.
Since the Blit doesn't provide color,
patterns and textures are used to identify different layers.
There has been talk of putting the TTL circuit design package
on the Blit, but no action.
.PP
Conventional graphics \(em curves of $y = f(x)$,
rotation of three-dimensional surfaces, and the like \(em
have been tried,
but are not heavily used.
.SH
Graphics input
.PP
The mouse provides a nice way to point at any arbitrary point on the screen,
and makes possible interactions that aren't feasible with just
a keyboard.
(To answer one potential question:
a touch screen doesn't have as much resolution as the mouse,
nor does it provide the function buttons
that are used to say what to do with
the indicated point.)
.PP
The first graphics input programs were games
like mazewar, asteroids and pool,
but since this is serious business, we pass quickly on to ...
.PP
Sally Browning has written a picture editor called Cip.
A picture is
created with the mouse, from a menu of shapes
(boxes, lines, circles, ellipses, arcs, splines)
positioned and scaled to taste.
Text can be mixed with the graphic objects;
it is displayed in the proper size and font
as it is being typed in.
Parts of the figure can be replicated,
and any part can be revised.
When the picture is ``finished'',
it can be saved in a file in the host,
then used as input to the Pic preprocessor
to be typeset as part of some larger document.
.PP
There is an early version of a text editor
(mostly from Rob Pike)
that makes heavy use of the mouse
to identify the regions of text
to be edited.
The style of use of this editor
is very different from normal editors,
even so-called ``visual'' editors,
since almost all commands involve pushes
of the buttons on the mouse rather than keystrokes.
.PP
Danny Sleator has an electronic blackboard
that permits pictures drawn on one Blit to be
printed on another.
.SH
Layers
.PP
This brings us to what is probably
the most interesting and novel part of the Blit software,
the use of ``layers''.
.PP
With suitable software,
the physical screen of the Blit
can become several logical screens at once.
These logical screens do not have to be separate \(em
they can overlap, with one partially or completely obscuring another.
.PP
Layers are independent of each other.
Each is controlled by a separate, independent process,
operating concurrently.
Each is logically a complete terminal (a Blitlet?)
with all the capabilities of the original,
except for being smaller and perhaps somewhat slower.
For each layer program, there is a corresponding host program.
.PP
Layers are managed by software in the Blit,
but the programs running in separate layers
do not interact with each other,
nor do they have to be concerned
with the fact that they are running in a layer.
They can believe that they have the entire screen
at their disposal.
.PP
A controlling program in the Blit
manages the layers.
It is responsible for collecting keyboard characters
and giving them to the proper program,
for assigning the mouse to the proper program,
for keeping the contents and visibility of each layer correct,
and for managing communications with the host
for 
.I each
layer program.
This controlling program is called
.I mpx ,
since it multiplexes a variety of resources.
.PS
define b / box ht .15 wid .15 $1 /
U: box "mpx"
line
C: line
line
B: box "mpx"
line dashed up from C.c
line dashed down from C.c
box invis "Unix" with .s at U.n
box invis "Blit" with .s at B.n
line left from U.w; b("3") with .e at last line.end
	box invis "Unix" "programs" with .e at last box.w
line left up .2 from U.w; b("2") with .e at last line.end
line left up .4 from U.w; b("1") with .se at last line.end
line left down .2 from U.w; b("4") with .e at last line.end
line left down .4 from U.w; b("5") with .ne at last line.end

line right from B.e; b("3") with .w at last line.end
	box invis "Blit" "layer" "programs" with .w at last box.e
line right up .2 from B.e; b("2") with .w at last line.end
line right up .4 from B.e; b("1") with .sw at last line.end
line right down .2 from B.e; b("4") with .w at last line.end
line right down .4 from B.e; b("5") with .nw at last line.end

[ line down; box ht 1.5*boxht wid boxht "screen"] with .n at B.s
line down .2 left .2 from B.sw
box invis "mouse,  " "keyboard  " with .n at end of last line
.PE
.PP
As is the case with many Blit programs,
mpx
comes in two parts \(em
one that runs in the Blit
and one that runs on the host Unix system.
Each half of mpx deals with multiple processes in its own environment,
and bundles their communications into a single stream
on the communications line.
The mpx on the other end
does the demultiplexing.
.PP
Mpx provides the user with the ability to create
a layer of any size at any position,
by pointing with the mouse.
(Pushing a button on the mouse displays a self-explanatory menu.)
When a layer is created,
a copy of the standard dumb-terminal program
is associated with it on the Blit end,
and a separate Unix shell (command interpreter)
is associated with it on the Unix end.
Thus each layer begins life as a separate dumb terminal
running as an independent Unix program,
just as if the user had logged in on separate terminals.
.PP
At this point,
the user can simply run normal Unix programs,
or
he can load some other program
to run in the layer,
such as asteroids or the Blit editor.
.PP
In either case, each layer is kept up to date by mpx,
regardless of whether it is currently visible,
overlapped, or totally hidden.
The user can switch his attention to any layer
and bring it to the top
(i.e., make it completely visible)
by pointing the mouse at it and pushing a button.
.PP
The notion that all layers are always active,
regardless of their visibility,
is believed to be unique to the Blit.
It turns out to be extremely convenient in practice,
since it means that a user can
run independent processes and keep an eye on their progress
without actually having to poll them periodically.
One typical use
makes use of the fact that our Unix machines are connected
with a Datakit network.
One can work on one machine,
but create a small window that is logged in on another machine
where mail is announced as it arrives.
When interesting mail arrives,
the user can enlarge the window and read it.
.PP
Most of the applications of layers so far
have focused on program development.
The normal mode of operation is to have
one window for compilation and printing error messages,
another for editing the source text,
a third for running the program and a fourth for the debugger.
Other windows are created and removed as needed
for operations like sending mail,
examining some related program,
and so on.
.PP
As we have gained experience using layers,
we have seen changes in the way
programs are used and thus in the way they will be written
in the future.
For example,
many existing Unix programs have ``escape'' commands
that permit temporary exit from the program
to do something else;
the ``!'' command in the editor and mail programs is one instance.
With layers,
no such commands are needed.
One simply stops typing (even in the middle of a word) in the current layer,
creates a new layer,
does whatever is needed,
then returns to the original one.
.PP
There are a number of interesting technical issues
related to layers \(em
storage management,
process control,
division of labor between Blit and host,
etc. \(em
that might be discussed.
The question of how a job should be divided
between host and terminal is probably the most intriguing
of these.
The bandwidth between the two is very narrow,
and the capabilities are quite different.
.PP
Bart Locanthi's program 
to display an integrated circuit layout on the Blit
is a good example.
When a user requests that part of a circuit be displayed,
that request is sent to the host.
The host figures out what parts of the circuit
will be visible
and what parts are repetitive,
and sends a compactly-encoded representation of
the shapes and patterns to the terminal.
The terminal creates a bitmap for display.
Subsequent requests are handled the same way,
but each time a bitmap is computed it is remembered
(so long as space permits),
so that if a request involves redisplay of anything
already printed,
that can be done locally (in the terminal)
by printing existing bitmaps \(em
the host is not involved.
.NH
Nasty questions
.PP
This section attempts to answer some of the questions
that are often asked about the Blit.
.LP
.I
The Xerox Alto did this 10 years ago.
The Star, Perq, Apollo, Bitgraph and SUN terminals all do it today.
What's new?
.R
.PP
The Blit owes a considerable debt to the Alto,
which demonstrated the utility of bitmap displays,
and which invented the fundamental operations
for moving blocks of bits.
With the possible exception of the BBN Bitgraph,
however,
these are all personal computer systems,
not terminals
.I
per se.
.R
The Blit provides,
for relatively little money,
a terminal that greatly enhances the capabilities of
our existing Unix environments,
without forcing us to use small machines,
and without encouraging the fragmentation and isolation
that seems to go together with personal computers
even when connected by networks.
It seems better to add to a good environment than to recreate it.
.LP
.I
Don't Smalltalk and the SUN have windows already?
.R
.PP
The asynchronous layers in the Blit are new.
In Smalltalk, for instance,
only the window at the front is active;
no other window is.
The windows are not even independent;
each is a subroutine in a single large program.
.LP
.I
Why not a MAC-32?
.R
.PP
A trivial answer:
The Motorola 68000 has been available for several years;
the MAC-32 is still not available.
A technical answer:
the architecture of the 68000 is better suited to the job.
For example,
the MAC-32 draws distinctions between register operations
and storage operations while the 68000 minimizes
the differences.
Since the Blit screen is just memory,
any distinction leads to more clumsy and therefore slower code.
We estimate that the MAC-32 would be about one half as fast
as the 68000 for critical operations like moving
a block of bits.
This would be unacceptable.
.LP
.I
Why isn't there a disk? (Etc.)
.R
.PP
The Blit was meant to be cheap enough that everyone
could have two (home and office).
A disk would about double the price.
A more serious concern is again
that local storage encourages
people to squirrel away private copies
of software that then have to be kept up to date.
This is already a terrible problem
even on small networks of bigger machines \(em
it would be far worse if everyone had a private machine.
.NH
Caveat Emptor
.PP
There are some limitations to keep in mind \(em
the Blit is not a panacea for anything.
.IP (1)
The 68000 is not a big, fast processor,
and complicated graphics operations can slow it down noticeably.
Furthermore, the memory is shared between the processor
and the display,
so the effective speed is only about three quarters of
the rated speed of the CPU.
.IP
The machine has no memory protection hardware either,
so undebugged programs can clobber their neighbors.
(This isn't a grave problem,
since the terminal user represents all the neighbors.)
.IP (2)
256K bytes is not a huge amount of memory.
This already limits the number of layers, or at least
their total area.
In practice, there are about 1.5 screenfuls worth of off-screen layer space.
.IP
As more complicated programs are written,
they will also increase the pressure on memory.
The 256K chips will help here.
.IP (3)
The communications link to the host is 
.I very
narrow,
especially when run at 1200 baud
for home Blits.
It can take several minutes to load a big program like Cip.
On the other hand, programs that can be factored,
like the integrated circuit plotter and the editor,
run adequately over phone lines.
.IP (4)
Raster devices are inherently poor at some operations,
such as scaling and rotating,
which leads to slower and more complicated software.
.NH
Philosophical stuff
.PP
The Blit hardware was designed by the people
who were going to write the software for it and who
were going to use it.
It has usually been a mistake to have hardware designed
by people who will not be involved with the software,
and it is usually a mistake to have software designed
by people who will not have to use it.
.PP
The hardware was designed to make it easy to put
any kind of software on the terminal.
There was no attempt made to produce ``smart'' hardware,
but rather to produce ``neutral'' hardware,
so that it is possible to make the terminal smart
in any desired way by writing appropriate software,
which can be loaded dynamically when needed.
.PP
As with Unix,
the number of truly new ideas in the Blit is small.
It is more a judicious selection of good old ideas,
adapted to the local environment.
In the same way that the Unix file system
conceals the strange properties of disks and storage management
to present the illusion of contiguous arrays of bytes,
the Blit layer operations conceal issues
of visibility, scaling and timing to present the programmer
with what appears to be a continuously visible screen
for each process.
