var api_socket;
var port = 8080;
var is_api_socket = false;
var is_last_api_socket = false;
var is_error = false;
var is_last_error = false;
var tab_id;

var last_websocket;
var last_websocket_id;
var is_last_websocket = false;

chrome.browserAction.onClicked.addListener(function(tab) {
	// chrome.debugger.attach({tabId:tab.id}, version,
	// 	onAttach.bind(null, tab.id));
	
	/* получаем список целей */
	/*
	chrome.debugger.getTargets(function(result){
		for (index = 0; index < result.length; index++) {
			console.log(index + ": "+result[index].url);
			var str = result[index].url;
			if(str.indexOf('888casino-com.evo-games.com/public/roulette/player/game') + 1) {
				if(is_api_socket) {
					api_socket.send('index: ' + index + ' result: ' + result[index].url);
				}
				chrome.debugger.attach({targetId:result[index].id}, version,
						onAttach.bind(null, result[index].id));
			}
		}
	});
	*/
	// chrome.windows.create(
	//	{url: "headers.html?" + tab.id, type: "popup", width: 256, height: 256});
});

var version = "1.1"; // 1.0

// function onAttach(tabId) {
function onAttach(targetId) {
	if (chrome.runtime.lastError) {
		alert(chrome.runtime.lastError.message);
		return;
	}
	tab_id = targetId;
	chrome.debugger.sendCommand({targetId:targetId}, "Network.enable");
	chrome.debugger.onEvent.addListener(onEvent);
}

function onEvent(debuggeeId, message, params) {
	// if (tab_id != debuggeeId.tabId) return;
	if (message == "Network.webSocketCreated") {
       // do something with params.response.payloadData,
       //   it contains the data SENT
		if(is_api_socket) {
		   // api_socket.send('{"ws_url":"' + params.url + '","payloadData":"' + params.response.payloadData + '","type":"send"}');
		   // api_socket.send('{"payloadData":"' + params.response.payloadData + '","type":"send"}');
		   // api_socket.send("Network.webSocketCreated");
		}
    } else
	if (message == "Network.webSocketFrameSent") {
       // do something with params.response.payloadData,
       //   it contains the data SENT
		if(is_api_socket) {
		   // api_socket.send('{"ws":"' + params.response.url + '","payloadData":"' + params.response.payloadData + '","type":"send"}');
		   // api_socket.send('{"payloadData":"' + params.response.payloadData + '","type":"send"}');
		   // api_socket.send("Network.webSocketFrameSent");
		}
    } else if (message == "Network.webSocketFrameReceived") {
       // do something with params.response.payloadData,
       //   it contains the data RECEIVED
		if(is_api_socket) {
			// api_socket.send('{"ws":"' + params.response.url + '","payloadData":"' + params.response.payloadData + '","type":"received"}');
			api_socket.send('{"payloadData":' + params.response.payloadData + ',"type":"received"}');
		}
    } else
	if (message == "Network.requestWillBeSent") {
		if(0)
		if(is_api_socket) {
			// api_socket.send('{"url":"' + params.request.url + '","redirectResponse":"' + params.redirectResponse + '","type":"request"}');
			api_socket.send(
				'{"url":"' + params.request.url + 
				'","headers":"' + JSON.stringify(params.request.headers) + 
				'","requestHeaders":"' + JSON.stringify(params.request.requestHeaders) + 
				'","status":"' + params.request.status + 
				'","response":"' + JSON.stringify(params.request) + 
				'","type":"request"}');
		}
		//console.info("CASINO888 params.request.url: " + params.request.url);
		//console.log("CASINO888 params.request.method: " + params.request.method);
		//if (params.redirectResponse) console.log("CASINO888 params.redirectResponse: " + params.redirectResponse);
		//console.log("CASINO888 params.requestId: " + params.requestId);
	} else 
	if (message == "Network.responseReceived") {
		if(0)
		if(is_api_socket) {
			api_socket.send(
				'{"url":"' + params.response.url + 
				'","headers":"' + JSON.stringify(params.response.headers) + 
				'","requestHeaders":"' + JSON.stringify(params.response.requestHeaders) + 
				'","status":"' + params.response.status + 
				'","response":"' + JSON.stringify(params.response) + 
				'","type":"response"}');
		}
		//console.log("CASINO888 params.requestId: " + params.requestId);
		//console.info("CASINO888 params.response: " + params.response);
		//console.log("CASINO888 response.status: " + params.response.status);
		//console.log("CASINO888 response.statusText: " + params.response.statusText);
		//console.log("CASINO888 response.headers: " + params.response.headers);
		//console.log("CASINO888 response.requestHeaders: " + params.response.requestHeaders);
	}
}

