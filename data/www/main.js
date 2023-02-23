const $ = document.querySelector.bind(document);

let socket;
var savelist=[]

function connect() {
	socket = new WebSocket("ws://" + location.host + "/ws");

	socket.addEventListener("open", (event) => {
		showMessage("websocket connected");
	});

	socket.addEventListener("message", (event) => {
		const msg = JSON.parse(event.data);
		if (msg.logMsg) {
			showMessage(msg.logMsg, false);
		}
		if (msg.errMsg) {
			showMessage(msg.errMsg, true);
		}
		if (msg.gatewidth) {
			loadWaveform(msg);
		}
	});

	socket.addEventListener("close", (event) => {
		showMessage(`websocket closed ${event.code}`);
		setTimeout(connect, 5000);
	});
}

window.addEventListener("load", function () {

	fillvals();
	connect();

	fetch("/getwaveform?temp=30")

	$('.patternbtn').addEventListener("click", (event) => {
		let currentElement = event.target;
		for (const element of document.querySelectorAll('.patternbtn button')) {
			element.style.backgroundColor = ''
		}
		if (currentElement.value) {
			currentElement.style.backgroundColor = '#909090'
			fetch("/pattern?id=" + currentElement.value)
		}
	})

	document.addEventListener("change", (event) => {
		setcolor()
	})

	$('#save').addEventListener("click", (event) => {
		let jsonObj = saveWaveform()
		jsonObj.name = ($('#savename').value ? $('#savename').value : 'untitled')
		let id = savelist.push(jsonObj)
		let opt = document.createElement("option");
		opt.value = id-1;
		opt.text = jsonObj.name;
		$('#savedlist').add(opt, null);		
	})

	$('#savedlist').addEventListener("change", (event) => {
		let jsonObj = savelist[event.target.value]
		loadWaveform(jsonObj)
	})

	$('#loadwaveform').addEventListener("click", (event) => {
		fetch("/getwaveform?temp=" + $("#templist").value)
	})

	$('#exportbtn').addEventListener("click", (event) => {
		exportWaveform()
		$('.export').style.display='block'
	})

	$('.groups').addEventListener("click", (event) => {
		if (event.target.nodeName === 'BUTTON') {
			let currentElement = event.target;
			if (currentElement.value) {
				let jsonObj = saveWaveform()
				jsonObj.solo = currentElement.value
				const options = {
					method: 'POST',
					headers: {
						'Content-Type': 'application/json'
					},
					body: JSON.stringify(jsonObj)
				};
				fetch("/putwaveform", options)
			}
			if (currentElement.hasAttribute('data-disabled')) {
				currentElement.dataset.disabled = (currentElement.dataset.disabled === '1' ? '0' : '1')
				if (currentElement.dataset.disabled === '1') currentElement.innerText = 'enable group'; else currentElement.innerText = 'disable group'
				setcolor()
			}
		}
	})

	$('.runbtn').addEventListener("click", (event) => {
		let currentElement = event.target;
		if (currentElement.value) {
			let jsonObj = saveWaveform()
			jsonObj.cmd = currentElement.value
			const options = {
				method: 'POST',
				headers: {
					'Content-Type': 'application/json'
				},
				body: JSON.stringify(jsonObj)
			};
			fetch("/putwaveform", options)
		}
	})

	for (let i = -20; i < 100; i += (i >= 0 && i < 30) ? 2 : 10) {
		let opt = document.createElement("option");
		opt.value = i;
		opt.text = i + '\u00B0C';
		$('#templist').add(opt, null);
	}
	$('#templist').value = 30

});

function showMessage(message, iserr) {
	const messages = $('#messages');
	var date = new Date(),
		time = date.toLocaleTimeString('nl-NL', { hour12: false, hour: '2-digit', minute: '2-digit', second: '2-digit' });
	if (iserr) {
		messages.insertAdjacentHTML("afterbegin", '<li class="new error">' + htmlEncode(time + ' ' + message) + '</li>');
	} else {
		messages.insertAdjacentHTML("afterbegin", '<li class="new">' + htmlEncode(time + ' ' + message) + '</li>');
	}
}

