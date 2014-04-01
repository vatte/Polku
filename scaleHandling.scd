
r.stop;
p.close;

~allscales[1][(~iCurrVal+1)%~nPrevValues] - ~allscales[1][~iCurrVal]

~allscales

//Scale-logic
(

//VARIABLES
~nScales = 6;
~nPrevValues = 10;
~minScaleVal = 150;
~maxIdDiff = 20;
~maxValDiff = 5;
~nPrevIds = 6; //how many id's are stored
~nIds = 6; // how many different ids are there
~colors = [ [120, 0, 0], [0, 120, 0], [0, 0, 120], [60, 60, 0], [60, 0, 60], [0, 60, 60] ];


~iCurrVal = 0;
~iCurrId = 0;
~allscales = Array.fill(~nPrevValues, {0}) ! ~nScales;
~calibratedScales = Array.fill(~nPrevValues, {0}) ! ~nScales;
~allIds = Array.fill(~nPrevIds, {inf});
~idsAtScales = Array.fill(~nScales, {-1});

~minScales = 0 ! ~nScales;
~calibrationScales = 1 ! ~nScales;



//FUNCTIONS

~colorToScale = { | index, color |
	var str;
	str = "I" ++ " " ++ index ++ " " ++ color[0] ++ " " ++ color[1] ++ " " ++ color[2];
	p.putAll(str); //writes to arduino
};

~stableValue = { | index |
	var stable = false;
	(~nPrevValues - 3).do { | i |
		var d1, d2;
		d1 = ~allscales[index][(~iCurrVal + i + 1)%~nPrevValues] - ~allscales[index][(~iCurrVal + i + 2)%~nPrevValues];
		d2 = ~allscales[index][(~iCurrVal + i + 2)%~nPrevValues] - ~allscales[index][(~iCurrVal + i + 3)%~nPrevValues];
		if(((d1 < 0) != (d2 < 0)) || d1 == 0 || d2 == 0, {
			stable = true;
		});
	};
	//stable.postln;
	stable.value;
	//abs(~allscales[index][(~iCurrVal+1)%~nPrevValues] - ~allscales[index][~iCurrVal]) < ~maxValDiff;
};


~updatedata = { | index, value |
	if(value != nil, {
		~calibratedScales[index][~iCurrVal] = (value - ~minScales[index]) / (~calibrationScales[index] - ~minScales[index]);
		~allscales[index][~iCurrVal] = value;
	});

	/*
	var id = ~identify.value(index);
	~allscales[index][~iCurrVal] = value;

	if( ~idsAtScales[index] != id, {
		var color;
		if(id == -1,
			{ color = [0, 0, 0]; },
			{ color = ~colors[id]; }
		);
		~idsAtScales[index].postln;
		~allIds[~idsAtScales[index]].postln;

		id.postln;
		~allIds[id].postln;
		~idsAtScales[index] = id;

		~colorToScale.value(index, color);
	});*/

};



~identify = { | index |
	var val = ~allscales[index].mean;
	//if(index == 0, {val.postln;});
	if( (val > ~minScaleVal) && ((~allscales[index].maxItem - ~allscales[index].minItem) < ~maxValDiff),
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



//Arduino communication
//p = SerialPort("/dev/tty.usbmodem621", baudrate: 9600, crtscts: true);
//p = SerialPort("/dev/tty.usbmodem441", baudrate: 9600, crtscts: true);
p = SerialPort("/dev/tty.usbmodemfa131", baudrate: 57600, crtscts: true);

//A routine for handling the arduino input data
(
r= Routine({
	var byte, str, res;
    inf.do{|i|
        if(p.read==10, {
            str = "";
            while({byte = p.read; byte !=13 }, {
                str= str++byte.asAscii;
            });
			res = str.split($,).asInteger;
		    //res.postln;
			~iCurrVal = (~iCurrVal + 1) % ~nPrevValues;
			~nScales.do{ | i |
				~updatedata.value(i, res[i]);

			};
        });
    };
}).play;
)











//Scale calibration

~calibrateScales.play;
~calibrateScales.stop;
~calibrateScales.reset;

(
~calibrateScales = Routine({
	~nScales.do { | i |
		var calibrated = false;
		i.postln;
		~minScales[i] = ~allscales[i].mean;
		~colorToScale.value(i, [255, 0, 0]);
		while({ calibrated.not }, {
			if(~allscales[i].mean > ~minScaleVal && ~stableValue.value(i), {
				i.postln;
				~calibrationScales[i] = ~allscales[i].mean;
				~colorToScale.value(i, [0, 0, 0]);
				calibrated = true;
			});
			0.01.wait;
		});
		0.1.wait;
	};
	0.5.wait;
	5.do {
		8.do { |i|
			~colorToScale.value(i, [0, 0, 255]);
			//0.05.wait;
		};
		0.5.wait;
		8.do { |i|
			~colorToScale.value(i, [0, 0, 0]);
			//0.05.wait;
		};
		0.5.wait;
	};
});
)









/******************************************************************************/
//testing---stuff

SerialPort.listDevices



(
{
8.do { |i|
	~colorToScale.value(i, [0, 0, 0]);
	0.1.wait;
}
}.fork;
)


(
{
	100.do {
		~nScales.do { |i|
			~calibratedScales[i].mean.post;
			" ".post;
		};
		"".postln;
		0.5.wait;
	};
}.fork;
)


~colorToScale.value(3, [0, 0, 0]);
~colorToScale.value(3, [0, 255, 0]);


(
fork{
	40.do {|i|
		~identify.value(0).postln;
		1.wait;
	}
}
)



(
d=Routine ( {
	10.do { | i |
		i.postln;
	}
});
)
d.play;
d.reset;

(
f = { |argument = 0|
{
8.do { |i|
	(i*argument).postln;
	0.2.wait;
}
}.fork;
};
)

f.value(5);


(
SynthDef(\sinosc, { |amp=0.1, freq = 440, bus = 0|
	var out;
	out = SinOsc.ar(freq);
	Out.ar(bus, out*amp);
}).add;
)


10%6

~allscales

"123 321".split($ )[1].asInteger;

s.sendMsg(\s_new, \sinosc, 1010);

s.sendMsg(\n_set, 1010, \freq, 300);

s.sendMsg(\n_free, 1010);





