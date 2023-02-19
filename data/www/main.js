const $ = document.querySelector.bind(document);

let socket;

function connect() {
	socket = new WebSocket("ws://" + location.host + "/ws");

	socket.addEventListener("open", (event) => {
		showMessage("websocket connected");
	});

	socket.addEventListener("message", (event) => {
		console.log(event.data);
		const msg = JSON.parse(event.data);
		if (msg.logMsg) {
			showMessage(msg.logMsg, false);
		}
		if (msg.errMsg) {
			showMessage(msg.errMsg, true);
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

	$('.buttonbar').addEventListener("click", (event) => {
		let currentElement = event.target;
		console.log(currentElement.value);
		if (currentElement.value) {
			fetch("/pattern?id=" + currentElement.value)
		}
	})

	document.addEventListener("change", (event) => {
		console.log('change');
		setcolor()
	})

	$('#loadwaveform').addEventListener("click", (event) => {
		fetch("/getwaveform")
			.then(response => response.json())
			.then(data => {
				loadWaveform(data);
			})
	})

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

	// VSH1/VSH2
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
		//groupdiv.querySelector('.repeatval').innerHTML = data.group[wavegroup].repeat;
		let input = document.createElement("input");
		input.setAttribute("id", "repeatval" + wavegroup);
		input.setAttribute("type", "text");
		input.setAttribute("value", data.group[wavegroup].repeat);
		groupdiv.querySelector('.repeat').appendChild(input);
		let lutname = ["Black","White","Red1","Red2","VCOM"];
		for (let lut = 0; lut < 5; lut++) {
			let newRow = groupdiv.querySelector('.luttable').insertRow(-1);
			let newCell = newRow.insertCell(0);
			newCell.innerHTML = "LUT " + lut + " " + lutname[lut];
			newCell = newRow.insertCell(1);
			var select = createdropdown(data.lut[lut][wavegroup].A);
			select.id = "group" + wavegroup + "lut" + lut + "phase0"
			newCell.appendChild(select);
			newCell = newRow.insertCell(2);
			var select = createdropdown(data.lut[lut][wavegroup].B);
			select.id = "group" + wavegroup + "lut" + lut + "phase1"
			newCell.appendChild(select);
			newCell = newRow.insertCell(3);
			var select = createdropdown(data.lut[lut][wavegroup].C);
			select.id = "group" + wavegroup + "lut" + lut + "phase2"
			newCell.appendChild(select);
			newCell = newRow.insertCell(4);
			var select = createdropdown(data.lut[lut][wavegroup].D);
			select.id = "group" + wavegroup + "lut" + lut + "phase3"
			newCell.appendChild(select);
		}

		let newRow = groupdiv.querySelector('.luttable').insertRow(-1);

		let newCell = newRow.insertCell(0);
		newCell.innerHTML = 'length';

		for (let phase = 0; phase < 4; phase++) {
			newCell = newRow.insertCell(phase+1);
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
	let voltage = [0, $('#vsh1options').options[$('#vsh1options').selectedIndex].text, $('#vsloptions').options[$('#vsloptions').selectedIndex].text, $('#vsh2options').options[$('#vsh2options').selectedIndex].text ]
	let lutcolor = ["#000000","#808080","#FF0000","#800000"]
	for (let wavegroup = 0; wavegroup < 7; wavegroup++) {
		for (let lut = 0; lut < 5; lut++) {
			for (let phase = 0; phase < 4; phase++) {
				let value = parseInt($("#group" + wavegroup + "lut" + lut + "phase" + phase).value);
				let color = "";
				if (value == 0) color="green";
				if (value == 1) color = "yellow";
				if (value == 2) color = "blue";
				if (value == 3) color = "red";
				$("#group" + wavegroup + "lut" + lut + "phase" + phase).parentElement.style.backgroundColor = color;
			}
		}
		let repeat = parseInt($("#repeatval" + wavegroup).value) + 1
		let totaltime = 0, charge = 0;
		for (let phase = 0; phase < 4; phase++) {
			totaltime += repeat * parseInt($("#group" + wavegroup + "length" + phase).value)
			//charge += (repeat * $("#group" + wavegroup + "length" + phase).value) * voltage[
		}
		$("#group" + wavegroup).querySelector('.timeframe').innerHTML = totaltime
		$("#group" + wavegroup).querySelector('.timesec').innerHTML = totaltime / $('#freqoptions').options[$('#freqoptions').selectedIndex].text / 2

		var canv = $("#group" + wavegroup).querySelector('.graph')
		canv.width = totaltime + 20;
		canv.height = 250;
		const ctx = canv.getContext("2d");
		const yMax = 40;
		const yMin = -40;
		ctx.fillStyle = '#FFFFFF';
		ctx.fillRect(0, 0, canv.width, canv.height);

		let yoffset = 50;
		for (let lut = 0; lut < 4; lut++) {
			ctx.beginPath();
			ctx.lineWidth = 1;
			ctx.moveTo(0, yoffset);
			ctx.lineTo(canv.width, yoffset);
			ctx.strokeStyle = "#c0c0c0";
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
					xlast += parseInt($("#group" + wavegroup + "length" + phase).value)
					ctx.lineTo(xlast, yoffset - volt);
				}
			}
			ctx.lineTo(xlast, yoffset);
			ctx.lineTo(xlast+10, yoffset);
			ctx.stroke();
			yoffset += 50
		}
	}
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
