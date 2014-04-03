s.boot

(
SynthDef(\sinosc, { |amp=0.1, freq = 440, bus = 0|
	var out;
	out = SinOsc.ar(freq);
	Out.ar(bus, out*amp);
}).add;
)



~audioBuses = [2, 3, 4, 5, 6, 7];


s.sendMsg(\s_new, \sinosc, 1010);

s.sendMsg(\n_set, 1010, \bus, 0);



b = Buffer.read(s, "/Users/polku/Sounds/tokyosoundsdraft_2.wav");

~amp = 0.5 ! 8;
a = 0.5 ! 8;

(
SynthDef(\help_PlayBuf, {| out = 2, bufnum = 0, amp = #[1, 1, 1, 1, 1, 1, 1, 1] |
	var sig, mouseX;
	sig = PlayBuf.ar(1, bufnum, BufRateScale.kr(bufnum), doneAction:2);
    Out.ar(out,
		[amp[0]*sig, amp[1]*sig, amp[2]*sig, amp[3]*sig, amp[4]*sig, amp[5]*sig, amp[6]*sig, amp[7]*sig];
    )
}).add;
)
.play(s, [\out, 0, \bufnum, b])

s.sendMsg(\s_new, \help_PlayBuf, 1011);

s.sendMsg(\n_set, 1011, \bufnum, b);



c = Bus.control;
{Out.kr(c, MouseX.kr(0, 7))}.play // let this running
c.getSynchronous; // get value here

a

(
{
	9999.do( {
		~mouseX = c.getSynchronous;
		a = [0, 0, 0, 0, 0, 0, 0, 0];
		8.do { | i |
			if(abs(i - ~mouseX) < 1,
				{ a[i] = 1 - abs(i-~mouseX); });

		};
		s.sendMsg(\n_set, 1011, \amp, a);

		0.01.wait;
	});
}.fork;
)

~amp