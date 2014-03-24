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



(
SynthDef(\help_PlayBuf, {| out = 2, bufnum = 0 |
    Out.ar(out,
        PlayBuf.ar(1, bufnum, BufRateScale.kr(bufnum), doneAction:2) ! 8
    )
}).play(s, [\out, 0, \bufnum, b]);
)