function htmlEncode(input) {
	const textArea = document.createElement("textarea");
	textArea.innerText = input;
	return textArea.innerHTML.split("<br>").join("\n");
}

const frequencyData = [[15, 121, 14], [20, 16, 14], [25, 38, 13], [30, 78, 12], [35, 24, 12], [40, 67, 11], [45, 26, 11], [50, 44, 10], [55, 13, 10], [60, 33, 9], [65, 7, 9], [70, 40, 8], [75, 17, 8], [80, 47, 7], [85, 26, 7], [90, 8, 7], [95, 50, 6], [100, 33, 6], [105, 17, 6], [110, 3, 6], [115, 34, 5], [120, 20, 5], [125, 7, 5], [135, 36, 4], [140, 24, 4], [145, 13, 4], [150, 3, 4], [155, 39, 3], [160, 28, 3], [165, 18, 3], [170, 9, 3], [175, 0, 3], [180, 47, 2], [185, 37, 2], [190, 28, 2], [195, 20, 2], [200, 12, 2]];

function fillvals() {

	var opt;
	var voltage = 2.4;
	var VSH1 = $('#vsh1options')
	var VSH2 = $('#vsh2options')

	for (let i = 0x8E; i < 0xCF; i++) {
		opt = document.createElement("option");
		opt.value = i;
		opt.text = voltage.toFixed(1);
		VSH1.add(opt, null);
		VSH2.add(opt.cloneNode(true), null);
		voltage += 0.1;
	}
	voltage += 0.1;
	for (let i = 0x23; i < 0x4C; i++) {
		opt = document.createElement("option");
		opt.value = i;
		opt.text = voltage.toFixed(1);
		VSH1.add(opt, null);
		VSH2.add(opt.cloneNode(true), null);
		voltage += 0.2;
	}

	var voltage = -9;
	var VSL = $('#vsloptions')

	for (let i = 0x1A; i < 0x3C; i += 2) {
		opt = document.createElement("option");
		opt.value = i;
		opt.text = voltage.toFixed(1);
		VSL.add(opt, null);
		voltage -= 0.5;
	}

	var voltage = 10;
	var VDG = $('#vghoptions')

	for (let i = 0x03; i < 0x18; i++) {
		opt = document.createElement("option");
		opt.value = i;
		opt.text = voltage.toFixed(1);
		VDG.add(opt, null);
		voltage += 0.5;
	}

	var freq = $('#freqoptions')
	for (const [frequency, param1, param2] of frequencyData) {
		const option = document.createElement("option");
		option.text = frequency;
		option.value = frequency;
		freq.appendChild(option);
	}

}

function saveWaveform() {
	let dummyline, gatewidth;
	for (let i = 0; i < frequencyData.length; i++) {
		if (frequencyData[i][0] == parseInt($('#freqoptions').value)) {
			dummyline = frequencyData[i][1];
			gatewidth = frequencyData[i][2];
			break;
		}
	}
	let jsonData = {
		vgh: $('#vghoptions').value,
		vsl: $('#vsloptions').value,
		vsh1: $('#vsh1options').value,
		vsh2: $('#vsh2options').value,
		dummyline: dummyline,
		gatewidth: gatewidth,
		lut: [],
		group: []
	};
	for (let wavegroup = 0; wavegroup < 7; wavegroup++) {
		let groupdisabled = ($('#disablebtn' + wavegroup).dataset.disabled === '1')
		jsonData.group[wavegroup] = {}
		jsonData.group[wavegroup].phaselength = []
		jsonData.group[wavegroup].repeat = parseInt($("#repeatval" + wavegroup).value)
		jsonData.group[wavegroup].disabled = groupdisabled
		for (let phase = 0; phase < 4; phase++) {
			jsonData.group[wavegroup].phaselength[phase] = (groupdisabled ? 0 : parseInt($("#group" + wavegroup + "length" + phase).value))
		}
		for (let lut = 0; lut < 5; lut++) {
			jsonData.lut[lut] = []
			for (let wavegroup = 0; wavegroup < 7; wavegroup++) {
				jsonData.lut[lut][wavegroup] = {}
				jsonData.lut[lut][wavegroup].A = parseInt($("#group" + wavegroup + "lut" + lut + "phase0").value)
				jsonData.lut[lut][wavegroup].B = parseInt($("#group" + wavegroup + "lut" + lut + "phase1").value)
				jsonData.lut[lut][wavegroup].C = parseInt($("#group" + wavegroup + "lut" + lut + "phase2").value)
				jsonData.lut[lut][wavegroup].D = parseInt($("#group" + wavegroup + "lut" + lut + "phase3").value)
			}
		}
	}
	return jsonData
}