function injected_main() {
	console.log("888 Casino Bridge launched");
	
	connect_api();
	
	function connect_api() {
        try {
			api_socket = new WebSocket("ws://localhost:" + port + "/888casino-api"), 
			api_socket.onopen = function() {
				is_api_socket = true;
				console.log("Соединение с сервером API установлено.");
				api_socket.send('{"event":"start"}');
				if(is_last_websocket) {
					api_socket.send('{"event":"connect"}');
				} else {
					api_socket.send('{"event":"disconnect"}');
				}
			}, api_socket.onclose = function(t) {
				is_api_socket = false;
				/* пробуем переподключиться*/
				connect_api(); 
				t.wasClean ? console.log("Соединение с сервером API закрыто чисто") : console.log("Обрыв соединения с сервером API"), 
				console.log("Код: " + t.code + " причина: " + t.reason);
			}, api_socket.onmessage = function(t) {
				console.log("Получены данные от сервера API: " + t.data); 
				var api_message = JSON.parse(t.data);
				
			}, api_socket.onerror = function(t) {
				console.log("Ошибка (сервер API) " + t.message);
				is_api_socket = false;
			}
		} catch(e) {
			
		}
    }
}

function update_second() { 
	console.log("СЕКУНДОЧШКУ А!");
	
	/* получаем список целей */
	chrome.debugger.getTargets(function(result){
		var is_found = false;
		var found_websocket;
		var found_websocket_id;
		for (index = 0; index < result.length; index++) {
			console.log(index + ": "+result[index].url);
			var str = result[index].url;
			// if(str.indexOf('888casino') + 1) {
			//	if(is_api_socket) {
			//		api_socket.send('{"index": ' + index + ',"result":"' + result[index].url + '"}');
			//	}
			// } 
			
			// 888casino-game.com/live-casino/new/#page/game
			// 888casino-com.evo-games.com/public/roulette/player/game
			// 888casino-com.evo-games.com/frontend/evo/
			if((str.indexOf('888casino') + 1) && (str.indexOf('frontend') + 1)) {
				is_found = true;
				found_websocket = str;
				found_websocket_id = result[index].id;
			} 
		}
		if(is_found) {
			if(found_websocket != last_websocket) {
				if(is_last_websocket) {
					chrome.debugger.detach({targetId: last_websocket_id});
					if(is_api_socket) {
						api_socket.send('{"event":"disconnect"}');
					}
				}
				if(is_api_socket) {
					api_socket.send('{"event":"wait_connect","url":"' + found_websocket + '"}');
				}
				chrome.debugger.attach({targetId:found_websocket_id}, version,
						onAttach.bind(null, found_websocket_id));
				last_websocket = found_websocket;
				last_websocket_id = found_websocket_id;
				is_last_websocket = true
				if(is_api_socket) {
					api_socket.send('{"event":"connect"}');
				}
			} 
		} else 
		if(is_last_websocket) {
			chrome.debugger.detach({targetId: last_websocket_id});
			is_last_websocket = false;
			last_websocket = "";
			if(is_api_socket) {
				api_socket.send('{"event":"disconnect"}');
			}
		}
	});
}

setInterval(update_second, 10000);

function try_again() {
	console.log("try_again!");
	injected_main();
}

try_again();