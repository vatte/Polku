s.boot


//Arduino Communication:
SerialPort.listDevices

p = SerialPort("/dev/tty.usbmodem621", baudrate: 9600, crtscts: true);

(
r= Routine({
	var byte, str, res;
    inf.do{|i|
        if(p.read==10, {
            str = "";
            while({byte = p.read; byte !=13 }, {
                str= str++byte.asAscii;
            });
            res = str.asInteger;
			~updatedata.value(0, res);
        });
    };
}).play;
)

r.stop;
p.close;



//Scale-logic

(
~nScales = 1;
~nPrevValues = 50;
~iCurrVal = 0;
~allscales = [Array.fill(~nPrevValues, {0})];
~minScaleVal = 50;
~maxIdDiff = 20;
~nPrevIds = 6; //how many id's are stored
~iCurrId = 0;
~nIds = 6; // how many different ids are there
~allIds = Array.fill(~nPrevIds, {inf});
~idsAtScales = Array.fill(~nScales, {-1});

~colors = [ [120, 0, 0], [0, 120, 0], [0, 0, 120], [60, 60, 0], [60, 0, 60], [0, 60, 60] ];
)

(
~updatedata = { | index, value |
	var id = ~identify.value(index);
	~allscales[index][~iCurrVal] = value;
	~iCurrVal = (~iCurrVal + 1) % ~nPrevValues;

	if( ~idsAtScales[index] != id, {
		var color;
		if(id == -1,
			{ color = [0, 0, 0]; },
			{ color = ~colors[id]; }
		);
		id.postln;
		~idsAtScales[index] = id;
		p.putAll("I" ++ index ++ " " ++ color[0] ++ " " ++ color[1] ++ color[2]); //writes to arduino
	});

};
)


~identify.value(0)

(
~identify = { | index |
	var val = ~allscales[index].mean;
	if( (val > ~minScaleVal) && ((~allscales[index].maxItem - ~allscales[index].minItem) < (2 * ~maxIdDiff)),
		{
			//stabile value
			var minmatch = inf;
			var matching_i = -1;
			~nPrevIds.do( { | i |
				var diff = ~allIds[i].absdif(val);
				if((diff < minmatch) && (diff < ~maxIdDiff), {
					matching_i = i;
					minmatch = diff
				});
			});
			if(matching_i == -1, {
				matching_i = ~iCurrId;
				~iCurrId = (~iCurrId + 1) % ~nPrevIds;
			});
			~allIds[matching_i] = val;
			(matching_i % ~nIds).value;
		},
		{
		-1.value;
		}
	);
}
)











/******************************************************************************/
//testing---stuff


(
fork{
	40.do {|i|
		~identify.value(0).postln;
		1.wait;
	}
}
)






(
SynthDef(\sinosc, { |amp=0.1, freq = 440, bus = 0|
	var out;
	out = SinOsc.ar(freq);
	Out.ar(bus, out*amp);
}).add;
)


~allscales

"123 321".split($ )[1].asInteger;

s.sendMsg(\s_new, \sinosc, 1010);

s.sendMsg(\n_set, 1010, \freq, 300);

s.sendMsg(\n_free, 1010);