function loadWaveform(data) {
	$('#vghoptions').value = data.vgh
	$('#vsloptions').value = data.vsl
	$('#vsh1options').value = data.vsh1
	$('#vsh2options').value = data.vsh2
	$('#vgh').innerHTML = getOptionText($('#vghoptions'), data.vgh);
	$('#vsl').innerHTML = getOptionText($('#vsloptions'), data.vsl);
	$('#vsh1').innerHTML = getOptionText($('#vsh1options'), data.vsh1);
	$('#vsh2').innerHTML = getOptionText($('#vsh2options'), data.vsh2);
	let freq = getFrequency(data.dummyline, data.gatewidth);
	$('#freq').innerHTML = freq;
	$('#freqoptions').value = freq;

	const element = $('#dummy');
	let nextSibling = element.nextSibling;
	while (nextSibling) {
		nextSibling.remove();
		nextSibling = element.nextSibling;
	}

	for (let wavegroup = 0; wavegroup < 7; wavegroup++) {
		let groupdiv = element.cloneNode(true)
		groupdiv.id = 'group' + wavegroup;
		groupdiv.querySelector('.grouptitle').innerHTML = 'group ' + wavegroup;
		groupdiv.querySelector('.solo').setAttribute("value", wavegroup);
		groupdiv.querySelector('.disable').setAttribute("id", 'disablebtn' + wavegroup);
		if (data.group[wavegroup].disabled) groupdiv.querySelector('.disable').dataset.disabled = '1'
		let input = document.createElement("input");
		input.setAttribute("id", "repeatval" + wavegroup);
		input.setAttribute("type", "text");
		input.setAttribute("value", data.group[wavegroup].repeat);
		groupdiv.querySelector('.repeat').appendChild(input);
		let lutname = ["Black", "White", "Red1", "Red2", "VCOM"];
		for (let lut = 0; lut < 5; lut++) {
			let newRow = groupdiv.querySelector('.luttable').insertRow(-1);
			let newCell = newRow.insertCell(0);
			newCell.innerHTML = "LUT " + lut + " " + lutname[lut];
			newCell = newRow.insertCell(1);
			var select = createdropdown(data.lut[lut][wavegroup].A);
			select.id = "group" + wavegroup + "lut" + lut + "phase0"
			if (lut == 4) select.style.backgroundColor = "#e0e0e0";
			newCell.appendChild(select);
			newCell = newRow.insertCell(2);
			var select = createdropdown(data.lut[lut][wavegroup].B);
			select.id = "group" + wavegroup + "lut" + lut + "phase1"
			if (lut == 4) select.style.backgroundColor = "#e0e0e0";
			newCell.appendChild(select);
			newCell = newRow.insertCell(3);
			var select = createdropdown(data.lut[lut][wavegroup].C);
			select.id = "group" + wavegroup + "lut" + lut + "phase2"
			if (lut == 4) select.style.backgroundColor = "#e0e0e0";
			newCell.appendChild(select);
			newCell = newRow.insertCell(4);
			var select = createdropdown(data.lut[lut][wavegroup].D);
			select.id = "group" + wavegroup + "lut" + lut + "phase3"
			if (lut == 4) select.style.backgroundColor = "#e0e0e0";
			newCell.appendChild(select);
		}

		let newRow = groupdiv.querySelector('.luttable').insertRow(-1);

		let newCell = newRow.insertCell(0);
		newCell.innerHTML = 'length';

		for (let phase = 0; phase < 4; phase++) {
			newCell = newRow.insertCell(phase + 1);
			let input = document.createElement("input");
			input.setAttribute("type", "text");
			input.setAttribute("value", data.group[wavegroup].phaselength[phase]);
			input.setAttribute("id", "group" + wavegroup + "length" + phase);
			newCell.appendChild(input);
		}

		$('.groups').appendChild(groupdiv, null);
	}
	setcolor();
}

function setcolor() {
	let voltage = [0, $('#vsh1options').options[$('#vsh1options').selectedIndex].text, $('#vsloptions').options[$('#vsloptions').selectedIndex].text, $('#vsh2options').options[$('#vsh2options').selectedIndex].text]
	let lutcolor = ["#000000", "#808080", "#C04040", "#FF0000"]
	let totalupdatetime = 0;
	let sumc = [0, 0, 0, 0];

	for (let wavegroup = 0; wavegroup < 7; wavegroup++) {
		let groupdisabled = ($('#disablebtn' + wavegroup).dataset.disabled === '1')
		$('#group' + wavegroup).style.opacity = (groupdisabled ? 0.5 : 1);
		for (let lut = 0; lut < 5; lut++) {
			for (let phase = 0; phase < 4; phase++) {
				let value = parseInt($("#group" + wavegroup + "lut" + lut + "phase" + phase).value);
				let color = "";
				if (value == 0) color = "green";
				if (value == 1) color = "yellow";
				if (value == 2) color = "blue";
				if (value == 3) color = "red";
				$("#group" + wavegroup + "lut" + lut + "phase" + phase).parentElement.style.backgroundColor = color;
			}
		}
		let repeat = parseInt($("#repeatval" + wavegroup).value) + 1
		let totalframes = 0, charge = 0;
		for (let phase = 0; phase < 4; phase++) {
			if (!groupdisabled) totalframes += repeat * parseInt($("#group" + wavegroup + "length" + phase).value)
		}
		$("#group" + wavegroup).querySelector('.timeframe').innerHTML = totalframes
		let totaltime = totalframes / parseInt($('#freqoptions').options[$('#freqoptions').selectedIndex].text) 
		$("#group" + wavegroup).querySelector('.timesec').innerHTML = totaltime.toFixed(2)
		totalupdatetime += totaltime

		var canv = $("#group" + wavegroup).querySelector('.graph')
		if (groupdisabled) {
			canv.style.display = "none";
		} else {
			canv.style.display = "block";
			canv.width = totalframes + 20;
			canv.height = 3*60+2*35;
			const ctx = canv.getContext("2d");
			ctx.fillStyle = '#FFFFFF';
			ctx.fillRect(0, 0, canv.width, canv.height);

			let yoffset = 35;
			for (let lut = 0; lut < 4; lut++) {
				ctx.beginPath();
				ctx.lineWidth = 1;
				ctx.moveTo(0, yoffset);
				ctx.lineTo(canv.width, yoffset);
				ctx.strokeStyle = "#40d040";
				ctx.stroke();
				ctx.beginPath();
				ctx.lineWidth = 1;
				ctx.moveTo(0, yoffset);
				ctx.lineTo(10, yoffset);
				let xlast = 10;
				ctx.strokeStyle = lutcolor[lut];
				for (let repetition = 0; repetition < repeat; repetition++) {
					for (let phase = 0; phase < 4; phase++) {
						let vcomvalue = parseInt($("#group" + wavegroup + "lut4phase" + phase).value);
						let vcomvolt = parseInt(voltage[vcomvalue])
						let value = parseInt($("#group" + wavegroup + "lut" + lut + "phase" + phase).value);
						let volt = parseInt(voltage[value]) - vcomvolt
						ctx.lineTo(xlast, yoffset - volt)
						let length = parseInt($("#group" + wavegroup + "length" + phase).value)
						if (groupdisabled) length = 0
						xlast += length
						sumc[phase] = sumc[phase] + length * volt
						ctx.lineTo(xlast, yoffset - volt);
					}
				}
				ctx.lineTo(xlast, yoffset);
				ctx.lineTo(xlast + 10, yoffset);
				ctx.stroke();
				yoffset += 60
			}
		}
	}
	$("#totalupdatetime").innerHTML = totalupdatetime.toFixed(2)
	//$("#sumc").innerHTML = sumc[0].toFixed(2) + ", " + sumc[1].toFixed(2) + ", " + sumc[2].toFixed(2) + ", " + sumc[3].toFixed(2)
	exportWaveform()
}

function createdropdown(value) {
	var select = document.createElement("select");

	var option2 = document.createElement("option");
	option2.value = "0";
	option2.textContent = "VSS";
	select.appendChild(option2);

	var option1 = document.createElement("option");
	option1.value = "2";
	option1.textContent = "VSL";
	select.appendChild(option1);

	var option3 = document.createElement("option");
	option3.value = "1";
	option3.textContent = "VSH1";
	select.appendChild(option3);

	var option4 = document.createElement("option");
	option4.value = "3";
	option4.textContent = "VSH2";
	select.appendChild(option4);

	select.value = value
	return select
}

function getOptionText(select, value) {
	for (let option of select.options) {
		if (option.value == value) {
			return option.text;
		}
	}
	return '?';
}

function getFrequency(param1, param2) {
	let closestFrequency = 0;
	let closestDifference = 255;

	frequencyData.forEach(([frequency, p1, p2]) => {
		if (p2 !== param2) {
			return;
		}

		const difference = Math.abs(p1 - param1);
		if (difference < closestDifference) {
			closestFrequency = frequency;
			closestDifference = difference;
		}
	});

	return closestFrequency;
}

function exportWaveform() {
	let dummyline, gatewidth;
	for (let i = 0; i < frequencyData.length; i++) {
		if (frequencyData[i][0] == parseInt($('#freqoptions').value)) {
			dummyline = frequencyData[i][1];
			gatewidth = frequencyData[i][2];
			break;
		}
	}
	var exporttxt = "const unsigned char waveform[] PROGMEM = {\r\n\t"
	for (let lut = 0; lut < 5; lut++) {
		for (let wavegroup = 0; wavegroup < 7; wavegroup++) {
			let vgroupByte = 0
			for (let phase = 0; phase < 4; phase++) {
				vgroupByte = (vgroupByte << 2) + parseInt($("#group" + wavegroup + "lut" + lut + "phase" + phase).value);
			}
			exporttxt += "0x" + vgroupByte.toString(16).padStart(2, "0") +","
		}
		exporttxt += "\r\n\t"
	}
	for (let wavegroup = 0; wavegroup < 7; wavegroup++) {
		let groupdisabled = ($('#disablebtn' + wavegroup).dataset.disabled === '1')
		for (let phase = 0; phase < 4; phase++) {
			exporttxt += (groupdisabled ? "0x00" : hexbyte($("#group" + wavegroup + "length" + phase).value)) + ","
		}
		exporttxt += hexbyte($("#repeatval" + wavegroup).value) + ",\r\n\t"
	}
	exporttxt += hexbyte($('#vghoptions').value) + ","
	exporttxt += hexbyte($('#vsh1options').value) + ","
	exporttxt += hexbyte($('#vsh2options').value) + ","
	exporttxt += hexbyte($('#vsloptions').value) + ","
	exporttxt += hexbyte(dummyline) + ","
	exporttxt += hexbyte(gatewidth) + "\r\n"
	exporttxt += "};"
	$("#exportbytes").value = exporttxt
}

function hexbyte(x) {
	return "0x" + parseInt(x).toString(16).padStart(2, "0")
}
